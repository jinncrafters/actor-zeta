#pragma once

#include "send.hpp"

namespace actor_zeta {

    using base::basic_async_actor;
    using base::supervisor_abstract;

    inline void link_imp(address_t& a1, address_t& a2) {
        send(a1, a2, "add_link", a2);
        send(a2, a1, "add_link", a1);
    }

    inline void link(basic_async_actor& actor1, basic_async_actor& actor2) {
        auto a1 = actor1.address();
        auto a2 = actor2.address();
        link_imp(a1, a2);
    }

    template<class Supervisor>
    void link(Supervisor* actor1, Supervisor* actor2) {
        auto a1 = actor1->address();
        auto a2 = actor2->address();
        link_imp(a1, a2);
    }

    template<class Supervisor>
    void link(Supervisor& actor1, Supervisor& actor2) {
        auto a1 = actor1.address();
        auto a2 = actor2.address();
        link_imp(a1, a2);
    }

    template<class Supervisor>
    void link(Supervisor& actor1, address_t& actor2) {
        auto a1 = actor1.address();
        link_imp(a1, actor2);
    }

    inline void link(address_t& actor1, address_t& actor2) {
        link_imp(actor1, actor2);
    }

} // namespace actor_zeta
