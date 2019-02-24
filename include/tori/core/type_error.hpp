// Copyright (c) 2018 mocabe(https://github.com/mocabe)
// This code is licensed under MIT license.

#pragma once

#include "../config/config.hpp"
#include "exception.hpp"

namespace TORI_NS::detail {

  // ------------------------------------------
  // Type errors

  enum class type_error_type : uint64_t
  {
    unknown = 0,
    circular_constraints = 1,
    type_missmatch = 2,
    bad_type_check = 3,
  };

  /// TypeErrorValue
  struct TypeErrorValue
  {
    /// type
    type_error_type error_type;

    // type_missmatch
    // bad_type_check
    object_ptr<const Type> expected;

    // circular_constraints
    // type_missmatch
    // bad_type_check
    object_ptr<const Type> provided;
  };

  namespace interface {

    // TypeError
    using TypeError = Box<TypeErrorValue>;

  } // namespace interface

  namespace interface {

    namespace type_error {

      /// type_error
      class type_error : public std::logic_error
      {
      public:
        /// Ctor string
        template <class T>
        explicit type_error(const std::string& what, object_ptr<T> src)
          : std::logic_error(what)
          , m_src {std::move(src)}
        {
        }

        template <class T>
        /// Ctor const char*
        explicit type_error(const char* what, object_ptr<T> src)
          : std::logic_error(what)
          , m_src {std::move(src)}
        {
        }

        /// get source node
        const object_ptr<const Object>& src() const
        {
          return m_src;
        }

      private:
        /// source node
        object_ptr<const Object> m_src;
      };

      /// unification error(circular constraint)
      class circular_constraint : public type_error
      {
      public:
        circular_constraint(
          object_ptr<const Object> src,
          object_ptr<const Type> var)
          : type_error("Circular constraints", std::move(src))
          , m_var {std::move(var)}
        {
        }

        /// var
        const object_ptr<const Type>& var() const
        {
          return m_var;
        }

      private:
        object_ptr<const Type> m_var;
      };

      /// unification error(missmatch)
      class type_missmatch : public type_error
      {
      public:
        type_missmatch(
          object_ptr<const Object> src,
          object_ptr<const Type> t1,
          object_ptr<const Type> t2)
          : type_error("Type missmatch", std::move(src))
          , m_t1 {std::move(t1)}
          , m_t2 {std::move(t2)}
        {
        }

        /// t1
        const object_ptr<const Type>& t1() const
        {
          return m_t1;
        }

        /// t2
        const object_ptr<const Type>& t2() const
        {
          return m_t2;
        }

      private:
        /// t1
        object_ptr<const Type> m_t1;
        /// t2
        object_ptr<const Type> m_t2;
      };

      /// bad type check
      class bad_type_check : public type_error
      {
      public:
        bad_type_check(
          object_ptr<const Type> expected,
          object_ptr<const Type> result,
          object_ptr<const Object> obj)
          : type_error(
              "type_error: check_type failed. Result type is invalid",
              std::move(obj))
          , m_expected {std::move(expected)}
          , m_result {std::move(result)}
        {
        }

        /// expected
        const object_ptr<const Type>& expected() const
        {
          return m_expected;
        }

        /// result
        const object_ptr<const Type>& result() const
        {
          return m_result;
        }

      private:
        /// expected type
        object_ptr<const Type> m_expected;
        /// result type
        object_ptr<const Type> m_result;
      };

    } // namespace type_error

  } // namespace interface

  // ------------------------------------------
  // Type errors

  const object_ptr<Exception> to_Exception(const type_error::type_error& e)
  {
    return make_object<Exception>(
      e.what(),
      make_object<TypeError>(type_error_type::unknown, nullptr, nullptr));
  }

  const object_ptr<Exception>
    to_Exception(const type_error::circular_constraint& e)
  {
    return make_object<Exception>(
      e.what(),
      make_object<TypeError>(
        type_error_type::circular_constraints, nullptr, e.var()));
  }

  const object_ptr<Exception> to_Exception(const type_error::type_missmatch& e)
  {
    return make_object<Exception>(
      e.what(),
      make_object<TypeError>(type_error_type::type_missmatch, e.t1(), e.t2()));
  }

  const object_ptr<Exception> to_Exception(const type_error::bad_type_check& e)
  {
    return make_object<Exception>(
      e.what(),
      make_object<TypeError>(
        type_error_type::bad_type_check, e.expected(), e.result()));
  }

} // namespace TORI_NS::detail

// TypeError
TORI_DECL_TYPE(TypeError)