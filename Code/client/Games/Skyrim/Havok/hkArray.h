#pragma once

template <typename T>
struct hkArrayBase
{
    T* m_data;
    int32_t m_size;
    int32_t m_capacityAndFlags;

    // TODO: Resize, etc?

    // Accessors
    T* data() noexcept { return m_data; }
    const T* data() const noexcept { return m_data; }
    
    int32_t size() const noexcept { return m_size; }
    bool empty() const noexcept { return m_size == 0; }
    
    int32_t capacity() const noexcept
    { 
        return m_capacityAndFlags & 0x3FFFFFFF;
    }

    // Element access
    T& operator[](int32_t index) noexcept { return m_data[index]; }
    const T& operator[](int32_t index) const noexcept { return m_data[index]; }
    
    T& get(int32_t index)
    { 
        if (index >= m_size || index < 0)
        {
            throw std::out_of_range("hkArray index out of range");
        }
        return m_data[index]; 
    }

    const T& get(int32_t index) const
    { 
        if (index >= m_size || index < 0)
        {
            throw std::out_of_range("hkArray index out of range");
        }
        return m_data[index]; 
    }
    
    T& front() noexcept { return m_data[0]; }
    const T& front() const noexcept { return m_data[0]; }
    
    T& back() noexcept { return m_data[m_size - 1]; }
    const T& back() const noexcept { return m_data[m_size - 1]; }

    // Iterator support for range-based for loops
    using iterator = T*;
    using const_iterator = const T*;
    
    iterator begin() noexcept { return m_data; }
    iterator end() noexcept { return m_data + m_size; }
    
    const_iterator begin() const noexcept { return m_data; }
    const_iterator end() const noexcept { return m_data + m_size; }
    
    const_iterator cbegin() const noexcept { return m_data; }
    const_iterator cend() const noexcept { return m_data + m_size; }
};
static_assert(sizeof(hkArrayBase<void*>) == 0x10);

template <typename T, typename Allocator = void>
    struct hkArray : hkArrayBase<T> {};
static_assert(sizeof(hkArray<void*>) == 0x10);

// TODO: not sure if needed
template <typename T, std::size_t N, typename Allocator = void>
struct hkInplaceArray : hkArray<T, Allocator>
{
    T storage[N];  // 10
};
