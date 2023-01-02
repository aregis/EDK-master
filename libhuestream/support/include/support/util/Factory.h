/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#pragma once

#include <memory>
#include <functional>
#include <type_traits>
#include <utility>

#include "support/util/Provider.h"

template <typename Product, typename ...Args>
std::unique_ptr<Product> huesdk_lib_default_factory(Args... args);

template <typename Context, typename Product, typename ...Args>
std::unique_ptr<Product> huesdk_lib_contextual_factory(Args... args);

template <typename Product, typename ...Args>
struct default_object<std::function<std::unique_ptr<Product>(Args...)>, support::detail::DefaultContext> {
    static std::function<std::unique_ptr<Product>(Args...)> get() {
        return huesdk_lib_default_factory<Product, Args...>;
    }
};

template <typename Context, typename Product, typename ...Args>
struct default_object<std::function<std::unique_ptr<Product>(Args...)>, Context> {
    static std::function<std::unique_ptr<Product>(Args...)> get() {
        return huesdk_lib_contextual_factory<Context, Product, Args...>;
    }
};

namespace support {

    template <typename Function, typename Context = detail::DefaultContext>
    class Factory;

    template <typename Context, typename Product, typename ...Args>
    class Factory<std::unique_ptr<Product>(Args...), Context> {
    public:
        using context = Context;
        using product = Product;
        using FactoryMethod = std::function<std::unique_ptr<Product>(Args...)>;

        static std::unique_ptr<Product> create(Args... args) {
            auto factory_method = Provider<FactoryMethod, Context>::get();
            return factory_method(std::forward<Args>(args)...);
        }

        static void set_factory_method(std::function<std::unique_ptr<Product>(Args...)> factory_method) {
            Provider<FactoryMethod, Context>::set(factory_method);
        }

        // For mocking in testing, it is convenient to not have to care about the factory creation arguments
        template <typename Dummy = void>
        static auto set_factory_method(std::function<std::unique_ptr<Product>()> f)
           -> typename std::enable_if<sizeof...(Args) != 0, Dummy>::type {
            auto wrapper_lambda = [f](Args...) { return f(); };
            Provider<FactoryMethod, Context>::set(wrapper_lambda);
        }
    };

    template <typename Function, typename Context = detail::DefaultContext>
    class ScopedFactory;

    template <typename Context, typename Product, typename ...Args>
    class ScopedFactory<std::unique_ptr<Product>(Args...), Context>
            : public ScopedProvider<std::function<std::unique_ptr<Product>(Args...)>, Context> {
    public:
        using factory = Factory<std::unique_ptr<Product>(Args...), Context>;

        explicit ScopedFactory(const std::function<std::unique_ptr<Product>(Args...)>& f)
            : ScopedProvider<std::function<std::unique_ptr<Product>(Args...)>, Context>(f) {}

        // For mocking in testing, it is convenient to not have to care about the factory creation arguments
        template <typename Dummy = void, typename = typename std::enable_if<sizeof...(Args) != 0, Dummy>::type>
        explicit ScopedFactory(const std::function<std::unique_ptr<Product>()>& f)
            : ScopedProvider<std::function<std::unique_ptr<Product>(Args...)>, Context>(
                    [f](Args...) { return f(); }) {}
    };

}  // namespace support

