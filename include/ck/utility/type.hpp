// SPDX-License-Identifier: MIT
// Copyright (c) 2018-2022, Advanced Micro Devices, Inc. All rights reserved.

#pragma once

#include "ck/ck.hpp"
#include "ck/utility/integral_constant.hpp"
#include "ck/utility/enable_if.hpp"

namespace ck {

template <typename X, typename Y>
struct is_same : public integral_constant<bool, false>
{
};

template <typename X>
struct is_same<X, X> : public integral_constant<bool, true>
{
};

template <typename X, typename Y>
inline constexpr bool is_same_v = is_same<X, Y>::value;

template <typename T>
using remove_reference_t = typename std::remove_reference<T>::type;

template <typename T>
using remove_cv_t = typename std::remove_cv<T>::type;

template <typename T>
using remove_cvref_t = remove_cv_t<std::remove_reference_t<T>>;

template <typename T>
using remove_pointer_t = typename std::remove_pointer<T>::type;

template <typename T>
inline constexpr bool is_pointer_v = std::is_pointer<T>::value;

template <typename Y, typename X, typename enable_if<sizeof(X) == sizeof(Y), bool>::type = false>
__host__ __device__ constexpr Y bit_cast(const X& x)
{
#if CK_EXPERIMENTAL_USE_MEMCPY_FOR_BIT_CAST
    Y y;

    __builtin_memcpy(&y, &x, sizeof(X));

    return y;
#else
    union AsType
    {
        X x;
        Y y;
    };

    return AsType{x}.y;
#endif
}

namespace detail {
template <typename T>
struct sgpr_ptr
{
    static_assert(!std::is_const_v<T> && !std::is_reference_v<T> &&
                  std::is_trivially_copyable_v<T>);

    __device__ explicit sgpr_ptr(const T& obj) noexcept
    {
        /// TODO: copy object content into member 'memory' by __builtin_amdgcn_readfirstlane()
        __builtin_memcpy(memory, &obj, sizeof(obj));
    }

    __device__ T& operator*() { return *(this->operator->()); }

    __device__ const T& operator*() const { return *(this->operator->()); }

    __device__ T* operator->() { return reinterpret_cast<T*>(memory); }

    __device__ const T* operator->() const { return reinterpret_cast<const T*>(memory); }

    private:
    alignas(T) unsigned char memory[sizeof(T) + 3];
};
} // namespace detail

template <typename T>
__device__ constexpr auto readfirstlane(const T& obj)
{
    return detail::sgpr_ptr<T>(obj);
}

} // namespace ck
