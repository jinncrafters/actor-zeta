#pragma once

#include <actor-zeta/config.hpp>

#include <cassert>
#include <cstddef>

#include <new>
#include <tuple>
#include <utility>

#if CPP17_OR_GREATER
#if __has_include(<memory_resource>)
#include <memory_resource>
#elif __has_include(<experimental/memory_resource>)
#include <experimental/memory_resource>
#endif
#elif CPP14_OR_GREATER or CPP11_OR_GREATER
#include "emulate_tuple_cat_result.hpp"
#include <actor-zeta/detail/pmr/default_resource.hpp>
#include <actor-zeta/detail/pmr/memory_resource.hpp>
#include <actor-zeta/detail/pmr/uses_allocator.hpp>
#include <actor-zeta/detail/type_traits.hpp>
#endif

namespace actor_zeta { namespace detail { namespace pmr {

#if CPP17_OR_GREATER
#if __has_include(<memory_resource>)

    template<class T>
    using polymorphic_allocator = std::pmr::polymorphic_allocator<T>;

#else

    template<class T>
    using polymorphic_allocator = std::experimental::pmr::polymorphic_allocator<T>;

#endif

#elif CPP14_OR_GREATER or CPP11_OR_GREATER

    template<typename T>
    class polymorphic_allocator {
    private:
        using default_resource = actor_zeta::detail::pmr::default_resource;

    public:
        using value_type = T;
        using pointer = value_type*;

        polymorphic_allocator() noexcept
            : resource_(default_resource::get()) {
        }

        polymorphic_allocator(memory_resource* ptr)
            : resource_(ptr) { assert(ptr); }

        polymorphic_allocator(const polymorphic_allocator& other) = default;

        template<typename _Up>
        polymorphic_allocator(const polymorphic_allocator<_Up>& other) noexcept
            : resource_(other.resource()) {}

        polymorphic_allocator&
        operator=(const polymorphic_allocator& other) = default;

        pointer allocate(size_t size) {
            return static_cast<pointer>(resource_->allocate(size * sizeof(value_type), alignof(value_type)));
        }

        void
        deallocate(pointer ptr, size_t size) { resource_->deallocate(ptr, size * sizeof(value_type), alignof(value_type)); }

        template<typename T1, typename... Args>
        void
        construct(T1* ptr, Args&&... args) {
            uses_allocator_construct(this->resource(), ptr, std::forward<Args>(args)...);
        }

        template<
            typename T1,
            typename T2,
            typename... Args1,
            typename... Args2>
        void
        construct(std::pair<T1, T2>* p, std::piecewise_construct_t, std::tuple<Args1...> x, std::tuple<Args2...> y) {
            memory_resource* const resource = this->resource();
            auto x_use_tag = use_alloc<T1, memory_resource*, Args1...>(resource);
            auto y_use_tag = use_alloc<T2, memory_resource*, Args2...>(resource);
            ///std::__use_alloc

            ::new (p) std::pair<T1, T2>(
                std::piecewise_construct,
                construct_(x_use_tag, x),
                construct_(y_use_tag, y));
        }

        template<typename T1, typename T2>
        void
        construct(std::pair<T1, T2>* ptr) {
            this->construct(
                ptr,
                std::piecewise_construct,
                std::tuple<>(),
                std::tuple<>());
        }

        template<
            typename T1,
            typename T2,
            typename U,
            typename V>
        void
        construct(std::pair<T1, T2>* ptr, U&& x, V&& y) {
            this->construct(
                ptr,
                std::piecewise_construct,
                std::forward_as_tuple(std::forward<U>(x)),
                std::forward_as_tuple(std::forward<V>(y)));
        }

        template<
            typename T1,
            typename T2,
            typename U,
            typename V>
        void
        construct(std::pair<T1, T2>* ptr, const std::pair<U, V>& pr) {
            this->construct(
                ptr,
                std::piecewise_construct,
                std::forward_as_tuple(pr.first),
                std::forward_as_tuple(pr.second));
        }

        template<
            typename T1,
            typename T2,
            typename U,
            typename V>
        void
        construct(std::pair<T1, T2>* ptr, std::pair<U, V>&& pr) {
            this->construct(
                ptr,
                std::piecewise_construct,
                std::forward_as_tuple(std::forward<U>(pr.first)),
                std::forward_as_tuple(std::forward<V>(pr.second)));
        }

        template<typename U>
        void destroy(U* ptr) {
            ptr->~U();
        }

        polymorphic_allocator select_on_container_copy_construction() const { return polymorphic_allocator(); }

        memory_resource* resource() const { return resource_; }

    private:
        using uses_alloc1_ = uses_alloc_1<memory_resource*>;
        using uses_alloc2_ = uses_alloc_2<memory_resource*>;

        template<typename Tuple>
        Tuple&&
        construct_(uses_alloc_0, Tuple& t) {
            return std::move(t);
        }

        template<typename... Args>
        constexpr auto
        construct_(uses_alloc1_ ua, std::tuple<Args...>& t) -> typename tuple_cat_result<Args...>::type {
            return std::tuple_cat(std::make_tuple(type_traits::allocator_arg, *(ua.a_)), std::move(t));
        }

        template<typename... Args>
        constexpr auto
        construct_(uses_alloc2_ ua, std::tuple<Args...>& t) -> typename tuple_cat_result<Args...>::type {
            return std::tuple_cat(std::move(t), std::make_tuple(*(ua.a_)));
        }

        memory_resource* resource_;
    };

    template<class T1, class T2>
    bool
    operator==(const polymorphic_allocator<T1>& a, const polymorphic_allocator<T2>& b) noexcept {
        return *a.resource() == *b.resource();
    }

    template<class T1, class T2>
    bool
    operator!=(const polymorphic_allocator<T1>& a, const polymorphic_allocator<T2>& b) noexcept {
        return !(a == b);
    }

#endif
}
}
} // namespace actor_zeta::detail::pmr
