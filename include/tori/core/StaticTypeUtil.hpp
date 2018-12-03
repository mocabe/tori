#pragma once

// Copyright (c) 2018 mocabe(https://github.com/mocabe)
// This code is licensed under MIT license.

/// \file Template type utility

#include <type_traits>
#include <utility>

// false_v
#include "../config/config.hpp"

namespace TORI_NS::detail {

  // ------------------------------------------
  // Utility
  // ------------------------------------------
  template <class T, class E>
  struct append_tuple {
    using type = std::tuple<T, E>;
  };
  template <class... Ts, class E>
  struct append_tuple<std::tuple<Ts...>, E> {
    using type = std::tuple<Ts..., E>;
  };
  /// append tuples
  template <class T, class E>
  using append_tuple_t = typename append_tuple<T, E>::type;

  template <class T1, class T2>
  struct concat_tuple {};
  template <class... T1s, class... T2s>
  struct concat_tuple<std::tuple<T1s...>, std::tuple<T2s...>> {
    using type = std::tuple<T1s..., T2s...>;
  };
  /// concat tuples
  template <class T1, class T2>
  using concat_tuple_t = typename concat_tuple<T1, T2>::type;

  template <class X, class T>
  struct set_belong {};
  template <class X>
  struct set_belong<X, std::tuple<>> {
    static constexpr bool value = false;
  };
  template <class X, class TH, class... TT>
  struct set_belong<X, std::tuple<TH, TT...>> {
    static constexpr bool value =
      std::is_same_v<X, TH> || set_belong<X, std::tuple<TT...>>::value;
  };
  /// Does X occur in Set?
  template <class X, class Set>
  static constexpr bool set_belong_v = set_belong<X, Set>::value;

  template <class X, class T>
  struct set_include {};
  template <class T>
  struct set_include<std::tuple<>, T> {
    static constexpr bool value = true;
  };
  template <class XH, class... XT, class T>
  struct set_include<std::tuple<XH, XT...>, T> {
    static constexpr bool value =
      set_belong_v<XH, T> && set_include<std::tuple<XT...>, T>::value;
  };
  /// Does S2 include S1?
  template <class S1, class S2>
  static constexpr bool set_include_v = set_include<S1, S2>::value;

  template <class X, class Y>
  struct set_eq {
    static constexpr bool value =
      set_include<X, Y>::value && set_include_v<Y, X>;
  };
  /// Are X and Y same set?
  template <class X, class Y>
  static constexpr bool set_eq_v = set_eq<X, Y>::value;

  // ------------------------------------------
  // Types
  // ------------------------------------------

  /// Arrow type
  template <class T1, class T2>
  struct arrow {};
  /// Type variable
  template <class Tag>
  struct var {};
  /// Value type
  template <class Tag>
  struct value {};
  // Var value
  template <class Tag>
  struct varvalue {};

  /// Type mapping
  template <class T1, class T2>
  struct tyarrow {};

  /// error_type
  template <class Tag>
  struct error_type {};

  namespace error_tags {
    /// type_missmatch
    template <class T1, class T2, class Other = std::tuple<>>
    struct type_missmatch {};
    /// unsolvable_constraints
    template <class T1, class T2, class Other = std::tuple<>>
    struct unsolvable_constraints {};
    /// circular_constraints
    template <class Var, class Other = std::tuple<>>
    struct circular_constraints {};
    /// none
    struct none {};
  } // namespace error_tags

  template <class T>
  struct is_error_type : std::false_type {};
  template <class T1, class T2, class Other>
  struct is_error_type<error_type<error_tags::type_missmatch<T1, T2, Other>>>
    : std::true_type {};
  template <class T1, class T2, class Other>
  struct is_error_type<
    error_type<error_tags::unsolvable_constraints<T1, T2, Other>>>
    : std::true_type {};
  template <class Var, class Other>
  struct is_error_type<error_type<error_tags::circular_constraints<Var, Other>>>
    : std::true_type {};
  /// is_error_type_v
  template <class T>
  static constexpr bool is_error_type_v = is_error_type<T>::value;

