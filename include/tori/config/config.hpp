// Copyright (c) 2018 mocabe(https://github.com/mocabe)
// This code is licensed under MIT license.

/// \file
/// Tori config

#pragma once

#include <climits>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <type_traits>

// namespace
#ifndef TORI_NS
#  define TORI_NS tori
#endif

// import SIMD detect macros
#include "intrin.hpp"

// namespace config
namespace TORI_NS {
  namespace detail {
    /// interface namespace
    namespace interface {}
    using namespace interface;
  } // namespace detail
  // make interface visible
  using namespace detail::interface;
} // namespace TORI_NS

namespace TORI_NS::detail {

// debug macros
#if defined(NDEBUG)
  constexpr bool is_debug = false;
#else
  constexpr bool is_debug = true;
#endif

// env macros
#if defined(_WIN32) || defined(_WIN64)
#  if defined(_WIN64)
  constexpr bool is_64bit = true;
#  else
  constexpr bool is_64bit = false;
#  endif
#else
#  if defined(__GNUC__)
#    if defined(__x86_64__) || defined(__ppc64__)
  constexpr bool is_64bit = true;
#    else
  constexpr bool is_64bit = false;
#    endif
#  endif
#endif

// likely
#if defined(__GNUC__)
#  define likely(expr) __builtin_expect(!!(expr), 1)
#  define unlikely(expr) __builtin_expect(!!(expr), 0)
#else
#  define likely(expr) expr
#  define unlikely(expr) expr
#endif

#if defined(__GNUC__)
#  define unreachable() __builtin_unreachable()
#else
#  define unreachable() __assume(0)
#endif

// inline
#define TORI_INLINE inline

  // for std::visit
  template <class... Ts>
  struct overloaded : Ts... {
    using Ts::operator()...;
  };
  template <class... Ts>
  overloaded(Ts...)->overloaded<Ts...>;

  // for void_t
  template <class...>
  struct make_void {
    using type = void;
  };
  template <class... Ts>
  using void_t = typename make_void<Ts...>::type;

  // for false_v
  template <class...>
  struct make_false {
    static constexpr bool value = false;
  };
  template <class... Ts>
  static constexpr bool false_v = make_false<Ts...>::value;

  // for unused variable
  template <class... Args>
  constexpr void ignore(Args&&...) noexcept {}

  // for std::variant
  template <class T>
  struct monostate_base {};

  template <class T>
  constexpr bool operator<(monostate_base<T>, monostate_base<T>) noexcept {
    return false;
  }
  template <class T>
  constexpr bool operator>(monostate_base<T>, monostate_base<T>) noexcept {
    return false;
  }
  template <class T>
  constexpr bool operator<=(monostate_base<T>, monostate_base<T>) noexcept {
    return true;
  }
  template <class T>
  constexpr bool operator>=(monostate_base<T>, monostate_base<T>) noexcept {
    return true;
  }
  template <class T>
  constexpr bool operator==(monostate_base<T>, monostate_base<T>) noexcept {
    return true;
  }
  template <class T>
  constexpr bool operator!=(monostate_base<T>, monostate_base<T>) noexcept {
    return false;
  }

  // ------------------------------------------
  // Offset of member
  // ------------------------------------------

  template <typename T1, typename T2>
  struct offset_of_member_impl {
    union U {
      U() : c{} {}
      ~U() {}
      char c[sizeof(T2)];
      T2 o;
    };
    static U u;
    static constexpr size_t get(T1 T2::*member) {
      size_t i = 0;
      for (; i < sizeof(T2); ++i)
        if (((void*)&(u.c[i])) == &(u.o.*member)) break;

      // g++ bug 67371 workaround
      if (i >= sizeof(T2))
        throw;
      else
        return i;
    }
  };
  template <class T1, class T2>
  typename offset_of_member_impl<T1, T2>::U offset_of_member_impl<T1, T2>::u{};

  /// get offset of member
  template <class T1, class T2>
  constexpr size_t offset_of_member(T1 T2::*member) {
    return offset_of_member_impl<T1, T2>::get(member);
  }

  // ABI requirements
  static_assert(is_64bit, "64bit only");
  static_assert(CHAR_BIT == 8, "1byte != 8bit");
  static_assert(sizeof(char) == sizeof(unsigned char));
  static_assert(sizeof(char) == 1);
  static_assert(sizeof(void*) == 8);
  static_assert(sizeof(overloaded<>) == 1);

  namespace interface {
    using int8_t = std::int8_t;
    using int16_t = std::int16_t;
    using int32_t = std::int32_t;
    using int64_t = std::int64_t;

    using uint8_t = std::uint8_t;
    using uint16_t = std::uint16_t;
    using uint32_t = std::uint32_t;
    using uint64_t = std::uint64_t;

    using int_t = int32_t;
    using uint_t = uint32_t;
    using long_t = int64_t;
    using ulong_t = uint64_t;

    using size_t = std::size_t;
    static_assert(std::is_same_v<size_t, uint64_t>);

    using nullptr_t = std::nullptr_t;

    static_assert(sizeof(bool) == 1, "bool should be 8bit");
    using bool_t = bool;

    static_assert(
      std::numeric_limits<float>::is_iec559, "Size of float should be 32bit");
    using float_t = float;

    static_assert(
      std::numeric_limits<double>::is_iec559, "Size of double should be 64bit");
    using double_t = double;
  } // namespace interface
} // namespace TORI_NS::detail
