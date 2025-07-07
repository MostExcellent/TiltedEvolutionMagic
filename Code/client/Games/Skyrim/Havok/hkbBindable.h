#pragma once

#include "hkArray.h"
#include "hkRefPtr.h"
#include "hkReferencedObject.h"

struct hkRefVariant;
struct hkbVariableBindingSet;

struct hkbBindable : hkReferencedObject
{
    ~hkbBindable() override; // 00

    virtual void Unk_03(); // 03

    hkRefPtr<hkbVariableBindingSet> variableBindingSet;  // 10
    hkArray<hkRefVariant>           cachedBindables;     // 18
    bool                            areBindablesCached;  // 28
    uint8_t                         pad29;               // 29
    uint16_t                        pad2A;               // 2A
    uint32_t                        pad2C;               // 2C
};
static_assert(sizeof(hkbBindable) == 0x30);