  // ------------------------------------------
  // Terms
  // ------------------------------------------

  /// tm_apply
  template <class T1, class T2>
  struct tm_apply {};
  /// tm_closure
  template <class... Ts>
  struct tm_closure {};
  /// tm_value
  template <class T>
  struct tm_value {
    using type = value<T>;
  };
  /// tm_var
  template <class Tag>
  struct tm_var {
    using type = var<Tag>;
  };
  /// tm_varvalue
  template <class Tag>
  struct tm_varvalue {
    using type = varvalue<Tag>;
  };

  /// tm_fix
  template <class Tag>
  struct tm_fix {};

  template <class T, class = void>
  struct has_term : std::false_type {};
  template <class T>
  struct has_term<T, void_t<typename T::term>> : std::true_type {};
  template <class T>
  static constexpr bool has_term_v = has_term<T>::value;

  // ------------------------------------------
  // Subst
  // ------------------------------------------
  template <class TyArrow, class Ty>
  struct subst_impl {};
  template <class TyT1, class TyT2, class T1, class T2>
  struct subst_impl<tyarrow<TyT1, TyT2>, arrow<T1, T2>> {
    using type = arrow<
      typename subst_impl<tyarrow<TyT1, TyT2>, T1>::type,
      typename subst_impl<tyarrow<TyT1, TyT2>, T2>::type>;
  };
  template <class TyT1, class TyT2, class Tag>
  struct subst_impl<tyarrow<TyT1, TyT2>, value<Tag>> {
    using type = value<Tag>;
  };
  template <class TyT1, class TyT2, class Tag>
  struct subst_impl<tyarrow<TyT1, TyT2>, varvalue<Tag>> {
    using type = varvalue<Tag>;
  };
  template <class TyT1, class TyT2, class Tag>
  struct subst_impl<tyarrow<TyT1, TyT2>, var<Tag>> {
    using type =
      std::conditional_t<std::is_same_v<TyT1, var<Tag>>, TyT2, var<Tag>>;
  };
  /// Apply TyArrow to Ty
  template <class TyArrow, class Ty>
  using subst_t = typename subst_impl<TyArrow, Ty>::type;

  template <class Arrows, class Ty>
  struct subst_all_impl;
  template <class Ty>
  struct subst_all_impl<std::tuple<>, Ty> {
    using type = Ty;
  };
  template <class TyArrow, class... Tail, class Ty>
  struct subst_all_impl<std::tuple<TyArrow, Tail...>, Ty> {
    using type =
      typename subst_all_impl<std::tuple<Tail...>, subst_t<TyArrow, Ty>>::type;
  };
  /// Apply all TyArrows to Ty
  template <class TyArrows, class Ty>
  using subst_all_t = typename subst_all_impl<TyArrows, Ty>::type;

  // ------------------------------------------
  // Constr
  // ------------------------------------------

  /// Type constraint
  template <class T1, class T2>
  struct constr {
    using type = constr<T1, T2>;
    using t1 = T1;
    using t2 = T2;
  };

  template <class TyArrow, class Constr>
  struct subst_constr_impl {};
  template <class TyT1, class TyT2, class T1, class T2>
  struct subst_constr_impl<tyarrow<TyT1, TyT2>, constr<T1, T2>> {
    using type = constr<
      subst_t<tyarrow<TyT1, TyT2>, T1>,
      subst_t<tyarrow<TyT1, TyT2>, T2>>;
  };
  /// Apply TyArrow to Constr
  template <class TyArrow, class Constr>
  using subst_constr_t = typename subst_constr_impl<TyArrow, Constr>::type;

