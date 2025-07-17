#pragma once

#include "hkbGenerator.h"

#include <Havok/hkbVariableValueSet.h>

struct hkEventContext;
struct hkEventType;
struct hkbGenerator;
struct hkbSymbolIdMap;
struct hkbStateMachine;

struct hkbNodeInfo
{
    void*    unk00;                     // 00
    int64_t  unk08;                     // 08
    int64_t  unk10;                     // 10
    void*    unk18;                     // 18
    uint8_t pad20[0x50 - 0x20];
    hkbNode* nodeTemplate;              // 50
    hkbNode* nodeClone;                 // 58
    // TODO: Check if this is a behavior graph
    hkbNode* behavior;                  // 60
    uint64_t unk68;                     // 68
    uint64_t unk70;                     // 70
    uint64_t unk78;                     // 78
    uint8_t unk80[0x84 - 0x80];         // 80 - 0x80 to 0x88 may be flags + padding?
    uint8_t byte84;                     // 84 - Some flag, see AnimationExperiments.cpp
    uint8_t byte85;                     // 85 - Ditto
    uint8_t pad86[0x88 - 0x86];         // 86 - Check if actually padding?
    bool unk88;
};

static_assert(offsetof(hkbNodeInfo, nodeClone) == 0x58);
static_assert(offsetof(hkbNodeInfo, behavior) == 0x60);
static_assert(offsetof(hkbNodeInfo, byte84) == 0x84);
static_assert(sizeof(hkbNodeInfo) == 0x90);

struct hkbBehaviorGraph : hkbGenerator
{
    enum class VariableMode : uint8_t
    {
        kDiscardWhenActive = 0,
        kMaintainValuesWhenInactive = 1,
    };

    ~hkbBehaviorGraph() override;

    // override (hkbGenerator)
    hkClass* GetClassType() const override;                                                                // 01
    void CalcContentStatistics(hkStatisticsCollector* a_collector, const hkClass* a_class) const override; // 02
    void Activate(const hkbContext& a_context) override;                                                   // 04
    void Update(const hkbContext& a_context, float a_timestep) override;                                   // 05
    void SendEvent(hkEventContext& aContext, hkEventType& aType) override;                                 // 06
    void Deactivate(const hkbContext& a_context) override;                                                 // 07
    void Unk_09() override;                                                                                // 09
    void Unk_0C() override;                                                                                // 0C
    void Unk_16() override;                                                                                // 16 - { return 1; }
    void Generate(const hkbContext& a_context) override;                                                   // 17
    void Unk_18() override;                                                                                // 18 - { return 1; }
    void UpdateSync(const hkbContext& a_context) override;                                                 // 19

    // Reverse engineering do not use
    void ReSendEvent(hkEventContext& aContext, hkEventType& aType);
    void ReHandleEvent(hkEventContext& aContext, hkEventType& aType);

    VariableMode                   variableMode;                     // 048
    uint8_t                        pad49;                            // 049
    uint16_t                       pad4A;                            // 04A
    uint32_t                       pad4C;                            // 04C
    hkArray<hkRefVariant>          uniqueIDPool;                     // 050
    hkRefVariant                   idToStateMachineTemplateMap;      // 060
    hkArray<hkRefVariant>          mirroredExternalIDMap;            // 068
    hkRefVariant                   pseudoRandomGenerator;            // 078
    // TODO: Could this ever not be a state machine?
    hkRefPtr<hkbStateMachine>      rootStateMachine;                 // 080
    hkRefPtr<hkbBehaviorGraphData> data;                             // 088
    hkRefVariant                   rootGeneratorClone;               // 090
    hkArray<hkbNodeInfo>*          activeNodeInfos;                  // 098
    hkRefVariant                   activeNodeTemplateToIndexMap;     // 0A0
    hkRefVariant                   activeNodesChildrenIndices;       // 0A8
    hkRefVariant                   globalTransitionData;             // 0B0
    hkRefPtr<hkbSymbolIdMap>       symbolIdMap;                      // B8
    hkRefVariant                   attributeIDMap;                   // 0C0
    hkRefVariant                   variableIDMap;                    // 0C8
    hkRefVariant                   characterPropertyIDMap;           // 0D0
    hkbVariableValueSet*           animationVariables;               // 0D8
    hkRefVariant                   nodeTemplateToCloneMap;           // 0E0
    hkRefVariant                   nodeCloneToTemplateMap;           // 0E8
    hkRefVariant                   stateListenerTemplateToCloneMap;  // 0F0
    hkRefVariant                   nodePartitionInfo;                // 0F8
    int32_t                        numIntermediateOutputs;           // 100
    uint32_t                       pad104;                           // 104
    hkArray<hkRefVariant>          jobs;                             // 108
    hkArray<hkRefVariant>          allPartitionMemory;               // 118
    int16_t                        numStaticNodes;                   // 128
    int16_t                        nextUniqueID;                     // 12A
    bool                           isActive;                         // 12C
    bool                           isLinked;                         // 12D
    bool                           updateActiveNodes;                // 12E
    bool                           stateOrTransitionChanged;         // 12F
};
static_assert(sizeof(hkbBehaviorGraph) == 0x130);
static_assert(offsetof(hkbBehaviorGraph, activeNodeInfos) == 0x98);
static_assert(offsetof(hkbBehaviorGraph, symbolIdMap) == 0xB8);
static_assert(offsetof(hkbBehaviorGraph, animationVariables) == 0xD8);
static_assert(offsetof(hkbBehaviorGraph, isActive) == 0x12C);
