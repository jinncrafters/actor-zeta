#include <iostream>
#include <vector>

// clang-format off
#include <actor-zeta/base/communication_module.hpp>
#include <actor-zeta/base/handler.hpp>
#include <actor-zeta/base/address.hpp>
#include <actor-zeta/base/message.hpp>
#include <actor-zeta/impl/handler.ipp>
// clang-format on

namespace actor_zeta { namespace base {

    void error_sync_contacts(detail::string_view name, detail::string_view error) {
        std::cerr << "WARNING" << '\n';
        std::cerr << "Actor name : " << name << '\n';
        std::cerr << "Not initialization address type:" << error << '\n';
        std::cerr << "WARNING" << std::endl;
    }

    void error_duplicate_handler(detail::string_view error) {
        std::cerr << "Duplicate" << '\n';
        std::cerr << "Handler: " << error << '\n';
        std::cerr << "Duplicate" << std::endl;
    }

    void error_add_handler(detail::string_view error) {
        std::cerr << "error add handler" << '\n';
        std::cerr << "Handler: " << error << '\n';
        std::cerr << "error add handler" << std::endl;
    }

    void error_skip(detail::string_view sender, detail::string_view reciever, detail::string_view handler) {
        std::cerr << "WARNING" << '\n';
        std::cerr << "Skip, can't find handler: " << reciever << "::" << handler;
        std::cerr << " sender: " << sender << "\n";
        std::cerr << "WARNING" << std::endl;
    }

    void communication_module::execute() {
        auto it = handlers_.find(current_message()->command());
        if (it != handlers_.end()) {
            return it->second->invoke(*this);
        } else {
            auto sender = current_message()->sender().type();
            auto reciever = this->type();
            error_skip(sender, reciever, current_message()->command());
        }
    }

    bool communication_module::on(detail::string_view name, handler* aa) {
        auto it = handlers_.find(name);
        bool status = false;
        if (it == handlers_.end()) {
            auto it1 = handlers_.emplace(name, std::unique_ptr<handler>(aa));
            status = it1.second;
            if (status == false) {
                error_add_handler(name);
            }
        } else {
            error_duplicate_handler(name);
        }

        return status;
    }

    void communication_module::enqueue(message_ptr msg) {
        enqueue(std::move(msg), nullptr);
    }

    auto communication_module::message_types() const -> std::set<std::string> {
        std::set<std::string> types;

        for (const auto& i : handlers_) {
            types.emplace(std::string(i.first.begin(), i.first.end()));
        }

        return types;
    }

    auto communication_module::all_view_address() const -> std::set<std::string> {
        std::set<std::string> tmp;

        for (const auto& i : contacts_) {
            tmp.emplace(std::string(i.first.begin(), i.first.end()));
        }

        return tmp;
    }

    auto communication_module::address_book(detail::string_view type) -> address_t {
        auto result = contacts_.find(type);
        if (result != contacts_.end()) {
            return *(result->second.begin());
        }
    }

    auto communication_module::type() const -> detail::string_view {
        return detail::string_view(type_.data(), type_.size());
    }

    communication_module::~communication_module() {}

    communication_module::communication_module(std::string type)
        : contacts_()
        , type_(std::move(type)) {
        add_handler(
            "add_link",
            &communication_module::add_link);

        add_handler(
            "remove_link",
            &communication_module::remove_link);
    }

    void communication_module::add_link(address_t& address) {
        if (address) {
            auto name = address.type();
            auto it = contacts_.find(name);
            if (it == contacts_.end()) {
                auto result = contacts_.emplace(name, storage_contact_t());
                result.first->second.emplace_back(std::move(address));
            } else {
                it->second.emplace_back(std::move(address));
            }
        } else {
            error_sync_contacts(type(), address.type());
        }
    }

    void communication_module::remove_link(const address_t& address) {
        auto name = address.type();
        auto it = contacts_.find(name);
        if (it == contacts_.end()) {
            // not find
        } else {
            auto end = it->second.end();
            for (auto i = it->second.begin(); i != end; ++i) {
                if (address.get() == i->get()) {
                    it->second.erase(i);
                }
            }
        }
    }

    auto communication_module::broadcast(message_ptr msg) -> void {
        auto tmp = std::move(msg);

        for (auto& i : contacts_) {
            for (auto& j : i.second) {
                j.enqueue(message_ptr(tmp->clone()));
            }
        }
    }

    auto communication_module::broadcast(detail::string_view type, message_ptr msg) -> void {
        auto tmp = std::move(msg);

        auto range = contacts_.find(type);
        for (auto& i : range->second) {
            i.enqueue(message_ptr(tmp->clone()));
        }
    }

    void communication_module::enqueue(message_ptr msg, executor::execution_device* e) {
        enqueue_base(std::move(msg), e);
    }

}} // namespace actor_zeta::base