  template <class TyArrow, class Constrs>
  struct subst_constr_all_impl {
    static_assert(false_v<TyArrow>, "Invalid argument(s)");
  };
  template <class TyT1, class TyT2, class... Cs>
  struct subst_constr_all_impl<tyarrow<TyT1, TyT2>, std::tuple<Cs...>> {
    using type = std::tuple<subst_constr_t<tyarrow<TyT1, TyT2>, Cs>...>;
  };
  /// Apply TyArrow to all constrains Cs
  template <class TyArrow, class Cs>
  using subst_constr_all_t = typename subst_constr_all_impl<TyArrow, Cs>::type;

  // ------------------------------------------
  // Occurs
  // ------------------------------------------
  template <class X, class T>
  struct occurs_impl {};
  template <class X, class T1, class T2>
  struct occurs_impl<X, arrow<T1, T2>> {
    static constexpr bool value =
      occurs_impl<X, T1>::value || occurs_impl<X, T2>::value;
  };
  template <class X, class Tag>
  struct occurs_impl<X, value<Tag>> {
    static constexpr bool value = false;
  };
  template <class X, class Tag>
  struct occurs_impl<X, var<Tag>> {
    static constexpr bool value = std::is_same_v<X, var<Tag>>;
  };
  template <class X, class Tag>
  struct occurs_impl<X, varvalue<Tag>> {
    static constexpr bool value = false;
  };
  /// Does X occur in T?
  template <class X, class T>
  static constexpr bool occurs_v = occurs_impl<X, T>::value;

  // ------------------------------------------
  // Unify
  // ------------------------------------------

  template <class ConstrList>
  struct unify_impl {
    // error
    using type = error_type<
      error_tags::
        unsolvable_constraints<error_tags::none, error_tags::none, ConstrList>>;
  };

  template <class T1, class T2, class... Ts>
  struct unify_impl<std::tuple<constr<T1, T2>, Ts...>> {
    // error
    using type =
      error_type<error_tags::unsolvable_constraints<T1, T2, std::tuple<Ts...>>>;
  };
  // helper
  template <class Var, class T, class Tail, bool B = !occurs_v<Var, T>>
  struct unify_h {
    // error
    using type = error_type<error_tags::circular_constraints<Var, Tail>>;
  };
  // helper2
  template <
    class Tag1,
    class Tag2,
    class Tail,
    bool B = std::is_same_v<Tag1, Tag2>>
  struct unify_h2 {
    // error
    using type =
      error_type<error_tags::type_missmatch<value<Tag1>, value<Tag2>, Tail>>;
  };
  // helper3
  template <
    class Tag1,
    class Tag2,
    class Tail,
    bool B = std::is_same_v<Tag1, Tag2>>
  struct unify_h3 {
    // error
    using type = error_type<
      error_tags::type_missmatch<varvalue<Tag1>, varvalue<Tag2>, Tail>>;
  };

  // helper
  template <class Var, class T, class Tail>
  struct unify_h<Var, T, Tail, true> {
    using _t =
      typename unify_impl<subst_constr_all_t<tyarrow<Var, T>, Tail>>::type;
    using type = std::conditional_t<
      is_error_type_v<_t>,
      _t,
      append_tuple_t<_t, tyarrow<Var, T>>>;
  };
  // helper2
  template <class Tag1, class Tag2, class Tail>
  struct unify_h2<Tag1, Tag2, Tail, true> {
    using type = typename unify_impl<Tail>::type;
  };
  // helper3
  template <class Tag1, class Tag2, class Tail>
  struct unify_h3<Tag1, Tag2, Tail, true> {
    using type = typename unify_impl<Tail>::type;
  };

