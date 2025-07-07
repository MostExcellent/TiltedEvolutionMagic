#include "hkReferencedObject.h"

void hkReferencedObject::AddReference() const
{
    TP_THIS_FUNCTION(TAddReference, void, hkReferencedObject)
    POINTER_SKYRIMSE(TAddReference, s_addReference, 57010);
    TiltedPhoques::ThisCall(s_addReference, const_cast<hkReferencedObject*>(this));
}

int32_t hkReferencedObject::GetAllocatedSize() const
{
    return memSizeAndFlags & kMemSize;
}

int32_t hkReferencedObject::GetReferenceCount() const
{
    return refCount;
}

void hkReferencedObject::RemoveReference() const
{
    TP_THIS_FUNCTION(TRemoveReference, void, hkReferencedObject)
    POINTER_SKYRIMSE(TRemoveReference, s_removeReference, 57011);
    TiltedPhoques::ThisCall(s_removeReference, const_cast<hkReferencedObject*>(this));
}
