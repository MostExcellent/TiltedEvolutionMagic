#include <TiltedOnlinePCH.h>

#include <Systems/AnimationSystem.h>

#include <Games/Animation/ActorMediator.h>
#include <Games/Animation/TESActionData.h>

#include <Games/References.h>

#include <AI/AIProcess.h>
#include <Forms/BGSAction.h>
#include <Misc/MiddleProcess.h>

#include <Messages/ClientReferencesMoveRequest.h>

#include <Components.h>
#include <World.h>

#include <Forms/TESObjectCELL.h>
#include <Forms/TESWorldSpace.h>

#include <Structs/ActionEvent.h>
#include <Structs/GameId.h>
#include <Structs/NetActionEvent.h>
#include <Systems/ModSystem.h>

extern thread_local const char* g_animErrorCode;

void AnimationSystem::Update(World& aWorld, Actor* apActor, RemoteAnimationComponent& aAnimationComponent,
                             const uint64_t aTick) noexcept
{
    auto& actions = aAnimationComponent.TimePoints;

    if (actions.empty() || !apActor->animationGraphHolder.IsReady())
        return;

    auto it = actions.begin();

    // Process multiple actions per update
    // Currently testing without any interpolation
    while (it != actions.end()) // && it->Tick <= aTick)
    {
        if (!apActor->animationGraphHolder.IsReady())
            break;

        const auto pAction = Cast<BGSAction>(TESForm::GetById(it->ActionId));
        const auto pTarget = Cast<TESObjectREFR>(TESForm::GetById(it->TargetId));
        {
            BSScopedLock lock{apActor->actorLock};
            // Load actor state flags (primarily movement state)
            apActor->actorState.flags1 = it->State1;
            apActor->actorState.flags2 = it->State2;

            // Action system functions without this in most if not all vanilla cases
            // Maybe there was originally a reason why?
            // Mod compatibility may depend on this now, though
            apActor->LoadAnimationVariables(it->Variables);
        }

        // TODO: copy after the end of batch processing,
        //       if multiple actions are processed?
        aAnimationComponent.LastRanAction = *it;

        // Corresponds to unkInput
        uint32_t unkInput = it->Type & 0x3;

        // Play the animation
        TESActionData actionData(unkInput, apActor, pAction, pTarget);
        actionData.eventName = BSFixedString(it->EventName.c_str());
        actionData.sequence = Cast<TESIdleForm>(TESForm::GetById(it->SequenceId));
        actionData.idleForm = Cast<TESIdleForm>(TESForm::GetById(it->IdleId));
        actionData.sequenceIndex = actionData.sequence ? it->SequenceIndex : 0;
        actionData.someFlag = ((it->Type & 0x4) != 0) ? 1 : 0;

        // Mark this as an STR-controlled action
        // This is needed to allow the game's recursive processing calls
        actionData.someFlag |= BGSActionData::kSTRControlled;

        // With more RE, we could maybe perform actions through an even higher level system?
        // Add to sequencer, do stuff, etc.
        const auto result = ActorMediator::Get()->PerformAction(&actionData);

        if (result)
            spdlog::info("Action {} processed with result: {}", it->EventName, result);
        else
            spdlog::warn("Action {} failed with result: {}", it->EventName, result);

        ++it;
    }

    // Remove all processed actions
    actions.erase(actions.begin(), it);
}

void AnimationSystem::Setup(World& aWorld, const entt::entity aEntity) noexcept
{
    aWorld.emplace_or_replace<RemoteAnimationComponent>(aEntity);
}

void AnimationSystem::Clean(World& aWorld, const entt::entity aEntity) noexcept
{
    if (aWorld.all_of<RemoteAnimationComponent>(aEntity))
        aWorld.remove<RemoteAnimationComponent>(aEntity);
}

void AnimationSystem::AddAction(RemoteAnimationComponent& aAnimationComponent, const std::string& acActionDiff) noexcept
{
    auto& lastProcessedAction = aAnimationComponent.LastProcessedAction;

    TiltedPhoques::ViewBuffer buffer((uint8_t*)acActionDiff.data(), acActionDiff.size());
    Buffer::Reader reader(&buffer);

    lastProcessedAction.ApplyDifferential(reader);

    aAnimationComponent.TimePoints.push_back(lastProcessedAction);
}

