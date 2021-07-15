// clang-format off
#include <actor-zeta/base/handler.hpp>
#include <actor-zeta/base/address.hpp>
#include <actor-zeta/base/message.hpp>
#include <actor-zeta/base/basic_actor.hpp>
#include <actor-zeta/base/supervisor.hpp>
#include <actor-zeta/impl/handler.ipp>
// clang-format on
#include <actor-zeta/base/actor.hpp>
#include <actor-zeta/base/supervisor_abstract.hpp>
#include <actor-zeta/link.hpp>

namespace actor_zeta {
namespace base {

    static constexpr std::size_t DEFAULT_ALIGNMENT{alignof(std::max_align_t)};

    constexpr bool is_pow2(std::size_t n) { return (0 == (n & (n - 1))); }

    constexpr bool is_supported_alignment(std::size_t alignment) { return is_pow2(alignment); }

    template<typename Alloc>
    void* aligned_allocate(std::size_t bytes, std::size_t alignment, Alloc alloc) {
        assert(is_pow2(alignment));

        std::size_t padded_allocation_size{bytes + alignment + sizeof(std::ptrdiff_t)};

        char* const original = static_cast<char*>(alloc(padded_allocation_size));

        void* aligned{original + sizeof(std::ptrdiff_t)};

        std::align(alignment, bytes, aligned, padded_allocation_size);

        std::ptrdiff_t offset = static_cast<char*>(aligned) - original;

        *(static_cast<std::ptrdiff_t*>(aligned) - 1) = offset;

        return aligned;
    }

    template<typename Dealloc>
    void aligned_deallocate(void* p, std::size_t bytes, std::size_t alignment, Dealloc dealloc) {
        (void) alignment;
        (void) bytes;

        std::ptrdiff_t const offset = *(reinterpret_cast<std::ptrdiff_t*>(p) - 1);

        void* const original = static_cast<char*>(p) - offset;

        dealloc(original);
    }

    class new_delete_resource final : public detail::pmr::memory_resource {
    public:
    private:
        void* do_allocate(std::size_t bytes, std::size_t alignment = DEFAULT_ALIGNMENT) override {
            alignment = (is_supported_alignment(alignment)) ? alignment : DEFAULT_ALIGNMENT;
            return aligned_allocate(bytes, alignment, [](std::size_t size) { return ::operator new(size); });
        }

        void do_deallocate(void* p, std::size_t bytes, std::size_t alignment = DEFAULT_ALIGNMENT) override {
            aligned_deallocate(p, bytes, alignment, [](void* p) { ::operator delete(p); });
        }

        bool do_is_equal(const memory_resource& other) const noexcept override { return &other == this; }
    };

    supervisor_abstract::supervisor_abstract(std::string name, detail::pmr::memory_resource* memory_resource)
        : communication_module(std::move(name))
        , memory_resource_(memory_resource) {
        add_handler("spawn_actor", &supervisor_abstract::spawn_actor);
        add_handler("spawn_supervisor", &supervisor_abstract::spawn_supervisor);
        add_handler("delegate", &supervisor_abstract::redirect);
    }

    supervisor_abstract::supervisor_abstract(std::string name)
        : communication_module(std::move(name))
        , memory_resource_(new new_delete_resource) {
        add_handler("spawn_actor", &supervisor_abstract::spawn_actor);
        add_handler("spawn_supervisor", &supervisor_abstract::spawn_supervisor);
        add_handler("delegate", &supervisor_abstract::redirect);
    }

    supervisor_abstract::supervisor_abstract(supervisor_abstract* ptr, std::string name)
        : communication_module(std::move(name))
        , memory_resource_(ptr->resource()) {
        add_handler("spawn_actor", &supervisor_abstract::spawn_actor);
        add_handler("spawn_supervisor", &supervisor_abstract::spawn_supervisor);
        add_handler("delegate", &supervisor_abstract::redirect);
    }
    supervisor_abstract::~supervisor_abstract() {}

    auto supervisor_abstract::current_message() -> message* {
        return current_message_;
    }

    auto supervisor_abstract::set_current_message(message_ptr msg) -> void {
        current_message_ = msg.release();
    }

    auto supervisor_abstract::resource() const -> detail::pmr::memory_resource* {
        return memory_resource_;
    }

    auto supervisor_abstract::executor() noexcept -> executor::abstract_executor* {
        return executor_impl();
    }

    auto supervisor_abstract::spawn_actor(default_spawn_actor& construct) -> void {
        auto actor_tmp = std::move(construct(this));
        auto address = actor_tmp->address();
        add_actor_impl(std::move(actor_tmp));
        link(*this, address);
    }

    auto supervisor_abstract::spawn_supervisor(default_spawn_supervisor& construct) -> void {
        auto supervisor = std::move(construct(this));
        auto address = supervisor->address();
        add_supervisor_impl(std::move(supervisor));
        link(*this, address);
    }

    auto supervisor_abstract::redirect(std::string& type, message* msg) -> void {
        message_ptr tmp(std::move(msg));
        auto type_t = std::move(type);
        tmp->sender() = std::move(address());
        send(address_book(type_t.c_str()), std::move(tmp));
    }

    auto supervisor_abstract::address() noexcept -> address_t {
        return address_t(this);
    }
}
} // namespace actor_zeta::base
