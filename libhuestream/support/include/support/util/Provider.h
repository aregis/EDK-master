/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#pragma once

#include <functional>
#include <memory>
#include <mutex>

/* Provider acts as a (thread safe) singleton containing data of a type T. The context argument
 * can be used for more granular control of the data */

namespace support {
    namespace detail {
        struct DefaultContext {};
    }  // namespace detail
}  // namespace support

template <typename T, typename Context = support::detail::DefaultContext>
struct default_object {
    static T get();
};

namespace support {

    template <typename T, typename Context = detail::DefaultContext>
    class Provider {
    public:
        using context = Context;
        using type = T;

        static T get() {
            auto& provider_data = get_provider_data();

            std::lock_guard<std::mutex> lock{provider_data._mutex};

            if (!provider_data._is_initialized) {
                provider_data._data = default_object<T, Context>::get();
                provider_data._is_initialized = true;
            }

            return provider_data._data;
        }

        static void set(T data) {
            auto& provider_data = get_provider_data();
            std::lock_guard<std::mutex> lock{provider_data._mutex};
            provider_data._data = data;
            provider_data._is_initialized = true;
        }

    private:
        Provider() = default;

        struct ProviderData {
            T _data;
            bool _is_initialized =  false;
            std::mutex _mutex;
        };

        static ProviderData& get_provider_data() {
            static ProviderData _data;
            return _data;
        }
    };

    template <typename T, typename Context = detail::DefaultContext>
    class ScopedProvider {
    public:
        using context = Context;
        using type = typename Provider<T, Context>::type;
        using provider_type = Provider<T, Context>;

        explicit ScopedProvider(const T& data) {
            _original = provider_type::get();
            provider_type::set(data);
        }

        ~ScopedProvider() {
            provider_type::set(_original);
        }

        ScopedProvider(const ScopedProvider&) = delete;
        ScopedProvider(ScopedProvider&&) = delete;

    private:
        T _original;
    };

}  // namespace support