  // empty set
  template <>
  struct unify_impl<std::tuple<>> {
    using type = std::tuple<>;
  };
  template <class Tag1, class Tag2, class... Cs>
  struct unify_impl<std::tuple<constr<value<Tag1>, value<Tag2>>, Cs...>> {
    using type = typename unify_h2<Tag1, Tag2, std::tuple<Cs...>>::type;
  };
  template <class Tag1, class Tag2, class... Cs>
  struct unify_impl<std::tuple<constr<varvalue<Tag1>, varvalue<Tag2>>, Cs...>> {
    using type = typename unify_h3<Tag1, Tag2, std::tuple<Cs...>>::type;
  };
  template <class Tag1, class Tag2, class... Cs>
  struct unify_impl<std::tuple<constr<var<Tag1>, var<Tag2>>, Cs...>> {
    using type = std::conditional_t<
      std::is_same_v<var<Tag1>, var<Tag2>>,
      typename unify_impl<std::tuple<Cs...>>::type,
      typename unify_h<var<Tag2>, var<Tag1>, std::tuple<Cs...>>::type>;
  };
  template <class T1, class Tag, class... Cs>
  struct unify_impl<std::tuple<constr<T1, var<Tag>>, Cs...>> {
    using type = std::conditional_t<
      std::is_same_v<T1, var<Tag>>,
      typename unify_impl<std::tuple<Cs...>>::type,
      typename unify_h<var<Tag>, T1, std::tuple<Cs...>>::type>;
  };
  template <class Tag, class T2, class... Cs>
  struct unify_impl<std::tuple<constr<var<Tag>, T2>, Cs...>> {
    using type = std::conditional_t<
      std::is_same_v<var<Tag>, T2>,
      typename unify_impl<std::tuple<Cs...>>::type,
      typename unify_h<var<Tag>, T2, std::tuple<Cs...>>::type>;
  };
  template <class S1, class S2, class T1, class T2, class... Cs>
  struct unify_impl<std::tuple<constr<arrow<S1, S2>, arrow<T1, T2>>, Cs...>> {
    using type = typename unify_impl<
      std::tuple<constr<S1, T1>, constr<S2, T2>, Cs...>>::type;
  };

  template <class T>
  struct unify_assert {
    using type = T;
  };

  template <class T1, class T2, class Other>
  struct unify_assert<error_type<error_tags::type_missmatch<T1, T2, Other>>> {
    using info1 = typename T1::_error_expected;
    using info2 = typename T2::_error_provided;
    using info3 = typename Other::_error_other;
    static_assert(false_v<T1>, "Unification error: Type missmatch");
  };

  template <class Constrs>
  struct unify_assert<error_type<
    error_tags::
      unsolvable_constraints<error_tags::none, error_tags::none, Constrs>>> {
    using info = typename Constrs::_error_constraints;
    static_assert(
      false_v<Constrs>,
      "Unification error: Unsolvable constraints");
  };

  template <class T1, class T2, class Other>
  struct unify_assert<
    error_type<error_tags::unsolvable_constraints<T1, T2, Other>>> {
    using info1 = typename T1::_error_lhs;
    using info2 = typename T2::_error_rhs;
    using info3 = typename Other::_error_other;
    static_assert(false_v<T1>, "Unification error: Unsolvable constraints");
  };

  template <class Var, class Other>
  struct unify_assert<
    error_type<error_tags::circular_constraints<Var, Other>>> {
    using info1 = typename Var::_error_var;
    using info2 = typename Other::_error_other;
    static_assert(false_v<Var>, "Unification error: Circular constraints");
  };

  template <class Cs, bool Assert>
  struct unify_switch {
    using _t = typename unify_impl<Cs>::type;
    using type = typename unify_assert<_t>::type;
  };

  template <class Cs>
  struct unify_switch<Cs, false> {
    using type = typename unify_impl<Cs>::type;
  };

  /// Unification
  template <class Cs, bool Assert = true>
  using unify_t = typename unify_switch<Cs, Assert>::type;

  // ------------------------------------------
  // tm_closure to Tuple
  // ------------------------------------------
  template <class T>
  struct to_tuple {};

  template <class... Ts>
  struct to_tuple<tm_closure<Ts...>> {
    using type = std::tuple<Ts...>;
  };

  template <class T>
  using to_tuple_t = typename to_tuple<T>::type;

  // ------------------------------------------
  // Tuple to tm_closure
  // ------------------------------------------

