#include <actor-zeta.hpp>
#include <cassert>
#include <iostream>
#include <testsuite/network/fake_multiplexer.hpp>

namespace actor_zeta {
namespace network {

    std::size_t fake_multiplexer::start() {
        assert(!scenario.empty());
        assert(!tasks.empty());
        while (enabled) {
            std::this_thread::sleep_for(std::chrono::seconds(0));
            while (!tasks.empty()) {
                auto task = std::move(tasks.front());
                tasks.pop_front();
                task();
            }
        }

        return 0;
    }

    fake_multiplexer::~fake_multiplexer() {
        enabled = false;
        sockets.clear();
        scenario.clear();
    }

    void fake_multiplexer::new_tcp_listener(
        const std::string& host,
        uint16_t port,
        base::address_t address) {
        connection_identifying id(network::connect_type::tcp, host, port);
        connection<fake_socket> connection_(std::move(scenario.at(id)), std::move(address));
        sockets.emplace(id, std::move(connection_));

        tasks.emplace_back(
            [id, this]() -> void {
                auto& connection = sockets.at(id);
                query_raw_t query_raw;
                query_raw.id = id;
                query_raw.raw = connection.socket().read();
                send(connection.address(), base::address_t(), "read", std::move(query_raw));
            });
    }

    void fake_multiplexer::write(const connection_identifying& id, const buffer& b) {
        auto& client = sockets.at(id);
        client.socket().write(b);
    }

    void fake_multiplexer::close(const connection_identifying& id) {
        auto& client = sockets.at(id);
        client.socket().close();
        sockets.erase(id);
    }

    void fake_multiplexer::new_tcp_connection(
        const std::string& /**host*/,
        uint16_t /**port*/,
        base::address_t /**address*/
    ) {
        /**
             * TODO: Implementation
            */
    }

    fake_socket& fake_multiplexer::add_scenario(const std::string& host, uint16_t port) {
        connection_identifying id(network::connect_type::tcp, host, port);
        scenario.emplace(id, fake_socket());
        return scenario.at(id);
    }

    fake_multiplexer::fake_multiplexer()
        : enabled(true) {
    }

}
} // namespace actor_zeta::network
