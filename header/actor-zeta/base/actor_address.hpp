#pragma once

#include <actor-zeta/base/communication_module.hpp>
#include <actor-zeta/detail/intrusive_ptr.hpp>
#include <actor-zeta/forwards.hpp>

namespace actor_zeta { namespace base {
    ///
    /// @brief A compact location expressor
    ///
    class actor_address final {
    public:
        actor_address() = default;

        actor_address(actor_address&&) = default;

        actor_address(const actor_address&) = default;

        actor_address& operator=(actor_address&&) = default;

        actor_address& operator=(const actor_address&) = default;

        explicit actor_address(communication_module*);

        ~actor_address();

        inline communication_module* operator->() const noexcept {
            return ptr_.get();
        }

        inline operator bool() const noexcept {
            return static_cast<bool>(ptr_);
        }

        inline bool operator!() const noexcept {
            return !ptr_;
        }

    private:
        detail::intrusive_ptr<communication_module> ptr_;
    };

}} // namespace actor_zeta::base
