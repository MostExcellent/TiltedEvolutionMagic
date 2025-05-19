
#include <Events/SentAnimEventEvent.h>
#include <Games/Animation/IAnimationGraphManagerHolder.h>
#include <ModCompat/ScriptExtenderPluginList.h>

// Directly sent AnimEvents bypass the normal Action system, so we have to replicate them separately
TP_THIS_FUNCTION(TSendAnimEvent, bool, IAnimationGraphManagerHolder, BSFixedString*)
static TSendAnimEvent* RealSendAnimationEvent;

bool TP_MAKE_THISCALL(HookSendAnimationEvent, IAnimationGraphManagerHolder, BSFixedString* apAnimEvent)
{
    String eventNameString = String(apAnimEvent->AsAscii());
    bool result = TiltedPhoques::ThisCall(RealSendAnimationEvent, apThis, apAnimEvent);
    if (result)
    {
        // TODO: Should we do RTTI cast?
        //      I don't think so, as a result of checks later, but idk
        TESObjectREFR* pRefr = nullptr;
        pRefr = reinterpret_cast<TESObjectREFR*>(
            reinterpret_cast<char*>(apThis) - offsetof(TESObjectREFR, animationGraphHolder)
            );
        if (pRefr && pRefr->formID != 0)
        {
            SentAnimEventEvent directAnimEvent(pRefr->formID, eventNameString);
            World::Get().GetDispatcher().trigger(directAnimEvent);
        }
    }
    return result;
}

bool DirectSendAnimEvent(IAnimationGraphManagerHolder* a_pHolder, BSFixedString* a_pEventName)
{
    return TiltedPhoques::ThisCall(RealSendAnimationEvent, a_pHolder, a_pEventName);
}

static TiltedPhoques::Initializer s_DirectAnimEventInitializer(
    []() {
        bool bSKSEPluginsPresent = !ScriptExtenderPluginList::Get()->GetPlugins().empty();

        POINTER_SKYRIMSE(void*, s_GraphManagerHolderVtbl, 205174);
        void** graphManagerHolderVTable = s_GraphManagerHolderVtbl.Get();
        RealSendAnimationEvent = static_cast<TSendAnimEvent*>(graphManagerHolderVTable[1]);

        // If no script extender plugins are installed, we don't need the hook
        if (bSKSEPluginsPresent)
        {
            TP_HOOK(&RealSendAnimationEvent, HookSendAnimationEvent);
        }
    }
    );
