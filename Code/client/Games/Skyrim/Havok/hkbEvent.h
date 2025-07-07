#pragma once

#include "hkRefPtr.h"
#include "hkRefVariant.h"

struct hkbEventPayload;

struct hkbEventBase
{
    enum SystemEventIDs : uint32_t
    {
        kNull = static_cast<std::underlying_type_t<SystemEventIDs>>(-1)
    };
    
    SystemEventIDs            id;       // 00
    std::uint32_t             pad04;    // 04
    hkRefPtr<hkbEventPayload> payload;  // 08
};
static_assert(sizeof(hkbEventBase) == 0x10);

struct hkbEvent : hkbEventBase
{
    hkRefVariant sender;  // 10
};
static_assert(sizeof(hkbEvent) == 0x18);