  template <class T>
  struct to_tm_closure {};
  template <class... Ts>
  struct to_tm_closure<std::tuple<Ts...>> {
    using type = tm_closure<Ts...>;
  };
  template <class T>
  using to_tm_closure_t = typename to_tm_closure<T>::type;

  // ------------------------------------------
  // Typing
  // ------------------------------------------

  /// Tag type generator
  template <size_t N>
  struct taggen {};

  template <class GenTag>
  struct genvar_impl {};

  template <size_t N>
  struct genvar_impl<taggen<N>> {
    using type = var<taggen<N>>;
    using next = taggen<N + 1>;
  };

  /// Generate new type variable
  template <class Gen>
  using genvar_t = typename genvar_impl<Gen>::type;

  /// Get next tag generator
  template <class Gen>
  using nextgen_t = typename genvar_impl<Gen>::next;

  template <class From, class To, class In>
  struct subst_term_impl {};

  template <class From, class To, class... Ts>
  struct subst_term_impl<From, To, tm_closure<Ts...>> {
    using type = std::conditional_t<
      std::is_same_v<From, tm_closure<Ts...>>,
      To,
      tm_closure<typename subst_term_impl<From, To, Ts>::type...>>;
  };

  template <class From, class To, class T1, class T2>
  struct subst_term_impl<From, To, tm_apply<T1, T2>> {
    using type = std::conditional_t<
      std::is_same_v<From, tm_apply<T1, T2>>,
      To,
      tm_apply<
        typename subst_term_impl<From, To, T1>::type,
        typename subst_term_impl<From, To, T2>::type>>;
  };

  template <class From, class To, class T>
  struct subst_term_impl<From, To, tm_value<T>> {
    using type =
      std::conditional_t<std::is_same_v<From, tm_value<T>>, To, tm_value<T>>;
  };

  template <class From, class To, class Tag>
  struct subst_term_impl<From, To, tm_var<Tag>> {
    using type =
      std::conditional_t<std::is_same_v<From, tm_var<Tag>>, To, tm_var<Tag>>;
  };

  template <class From, class To, class Tag>
  struct subst_term_impl<From, To, tm_varvalue<Tag>> {
    using type = std::conditional_t<
      std::is_same_v<From, tm_varvalue<Tag>>,
      To,
      tm_varvalue<Tag>>;
  };

  template <class From, class To, class Tag>
  struct subst_term_impl<From, To, tm_fix<Tag>> {
    using type =
      std::conditional_t<std::is_same_v<From, tm_fix<Tag>>, To, tm_fix<Tag>>;
  };

  template <class From, class To, class Term>
  using subst_term_t = typename subst_term_impl<From, To, Term>::type;

  template <class T, class Gen, class Target>
  struct genpoly_impl {
    using term = Target;
    using gen = Gen;
  };

  template <class T, class Gen, class Target>
  struct genpoly_impl2 {};

  template <class Tag, class... Ts, class Gen, class Target>
  struct genpoly_impl2<std::tuple<tm_var<Tag>, Ts...>, Gen, Target> {
    using _var = tm_var<Gen>;
    using t = genpoly_impl2<
      std::tuple<Ts...>,
      nextgen_t<Gen>,
      subst_term_t<tm_var<Tag>, _var, Target>>;

    using term = typename t::term;
    using gen = typename t::gen;
  };

  template <class T, class... Ts, class Gen, class Target>
  struct genpoly_impl2<std::tuple<T, Ts...>, Gen, Target> {
    using t = genpoly_impl2<std::tuple<Ts...>, Gen, Target>;
    using term = typename t::term;
    using gen = typename t::gen;
  };

  template <class Tag, class Gen, class Target>
  struct genpoly_impl2<std::tuple<tm_var<Tag>>, Gen, Target> {
    using _var = tm_var<Gen>;
    using term = subst_term_t<tm_var<Tag>, _var, Target>;
    using gen = nextgen_t<Gen>;
  };

