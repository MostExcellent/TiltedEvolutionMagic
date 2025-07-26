#pragma once

#include <Components.h>

struct World;
struct Actor;
struct ClientReferencesMoveRequest;
struct ActionEvent;
struct NetActionEvent;
struct ModSystem;

/**
 * @brief Applies animations coming from remote actors.
 */
struct AnimationSystem
{
    /**
     * @brief Ran periodically to check for new animations to apply.
     * @param aWorld The registry where the actor in question lives.
     * @param apActor The actor to-be-updated.
     * @param aAnimationComponent The animation component attached to the actor.
     * @param aTick The current tick.
     */
    static void Update(World& aWorld, Actor* apActor, RemoteAnimationComponent& aAnimationComponent,
                       uint64_t aTick) noexcept;
    /**
     * @brief Sets up the animation system for a particular actor.
     * @param aWorld The registry where the actor in question lives.
     * @param aEntity The entity attached to the actor.
     */
    static void Setup(World& aWorld, entt::entity aEntity) noexcept;
    /**
     * @brief Unregisters an actor from receiving remote animations.
     *
     * This function is not being used.
     *
     * @param aWorld The registry where the actor in question lives.
     * @param aEntity The entity attached to the actor.
     */
    static void Clean(World& aWorld, entt::entity aEntity) noexcept;
    /**
     * @brief Adds an action (animation) to be processed.
     * @param aAnimationComponent The animation component attached to the actor in question.
     * @param acActionDiff The differential data of the animation.
     */
    static void AddAction(RemoteAnimationComponent& aAnimationComponent, const std::string& acActionDiff) noexcept;
    /**
     * @brief Serializes the actions to-be-sent.
     * @param aWorld The registry where the actor in question lives.
     * @param aMovementSnapshot The output of the animation data.
     * @param localComponent The local component of the actor whose data is to be serialized.
     * @param animationComponent The local animation component of the actor, used to give the output the server id of the actor.
     * @param formIdComponent The form id component of the actor, used to fetch the actor pointer.
     */
    static void Serialize(World& aWorld, ClientReferencesMoveRequest& aMovementSnapshot, LocalComponent& localComponent,
                          LocalAnimationComponent& animationComponent, FormIdComponent& formIdComponent);
    /**
     * @brief Serializes the actions to-be-sent.
     *
     * This function is not being used.
     */
    [[maybe_unused]] static bool Serialize(World& aWorld, const ActionEvent& aActionEvent,
                                           const ActionEvent& aLastProcessedAction, std::string* apData);

    /**
     * @brief Convert a local ActionEvent to a NetActionEvent for network transmission
     * @param aActionEvent The local ActionEvent with FormIDs
     * @param aWorld World instance for entity lookup
     * @return NetActionEvent ready for network transmission
     */
    static NetActionEvent ToNetActionEvent(const ActionEvent& aActionEvent, World& aWorld) noexcept;

    /**
     * @brief Convert a received NetActionEvent to a local ActionEvent
     * @param aNetActionEvent The network ActionEvent with entity handles and GameIds
     * @param aWorld World instance for entity to FormID conversion
     * @return ActionEvent ready for local use
     */
    static ActionEvent ToActionEvent(const NetActionEvent& aNetActionEvent, World& aWorld) noexcept;
};
