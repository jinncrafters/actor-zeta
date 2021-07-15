// clang-format off
#include <actor-zeta/base/address.hpp>
#include <actor-zeta/base/message.hpp>
#include <actor-zeta/base/supervisor.hpp>
// clang-format on

#include <actor-zeta/base/supervisor_abstract.hpp>

namespace actor_zeta { namespace base {

    supervisor::~supervisor() {}

    address_t supervisor::address() const noexcept {
        return address_t(ptr_.get());
    }

    detail::string_view supervisor::type() const {
        return ptr_->type();
    }

    void supervisor::swap(supervisor& other) noexcept {
        using std::swap;
        ptr_.swap(other.ptr_);
    }

    supervisor::supervisor(std::nullptr_t)
        : ptr_(nullptr) {}

    supervisor& supervisor::operator=(std::nullptr_t) {
        ptr_.reset();
        return *this;
    }

}} // namespace actor_zeta::base