void AnimationSystem::Serialize(World& aWorld, ClientReferencesMoveRequest& aMovementSnapshot,
                                LocalComponent& localComponent, LocalAnimationComponent& animationComponent,
                                FormIdComponent& formIdComponent)
{
    const auto pForm = TESForm::GetById(formIdComponent.Id);
    const auto pActor = Cast<Actor>(pForm);
    if (!pActor)
        return;

    auto& update = aMovementSnapshot.Updates[localComponent.Id];
    auto& movement = update.UpdatedMovement;

    if (const auto pCell = pActor->parentCell)
        World::Get().GetModSystem().GetServerModId(pCell->formID, movement.CellId.ModId, movement.CellId.BaseId);

    if (const auto pWorldSpace = pActor->GetWorldSpace())
        World::Get().GetModSystem().GetServerModId(pWorldSpace->formID, movement.WorldSpaceId.ModId,
                                                   movement.WorldSpaceId.BaseId);

    movement.Position = pActor->position;

    movement.Rotation.x = pActor->rotation.x;
    movement.Rotation.y = pActor->rotation.z;

    pActor->SaveAnimationVariables(movement.Variables);

    if (pActor->currentProcess && pActor->currentProcess->middleProcess)
    {
        movement.Direction = pActor->currentProcess->middleProcess->direction;
    }

    for (auto& entry : animationComponent.Actions)
    {
        update.ActionEvents.emplace_back(ToNetActionEvent(entry, aWorld));
    }

    auto latestAction = animationComponent.GetLatestAction();

    if (latestAction)
        localComponent.CurrentAction = latestAction.MoveResult();

    animationComponent.Actions.clear();
}

bool AnimationSystem::Serialize(World& aWorld, const ActionEvent& aActionEvent, const ActionEvent& aLastProcessedAction,
                                std::string* apData)
{
    uint32_t actionBaseId = 0;
    uint32_t actionModId = 0;
    if (!aWorld.GetModSystem().GetServerModId(aActionEvent.ActionId, actionModId, actionBaseId))
        return false;

    uint32_t targetBaseId = 0;
    uint32_t targetModId = 0;
    if (!aWorld.GetModSystem().GetServerModId(aActionEvent.TargetId, targetModId, targetBaseId))
        return false;

    uint8_t scratch[1 << 14];
    TiltedPhoques::ViewBuffer buffer(scratch, std::size(scratch));
    Buffer::Writer writer(&buffer);
    aActionEvent.GenerateDifferential(aLastProcessedAction, writer);

    apData->assign(buffer.GetData(), buffer.GetData() + writer.Size());

    return true;
}

NetActionEvent AnimationSystem::ToNetActionEvent(const ActionEvent& aActionEvent, World& aWorld) noexcept
{
    NetActionEvent netEvent{};

    // Copy basic data
    netEvent.Tick = aActionEvent.Tick;
    netEvent.State1 = aActionEvent.State1;
    netEvent.State2 = aActionEvent.State2;
    netEvent.Type = aActionEvent.Type;
    netEvent.SequenceIndex = aActionEvent.SequenceIndex;
    netEvent.EventName = aActionEvent.EventName;
    netEvent.TargetEventName = aActionEvent.TargetEventName;
    netEvent.Variables = aActionEvent.Variables;

    // Convert Actor FormID to entity handle
    if (aActionEvent.ActorId != 0)
    {
        auto view = aWorld.view<FormIdComponent, LocalComponent>();
        for (entt::entity entity : view)
        {
            const auto& formIdComponent = view.get<FormIdComponent>(entity);
            if (formIdComponent.Id == aActionEvent.ActorId)
            {
                const auto& localComponent = view.get<LocalComponent>(entity);
                netEvent.ActorId = localComponent.Id;
                break;
            }
        }
    }

    // Convert Target FormID - check if it's an entity or world object
    if (aActionEvent.TargetId != 0)
    {
        bool isEntity = false;
        auto view = aWorld.view<FormIdComponent>();
        for (entt::entity entity : view)
        {
            const auto& formIdComponent = view.get<FormIdComponent>(entity);
            if (formIdComponent.Id == aActionEvent.TargetId)
            {
                // Target is an entity, get its handle
                if (auto* localComponent = aWorld.try_get<LocalComponent>(entity))
                {
                    netEvent.TargetId = localComponent->Id;
                    isEntity = true;
                    break;
                }
                else if (auto* remoteComponent = aWorld.try_get<RemoteComponent>(entity))
                {
                    netEvent.TargetId = remoteComponent->Id;
                    isEntity = true;
                    break;
                }
            }
        }

        if (!isEntity)
        {
            // Target is a world object, convert to GameId
            uint32_t baseId = 0;
            uint32_t modId = 0;
            if (aWorld.GetModSystem().GetServerModId(aActionEvent.TargetId, modId, baseId))
            {
                netEvent.TargetGameId = GameId(modId, baseId);
            }
        }
    }

    // Convert Form IDs to GameIds
    if (aActionEvent.ActionId != 0)
    {
        uint32_t baseId = 0;
        uint32_t modId = 0;
        if (aWorld.GetModSystem().GetServerModId(aActionEvent.ActionId, modId, baseId))
        {
            netEvent.ActionId = GameId(modId, baseId);
        }
    }

    if (aActionEvent.SequenceId != 0)
    {
        uint32_t baseId = 0;
        uint32_t modId = 0;
        if (aWorld.GetModSystem().GetServerModId(aActionEvent.SequenceId, modId, baseId))
        {
            netEvent.SequenceId = GameId(modId, baseId);
        }
    }

    if (aActionEvent.IdleId != 0)
    {
        uint32_t baseId = 0;
        uint32_t modId = 0;
        if (aWorld.GetModSystem().GetServerModId(aActionEvent.IdleId, modId, baseId))
        {
            netEvent.IdleId = GameId(modId, baseId);
        }
    }

    return netEvent;
}

