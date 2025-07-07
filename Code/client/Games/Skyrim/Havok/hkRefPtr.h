#pragma once

#include <cassert> 
#include <memory>
#include <type_traits>
#include <utility>

template <typename T>
struct hkRefPtr
{
	// 1
    constexpr hkRefPtr() noexcept :
		_ptr(nullptr)
	{}

	// 2
    constexpr hkRefPtr(std::nullptr_t) noexcept :
		_ptr(nullptr)
	{}

	// 3
	template <typename Y>
    explicit hkRefPtr(Y* a_rhs) requires (std::is_convertible_v<Y*, T*>) :
		_ptr(a_rhs)
	{
		TryAttach();
	}

	// 9a
    hkRefPtr(const hkRefPtr& a_rhs) :
		_ptr(a_rhs._ptr)
	{
		TryAttach();
	}

	// 9b
	template <typename Y>
    hkRefPtr(const hkRefPtr<Y>& a_rhs) requires (std::is_convertible_v<Y*, T*>) :
		_ptr(a_rhs._ptr)
	{
		TryAttach();
	}

	// 10a
    hkRefPtr(hkRefPtr&& a_rhs) noexcept :
		_ptr(std::move(a_rhs._ptr))
	{
		a_rhs._ptr = nullptr;
	}

	// 10b
	template <typename Y>
    hkRefPtr(hkRefPtr<Y>&& a_rhs) noexcept requires (std::is_convertible_v<Y*, T*>) :
		_ptr(std::move(a_rhs._ptr))
	{
		a_rhs._ptr = nullptr;
	}

    ~hkRefPtr()
	{
		TryDetach();
	}

	// 1a
    hkRefPtr& operator=(const hkRefPtr& a_rhs)
	{
		if (this != std::addressof(a_rhs)) {
			TryDetach();
			_ptr = a_rhs._ptr;
			TryAttach();
		}
		return *this;
	}

	// 1b
	template <typename Y>
    hkRefPtr& operator=(const hkRefPtr<Y>& a_rhs)
        requires (std::is_convertible_v<
            Y*,
            T*>)
    {
		TryDetach();
		_ptr = a_rhs._ptr;
		TryAttach();
		return *this;
	}

	// 2a
    hkRefPtr& operator=(hkRefPtr&& a_rhs) noexcept
    {
		if (this != std::addressof(a_rhs)) {
			TryDetach();
			_ptr = std::move(a_rhs._ptr);
			a_rhs._ptr = nullptr;
		}
		return *this;
	}

	// 2b
	template <typename Y>
    hkRefPtr& operator=(hkRefPtr<Y>&& a_rhs) requires (std::is_convertible_v<Y*, T*>)
    {
		TryDetach();
		_ptr = std::move(a_rhs._ptr);
		a_rhs._ptr = nullptr;
		return *this;
	}

    void reset()
	{
		TryDetach();
	}

	template <typename Y>
    void reset(Y* a_ptr)
        requires (std::is_convertible_v<Y*, T*>)
    {
		if (_ptr != a_ptr) {
			TryDetach();
			_ptr = a_ptr;
			TryAttach();
		}
	}

	[[nodiscard]] constexpr T* get() const noexcept
	{
		return _ptr;
	}

	[[nodiscard]] explicit constexpr operator bool() const noexcept
	{
		return static_cast<bool>(_ptr);
	}

	[[nodiscard]] constexpr T& operator*() const noexcept
	{
		assert(static_cast<bool>(*this));
		return *_ptr;
	}

	[[nodiscard]] constexpr T* operator->() const noexcept
	{
		assert(static_cast<bool>(*this));
		return _ptr;
	}

protected:
	template <typename>
	friend struct hkRefPtr;

	void TryAttach()
	{
		if (_ptr) {
			_ptr->AddReference();
		}
	}

	void TryDetach()
	{
		if (_ptr) {
			_ptr->RemoveReference();
			_ptr = nullptr;
		}
	}

	// members
	T* _ptr;  // 0
};
static_assert(sizeof(hkRefPtr<void*>) == 0x8);

template <typename T, class... Args>
[[nodiscard]] hkRefPtr<T> make_hkref(Args&&... a_args)
{
	return hkRefPtr<T>{ new T(std::forward<Args>(a_args)...) };
}

template <typename T1, typename T2>
[[nodiscard]] constexpr bool operator==(const hkRefPtr<T1>& a_lhs, const hkRefPtr<T2>& a_rhs)
{
	return a_lhs.get() == a_rhs.get();
}

template <typename T1, typename T2>
[[nodiscard]] constexpr bool operator!=(const hkRefPtr<T1>& a_lhs, const hkRefPtr<T2>& a_rhs)
{
	return !(a_lhs == a_rhs);
}

template <typename T>
[[nodiscard]] constexpr bool operator==(const hkRefPtr<T>& a_lhs, std::nullptr_t) noexcept
{
	return !a_lhs;
}

template <typename T>
[[nodiscard]] constexpr bool operator==(std::nullptr_t, const hkRefPtr<T>& a_rhs) noexcept
{
	return !a_rhs;
}

template <typename T>
[[nodiscard]] constexpr bool operator!=(const hkRefPtr<T>& a_lhs, std::nullptr_t) noexcept
{
	return static_cast<bool>(a_lhs);
}

template <typename T>
[[nodiscard]] constexpr bool operator!=(std::nullptr_t, const hkRefPtr<T>& a_rhs) noexcept
{
	return static_cast<bool>(a_rhs);
}

template <typename T>
hkRefPtr(T*) -> hkRefPtr<T>;
