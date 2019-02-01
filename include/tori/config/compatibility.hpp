// Copyright (c) 2018 mocabe(https://github.com/mocabe)
// This code is licensed under MIT license.

#pragma once

/// \file compatibility check for cross-compiler linking

#include "../core.hpp"

namespace TORI_NS::detail {

  // ------------------------------------------
  // Soft ABI requirements for cross-compiler linking

  // detect 64bit operating system
  static_assert(is_64bit, "Build environment might not be 64bit");
  // char should be 1 byte length
  static_assert(CHAR_BIT == 8);
  static_assert(sizeof(char) == sizeof(unsigned char));
  static_assert(sizeof(char) == 1);
  // pointer size should be 8 byte
  static_assert(sizeof(void*) == 8);
  // bool should be 1 byte
  static_assert(sizeof(bool) == 1, "sizeof(bool) is not 1byte");

  // IEEE 754
  static_assert(
    std::numeric_limits<float>::is_iec559,
    "float should be IEEE754 32bit floating point representation");
  static_assert(
    std::numeric_limits<double>::is_iec559,
    "double should be  IEEE754 64bit floating point representation");

  // ------------------------------------------
  // class layout tests

  // Object
  static_assert(sizeof(Object) == 16);
  static_assert(offset_of_member(&Object::refcount) == 0);
  static_assert(offset_of_member(&Object::info_table) == 8);

  // object_info_table
  static_assert(sizeof(object_info_table) == 40);
  static_assert(offset_of_member(&object_info_table::obj_type) == 0);
  static_assert(offset_of_member(&object_info_table::obj_size) == 8);
  static_assert(offset_of_member(&object_info_table::obj_ext_bytes) == 16);
  static_assert(offset_of_member(&object_info_table::destroy) == 24);
  static_assert(offset_of_member(&object_info_table::clone) == 32);

  // closure_info_table
  static_assert(sizeof(closure_info_table) == 64);
  static_assert(offset_of_member(&closure_info_table::n_args) == 40);
  static_assert(offset_of_member(&closure_info_table::clsr_ext_bytes) == 48);
  static_assert(offset_of_member(&closure_info_table::code) == 56);

#if !defined(OBJECT_HEADER_EXTEND_BYTES)

  static_assert(offset_of_member(&Box<char>::_value) == 16);
  static_assert(offset_of_member(&Box<int>::_value) == 16);
  static_assert(offset_of_member(&Box<long>::_value) == 16);
  // ...

  static_assert(offset_of_member(&Closure<>::_arity) == 16);

#  if !defined(CLOSURE_HEADER_EXTEND_BYTES)

  static_assert(offset_of_member(&ClosureN<1>::_args) == 24);
  static_assert(offset_of_member(&ClosureN<2>::_args) == 24);
  static_assert(offset_of_member(&ClosureN<3>::_args) == 24);
  static_assert(offset_of_member(&ClosureN<4>::_args) == 24);
  // ...

#  endif

#endif

}