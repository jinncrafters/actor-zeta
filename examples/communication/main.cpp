#include <cassert>

#include <map>
#include <unordered_set>
#include <vector>

#include <actor-zeta.hpp>

using actor_zeta::abstract_executor;
using actor_zeta::basic_async_actor;
using actor_zeta::supervisor_abstract;

using actor_zeta::link;
using actor_zeta::message_ptr;

class dummy_executor final : public abstract_executor {
public:
    dummy_executor()
        : abstract_executor(1, 10000) {}

    void execute(actor_zeta::executable* ptr) override {
        ptr->run(nullptr, max_throughput());
    }
    void start() override {}
    void stop() override {}
};

class supervisor_lite final : public supervisor_abstract {
public:
    explicit supervisor_lite(dummy_executor* ptr)
        : supervisor_abstract("network")
        , e_(ptr)
        , cursor(0)
        , system_{"sync_contacts", "add_link", "remove_link"} {
    }

    ~supervisor_lite() override = default;

    auto executor_impl() noexcept -> actor_zeta::abstract_executor* final {
        return e_;
    }

    auto add_actor_impl(actor_zeta::actor) -> void override {}
    auto add_supervisor_impl(actor_zeta::supervisor) -> void override {}

    /*

    auto join(actor_zeta::actor t) -> actor_zeta::actor_address final {
        auto tmp = std::move(t);
        auto address = tmp->address();
        actors_.push_back(std::move(tmp));
        return address;
    }
     */

    void enqueue_base(message_ptr msg, actor_zeta::execution_device*) final {
        auto msg_ = std::move(msg);
        auto it = system_.find(msg_->command());
        if (it != system_.end()) {
            local(std::move(msg_));
        } else {
            redirect_robin(std::move(msg_));
        }
    }

private:
    auto local(message_ptr msg) -> void {
        set_current_message(std::move(msg));
        execute();
    }

    auto redirect_robin(message_ptr msg) -> void {
        if (!actors_.empty()) {
            actors_[cursor]->enqueue(std::move(msg));
            ++cursor;
            if (cursor >= actors_.size()) {
                cursor = 0;
            }
        }
    }

    abstract_executor* e_;
    std::vector<actor_zeta::actor> actors_;
    std::size_t cursor;
    std::unordered_set<actor_zeta::detail::string_view> system_;
};

class storage_t final : public basic_async_actor {
public:
    explicit storage_t(actor_zeta::supervisor_abstract* ptr)
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
    }

    ~storage_t() override = default;
};

class network_t final : public basic_async_actor {
public:
    ~network_t() override = default;

    explicit network_t(actor_zeta::supervisor_abstract* ptr)
        : basic_async_actor(ptr, "network") {}
};

int main() {
    actor_zeta::supervisor supervisor_tmp(new supervisor_lite(new dummy_executor));

    actor_zeta::spawn_actor<storage_t>(supervisor_tmp);

    actor_zeta::spawn_actor<network_t>(supervisor_tmp);

    actor_zeta::supervisor supervisor1(new supervisor_lite(new dummy_executor));

    actor_zeta::link(supervisor_tmp, supervisor1);

    actor_zeta::spawn_actor<storage_t>(supervisor1);

    actor_zeta::spawn_actor<network_t>(supervisor1);

    return 0;
}

/*
  int main() {
    std::unique_ptr<supervisor_lite> supervisor(new supervisor_lite(new dummy_executor));

    auto storage = join<storage_t>(supervisor.get());

    auto network = join<network_t>(supervisor.get());

    actor_zeta::link(storage, network);

    std::unique_ptr<supervisor_lite> supervisor1(new supervisor_lite(new dummy_executor));

    actor_zeta::link(*supervisor, *supervisor1);

    auto storage1 = join<storage_t>(supervisor1.get());

    auto network1 = join<network_t>(supervisor1.get());

    actor_zeta::link(storage1, network1);

    return 0;
}
*/