  template <class T, class Gen, class Target>
  struct genpoly_impl2<std::tuple<T>, Gen, Target> {
    using term = typename genpoly_impl<T, Gen, Target>::term;
    using gen = typename genpoly_impl<T, Gen, Target>::gen;
  };

  template <class T1, class T2, class Gen, class Target>
  struct genpoly_impl<tm_apply<T1, T2>, Gen, Target> {
    using t1 = genpoly_impl<T1, Gen, Target>;
    using t2 = genpoly_impl<T2, typename t1::gen, typename t1::term>;
    using term = typename t2::term;
    using gen = typename t2::gen;
  };

  template <class... Ts, class Gen, class Target>
  struct genpoly_impl<tm_closure<Ts...>, Gen, Target> {
    using t = genpoly_impl2<std::tuple<Ts...>, Gen, Target>;
    using term = typename t::term;
    using gen = typename t::gen;
  };

  template <class Term, class Gen>
  using genpoly_term = typename genpoly_impl<Term, Gen, Term>::term;

  template <class Term, class Gen>
  using genpoly_gen = typename genpoly_impl<Term, Gen, Term>::gen;

  template <class T, class Gen, bool Assert>
  struct type_of_impl;

  template <class T, class Gen, bool Assert>
  struct type_of_h {
    using type = typename type_of_impl<T, Gen, Assert>::type;
    using gen = typename type_of_impl<T, Gen, Assert>::gen;
  };

  template <class T, class... Ts, class Gen, bool Assert>
  struct type_of_h<tm_closure<T, Ts...>, Gen, Assert> {
    // t1
    using _t1 = type_of_h<T, Gen, Assert>;
    using _t1_t = typename _t1::type;
    using _t1_gen = typename _t1::gen;
    // t2
    using _t2 = type_of_h<tm_closure<Ts...>, _t1_gen, Assert>;
    using _t2_t = typename _t2::type;
    using _t2_gen = typename _t2::gen;
    // here we go...
    using type = arrow<_t1_t, _t2_t>;
    using gen = _t2_gen;
  };
  template <class T, class Gen, bool Assert>
  struct type_of_h<tm_closure<T>, Gen, Assert> {
    // unwrap
    using type = typename type_of_h<T, Gen, Assert>::type;
    using gen = typename type_of_h<T, Gen, Assert>::gen;
  };

  template <class Tag, class Gen, bool Assert>
  struct type_of_impl<tm_value<Tag>, Gen, Assert> {
    using type = typename tm_value<Tag>::type;
    using gen = Gen;
  };
  template <class Tag, class Gen, bool Assert>
  struct type_of_impl<tm_var<Tag>, Gen, Assert> {
    using type = typename tm_var<Tag>::type;
    using gen = Gen;
  };
  template <class Tag, class Gen, bool Assert>
  struct type_of_impl<tm_varvalue<Tag>, Gen, Assert> {
    using type = typename tm_varvalue<Tag>::type;
    using gen = Gen;
  };
  template <class... Ts, class Gen, bool Assert>
  struct type_of_impl<tm_closure<Ts...>, Gen, Assert> {
    using rcn = type_of_h<
      genpoly_term<tm_closure<Ts...>, Gen>,
      genpoly_gen<tm_closure<Ts...>, Gen>,
      Assert>;
    using type = typename rcn::type;
    using gen = typename rcn::gen;
  };

  template <class S, class Gen, bool Error = is_error_type_v<S>>
  struct type_of_impl_applyfix_impl {
    using _var = genvar_t<Gen>;
    using type = subst_all_t<S, _var>;
    using gen = nextgen_t<Gen>;
  };

  template <class S, class Gen>
  struct type_of_impl_applyfix_impl<S, Gen, true> {
    using type = S;
    using gen = void;
  };

