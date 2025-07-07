#include "hkStringPtr.h"

hkStringPtr::hkStringPtr(const char* aData) : m_data(aData)
{
    TP_THIS_FUNCTION(TCtor, void, hkStringPtr, const char*)
    POINTER_SKYRIMSE(TCtor, s_ctor, 57231);
    TiltedPhoques::ThisCall(s_ctor, this, aData);
}

const char* hkStringPtr::data() const noexcept
{
    return reinterpret_cast<const char*>(reinterpret_cast<std::uintptr_t>(m_data) & ~kManaged);
}

const char* hkStringPtr::c_str() const noexcept
{
    return data();
}

bool hkStringPtr::empty() const noexcept
{
    return !m_data ||m_data[0] == '\0';
}

int32_t hkStringPtr::size() const
{
    return static_cast<int32_t>(std::strlen(data()));
}

int32_t hkStringPtr::length() const
{
    return size();
}
