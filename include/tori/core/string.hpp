// Copyright (c) 2018-2019 mocabe(https://github.com/mocabe)
// This code is licensed under MIT license.

#pragma once

/// \file String

#if !defined(TORI_NO_LOCAL_INCLUDE)
#  include "type_gen.hpp"
#endif

#include <string>
#include <cstring>
#include <string>

// utf8 macro
#define utf8(Str) ::TORI_NS::detail::to_u8(u8##Str)

namespace TORI_NS::detail {

  class string_object_value
  {
    // TODO: support char8_t in C++20
  public:
    string_object_value(nullptr_t) = delete;

    string_object_value()
    {
      m_ptr = new char[1] {'\0'};
    }

    string_object_value(const char* str)
    {
      size_t s = std::strlen(str);
      auto buff = new char[s + 1];
      std::copy(str, str + s + 1, buff);
      m_ptr = buff;
    }

    string_object_value(const std::string& str)
    {
      auto buff = new char[str.size() + 1];
      std::copy(str.c_str(), str.c_str() + str.size() + 1, buff);
      m_ptr = buff;
    }

    string_object_value(const string_object_value& other)
    {
      auto len = std::strlen(other.c_str());
      auto buff = new char[len + 1];
      std::copy(other.m_ptr, other.m_ptr + len + 1, buff);
      m_ptr = buff;
    }

    string_object_value(string_object_value&& other)
    {
      m_ptr = other.m_ptr;
      other.m_ptr = nullptr;
    }

    ~string_object_value() noexcept
    {
      delete[] m_ptr;
    }

    /// c_str
    [[nodiscard]] const char* c_str() const noexcept
    {
      return reinterpret_cast<const char*>(m_ptr);
    }

  private:
    char* m_ptr;
  };

  namespace interface {

    /// UTF-8 String object.
    /// Does not guarantee anything about encoding. User must ensure
    /// input byte sequence is null(`0x00`)-terminated UTF-8 string.
    using String = Box<string_object_value>;

    namespace literals {

      /// String object literal
      [[nodiscard]] inline object_ptr<String>
        operator"" _S(const char* str, size_t)
      {
        return make_object<String>(str);
      }

    } // namespace literals

  } // namespace interface

} // namespace TORI_NS::detail

// String
TORI_DECL_TYPE(String)
