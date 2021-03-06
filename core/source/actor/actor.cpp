
// clang-format off
#include <actor-zeta/base/context.hpp>
#include <actor-zeta/base/actor_address.hpp>
#include <actor-zeta/messaging/message.hpp>
#include <actor-zeta/base/actor.hpp>
#include <actor-zeta/base/abstract_actor.hpp>
// clang-format on

namespace actor_zeta { namespace base {
        actor::~actor() {}

        actor_address actor::address() const noexcept {
            return ptr_->address();
        }

        detail::string_view actor::name() const {
            return ptr_->name();
        }

        void actor::swap(actor &other) noexcept {
            using std::swap;
            ptr_.swap(other.ptr_);
        }

        actor::actor(std::nullptr_t) : ptr_(nullptr) {}

        actor &actor::operator=(std::nullptr_t) {
            ptr_.reset();
            return *this;
        }
}}
