#pragma once

#include "hkRefVariant.h"

struct hkaDefaultAnimationControlMapperData;
struct hkbGeneratorOutputListener;
struct hkbEventQueue;
struct hkbAttachmentManager;

struct hkbContext
{
    hkbCharacter*                         character;                 // 00
    hkbBehaviorGraph*                     behavior;                  // 08
    hkRefVariant                          nodeToIndexMap;            // 10 hkPointerMap<const hkbNode*, int>
    hkbEventQueue*                        eventQueue;                // 18
    hkaDefaultAnimationControlMapperData* sharedEventQueue;          // 20
    hkRefPtr<hkbGeneratorOutputListener>  generatorOutputListener;   // 28
    bool                                  eventTriggeredTransition;  // 30
    uint8_t                               pad31[7];                  // 31
    hkRefVariant                          world;                     // 38
    hkbAttachmentManager*                 attachmentManager;         // 40
    hkRefVariant                          animationCache;            // 48
};
static_assert(sizeof(hkbContext) == 0x50);
