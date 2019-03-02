#pragma once

// Copyright (c) 2018 mocabe(https://github.com/mocabe)
// This code is licensed under MIT license.

#include "object_ptr.hpp"

#include <array>
#include <cstring>

namespace TORI_NS::detail {

  // ------------------------------------------
  // TypeValue union values

  /// Value type
  struct ValueType
  {
    /// Max name length
    static constexpr uint64_t max_name_size = 32;
    /// buffer type
    using buffer_type = std::array<char, max_name_size>;
    /// buffer
    const buffer_type* name;

    /// get C-style string
    const char* c_str() const
    {
      return name->data();
    }

    /// compare two value types
    static bool compare(const ValueType& lhs, const ValueType& rhs)
    {
      if constexpr (has_AVX2 && max_name_size == 32) {
        // AVX2
        // buffers should be aligned as 32byte
        // load each buffers into 256-bit registers
        auto ymm0 =
          _mm256_load_si256(reinterpret_cast<const __m256i*>(lhs.name->data()));
        auto ymm1 =
          _mm256_load_si256(reinterpret_cast<const __m256i*>(rhs.name->data()));
        // compare
        auto cmpeq = _mm256_cmpeq_epi8(ymm0, ymm1);
        // get mask
        unsigned mask = _mm256_movemask_epi8(cmpeq);
        // clear upper bits for other SIMD operations
        _mm256_zeroupper();
        return mask == 0xffffffffU;
      } else if constexpr (has_AVX && max_name_size == 32) {
        // AVX
        // buffers should be aligned as 16byte
        // load buffer into 2 xmm registers
        auto xmm0 = _mm_load_si128(
          reinterpret_cast<const __m128i*>(lhs.name->data() + 0));
        auto xmm1 = _mm_load_si128(
          reinterpret_cast<const __m128i*>(lhs.name->data() + 16));
        // compare registers to another buffer on memory
        auto cmp1 = _mm_cmpeq_epi8(
          xmm0, *reinterpret_cast<const __m128i*>(rhs.name->data() + 0));
        auto cmp2 = _mm_cmpeq_epi8(
          xmm1, *reinterpret_cast<const __m128i*>(rhs.name->data() + 16));
        // get cmp result
        auto cmp = _mm_and_si128(cmp1, cmp2);
        // get mask
        auto mask = _mm_movemask_epi8(cmp);
        return mask == 0xffffU;
      } else {
        // fallback to memcmp
        return std::memcmp(        //
                 lhs.name->data(), //
                 rhs.name->data(), //
                 max_name_size) == 0;
      }
    }
  };

  /// Arrow type
  struct ArrowType
  {
    /// argument type
    object_ptr<const Type> captured;
    /// return type
    object_ptr<const Type> returns;
  };

  /// Any type
  struct VarType
  {
    /// unique id for VarTpye object
    uint64_t id;
  };

  // ------------------------------------------
  // TypeValue

  struct type_object_value_storage
  {

    /// default ctor is disabled
    type_object_value_storage() = delete;

    // initializers
    type_object_value_storage(ValueType t)
      : value {std::move(t)}
      , index {value_index}
    {
    }

    type_object_value_storage(ArrowType t)
      : arrow {std::move(t)}
      , index {arrow_index}
    {
    }

    type_object_value_storage(VarType t)
      : var {std::move(t)}
      , index {var_index}
    {
    }

    /// Copy constructor
    type_object_value_storage(const type_object_value_storage& other)
      : index {other.index}
    {
      // copy union
      if (other.index == value_index) {
        value = other.value;
      }
      if (other.index == arrow_index) {
        arrow = other.arrow;
      }
      if (other.index == var_index) {
        var = other.var;
      }
      throw std::bad_cast();
    }

    /// Destructor
    ~type_object_value_storage() noexcept
    {
      // call destructor
      if (index == value_index)
        value.~ValueType();
      if (index == arrow_index)
        arrow.~ArrowType();
      if (index == var_index)
        var.~VarType();
    }

    template <class T>
    static constexpr uint64_t type_index()
    {
      if constexpr (std::is_same_v<std::decay_t<T>, ValueType>) {
        return value_index;
      } else if constexpr (std::is_same_v<std::decay_t<T>, ArrowType>) {
        return arrow_index;
      } else if constexpr (std::is_same_v<std::decay_t<T>, VarType>) {
        return var_index;
      } else {
        static_assert(false_v<T>);
      }
    }

