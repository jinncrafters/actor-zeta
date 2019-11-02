#include <actor-zeta/actor/context.hpp>
#include <actor-zeta/actor/actor_address.hpp>
#include <actor-zeta/messaging/message_header.hpp>
#include <actor-zeta/messaging/message.hpp>

namespace actor_zeta { namespace actor {

      actor_address::actor_address(message_passing_interface *aa) : ptr_(aa) {}

      actor_address::~actor_address() {}

        message_passing_interface *actor_address::operator->() const noexcept {
            return ptr_.get();
        }

        actor_address::operator bool() const noexcept {
            return static_cast<bool>(ptr_);
        }

        bool actor_address::operator!() const noexcept {
            return !ptr_;
        }

}}
