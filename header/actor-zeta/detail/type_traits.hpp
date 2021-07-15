#pragma once

#include <actor-zeta/config.hpp>
#include <type_traits>
#include <utility>

namespace actor_zeta { namespace type_traits {

#if CPP17_OR_GREATER

    using std::decay_t;
    using std::enable_if_t;
    using std::index_sequence;
    using std::make_index_sequence;
    using std::remove_reference_t;

#elif CPP14_OR_GREATER

    using std::aligned_storage_t;
    using std::decay_t;
    using std::enable_if_t;
    using std::index_sequence;
    using std::make_index_sequence;
    using std::remove_reference_t;

#elif CPP11_OR_GREATER

    template<class T>
    using decay_t = typename std::decay<T>::type;

    template<bool B, class T, class F>
    using conditional_t = typename std::conditional<B, T, F>::type;

    template<bool V, class T = void>
    using enable_if_t = typename std::enable_if<V, T>::type;

    template<std::size_t Len>
    struct aligned_storage_msa final {
        union type {
            unsigned char data_[Len];
            struct __attribute__((__aligned__)) {
            } align_;
        };
    };

    template<size_t Len, size_t Align = __alignof__(typename aligned_storage_msa<Len>::type)>
    using aligned_storage_t = typename std::aligned_storage<Len, Align>::type;

    template<class T>
    using remove_reference_t = typename std::remove_reference<T>::type;

    template<std::size_t...>
    struct index_sequence {};

    template<std::size_t N, std::size_t... Next>
    struct index_sequence_helper : public index_sequence_helper<N - 1U, N - 1U, Next...> {};

    template<std::size_t... Next>
    struct index_sequence_helper<0U, Next...> {
        using type = index_sequence<Next...>;
    };

    template<std::size_t N>
    using make_index_sequence = typename index_sequence_helper<N>::type;

    template<typename _Tp>
    using add_rvalue_reference_t = typename std::add_rvalue_reference<_Tp>::type;

    template<typename _Tp>
    using add_rvalue_reference_t = typename std::add_rvalue_reference<_Tp>::type;

#endif

#if CPP17_OR_GREATER

#elif CPP14_OR_GREATER or CPP11_OR_GREATER

    template<bool v>
    using bool_constant = std::integral_constant<bool, v>;

    namespace detail {
        struct in_place_tag final {};
        template<class>
        struct in_place_type_tag final {};
        template<size_t>
        struct in_place_index_tag final {};
    } // namespace detail

    struct in_place_tag final {
        in_place_tag() = delete;

    private:
        explicit in_place_tag(detail::in_place_tag) {}
        friend inline in_place_tag internal_construct_in_place_tag();
    };

    inline in_place_tag internal_construct_in_place_tag() { return in_place_tag(detail::in_place_tag{}); }

    using in_place_t = in_place_tag (&)(detail::in_place_tag);

    template<class T>
    using in_place_type_t = in_place_tag (&)(detail::in_place_type_tag<T>);

    template<size_t N>
    using in_place_index_t = in_place_tag (&)(detail::in_place_index_tag<N>);

    inline in_place_tag in_place(detail::in_place_tag) { return internal_construct_in_place_tag(); }

    template<class T>
    inline in_place_tag in_place(detail::in_place_type_tag<T>) { return internal_construct_in_place_tag(); }

    template<std::size_t I>
    inline in_place_tag in_place(detail::in_place_index_tag<I>) { return internal_construct_in_place_tag(); }

    template<typename _Tp>
    using remove_cvref_t = typename std::remove_cv<typename std::remove_reference<_Tp>::type>::type;

#endif

    template<typename...>
    struct _or_;

    template<>
    struct _or_<>
        : public std::false_type {};

    template<typename _B1>
    struct _or_<_B1>
        : public _B1 {};

    template<typename _B1, typename _B2>
    struct _or_<_B1, _B2>
        : public std::conditional<_B1::value, _B1, _B2>::type {};

    template<typename _B1, typename _B2, typename _B3, typename... _Bn>
    struct _or_<_B1, _B2, _B3, _Bn...>
        : public std::conditional<_B1::value, _B1, _or_<_B2, _B3, _Bn...>>::type {};

    template<typename...>
    using void_t = void;

    struct erased_type {};

    struct allocator_arg_t {
        explicit allocator_arg_t() = default;
    };

    constexpr allocator_arg_t allocator_arg = allocator_arg_t();

}} // namespace actor_zeta::type_traits
