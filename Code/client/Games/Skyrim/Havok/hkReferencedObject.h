#pragma once

#include "hkBaseObject.h"

class hkClass;

struct hkReferencedObject : hkBaseObject
{
    enum class LockMode
    {
        kNone = 0,
        kAuto,
        kManual
    };

    enum
    {
        kMemSize = 0x7FFF
    };
    
    virtual const hkClass* GetClassType() const; // 01 - { return 0; }
    virtual void CalcContentStatistics(hkStatisticsCollector* a_collector, const hkClass* a_class) const;  // 02

    void AddReference() const;
    int32_t GetAllocatedSize() const;
    int32_t GetReferenceCount() const;
    void RemoveReference() const;
    
    uint16_t memSizeAndFlags;          // 08
    volatile mutable int16_t refCount; // 0A
    uint32_t pad0C;                    // 0C
};
static_assert(sizeof(hkReferencedObject) == 0x10);
