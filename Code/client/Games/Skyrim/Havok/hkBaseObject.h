#pragma once

struct hkBaseObject
{
    virtual ~hkBaseObject();
};
static_assert(sizeof(hkBaseObject) == 0x8);
