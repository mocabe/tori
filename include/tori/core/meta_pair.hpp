// Copyright (c) 2018-2019 mocabe(https://github.com/mocabe)
// This code is licensed under MIT license.

#pragma once

#if !defined(TORI_NO_LOCAL_INCLUDE)
#  include "../config/config.hpp"
#  include "meta_type.hpp"
#endif

namespace TORI_NS::detail{

  template <class First, class Second>
  struct meta_pair
  {
    constexpr auto first() const
    {
      return type_c<First>;
    }
    constexpr auto second() const
    {
      return type_c<Second>;
    }
  };

  template <class First, class Second>
  static constexpr meta_pair<First, Second> pair_c {};

  template <class F, class S>
  constexpr auto make_pair(meta_type<F>, meta_type<S>)
  {
    return pair_c<F, S>;
  }
}