#include <cassert>

#include <chrono>
#include <iostream>
#include <memory>
#include <queue>
#include <thread>
#include <unordered_set>
#include <vector>

#include <actor-zeta.hpp>

using actor_zeta::abstract_executor;
using actor_zeta::basic_async_actor;
using actor_zeta::supervisor;

using actor_zeta::abstract_executor;
using actor_zeta::executor_t;
using actor_zeta::work_sharing;

using actor_zeta::make_message;

template<typename Task, typename... Args>
auto make_task(actor_zeta::supervisor& executor_, const std::string& command, Args... args) -> void {
    actor_zeta::send(executor_, executor_->address(), command, std::move(Task(std::forward<Args>(args)...)));
}

template<typename Task, typename... Args>
auto make_task_broadcast(actor_zeta::supervisor& executor_, const std::string& command, Args... args) -> void {
    auto address = executor_->address();
    actor_zeta::broadcast(executor_, address, command, std::move(Task(std::forward<Args>(args)...)));
}

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
              "spawn_actor"} {
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

struct download_data final {
    download_data(const std::string& url, const std::string& user, const std::string& passwod)
        : url_(url)
        , user_(user)
        , passwod_(passwod) {}

    ~download_data() = default;
    std::string url_;
    std::string user_;
    std::string passwod_;
};

struct work_data final {
    ~work_data() = default;

    work_data(const std::string& data, const std::string& operatorName)
        : data_(data)
        , operator_name_(operatorName) {}

    std::string data_;
    std::string operator_name_;
};

static std::atomic<uint64_t> counter_download_data{0};
static std::atomic<uint64_t> counter_work_data{0};

class worker_t final : public actor_zeta::basic_async_actor {
public:
    explicit worker_t(actor_zeta::supervisor_abstract* ptr)
        : actor_zeta::basic_async_actor(ptr, "bot") {
        add_handler(
            "download",
            &worker_t::download);

        add_handler(
            "work_data",
            &worker_t::work_data);
    }

    void download(download_data& data) {
        tmp_ = data.url_;
        counter_download_data++;
    }

    void work_data(work_data& data) {
        tmp_ = data.data_;
        counter_work_data++;
    }

    ~worker_t() override = default;

private:
    std::string tmp_;
};

int main() {
    actor_zeta::supervisor supervisor(new supervisor_lite());

    int const actors = 10;

    for (auto i = actors - 1; i > 0; --i) {
        actor_zeta::spawn_actor<worker_t>(supervisor);
    }

    //actor_zeta::spawn_actor<worker_t>(supervisor);

    int const task = 10000;

    for (auto i = task - 1; i > 0; --i) {
        make_task<download_data>(supervisor, "download", std::string("fb"), std::string("jack"), std::string("1"));
    }

    for (auto i = task - 1; i > 0; --i) {
        make_task_broadcast<work_data>(supervisor, "work_data", std::string("fb"), std::string("jack"));
    }

    std::this_thread::sleep_for(std::chrono::seconds(180));

    std::cerr << " Finish " << std::endl;
    std::cerr << "counter_download_data :" << counter_download_data.load() << std::endl;
    std::cerr << "counter_work_data :" << counter_work_data.load() << std::endl;
    assert(counter_download_data.load() == 9999);
    assert(counter_work_data.load() == 9999);
    return 0;
}
