#pragma once

#include "hkReferencedObject.h"

union hkbVariableValue
{
    char b;
    int32_t i;
    float f;
    uint32_t raw;
};

struct hkbVariableValueSet : hkReferencedObject
{
    ~hkbVariableValueSet() override;

    hkbVariableValue* data;           // 10
    uint32_t size;                    // 18
};

static_assert(offsetof(hkbVariableValueSet, data) == 0x10);
static_assert(offsetof(hkbVariableValueSet, size) == 0x18);
