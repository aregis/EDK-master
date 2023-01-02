/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#pragma once

#include <memory>
#include <utility>

#include "Converter.h"
#include "ConverterCore.h"
#include "JObjectFactory.h"
#include "ClassInfo.h"
#include "Functional.h"

#include "support/util/ExceptionUtil.h"

namespace huesdk_jni_core {

    template<typename T>
    struct from<std::shared_ptr<T>> : from_base<std::shared_ptr<T>> {
        explicit from(std::shared_ptr<T> value) {
            this->_value = create_jobject(value);
            if (this->_value == nullptr) {
                this->_value = convert<std::shared_ptr<T>, jobject>(value);
            }
        }
    };

    template<typename T>
    struct to<std::shared_ptr<T>> : to_base<std::shared_ptr<T>> {
        explicit to(jobject instance) {
            auto proxy = get_proxy<T>(instance);
            if (proxy != nullptr) {
                this->_value = proxy->take_shared_ptr();
            }
        }
    };

    namespace detail {
        template<typename To, typename From>
        std::unique_ptr<To> unique_dynamic_cast(std::unique_ptr<From> from) {
            if (dynamic_cast<To*>(from.get()) != nullptr) {
                return std::unique_ptr<To>(dynamic_cast<To*>(from.release()));
            } else {
                return nullptr;
            }
        }
    }

    template<typename T>
    struct from<T&> : from_base<T&> {
        template<typename S, typename = typename std::enable_if<std::is_base_of<S, T>::value>::type>
        explicit from(std::unique_ptr<S> value) {
            this->_value = create_jobject(detail::unique_dynamic_cast<T>(std::move(value)));
        }

        explicit from(std::unique_ptr<T> value) {
            this->_value = create_jobject(std::move(value));
        }

        explicit from(const T& value) {
            this->_value = create_jobject(std::unique_ptr<T>(value.clone()));
        }
    };

    template<typename T>
    struct to<T&> {
        using Target = T&;

        explicit to(jobject instance) {
            auto proxy = get_proxy<T>(instance);
            if (proxy != nullptr) {
                this->_value = proxy->take_shared_ptr();
            } else {
                support::throw_exception<std::runtime_error>("No proxy found");
            }
        }

        Target value() const { return *_value; }
        operator Target() const { return *_value; }  // NOLINT

        std::shared_ptr<T> _value;
    };

    template<typename T>
    struct from<boost::optional<std::shared_ptr<T>>> : from_base<boost::optional<std::shared_ptr<T>>> {
        explicit from(const std::shared_ptr<T>& value) {
            if (value == nullptr) {
                this->_value = nullptr;
            } else {
                this->_value = from<std::shared_ptr<T>>(value).value();
            }
        }

        explicit from(const std::shared_ptr<const T>& value) {
            if (value == nullptr) {
                this->_value = nullptr;
            } else {
                this->_value = from<std::shared_ptr<T>>(std::shared_ptr<T>(dynamic_cast<T*>(value->clone().release()))).value();
            }
        }

        // Downcasting converter
        template<typename S, typename = typename std::enable_if<std::is_base_of<S, T>::value>::type>
        explicit from(const std::shared_ptr<const S>& value) {
            auto cast_value = std::dynamic_pointer_cast<const T>(value);
            if (cast_value == nullptr) {
                this->_value = nullptr;
            } else {
                auto x = std::shared_ptr<S>(cast_value->clone());
                this->_value = from<std::shared_ptr<T>>(std::dynamic_pointer_cast<T>(x)).value();
            }
        }

        explicit from(const boost::optional<std::shared_ptr<T>>& value) {
            if (value == boost::none) {
                this->_value = nullptr;
            } else {
                this->_value = from<std::shared_ptr<T>>(value.value()).value();
            }
        }

        explicit from(std::unique_ptr<T>&& value) {
            if (value == nullptr) {
                this->_value = nullptr;
            } else {
                this->_value = from<std::shared_ptr<T>>(value).value();
            }
        }
    };

    template<typename T>
    struct to<boost::optional<std::shared_ptr<T>>> : to_base<boost::optional<std::shared_ptr<T>>> {
        explicit to(jobject value) {
            if (value == nullptr) {
                this->_value = boost::none;
            } else {
                this->_value = to<std::shared_ptr<T>>(value).value();
            }
        }

        // For backwards compatibilty, as SDK uses nullptr instead of boost::none
        std::shared_ptr<T> value() const {
            return this->_value.value_or(std::shared_ptr<T>());
        }

        // For backwards compatibilty, as SDK uses nullptr instead of boost::none
        operator std::shared_ptr<T> () const {
            return this->_value.value_or(std::shared_ptr<T>());
        }

        // For backwards compatibilty, as SDK uses nullptr instead of boost::none
        operator std::weak_ptr<T> () const {
            return this->_value.value_or(std::shared_ptr<T>());
        }
    };

    template<typename T>
    struct from<std::unique_ptr<T>> : from_base<std::unique_ptr<T>> {
        explicit from(std::unique_ptr<T> value) {
            this->_value = create_jobject(std::move(value));
        }

        template<typename S, typename = typename std::enable_if<std::is_base_of<S, T>::value>::type>
        explicit from(std::unique_ptr<S> value) {
            this->_value = create_jobject(detail::unique_dynamic_cast<T>(std::move(value)));
        }
    };

}  // namespace huesdk_jni_core
