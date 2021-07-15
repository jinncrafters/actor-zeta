#pragma once

#include <actor-zeta/base/communication_module.hpp>
#include <actor-zeta/detail/pmr/memory_resource.hpp>
#include <actor-zeta/forwards.hpp>

namespace actor_zeta { namespace base {

    using default_spawn_actor = std::function<actor(supervisor_abstract*)>;
    using default_spawn_supervisor = std::function<supervisor(supervisor_abstract*)>;

    class supervisor_abstract : public communication_module {
    public:
        supervisor_abstract(std::string, detail::pmr::memory_resource*);
        supervisor_abstract(std::string);
        supervisor_abstract(supervisor_abstract*, std::string);
        ~supervisor_abstract() override;
        auto executor() noexcept -> executor::abstract_executor*;
        using communication_module::broadcast;
        auto resource() const -> detail::pmr::memory_resource*;
        auto address() noexcept -> address_t;

    protected:
        using communication_module::add_handler;
        virtual auto executor_impl() noexcept -> executor::abstract_executor* = 0;
        virtual auto add_actor_impl(actor) -> void = 0;
        virtual auto add_supervisor_impl(supervisor) -> void = 0;

        auto set_current_message(message_ptr) -> void;
        auto current_message() -> message* final;

    private:
        auto redirect(std::string& type, message* msg) -> void;
        auto spawn_actor(default_spawn_actor&) -> void;
        auto spawn_supervisor(default_spawn_supervisor&) -> void;
        message* current_message_;
        detail::pmr::memory_resource* memory_resource_;
    };

}} // namespace actor_zeta::base
