#pragma once

#include "hkbEvent.h"
#include "hkbGenerator.h"

struct hkbStateMachine : hkbGenerator
{
    enum class StartStateMode : uint8_t
    {
        kDefault = 0,
        kSync = 1,
        kRandom = 2,
        kChooser = 3
    };

    enum class StateMachineSelfTransitionMode : uint8_t
    {
        kNoTransition = 0,
        kTransitionToStartState = 1,
        kForceTransitionToStartState = 2
    };

    struct StateInfo : hkbBindable
    {
        ~StateInfo() override;

        // members
        uint64_t unk30;  // 30
        uint64_t unk38;  // 38
        uint64_t unk40;  // 40
        uint64_t unk48;  // 48
        uint64_t unk50;  // 50
        uint64_t unk58;  // 58
        uint64_t unk60;  // 60
        uint64_t unk68;  // 68
        uint64_t unk70;  // 70
    };
    static_assert(sizeof(StateInfo) == 0x78);

    struct TransitionInfoArray : hkReferencedObject
    {
        ~TransitionInfoArray() override;

        uint64_t unk10;  // 10
        uint64_t unk18;  // 18
    };
    static_assert(sizeof(TransitionInfoArray) == 0x20);
    
    virtual ~hkbStateMachine();

    // override (hkbGenerator)
    hkClass* GetClassType() const override;                                                                 // 01
    void CalcContentStatistics(hkStatisticsCollector* a_collector, const hkClass* a_class) const override;  // 02
    void Unk_03() override;                                                                                 // 03
    void Activate(const hkbContext& a_context) override;                                                    // 04
    void Update(const hkbContext& a_context, float a_timestep) override;                                    // 05
    void SendEvent(hkEventContext& aContext, hkEventType& aType) override;                                  // 06
    void Deactivate(const hkbContext& a_context) override;                                                  // 07
    void Unk_08() override;                                                                                 // 08
    void Unk_09() override;                                                                                 // 09
    void Unk_0A() override;                                                                                 // 0A
    void Unk_0C() override;                                                                                 // 0C
    void Unk_0D() override;                                                                                 // 0D
    void Unk_0E() override;                                                                                 // 0E
    void Unk_0F() override;                                                                                 // 0F
    void Unk_10() override;                                                                                 // 10
    void Unk_11() override;                                                                                 // 11
    void Unk_12() override;                                                                                 // 12
    void Unk_14() override;                                                                                 // 14
    void Generate(const hkbContext& a_context) override;                                                    // 17
    void Unk_18() override;                                                                                 // 18 - { return 1; }
    void UpdateSync(const hkbContext& a_context) override;                                                  // 19
    void Unk_1B() override;                                                                                 // 1B - { echoNextUpdate = true; }

    hkbEvent                       eventToSendWhenStateOrTransitionChanges;  // 048
    hkRefPtr<hkbStateChooser>      startStateChooser;                        // 060
    int32_t                        startStateID;                             // 068
    int32_t                        returnToPreviousStateEventID;             // 06C
    int32_t                        randomTransitionEventID;                  // 070
    int32_t                        transitionToNextHigherStateEventID;       // 074
    int32_t                        transitionToNextLowerStateEventID;        // 078
    int32_t                        syncVariableIndex;                        // 07C
    int32_t                        currentStateID;                           // 080
    bool                           wrapAroundStateID;                        // 084
    int8_t                         maxSimultaneousTransitions;               // 085
    StartStateMode                 startStateMode;                           // 086
    StateMachineSelfTransitionMode selfTransitionMode;                       // 087
    bool                           isActive;                                 // 088
    uint8_t                        pad41;                                    // 089
    uint16_t                       pad42;                                    // 08A
    uint32_t                       pad44;                                    // 08C
    hkArray<StateInfo*>            states;                                   // 090
    hkRefPtr<TransitionInfoArray>  wildcardTransitions;                      // 0A0
    hkRefVariant                   stateIDToIndexMap;                        // 0A8
    hkArray<hkRefVariant>          activeTransitions;                        // 0B0
    hkArray<hkRefVariant>          transitionFlags;                          // 0C0
    hkArray<hkRefVariant>          wildcardTransitionFlags;                  // 0D0
    hkArray<hkRefVariant>          delayedTransitions;                       // 0E0
    float                          timeInState;                              // 0F0
    float                          lastLocalTime;                            // 0F4
    int32_t                        previousStateID;                          // 0F8
    int32_t                        nextStartStateIndexOverride;              // 0FC
    bool                           stateOrTransitionChanged;                 // 100
    bool                           echoNextUpdate;                           // 101
    uint16_t                       currentStateIndexAndEntered;              // 102
    uint32_t                       pad0BC;
};

static_assert(offsetof(hkbStateMachine, name) == 0x38);