ActionEvent AnimationSystem::ToActionEvent(const NetActionEvent& aNetActionEvent, World& aWorld) noexcept
{
    ActionEvent event{};

    // Copy basic data
    event.Tick = aNetActionEvent.Tick;
    event.State1 = aNetActionEvent.State1;
    event.State2 = aNetActionEvent.State2;
    event.Type = aNetActionEvent.Type;
    event.SequenceIndex = aNetActionEvent.SequenceIndex;
    event.EventName = aNetActionEvent.EventName;
    event.TargetEventName = aNetActionEvent.TargetEventName;
    event.Variables = aNetActionEvent.Variables;

    // Convert Actor entity handle to FormID
    if (aNetActionEvent.ActorId != 0)
    {
        auto view = aWorld.view<FormIdComponent>();
        for (entt::entity entity : view)
        {
            if (auto* localComponent = aWorld.try_get<LocalComponent>(entity))
            {
                if (localComponent->Id == aNetActionEvent.ActorId)
                {
                    const auto& formIdComponent = view.get<FormIdComponent>(entity);
                    event.ActorId = formIdComponent.Id;
                    break;
                }
            }
            else if (auto* remoteComponent = aWorld.try_get<RemoteComponent>(entity))
            {
                if (remoteComponent->Id == aNetActionEvent.ActorId)
                {
                    const auto& formIdComponent = view.get<FormIdComponent>(entity);
                    event.ActorId = formIdComponent.Id;
                    break;
                }
            }
        }
    }

    // Convert Target - check if entity handle or GameId
    if (aNetActionEvent.TargetId != 0)
    {
        // Target is an entity, find its FormID
        auto view = aWorld.view<FormIdComponent>();
        for (entt::entity entity : view)
        {
            if (auto* localComponent = aWorld.try_get<LocalComponent>(entity))
            {
                if (localComponent->Id == aNetActionEvent.TargetId)
                {
                    const auto& formIdComponent = view.get<FormIdComponent>(entity);
                    event.TargetId = formIdComponent.Id;
                    break;
                }
            }
            else if (auto* remoteComponent = aWorld.try_get<RemoteComponent>(entity))
            {
                if (remoteComponent->Id == aNetActionEvent.TargetId)
                {
                    const auto& formIdComponent = view.get<FormIdComponent>(entity);
                    event.TargetId = formIdComponent.Id;
                    break;
                }
            }
        }
    }
    else if (aNetActionEvent.TargetGameId)
    {
        // Target is a world object, convert GameId to FormID
        event.TargetId = aWorld.GetModSystem().GetGameId(aNetActionEvent.TargetGameId);
    }

    // Convert GameIds to Form IDs
    if (aNetActionEvent.ActionId)
    {
        event.ActionId = aWorld.GetModSystem().GetGameId(aNetActionEvent.ActionId);
    }

    if (aNetActionEvent.SequenceId)
    {
        event.SequenceId = aWorld.GetModSystem().GetGameId(aNetActionEvent.SequenceId);
    }

    if (aNetActionEvent.IdleId)
    {
        event.IdleId = aWorld.GetModSystem().GetGameId(aNetActionEvent.IdleId);
    }

    return event;
}
