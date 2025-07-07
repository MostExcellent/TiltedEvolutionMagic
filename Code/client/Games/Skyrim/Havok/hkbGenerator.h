#pragma once

#include "hkbNode.h"

struct hkbGenerator : hkbNode
{
    ~hkbGenerator() override;

    void Unk_15() override;  // 15

    virtual void Generate(const hkbContext& a_context) = 0;  // 17
    virtual void Unk_18();                                   // 18 - { return 0; }
    virtual void UpdateSync(const hkbContext& a_context);    // 19
    virtual void Unk_1A();                                   // 1A - { return; }
    virtual void Unk_1B();                                   // 1B - { return; }
};
static_assert(sizeof(hkbGenerator) == 0x48);
