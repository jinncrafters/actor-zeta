#pragma once

#include <list>
#include <memory>
#include <set>
#include <string>
#include <unordered_map>
#include <vector>

#include <actor-zeta/base/address.hpp>
#include <actor-zeta/detail/callable_trait.hpp>
#include <actor-zeta/detail/ref_counted.hpp>
#include <actor-zeta/detail/string_view.hpp>
#include <actor-zeta/forwards.hpp>

namespace actor_zeta { namespace base {

    using message_ptr = std::unique_ptr<message>;

    class communication_module
        : public ref_counted {
    public:
        using key_type = detail::string_view;
        using storage_t = std::unordered_map<key_type, std::unique_ptr<handler>>;
        using storage_contact_t = std::list<address_t>;
        using contacts_t = std::unordered_map<key_type, storage_contact_t>;
        using contacts_iterator_t = storage_contact_t::iterator;
        using range_t = std::pair<contacts_iterator_t, contacts_iterator_t>;

        communication_module() = delete;

        communication_module(const communication_module&) = delete;

        communication_module& operator=(const communication_module&) = delete;

        ~communication_module() override;

        auto type() const -> detail::string_view;

        auto message_types() const -> std::set<std::string>;

        auto enqueue(message_ptr) -> void;

        void enqueue(message_ptr, executor::execution_device*);

        auto broadcast(message_ptr) -> void;

        auto broadcast(detail::string_view, message_ptr) -> void;

        virtual auto current_message() -> message* = 0;

    protected:
        communication_module(std::string);

        virtual void enqueue_base(message_ptr, executor::execution_device*) = 0;

        auto address_book(detail::string_view) -> address_t;

        template<class F>
        auto add_handler(detail::string_view name, F&& f) -> typename std::enable_if<!std::is_member_function_pointer<F>::value>::type {
            on(name, make_handler(std::forward<F>(f)));
        }

        template<typename F>
        auto add_handler(detail::string_view name, F&& f) -> typename std::enable_if<std::is_member_function_pointer<F>::value>::type {
            on(name, make_handler(std::forward<F>(f), static_cast<typename type_traits::get_callable_trait_t<F>::class_type*>(this)));
        }

        void execute();

        bool on(detail::string_view, handler*);

        /**
        * debug method
        */
        auto all_view_address() const -> std::set<std::string>;

    private:
        void add_link(address_t&);

        void remove_link(const address_t&);

        contacts_t contacts_;
        storage_t handlers_;
        std::string type_;
    };

}} // namespace actor_zeta::base