  // When Assert==true Error is always false
  template <
    class T1,
    bool Assert,
    bool Error = is_error_type_v<typename T1::type>>
  struct type_of_impl_applyfix {
    using _t1_type = typename T1::type;
    using _t1_gen = typename T1::gen;
    using _var = genvar_t<_t1_gen>;
    using _c = std::tuple<constr<_t1_type, arrow<_var, _var>>>;
    using _s = unify_t<_c, Assert>;
    using _impl = type_of_impl_applyfix_impl<_s, _t1_gen>;
    using type = typename _impl::type;
    using gen = typename _impl::gen;
  };

  // Assert==false && type check failed
  template <class T1, bool Assert>
  struct type_of_impl_applyfix<T1, Assert, true> {
    using type = typename T1::type;
    using gen = void;
  };

  template <class Tag, class T, class Gen, bool Assert>
  struct type_of_impl<tm_apply<tm_fix<Tag>, T>, Gen, Assert> {
    using _t1 = type_of_impl<T, Gen, Assert>;
    using _impl = type_of_impl_applyfix<_t1, Assert>;
    using type = typename _impl::type;
    using gen = typename _impl::gen;
  };

  template <class S, class Var, class Gen, bool Error = is_error_type_v<S>>
  struct type_of_impl_apply_impl {
    using type = subst_all_t<S, Var>;
    using gen = nextgen_t<Gen>;
  };

  template <class S, class Var, class Gen>
  struct type_of_impl_apply_impl<S, Var, Gen, true> {
    using type = S;
    using gen = void;
  };

  template <
    class T2,
    class T1T,
    class Gen,
    bool Assert,
    bool Error = is_error_type_v<typename T2::type>>
  struct type_of_impl_apply_t2 {
    using _t2_type = typename T2::type;
    using _t2_gen = typename T2::gen;
    // create new type variable
    using _var = genvar_t<_t2_gen>;
    // constraint
    using _c = std::tuple<constr<T1T, arrow<_t2_type, _var>>>;
    // solve constraint
    using _s = unify_t<_c, Assert>;
    // apply result

    using _impl = type_of_impl_apply_impl<_s, _var, _t2_gen>;
    using type = typename _impl::type;
    using gen = typename _impl::gen;
  };

  template <class T2, class T1T, class Gen, bool Assert>
  struct type_of_impl_apply_t2<T2, T1T, Gen, Assert, true> {
    using type = typename T2::type;
    using gen = Gen;
  };

  template <
    class T1,
    class T2,
    bool Assert,
    bool Error = is_error_type_v<typename T1::type>>
  struct type_of_impl_apply_t1 {
    using _t1_type = typename T1::type;
    using _t1_gen = typename T1::gen;
    // get type of T2
    using _t2 = type_of_impl<T2, _t1_gen, Assert>;

    using _impl = type_of_impl_apply_t2<_t2, _t1_type, _t1_gen, Assert>;
    using type = typename _impl::type;
    using gen = typename _impl::gen;
  };

  template <class T1, class T2, bool Assert>
  struct type_of_impl_apply_t1<T1, T2, Assert, true> {
    using type = typename T1::type;
    using gen = void;
  };

  template <class T1, class T2, class Gen, bool Assert>
  struct type_of_impl<tm_apply<T1, T2>, Gen, Assert> {
    // get type of T1
    using _t1 = type_of_impl<T1, Gen, Assert>;
    using _impl = type_of_impl_apply_t1<_t1, T2, Assert>;
    using type = typename _impl::type;
    using gen = typename _impl::gen;
  };

  /// Infer type of term
  template <class Term, bool Assert = true>
  using type_of_t = typename type_of_impl<Term, taggen<0>, Assert>::type;

  // ------------------------------------------
  // Util
  // ------------------------------------------
  template <class T>
  struct is_tm_apply : std::false_type {};
  template <class T1, class T2>
  struct is_tm_apply<tm_apply<T1, T2>> : std::true_type {};

  template <class T>
  struct is_tm_value : std::false_type {};
  template <class Tag>
  struct is_tm_value<tm_value<Tag>> : std::true_type {};

  template <class T>
  struct is_tm_closure : std::false_type {};
  template <class... Ts>
  struct is_tm_closure<tm_closure<Ts...>> : std::true_type {};

