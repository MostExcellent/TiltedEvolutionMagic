#pragma once

struct hkbSymbolIdMap : hkReferencedObject
{
    virtual ~hkbSymbolIdMap();

    uint8_t pad8[0x20 - 0x10];
    void* pointer20;
};

static_assert(offsetof(hkbSymbolIdMap, pointer20) == 0x20);
