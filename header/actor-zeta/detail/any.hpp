#pragma once

#include <actor-zeta/config.hpp>

#if CPP17_OR_GREATER

#include <any>
#include <utility>

namespace actor_zeta { namespace detail {

    using std::any;
    using std::any_cast;
    using std::make_any;
    using std::swap;

}} // namespace actor_zeta::detail

#elif CPP14_OR_GREATER or CPP11_OR_GREATER

#include <algorithm>
#include <cassert>
#include <initializer_list>
#include <new>
#include <type_traits>
#include <utility>

#include <actor-zeta/detail/type_traits.hpp>

namespace actor_zeta { namespace detail {
    ///
    /// @brief
    ///

    /// Drop-in replacement for C++17 std::any.

    namespace implementation {
        inline void implementation_hack_bad_any_cast() {
            assert(false);
        }

        template<typename T, typename... Args>
        void* default_construct(Args&&... args) {
            return ::new T(std::forward<Args>(args)...);
        }

        template<typename T>
        void default_destroy(T* p) {
            //p->~T();
            delete p;
        }
    } // namespace implementation

    class any final {
        enum class storage_operation {
            get,
            destroy,
            copy,
            move
        };

        union storage {
            using internal_storage_t = type_traits::aligned_storage_t<
                4 * sizeof(void*), std::alignment_of<void*>::value>;

            void* external_storage = nullptr;
            internal_storage_t internal_storage;
        };

        template<typename T>
        using use_internal_storage = type_traits::bool_constant<
            std::is_nothrow_move_constructible<T>::value && (sizeof(T) <= sizeof(storage)) &&
            (std::alignment_of<storage>::value % std::alignment_of<T>::value == 0)>;

        template<class ValueType>
        friend const ValueType* any_cast(const any* pAny) noexcept;

        template<class ValueType>
        friend ValueType* any_cast(any* pAny) noexcept;

        template<class ValueType>
        friend ValueType any_cast(const any& operand);

        template<class ValueType>
        friend ValueType any_cast(any& operand);

        template<class ValueType>
        friend ValueType any_cast(any&& operand);

        template<typename T>
        struct storage_handler_internal final {
            template<typename V>
            static void construct(storage& s, V&& v) {
                ::new (&s.internal_storage) T(std::forward<V>(v));
            }

            template<typename... Args>
            static void construct_in_place(storage& s, Args... args) {
                ::new (&s.internal_storage) T(std::forward<Args>(args)...);
            }

            template<class NT, class U, class... Args>
            static void construct_in_place(storage& s, std::initializer_list<U> il, Args&&... args) {
                ::new (&s.internal_storage) NT(il, std::forward<Args>(args)...);
            }

            static inline void destroy(any& ref) {
                T& t = *static_cast<T*>(static_cast<void*>(&ref.storage_.internal_storage));
                t.~T();

                ref.handler_ = nullptr;
            }

            static void* handler_func(storage_operation operation, const any* current, any* other) {
                switch (operation) {
                    case storage_operation::get: {
                        assert(current);
                        return (void*) (&current->storage_.internal_storage);
                    } break;

                    case storage_operation::destroy: {
                        assert(current);
                        destroy(const_cast<any&>(*current));
                    } break;

                    case storage_operation::copy: {
                        assert(current);
                        assert(other);
                        construct(other->storage_, *(T*) (&current->storage_.internal_storage));
                    } break;

                    case storage_operation::move: {
                        assert(current);
                        assert(other);
                        construct(other->storage_, std::move(*(T*) (&current->storage_.internal_storage)));
                        destroy(const_cast<any&>(*current));
                    } break;

                    default: {
                        assert(false);
                    } break;
                };

                return nullptr;
            }
        };

        template<typename T>
        struct storage_handler_external final {
            template<typename V>
            static inline void construct(storage& s, V&& v) {
                s.external_storage = implementation::default_construct<T>(std::forward<V>(v));
            }

            template<typename... Args>
            static inline void construct_in_place(storage& s, Args... args) {
                s.external_storage = implementation::default_construct<T>(std::forward<Args>(args)...);
            }

