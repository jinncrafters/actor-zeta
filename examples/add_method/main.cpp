#include <cassert>

#include <set>
#include <string>

#include <actor-zeta.hpp>

using actor_zeta::basic_async_actor;
using actor_zeta::execution_device;
using actor_zeta::message_ptr;
using actor_zeta::supervisor_abstract;

class dummy_supervisor final : public supervisor_abstract {
public:
    dummy_supervisor()
        : supervisor_abstract("dummy_supervisor") {}

    auto executor_impl() noexcept -> actor_zeta::abstract_executor* override {
        return ptr_;
    }

    auto add_actor_impl(actor_zeta::actor) -> void override {}
    auto add_supervisor_impl(actor_zeta::supervisor) -> void override {}

    void enqueue_base(message_ptr, actor_zeta::execution_device*) override {}

private:
    actor_zeta::abstract_executor* ptr_ = nullptr;
};

class storage_t final : public basic_async_actor {
public:
    storage_t(actor_zeta::supervisor_abstract* ptr)
        : basic_async_actor(ptr, "storage") {
        add_handler(
            "update",
            []() -> void {});

        add_handler(
            "find",
            []() -> void {});

        add_handler(
            "remove",
            []() -> void {});

        assert(actor_zeta::detail::string_view("storage") == type());

        auto tmp = message_types();

        std::set<std::string> control = {"add_link", "remove_link", "update", "remove", "find"};

        std::set<std::string> diff;

        std::set_difference(tmp.begin(), tmp.end(), control.begin(), control.end(), std::inserter(diff, diff.begin()));

        assert(diff.empty());
    }

    ~storage_t() override = default;
};

int main() {
    actor_zeta::supervisor supervisor(new dummy_supervisor());

    actor_zeta::spawn_actor<storage_t>(supervisor);

    return 0;
}
