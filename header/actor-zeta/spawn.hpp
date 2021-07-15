#pragma once

#include "actor-zeta/forwards.hpp"
#include "send.hpp"

#include <actor-zeta/base/supervisor.hpp>
#include <actor-zeta/base/supervisor_abstract.hpp>

namespace actor_zeta {

    using base::actor;
    using base::actor_abstract;

    namespace detail {

        template<
            class Actor,
            class Tuple, std::size_t... I,
            class = type_traits::enable_if_t<std::is_base_of<actor_abstract, Actor>::value>>
        Actor* created_actor(actor_zeta::supervisor_abstract* supervisor, Tuple&& args, type_traits::index_sequence<I...>) {
            auto allocate_byte = sizeof(Actor);
            auto allocate_byte_alignof = alignof(Actor);
            void* buffer = supervisor->resource()->allocate(allocate_byte, allocate_byte_alignof);
            return new (buffer) Actor(supervisor, std::get<I>(args)...);
        }
        /*
        template<
            class ChildrenSupervisor,
            class Tuple, std::size_t... I,
            class = type_traits::enable_if_t<std::is_base_of<supervisor_abstract, ChildrenSupervisor>::value>>
        ChildrenSupervisor* created_supervisor(actor_zeta::supervisor_abstract* supervisor, Tuple&& args, type_traits::index_sequence<I...>) {
            auto allocate_byte = sizeof(ChildrenSupervisor);
            auto allocate_byte_alignof = alignof(ChildrenSupervisor);
            void* buffer = supervisor->resource()->allocate(allocate_byte, allocate_byte_alignof);
            return new (buffer) ChildrenSupervisor(supervisor, std::get<I>(args)...);
        }
*/
    } // namespace detail

    template<
        class Actor,
        class... Args,
        class = type_traits::enable_if_t<std::is_base_of<actor_abstract, Actor>::value>>
    auto spawn_actor(base::supervisor& supervisor, Args&&... args) -> void {
        using args_types = type_traits::type_list<Args...>;
        static constexpr size_t number_of_arguments = type_traits::type_list_size<args_types>::value;
        send(
            supervisor->address(),
            supervisor->address(),
            "spawn_actor",
            std::forward<base::default_spawn_actor>(
                base::default_spawn_actor(
                    [&, args_ = std::move(std::tuple<Args&&...>(std::forward<Args&&>(args)...))](actor_zeta::supervisor_abstract* ptr) {
                        return detail::created_actor<Actor>(ptr, args_, type_traits::make_index_sequence<number_of_arguments>{});
                    })));
    }
    template<
        class Actor,
        class... Args,
        class = type_traits::enable_if_t<std::is_base_of<actor_abstract, Actor>::value>>
    auto spawn_actor(base::supervisor_abstract* supervisor, Args&&... args) -> void {
        using args_types = type_traits::type_list<Args...>;
        static constexpr size_t number_of_arguments = type_traits::type_list_size<args_types>::value;
        send(
            supervisor->address(),
            supervisor->address(),
            "spawn_actor",
            std::forward<base::default_spawn_actor>(
                base::default_spawn_actor(
                    [&, args_ = std::move(std::tuple<Args&&...>(std::forward<Args&&>(args)...))](actor_zeta::supervisor_abstract* ptr) {
                        return detail::created_actor<Actor>(ptr, args_, type_traits::make_index_sequence<number_of_arguments>{});
                    })));
    }
    /*
    template<
        class ChildrenSupervisor,
        class... Args,
        class = type_traits::enable_if_t<std::is_base_of<supervisor_abstract, ChildrenSupervisor>::value>>
    auto spawn_supervisor(base::supervisor& supervisor, Args&&... args) -> void {
        using args_types = type_traits::type_list<Args...>;
        static constexpr size_t number_of_arguments = type_traits::type_list_size<args_types>::value;

        send(
            supervisor->address(),
            supervisor->address(),
            "spawn_supervisor",
            std::move(
                base::default_spawn_actor(
                    [&, args_ = std::move(std::tuple<Args&&...>(std::forward<Args&&>(args)...))](actor_zeta::supervisor_abstract* ptr) {
                        return detail::created_supervisor<ChildrenSupervisor>(ptr, args_, type_traits::make_index_sequence<number_of_arguments>{});
                    })));
    }
    */

} // namespace actor_zeta
