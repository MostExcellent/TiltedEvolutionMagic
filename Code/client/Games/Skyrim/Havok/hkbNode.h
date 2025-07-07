#pragma once

#include "hkEventContext.h"
#include "hkEventType.h"
#include "hkbBindable.h"
#include "hkbContext.h"
#include "hkStringPtr.h"

struct hkbNode : hkbBindable
{
    ~hkbNode() override;  // 00
 
    virtual void Activate(const hkbContext& a_context);                   // 04 - { return; }
    virtual void Update(const hkbContext& a_context, float a_timestep);   // 05 - { userData |= 1; }
    virtual void SendEvent(hkEventContext& aContext, hkEventType& aType); // 06 - { return; }
    virtual void Deactivate(const hkbContext& a_context);                 // 07 - { return; }
    virtual void Unk_08();                                                // 08 - { return 2; }
    virtual void Unk_09();                                                // 09 - { return; }
    virtual void Unk_0A();                                                // 0A - { return 1; }
    virtual void Unk_0B();                                                // 0B - { return; }
    virtual void Unk_0C();                                                // 0C
    virtual void Unk_0D();                                                // 0D - { return 0; }
    virtual void Unk_0E();                                                // 0E - { return; }
    virtual void Unk_0F();                                                // 0F - { return; }
    virtual void Unk_10();                                                // 10 - { return; }
    virtual void Unk_11();                                                // 11 - { return; }
    virtual void Unk_12();                                                // 12 - { return; }
    virtual void Unk_13();                                                // 13 - { return 0; }
    virtual void Unk_14();                                                // 14 - { return; }
    virtual void Unk_15();                                                // 15 - { return 0; }
    virtual void Unk_16();                                                // 16 - { return 0; }
    
    enum class GetChildrenFlagBits : uint8_t
    {
        kActiveOnly = 1 << 0,
        kGeneratorsOnly = 1 << 1,
        kIgnoreReferencedBehaviour = 1 << 2
    };
    
    enum class CloneState : uint8_t
    {
        kDefault = 0,
        kTemplate = 1,
        kClone = 2,
        kShareable = 3
    };
    
    std::uint64_t userData;    // 30
    hkStringPtr   name;        // 38
    std::uint16_t id;          // 40
    CloneState    cloneState;  // 42
    std::uint8_t  pad43;       // 43
    std::uint32_t pad44;       // 44
};
static_assert(sizeof(hkbNode) == 0x48);
