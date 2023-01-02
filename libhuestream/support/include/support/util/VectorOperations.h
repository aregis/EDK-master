/*******************************************************************************
Copyright (C) 2019 Signify Holding
All Rights Reserved.
********************************************************************************/
#pragma once

#include <stddef.h>

#include <algorithm>
#include <cstring>
#include <list>
#include <memory>
#include <string>
#include <type_traits>
#include <vector>
#include <utility>
#include <cassert>
#include "boost/optional.hpp"

#include "support/util/MakeUnique.h"
#include "support/util/ExceptionUtil.h"

namespace support {

    namespace detail {
        template<typename T>
        T* optionally_release(T* p) {
            return p;
        }

        template<typename T>
        T* optionally_release(std::unique_ptr<T> p) {
            return p.release();
        }

        template<typename T>
        T* clone(T *data) {
            if (data == nullptr) {
                return nullptr;
            } else {
                T* clone = dynamic_cast<T *>(detail::optionally_release(data->clone()));
                assert(clone);
                return clone;
            }
        }

        template<typename T>
        std::unique_ptr<T> clone(const std::unique_ptr<T> &data) {
            return std::unique_ptr<T>{clone(data.get())};
        }

        template<typename T>
        std::shared_ptr<T> clone(const std::shared_ptr<T> &data) {
            return std::shared_ptr<T>{clone(data.get())};
        }
    }  // namespace detail

    template <typename T>
    std::vector<T> clone(const std::vector<T>& vector_data) {
        std::vector<T> cloned_vector;
        cloned_vector.reserve(vector_data.size());

        for (const auto& data : vector_data) {
            cloned_vector.push_back(detail::clone(data));
        }

        return cloned_vector;
    }

    template <typename T>
    std::vector<std::unique_ptr<T>> clone(const std::vector<std::unique_ptr<T>>& vector_data) {
        std::vector<std::unique_ptr<T>> cloned_vector;

        for (const auto& data : vector_data) {
            cloned_vector.push_back(detail::clone(data));
        }

        return cloned_vector;
    }

    template <typename PointerType>
    boost::optional<std::vector<PointerType>> clone(const boost::optional<std::vector<PointerType>>& org) {
        if (org != boost::none) {
            std::vector<PointerType> result;
            for (auto&& elem : *org) {
                result.emplace_back(detail::clone(elem));
            }
            return result;
        } else {
            return boost::none;
        }
    }

    template <typename OptionalRange, typename UnaryFunction>
    void for_each(const OptionalRange& range, UnaryFunction fun) {
        if (range == boost::none) {
            return;
        }
        for (auto&& elem : *range) {
           fun(elem);
        }
    }

    template <template<typename...> class SmartPtr, typename T>
    std::vector<SmartPtr<T>> clone_to_vector_with(const std::vector<T*>& org) {
        std::vector<SmartPtr<T>> return_value;

        for (const auto& item : org) {
            return_value.emplace_back(detail::clone(item));
        }

        return return_value;
    }

    template <template<typename...> class SmartPtr, typename T>
    std::vector<SmartPtr<T>> clone_shared_ptrs(const std::vector<std::shared_ptr<T>>& org) {
        std::vector<SmartPtr<T>> return_value;

        for (const auto& item : org) {
            return_value.emplace_back(SmartPtr<T>(detail::clone(item.get())));
        }

        return return_value;
    }

    template <typename T>
    std::vector<std::shared_ptr<T>> to_vector_with_shared_ptrs(std::vector<std::unique_ptr<T>> org) {
        std::vector<std::shared_ptr<T>> result;

        for (auto&& item : org) {
            result.push_back(std::move(item));
        }

        return result;
    }

    template <typename T>
    auto clone_to_std_vector(const std::vector<T>& org) -> std::vector<decltype(std::declval<T>().get())> {
        std::vector<decltype(std::declval<T>().get())> return_value;
        return_value.reserve(org.size());
        for (const auto& item : org) {
            return_value.push_back(detail::clone(item.get()));
        }
        return return_value;
    }

    template <template<typename...> class SmartPtr, typename T>
    std::vector<SmartPtr<T>> to_vector_with(std::vector<T*>&& org) {
        std::vector<SmartPtr<T>> return_value;

        std::vector<T*> copy_vector;
        org.swap(copy_vector);
        for (auto item : copy_vector) {
            return_value.emplace_back(item);
        }
        return return_value;
    }

    template <typename T>
    auto to_std_vector(const std::vector<T>& org) -> std::vector<decltype(std::declval<T>().get())> {
        std::vector<decltype(std::declval<T>().get())> return_value;
        return_value.reserve(org.size());

        for (auto& item : org) {
            return_value.push_back(item.get());
        }
        return return_value;
    }

    template<typename T>
    T* to_raw(T* value) {
        return value;
    }

    template<typename T>
    auto to_raw(T& value) -> decltype(value.get()) {
        return value.get();
    }

    template<typename T>
    std::vector<const T*> to_raw(const std::vector<std::unique_ptr<T>>& value) {
        std::vector<const T*> raw_vector;
        raw_vector.reserve(value.size());

        for (auto&& elem : value) {
            raw_vector.push_back(elem.get());
        }

        return raw_vector;
    }

    template<typename T>
    std::vector<const T*> to_raw(const std::vector<std::shared_ptr<T>>& value) {
        std::vector<const T*> raw_vector;
        raw_vector.reserve(value.size());

        for (auto&& elem : value) {
            raw_vector.push_back(elem.get());
        }

        return raw_vector;
    }

    template <typename T>
    void clean_and_delete_all_items(std::vector<T>& container) {
        for (auto& item : container) {
            delete item;
        }
        container.clear();
    }

    template<typename Container, typename Predicate>
    void remove_if(Container &container, Predicate predicate) {
        container.erase(std::remove_if(container.begin(), container.end(), predicate), container.end());
    }

    template<typename Container>
    void remove(Container &container, const typename Container::value_type &item) {
        remove_if(container, [&item](const typename Container::value_type &value) { return item == value; });
    }

    template<typename Container>
    auto pop_front(Container &container) -> typename std::iterator_traits<decltype(container.begin())>::value_type {
        if (container.empty()) {
            support::throw_exception<std::out_of_range>("Container is empty!");
        }

        auto result = *std::begin(container);
        container.erase(std::begin(container));
        return result;
    }

    template<typename Container>
    auto append(Container &container, const Container &other) -> decltype(std::begin(container)) {
        return container.insert(std::end(container), std::begin(other), std::end(other));
    }

}  // namespace support