            template<class NT, class U, class... Args>
            static inline void construct_in_place(storage& s, std::initializer_list<U> il, Args&&... args) {
                s.external_storage = implementation::default_construct<NT>(il, std::forward<Args>(args)...);
            }

            static inline void destroy(any& refAny) {
                implementation::default_destroy(static_cast<T*>(refAny.storage_.external_storage));
                refAny.handler_ = nullptr;
            }

            static void* handler_func(storage_operation op, const any* current, any* other) {
                switch (op) {
                    case storage_operation::get: {
                        assert(current);
                        assert(current->storage_.external_storage);
                        return static_cast<void*>(current->storage_.external_storage);
                    } break;

                    case storage_operation::destroy: {
                        assert(current);
                        destroy(*const_cast<any*>(current));
                    } break;

                    case storage_operation::copy: {
                        assert(current);
                        assert(other);
                        construct(other->storage_, *static_cast<T*>(current->storage_.external_storage));
                    } break;

                    case storage_operation::move: {
                        assert(current);
                        assert(other);
                        construct(other->storage_, std::move(*(T*) (current->storage_.external_storage)));
                        destroy(const_cast<any&>(*current));
                    } break;

                    default: {
                        assert(false);
                    } break;
                };

                return nullptr;
            }
        };

        using storage_handler_ptr = void* (*) (storage_operation, const any*, any*);

        template<typename T>
        using storage_handler = typename std::conditional<use_internal_storage<T>::value,
                                                          storage_handler_internal<T>,
                                                          storage_handler_external<T>>::type;

        storage storage_;
        storage_handler_ptr handler_;

    public:
        any() noexcept
            : storage_()
            , handler_(nullptr) {}

        any(const any& other)
            : handler_(nullptr) {
            if (other.handler_) {
                other.handler_(storage_operation::copy, &other, this);
                handler_ = other.handler_;
            }
        }

        any(any&& other) noexcept
            : handler_(nullptr) {
            if (other.handler_) {
                handler_ = std::move(other.handler_);
                other.handler_(storage_operation::move, &other, this);
            }
        }

        ~any() { reset(); }

        template<class ValueType>
        any(ValueType&& value,
            type_traits::enable_if_t<!std::is_same<type_traits::decay_t<ValueType>, any>::value>* = 0) {
            using decayed_value_type = type_traits::decay_t<ValueType>;
            static_assert(std::is_copy_constructible<decayed_value_type>::value, "ValueType must be copy-constructible");
            storage_handler<decayed_value_type>::construct(storage_, std::forward<ValueType>(value));
            handler_ = &storage_handler<decayed_value_type>::handler_func;
        }

        template<class T, class... Args>
        explicit any(type_traits::in_place_type_t<T>, Args&&... args) {
            static_assert(std::is_constructible<T, Args...>::value, "T must be constructible with Args...");
            using storage_handler_t = storage_handler<type_traits::decay_t<T>>;
            storage_handler_t::construct_in_place(storage_, std::forward<Args>(args)...);
            handler_ = &storage_handler_t::handler_func;
        }

        template<class T, class U, class... Args>
        explicit any(type_traits::in_place_type_t<T>,
                     std::initializer_list<U> il,
                     Args&&... args,
                     typename std::enable_if<std::is_constructible<T, std::initializer_list<U>&, Args...>::value, void>::type* = 0) {
            using storage_handler_t = storage_handler<type_traits::decay_t<T>>;

            storage_handler_t::construct_in_place(storage_, il, std::forward<Args>(args)...);
            handler_ = &storage_handler_t::handler_func;
        }

        template<class ValueType>
        any& operator=(ValueType&& value) {
            static_assert(std::is_copy_constructible<type_traits::decay_t<ValueType>>::value, "ValueType must be copy-constructible");
            any(std::forward<ValueType>(value)).swap(*this);
            return *this;
        }

        any& operator=(const any& other) {
            any(other).swap(*this);
            return *this;
        }

        any& operator=(any&& other) noexcept {
            any(std::move(other)).swap(*this);
            return *this;
        }