  template <class T>
  struct is_tm_var : std::false_type {};
  template <class Tag>
  struct is_tm_var<tm_var<Tag>> : std::true_type {};

  template <class T>
  struct is_tm_varvalue : std::false_type {};
  template <class Tag>
  struct is_tm_varvalue<tm_varvalue<Tag>> : std::true_type {};

  template <class T>
  struct is_tm_fix : std::false_type {};
  template <class Tag>
  struct is_tm_fix<tm_fix<Tag>> : std::true_type {};

  /// is_apply_v
  template <class T>
  static constexpr bool is_tm_apply_v = is_tm_apply<T>::value;
  /// has_apply_v
  template <class T>
  static constexpr bool has_tm_apply_v = is_tm_apply_v<typename T::term>;
  /// is_value_v
  template <class T>
  static constexpr bool is_tm_value_v = is_tm_value<T>::value;
  /// has_value_v
  template <class T>
  static constexpr bool has_tm_value_v = is_tm_value_v<typename T::term>;
  /// is_closure_v
  template <class T>
  static constexpr bool is_tm_closure_v = is_tm_closure<T>::value;
  /// has_closure_v
  template <class T>
  static constexpr bool has_tm_closure_v = is_tm_closure_v<typename T::term>;
  /// is_var_v
  template <class T>
  static constexpr bool is_tm_var_v = is_tm_var<T>::value;
  /// has_var_v
  template <class T>
  static constexpr bool has_tm_var_v = is_tm_var_v<typename T::term>;
  /// is_varvalue_v
  template <class T>
  static constexpr bool is_tm_varvalue_v = is_tm_varvalue<T>::value;
  /// has_varvalue_v
  template <class T>
  static constexpr bool has_tm_varvalue_v = is_tm_varvalue_v<typename T::term>;
  // is_tm_fix
  template <class T>
  static constexpr bool is_tm_fix_v = is_tm_fix<T>::value;
  // has_tm_fix
  template <class T>
  static constexpr bool has_tm_fix_v = is_tm_fix_v<typename T::term>;

  template <class T>
  struct is_value_type_impl : std::false_type {};

  template <class Tag>
  struct is_value_type_impl<value<Tag>> : std::true_type {};
  /// is_value_v
  template <class T>
  static constexpr bool is_value_type_v = is_value_type_impl<T>::value;

  template <class T>
  struct is_arrow_type_impl : std::false_type {};

  template <class T1, class T2>
  struct is_arrow_type_impl<arrow<T1, T2>> : std::true_type {};
  /// is_arrow_type_v
  template <class T>
  static constexpr bool is_arrow_type_v = is_arrow_type_impl<T>::value;

  template <class T>
  struct is_vartype_impl : std::false_type {};

  template <class Tag>
  struct is_vartype_impl<var<Tag>> : std::true_type {};
  /// is_vartype_v
  template <class T>
  static constexpr bool is_vartype_v = is_vartype_impl<T>::value;

  template <class T>
  struct is_varvalue_type_impl : std::false_type {};

  template <class Tag>
  struct is_varvalue_type_impl<varvalue<Tag>> : std::true_type {};
  /// is_varvalue_v
  template <class T>
  static constexpr bool is_varvalue_type_v = is_varvalue_type_impl<T>::value;

  template <class T>
  struct tag_of_impl {};
  template <class Tag>
  struct tag_of_impl<tm_value<Tag>> {
    using type = Tag;
  };
  template <class Tag>
  struct tag_of_impl<tm_var<Tag>> {
    using type = Tag;
  };
  template <class Tag>
  struct tag_of_impl<tm_varvalue<Tag>> {
    using type = Tag;
  };
  template <class Tag>
  struct tag_of_impl<tm_fix<Tag>> {
    using type = Tag;
  };
  template <class T>
  using tag_of_t = typename tag_of_impl<T>::type;

} // namespace TORI_NS::detail