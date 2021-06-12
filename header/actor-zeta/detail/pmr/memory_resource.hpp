#pragma once

#include <cstddef>

#if CPP17_OR_GREATER

#include <memory_resource>

#endif

namespace actor_zeta { namespace detail { namespace pmr {

#if CPP17_OR_GREATER

    using std::pmr::memory_resource;

#elif CPP14_OR_GREATER or CPP11_OR_GREATER

    class memory_resource {
    public:
        virtual ~memory_resource() {}

        void* allocate(std::size_t bytes, std::size_t alignment = alignof(std::max_align_t)) {
            return this->do_allocate(bytes, alignment);
        }

        void deallocate(void* p, std::size_t bytes, std::size_t alignment = alignof(std::max_align_t)) {
            return this->do_deallocate(p, bytes, alignment);
        }

        bool is_equal(const memory_resource& other) const noexcept {
            return this->do_is_equal(other);
        }

    protected:
        virtual void* do_allocate(std::size_t bytes, std::size_t alignment) = 0;

        virtual void do_deallocate(void* p, std::size_t bytes, std::size_t alignment) = 0;

        virtual bool do_is_equal(const memory_resource& other) const noexcept = 0;
    };

#endif

}}} // namespace actor_zeta::detail::pmr