        template<class T, class... Args>
        void emplace(Args&&... args) {
            static_assert(std::is_constructible<T, Args...>::value, "T must be constructible with Args...");
            using storage_handler_t = storage_handler<type_traits::decay_t<T>>;
            reset();
            storage_handler_t::construct_in_place(storage_, std::forward<Args>(args)...);
            handler_ = &storage_handler_t::handler_func;
        }

        template<class NT, class U, class... Args>
        typename std::enable_if<std::is_constructible<NT, std::initializer_list<U>&, Args...>::value, void>::type
        emplace(std::initializer_list<U> il, Args&&... args) {
            using storage_handler_t = storage_handler<type_traits::decay_t<NT>>;

            reset();
            storage_handler_t::construct_in_place(storage_, il, std::forward<Args>(args)...);
            handler_ = &storage_handler_t::handler_func;
        }

        void reset() noexcept {
            if (handler_) {
                handler_(storage_operation::destroy, this, nullptr);
            }
        }

        void swap(any& other) noexcept {
            using std::swap;
            if (this == &other) {
                return;
            }

            if (handler_ && other.handler_) {
                any tmp;
                tmp.handler_ = other.handler_;
                other.handler_(storage_operation::move, &other, &tmp);

                other.handler_ = handler_;
                handler_(storage_operation::move, this, &other);

                handler_ = tmp.handler_;
                tmp.handler_(storage_operation::move, &tmp, this);
            } else if (handler_ == nullptr && other.handler_) {
                std::swap(handler_, other.handler_);
                handler_(storage_operation::move, &other, this);
            } else if (handler_ && other.handler_ == nullptr) {
                std::swap(handler_, other.handler_);
                other.handler_(storage_operation::move, this, &other);
            }
        }

        bool has_value() const noexcept {
            return handler_ != nullptr;
        }
    };

    inline void swap(any& rhs, any& lhs) noexcept {
        rhs.swap(lhs);
    }

    template<class ValueType>
    inline ValueType any_cast(const any& operand) {
        static_assert(std::is_reference<ValueType>::value || std::is_copy_constructible<ValueType>::value, "ValueType must be a reference or copy constructible");
        auto* ptr = any_cast<typename std::add_const<type_traits::remove_reference_t<ValueType>>::type>(&operand);

        if (ptr == nullptr) {
            implementation::implementation_hack_bad_any_cast();
        }

        return *ptr;
    }

    template<class ValueType>
    inline ValueType any_cast(any& operand) {
        static_assert(std::is_reference<ValueType>::value || std::is_copy_constructible<ValueType>::value, "ValueType must be a reference or copy constructible");
        auto* ptr = any_cast<type_traits::remove_reference_t<ValueType>>(&operand);

        if (ptr == nullptr) {
            implementation::implementation_hack_bad_any_cast();
        }

        return *ptr;
    }

    template<class ValueType>
    inline ValueType any_cast(any&& operand) {
        static_assert(std::is_reference<ValueType>::value || std::is_copy_constructible<ValueType>::value, "ValueType must be a reference or copy constructible");
        auto* ptr = any_cast<type_traits::remove_reference_t<ValueType>>(&operand);

        if (ptr == nullptr) {
            implementation::implementation_hack_bad_any_cast();
        }

        return *ptr;
    }

    template<class ValueType>
    inline const ValueType* any_cast(const any* pAny) noexcept {
        return (pAny && pAny->handler_) ? static_cast<const ValueType*>(pAny->handler_(any::storage_operation::get, pAny, nullptr)) : nullptr;
    }

    template<class ValueType>
    inline ValueType* any_cast(any* pAny) noexcept {
        return (pAny && pAny->handler_) ? static_cast<ValueType*>(pAny->handler_(any::storage_operation::get, pAny, nullptr)) : nullptr;
    }

    template<class T, class... Args>
    inline any make_any(Args&&... args) {
        return any(type_traits::in_place<T>, std::forward<Args>(args)...);
    }

    template<class T, class U, class... Args>
    inline any make_any(std::initializer_list<U> il, Args&&... args) {
        return any(type_traits::in_place<T>, il, std::forward<Args>(args)...);
    }

}} // namespace actor_zeta::detail

#endif
