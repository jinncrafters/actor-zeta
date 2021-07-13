#pragma once

#include <actor-zeta/detail/rtt_management.hpp>

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <type_traits>
#include <utility>
#include <vector>

namespace actor_zeta { namespace detail {

    inline void dummy(std::initializer_list<int>) {}

#define EXPAND_VARIADIC(expression) \
    dummy({(expression, 0)...})

    template<class...>
    struct padded_size;

    template<>
    struct padded_size<> {
        static constexpr std::size_t value = 0;
    };

    template<class Head>
    struct padded_size<Head> {
        static constexpr std::size_t value = sizeof(Head);
    };

    template<class Head, class Mid, class... Tail>
    struct padded_size<Head, Mid, Tail...> {
        static constexpr std::size_t value = (sizeof(Head) + alignof(Mid) - 1) / alignof(Mid) * alignof(Mid) + padded_size<Mid, Tail...>::value;
    };

    // run-time tuple
    class rtt final {
    private:
        using object_info_container_type = std::vector<management::object_info_t>;

    public:
        using size_type = typename object_info_container_type::size_type;

    public:
        template<typename Head, typename... Tail,
                 typename =
                     type_traits::enable_if_t<
                         sizeof...(Tail) != 0 ||
                         not std::is_same<type_traits::decay_t<Head>, rtt>::value>>
        explicit rtt(Head&& head, Tail&&... tail)
            : m_capacity(padded_size<Head, Tail...>::value)
            , m_data(std::make_unique<std::int8_t[]>(m_capacity)) {
            try {
                push_back_no_realloc(std::forward<Head>(head));
                EXPAND_VARIADIC(push_back_no_realloc(std::forward<Tail>(tail)));
            } catch (...) {
                destroy_all();
                throw;
            }
        }

        rtt()
            : m_capacity(0)
            , m_data{}
            , m_objects{}
            , m_volume(0) {
        }

        rtt(rtt&& that)
            : rtt() {
            this->swap(that);
        }

        rtt& operator=(rtt&& that) {
            clear();

            this->swap(that);
            return *this;
        }

        rtt(const rtt& that)
            : m_capacity(that.m_volume)
            , m_data(std::make_unique<std::int8_t[]>(m_capacity))
            , m_objects(that.m_objects)
            , m_volume(that.m_volume) {
            management::copy(m_objects.begin(), m_objects.end(), that.data(), this->data());
        }

        rtt& operator=(const rtt& that) {
            clear();

            reserve(that.m_volume);
            management::copy(that.m_objects.begin(), that.m_objects.end(), that.data(), this->data());

            static_assert(std::is_nothrow_move_assignable<object_info_container_type>::value, "");
            m_objects = that.m_objects;
            m_volume = that.m_volume;

            return *this;
        }

        ~rtt() {
            destroy_all();
        }

        void swap(rtt& that) {
            using std::swap;

            swap(this->m_capacity, that.m_capacity);
            swap(this->m_data, that.m_data);
            swap(this->m_objects, that.m_objects);
            swap(this->m_volume, that.m_volume);
        }


        template<typename T>
        const T& get(size_type index) const {
            assert(management::conforms<T>(m_objects[index]));
            return get_by_offset<T>(offset(index));
        }

        template<typename T>
        T& get(size_type index) {
            assert(management::conforms<T>(m_objects[index]));
            return get_by_offset<T>(offset(index));
        }

        size_type offset(size_type index) const {
            return m_objects[index].offset;
        }

        template<typename T>
        const T& get_by_offset(size_type offset) const {
            return *static_cast<const T*>(static_cast<const void*>(data() + offset));
        }

        template<typename T>
        T& get_by_offset(size_type offset) {
            return *static_cast<T*>(static_cast<void*>(data() + offset));
        }

        size_type size() const {
            return m_objects.size();
        }

        std::size_t volume() const {
            return m_volume;
        }

        std::size_t capacity() const {
            return m_capacity;
        }

        bool empty() const {
            return m_objects.empty();
        }

    private:

        void reserve(std::size_t new_capacity) {
                if (new_capacity > m_capacity) {
                    reallocate(std::max(new_capacity, m_capacity * CAPACITY_INCREASING_FACTOR));
                }
            }

        void shrink_to_fit() {
            if (m_capacity > m_volume) {
                reallocate(m_volume);
            }
        }

        void clear() {
            destroy_all();
            m_objects.clear();
            m_volume = 0;
        }

        void reallocate(std::size_t new_capacity) {
            assert(new_capacity >= m_volume);
            auto new_data = std::make_unique<std::int8_t[]>(new_capacity);
            m_capacity = new_capacity;

            management::move(m_objects.begin(), m_objects.end(), data(), new_data.get());
            std::swap(m_data, new_data);
        }

        std::int8_t* data() {
            return m_data.get();
        }

        const std::int8_t* data() const {
            return m_data.get();
        }

        template<typename T>
        std::int8_t* set_up_creation_place(const T& object) {
            if (auto proposed_creation_place = try_to_align(object)) {
                return proposed_creation_place;
            } else {
                reserve(volume() + sizeof(T) + alignof(T));
                return force_align(object);
            }
        }

        template<typename T>
        void push_back_no_realloc(T&& object) {
            auto creation_place = force_align(object);
            accomodate(std::forward<T>(object), creation_place);
        }

        template<typename T>
        std::int8_t* try_to_align(const T&) {
            auto space_left = capacity() - volume();
            void* creation_place = data() + volume();
            auto aligned_place = std::align(alignof(T), sizeof(T), creation_place, space_left);
            return static_cast<std::int8_t*>(aligned_place);
        }

        template<typename T>
        std::int8_t* force_align(const T& object) {
            auto creation_place = try_to_align(object);
            assert(creation_place != nullptr);

            return creation_place;
        }

        template<typename T>
        void accomodate(T&& object, std::int8_t* creation_place) {
            using raw_type = std::decay_t<T>;
            new (creation_place) raw_type(std::forward<T>(object));

            const auto new_offset = static_cast<std::size_t>(creation_place - data());
            m_objects.push_back(management::make_object_info<raw_type>(new_offset));
            m_volume = new_offset + sizeof(raw_type);
        }

        void destroy_all() {
            management::destroy(m_objects.begin(), m_objects.end(), data());
        }

        static const auto CAPACITY_INCREASING_FACTOR = std::size_t{2};
        static const auto CAPACITY_DECREASING_THRESHOLD = std::size_t{4};

        std::size_t m_capacity = 0;
        std::unique_ptr<std::int8_t[]> m_data;

        object_info_container_type m_objects;
        std::size_t m_volume = 0;
    };

    inline void swap(rtt& left, rtt& right) {
        left.swap(right);
    }
}} // namespace actor_zeta::detail