    // hard-coded type index
    static constexpr uint64_t value_index = 0;
    static constexpr uint64_t arrow_index = 1;
    static constexpr uint64_t var_index = 2;

    // 16 byte union
    union
    {
      ValueType value;
      ArrowType arrow;
      VarType var;
    };

    // 8 byte index
    uint64_t index;
  };

  /// Base class for TypeValue
  class type_object_value : type_object_value_storage
  {
    friend const type_object_value_storage&    //
      _get_storage(const type_object_value&);  //
                                               //
    friend type_object_value_storage&          //
      _get_storage(type_object_value&);        //
                                               //
    friend const type_object_value_storage&&   //
      _get_storage(const type_object_value&&); //
                                               //
    friend type_object_value_storage&&         //
      _get_storage(type_object_value&&);       //

    using base = type_object_value_storage;

  public:

    /// default ctor is disabled
    type_object_value() = delete;

    // initializers
    type_object_value(ValueType t)
      : base {t}
    {
    }

    type_object_value(ArrowType t)
      : base {t}
    {
    }

    type_object_value(VarType t)
      : base {t}
    {
    }

  };

  const type_object_value_storage& _get_storage(const type_object_value& v)
  {
    return v;
  }

  type_object_value_storage& _get_storage(type_object_value& v)
  {
    return v;
  }

  const type_object_value_storage&& _get_storage(const type_object_value&& v)
  {
    return std::move(v);
  }

  type_object_value_storage&& _get_storage(type_object_value&& v)
  {
    return std::move(v);
  }

  // ------------------------------------------
  // TypeValue utility

  template <
    size_t Idx,
    class T,
    std::enable_if_t<
      std::is_same_v<std::decay_t<T>, type_object_value_storage>,
      nullptr_t>* = nullptr>
  constexpr decltype(auto) _access(T&& v)
  {
    if constexpr (Idx == type_object_value_storage::value_index) {
      auto&& ref = std::forward<T>(v).value;
      return ref;
    } else if constexpr (Idx == type_object_value_storage::arrow_index) {
      auto&& ref = std::forward<T>(v).arrow;
      return ref;
    } else if constexpr (Idx == type_object_value_storage::var_index) {
      auto&& ref = std::forward<T>(v).var;
      return ref;
    } else {
      static_assert(false_v<Idx>, "Invalid index for TypeValue");
    }
  }

  template <
    size_t Idx,
    class T,
    std::enable_if_t<
      std::is_same_v<std::decay_t<T>, type_object_value_storage>,
      nullptr_t>* = nullptr>
  constexpr decltype(auto) _get(T&& v)
  {
    if (v.index != Idx)
      throw std::bad_cast();
    return _access<Idx>(std::forward<T>(v));
  }

  /// std::get() equivalent
  template <class T>
  decltype(auto) get(const type_object_value& val)
  {
    auto&& storage = _get_storage(val);
    return _get<type_object_value_storage::type_index<T>()>(storage);
  }

  /// std::get() equivalent
  template <class T>
  decltype(auto) get(const type_object_value&& val)
  {
    auto&& storage = _get_storage(std::move(val));
    return _get<type_object_value_storage::type_index<T>()>(storage);
  }

  /// std::get() equivalent
  template <class T>
  decltype(auto) get(type_object_value& val)
  {
    auto&& storage = _get_storage(val);
    return _get<type_object_value_storage::type_index<T>()>(storage);
  }

  /// std::get() equivalent
  template <class T>
  decltype(auto) get(type_object_value&& val)
  {
    auto&& storage = _get_storage(std::move(val));
    return _get<type_object_value_storage::type_index<T>()>(storage);
  }

  /// std::get_if() equivalent
  template <class T>
  constexpr std::add_pointer_t<const T> get_if(const type_object_value* val)
  {
    constexpr auto Idx = type_object_value_storage::type_index<T>();
    if (val && Idx == _get_storage(*val).index)
      return &get<T>(*val);
    return nullptr;
  }

  /// std::get_if() equivalent
  template <class T>
  constexpr std::add_pointer_t<T> get_if(type_object_value* val)
  {
    constexpr auto Idx = type_object_value_storage::type_index<T>();
    if (val && Idx == _get_storage(*val).index)
      return &get<T>(*val);
    return nullptr;
  }

} // namespace TORI_NS::detail