#pragma once

#include <actor-zeta/detail/intrusive_ptr.hpp>
#include <actor-zeta/detail/string_view.hpp>
#include <actor-zeta/forwards.hpp>
#include <type_traits>

namespace actor_zeta { namespace base {

    class supervisor final {
    public:
        supervisor() = default;

        supervisor(const supervisor& a) = delete;

        supervisor(supervisor&& a) = default;

        supervisor& operator=(const supervisor& a) = delete;

        supervisor& operator=(supervisor&& a) = default;

        supervisor(std::nullptr_t);

        supervisor& operator=(std::nullptr_t);

        template<
            class T,
            class = type_traits::enable_if_t<std::is_base_of<supervisor_abstract, T>::value>>
        supervisor(intrusive_ptr<T> ptr)
            : ptr_(std::move(ptr)) {}

        template<
            class T,
            class = type_traits::enable_if_t<std::is_base_of<supervisor_abstract, T>::value>>
        supervisor(T* ptr)
            : ptr_(ptr) {}

        template<
            class T,
            class = type_traits::enable_if_t<std::is_base_of<supervisor_abstract, T>::value>>
        supervisor& operator=(intrusive_ptr<T> ptr) {
            supervisor tmp{std::move(ptr)};
            swap(tmp);
            return *this;
        }

        template<
            class T,
            class = type_traits::enable_if_t<std::is_base_of<supervisor_abstract, T>::value>>
        supervisor& operator=(T* ptr) {
            supervisor tmp{ptr};
            swap(tmp);
            return *this;
        }

        address_t address() const noexcept;

        ~supervisor();

        inline supervisor_abstract* operator->() const noexcept {
            return ptr_.get();
        }

        inline supervisor_abstract* get() const noexcept {
            return ptr_.get();
        }

        inline explicit operator bool() const noexcept {
            return static_cast<bool>(ptr_);
        }

        auto type() const -> detail::string_view;

        inline bool operator!() const noexcept {
            return !ptr_;
        }

    private:
        void swap(supervisor&) noexcept;

        intrusive_ptr<supervisor_abstract> ptr_;
    };

    static_assert(std::is_move_constructible<supervisor>::value, "");
    static_assert(not std::is_copy_constructible<supervisor>::value, "");

}
} // namespace actor_zeta::base
