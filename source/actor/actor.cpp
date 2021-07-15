// clang-format off
#include <actor-zeta/base/address.hpp>
#include <actor-zeta/base/message.hpp>
#include <actor-zeta/base/actor.hpp>
#include <actor-zeta/base/actor_abstract.hpp>
// clang-format on

namespace actor_zeta { namespace base {
    actor::~actor() {}

    address_t actor::address() const noexcept {
        return ptr_->address();
    }

    detail::string_view actor::type() const {
        return ptr_->type();
    }

    void actor::swap(actor& other) noexcept {
        using std::swap;
        ptr_.swap(other.ptr_);
    }

    actor::actor(std::nullptr_t)
        : ptr_(nullptr) {}

    actor& actor::operator=(std::nullptr_t) {
        ptr_.reset();
        return *this;
    }
}} // namespace actor_zeta::base
