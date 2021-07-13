#pragma once

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <stdexcept>
#include <type_traits>
#include <utility>

#include "type_traits.hpp"

namespace actor_zeta { namespace detail {

namespace management {
    template<typename T>
    auto copy(const void* source, void* destination)
        -> type_traits::enable_if_t<
            std::is_copy_constructible<T>::value> {
        new (destination) T(*static_cast<const T*>(source));
    }

    template<typename T>
    void move(void* source, void* destination) {
        new (destination) T(std::move(*static_cast<T*>(source)));
    }

    template<typename T>
    void destroy(void* object) {
        static_cast<T*>(object)->~T();
    }

    enum struct operation_t {
        copy,
        move,
        destroy,
        size,
    };

    using manager_t = void (*)(operation_t, const void*, void*);

    template<typename T>
    void manage(operation_t todo, const void* source, void* destination) {
        switch (todo) {
            case operation_t::copy: {
                copy<T>(source, destination);
                break;
            }
            case operation_t::move: {
                move<T>(const_cast<void*>(source), destination);
                break;
            }
            case operation_t::destroy: {
                assert(source == nullptr);
                destroy<T>(destination);
                break;
            }
            case operation_t::size: {
                assert(source == nullptr);
                *static_cast<std::size_t*>(destination) = sizeof(T);
                break;
            }
        }
    }

    struct object_info_t final {
        std::size_t offset;
        manager_t manage;
    };

    template<typename T>
    object_info_t make_object_info(std::size_t offset) {
        return object_info_t{offset, &manage<T>};
    }

    inline void destroy(const object_info_t& object, std::int8_t* data) {
        object.manage(operation_t::destroy, nullptr, data + object.offset);
    }

    template<typename InputIterator>
    void destroy(InputIterator first, InputIterator last, std::int8_t* data) {
        std::for_each(first, last,
                      [data](const auto& object) {
                          destroy(object, data);
                      });
    }

    template<typename ForwardIterator>
    void
    move(
        ForwardIterator first,
        ForwardIterator last,
        std::int8_t* source,
        std::int8_t* destination) {
        for (auto current = first; current != last; ++current) {
            try {
                current->manage(operation_t::move, source + current->offset, destination + current->offset);
            } catch (...) {
                destroy(first, current, destination);
                throw;
            }
        }
        destroy(first, last, source);
    }

    template<typename ForwardIterator>
    void
    copy(
        ForwardIterator first,
        ForwardIterator last,
        const std::int8_t* source,
        std::int8_t* destination) {
        for (auto current = first; current != last; ++current) {
            try {
                current->manage(operation_t::copy, source + current->offset, destination + current->offset);
            } catch (...) {
                destroy(first, current, destination);
                throw;
            }
        }
    }

    template<typename T>
    bool conforms(const object_info_t& object) {
        return &manage<T> == object.manage;
    }

    inline std::size_t size_of(const object_info_t& object) {
        std::size_t size;
        object.manage(operation_t::size, nullptr, std::addressof(size));
        return size;
    }
}
}} // namespace actor_zeta::detail::management
