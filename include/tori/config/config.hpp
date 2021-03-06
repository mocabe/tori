// Copyright (c) 2018-2019 mocabe(https://github.com/mocabe)
// This code is licensed under MIT license.

/// \file
/// Tori config

#pragma once

// namespace
#ifndef TORI_NS
#  define TORI_NS tori
#endif

#if !defined(TORI_NO_LOCAL_INCLUDE)
// import SIMD detect macros
#  include "intrin.hpp"
#endif

#include <climits>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <cassert>


/// top-level namespace
namespace TORI_NS {

  /// detail namespace
  namespace detail {

    /// interface namespace
    namespace interface {

      /// user defined literals
      namespace literals {
      }

    } // namespace interface

    using namespace interface;
    using namespace interface::literals;

  } // namespace detail

  // make interface visible
  using namespace detail::interface;

} // namespace TORI_NS

namespace TORI_NS::detail {

// debug macros
#if defined(NDEBUG)
  constexpr bool debug_mode = false;
#else
  constexpr bool debug_mode = true;
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
#if !defined(TORI_LIKELY)
#  if defined(__GNUC__)
#    define TORI_LIKELY(expr) __builtin_expect(!!(expr), 1)
#  else
#    define TORI_LIKELY(expr) expr
#  endif
#endif

// unlikely
#if !defined(TORI_UNLIKELY)
#  if defined(__GNUC__)
#    define TORI_UNLIKELY(expr) __builtin_expect(!!(expr), 0)
#  else
#    define TORI_UNLIKELY(expr) expr
#  endif
#endif

// unreachable
#if !defined(TORI_UNREACHABLE)
#  if defined(__GNUC__)
#    define TORI_UNREACHABLE() \
      assert(false);           \
      __builtin_unreachable()
#  elif defined(_MSC_VER)
#    define TORI_UNREACHABLE() \
      assert(false);           \
      __assume(0)
#  else
#    define TORI_UNREACHABLE() assert(false)
#  endif
#endif

// assert
#if !defined(TORI_ASSERT)
#  define TORI_ASSERT(x) assert(x)
#endif

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

    using nullptr_t = std::nullptr_t;
    using bool_t = bool;
    using float_t = float;
    using double_t = double;

  } // namespace interface
} // namespace TORI_NS::detail
