#pragma once

#include "hkbGenerator.h"

#include <Havok/hkbVariableValueSet.h>

struct hkEventContext;
struct hkEventType;
struct hkbGenerator;
struct hkbSymbolIdMap;
struct hkbStateMachine;

// Not 100% sure which structure this is specifically
struct ActiveNodeInfo
{
    uint8_t pad0[0x58];
    hkbGenerator* generator;         // 58 - The active generator node
    hkbBehaviorGraph* behaviorGraph; // 60 - The behavior graph this node belongs to
    uint8_t pad68[0x84 - 0x68];
    uint8_t byte84;                     // 84 - Activity flag?
    uint8_t byte85;                     // 85 - Special processing flag - see AnimationExperiments.cpp
    uint8_t pad86[0x90 - 0x86];
};

static_assert(offsetof(ActiveNodeInfo, generator) == 0x58);
static_assert(offsetof(ActiveNodeInfo, behaviorGraph) == 0x60);
static_assert(offsetof(ActiveNodeInfo, byte84) == 0x84);

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

    // TODO: Check if hkArray or simple array
    struct ActiveNodeInfoArray
    {
        ActiveNodeInfo* nodeInfos;  // Array of active node information
        int count;              // Number of active nodes
    };

    VariableMode                   variableMode;                     // 48
    uint8_t                        pad49;                            // 049
    uint16_t                       pad4A;                            // 04A
    uint32_t                       pad4C;                            // 04C
    hkArray<hkRefVariant>          uniqueIDPool;                     // 050
    hkRefVariant                   idToStateMachineTemplateMap;      // 060
    hkArray<hkRefVariant>          mirroredExternalIDMap;            // 068
    hkRefVariant                   pseudoRandomGenerator;            // 078
    // TODO: Check if state machine or regular generator
    hkRefPtr<hkbStateMachine>      stateMachine;                     // 80
    hkRefPtr<hkbBehaviorGraphData> data;                             // 088
    hkRefVariant                   rootGeneratorClone;               // 090
    ActiveNodeInfoArray*               activeNodes;                      // 98 - Currently active nodes in the graph
    hkRefVariant                   activeNodeTemplateToIndexMap;     // 0A0
    hkRefVariant                   activeNodesChildrenIndices;       // 0A8
    hkRefVariant                   globalTransitionData;             // 0B0
    hkRefPtr<hkbSymbolIdMap>       symbolIdMap;                      // B8
    hkRefVariant                   attributeIDMap;                   // 0C0
    hkRefVariant                   variableIDMap;                    // 0C8
    hkRefVariant                   characterPropertyIDMap;           // 0D0
    hkbVariableValueSet*           animationVariables;               // D8
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
static_assert(offsetof(hkbBehaviorGraph, activeNodes) == 0x98);
static_assert(offsetof(hkbBehaviorGraph, symbolIdMap) == 0xB8);
static_assert(offsetof(hkbBehaviorGraph, animationVariables) == 0xD8);
static_assert(offsetof(hkbBehaviorGraph, isActive) == 0x12C);
