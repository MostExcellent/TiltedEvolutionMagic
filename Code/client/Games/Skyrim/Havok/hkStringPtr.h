#pragma once

struct hkStringPtr
{
    hkStringPtr(const char* aData);

    [[nodiscard]] const char* data() const noexcept;
    [[nodiscard]] const char* c_str() const noexcept;

    [[nodiscard]] bool empty() const noexcept;
    [[nodiscard]] int32_t size() const;
    [[nodiscard]] int32_t length() const;

protected:
    enum
    {
        kManaged = 1 << 0
    };
    
    const char* m_data;
};
static_assert(sizeof(hkStringPtr) == 0x8);
