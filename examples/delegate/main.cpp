#include <iostream>
#include <map>
#include <vector>
#include <set>
#include <unordered_set>

#include <actor-zeta.hpp>

using actor_zeta::abstract_executor;
using actor_zeta::basic_async_actor;
using actor_zeta::execution_device;
using actor_zeta::message_ptr;
using actor_zeta::delegate;
using actor_zeta::supervisor_abstract;
using actor_zeta::actor;

using actor_zeta::abstract_executor;
using actor_zeta::executor_t;
using actor_zeta::work_sharing;

auto thread_pool_deleter = [](abstract_executor* ptr) {
    ptr->stop();
    delete ptr;
};

/// non thread safe
class supervisor_lite final : public actor_zeta::supervisor_abstract {
public:
    explicit supervisor_lite()
        : supervisor_abstract("network")
        , e_(new executor_t<work_sharing>(
                 1,
                 100),
             thread_pool_deleter)
        , cursor(0)
        , system_{
              "sync_contacts",
              "add_link",
              "remove_link",
              "spawn_actor","delegate"} {
        e_->start();
    }

    ~supervisor_lite() override = default;

    auto executor_impl() noexcept -> actor_zeta::abstract_executor* final { return e_.get(); }

    auto add_actor_impl(actor_zeta::actor t) -> void final {
        actors_.push_back(std::move(t));
    }

    auto add_supervisor_impl(actor_zeta::supervisor t) -> void final {
        supervisor_.emplace_back(std::move(t));
    }

    auto enqueue_base(actor_zeta::message_ptr msg, actor_zeta::execution_device*) -> void final {
        auto msg_ = std::move(msg);
        auto it = system_.find(msg_->command());
        if (it != system_.end()) {
            local(std::move(msg_));
        } else {
            redirect_robin(std::move(msg_));
        }
    }

private:
    auto local(actor_zeta::message_ptr msg) -> void {
        set_current_message(std::move(msg));
        execute();
    }

    auto redirect_robin(actor_zeta::message_ptr msg) -> void {
        if (!actors_.empty()) {
            actors_[cursor]->enqueue(std::move(msg));
            ++cursor;
            if (cursor >= actors_.size()) {
                cursor = 0;
            }
        }
    }

    std::unique_ptr<abstract_executor, decltype(thread_pool_deleter)> e_;
    std::vector<actor_zeta::actor> actors_;
    std::vector<actor_zeta::supervisor> supervisor_;
    std::size_t cursor;
    std::unordered_set<actor_zeta::detail::string_view> system_;
};

class storage_t final : public basic_async_actor {
public:
    storage_t(actor_zeta::supervisor_abstract* ptr)
        : basic_async_actor(ptr, "storage") {
        add_handler(
            "update",
            &storage_t::update);

        add_handler(
            "find",
            &storage_t::find);

        add_handler(
            "remove",
            &storage_t::remote);

        add_handler(
            "status",
            [](storage_t& ctx) {
                ctx.status();
            });
    }

    void status() {
        std::cerr << "status" << std::endl;
    }

    ~storage_t() override = default;

private:
    void update(std::string& data) {
        std::cerr << "update:" << data << std::endl;
    }

    void find() {
        std::cerr << "find" << std::endl;
    }

    void remote() {
        std::cerr << "remote" << std::endl;
    }
};

int main() {
    actor_zeta::supervisor dummy_supervisor(new supervisor_lite());
    actor_zeta::spawn_actor<storage_t>(dummy_supervisor);
    delegate(dummy_supervisor, "storage", "update", std::string("payload"));
    delegate(dummy_supervisor, "storage", "find");
    delegate(dummy_supervisor, "storage", "remove");
    delegate(dummy_supervisor, "storage", "status");
    return 0;
}