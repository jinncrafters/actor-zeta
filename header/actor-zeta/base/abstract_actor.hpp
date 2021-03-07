#pragma once

#include <actor-zeta/forwards.hpp>
#include <actor-zeta/detail/string_view.hpp>
#include <actor-zeta/base/communication_module.hpp>

namespace actor_zeta { namespace base {
///
/// @brief Abstract concept of an actor
///

        class abstract_actor : public communication_module  {
        public:
            abstract_actor()= delete;

            abstract_actor(const abstract_actor &) = delete;

            abstract_actor &operator=(const abstract_actor &) = delete;

            ~abstract_actor() override;

            ///TODO:
            ///virtual void launch(executor::execution_device*, bool /*hide*/) = 0;
        protected:
            abstract_actor(supervisor_t *,detail::string_view);

            auto context(executor::execution_device *) -> void;

            auto context() const -> executor::execution_device* ;

            auto supervisor() -> supervisor_t* ;

        private:
            supervisor_t* supervisor_;
            executor::execution_device *executor_;
        };

}}
