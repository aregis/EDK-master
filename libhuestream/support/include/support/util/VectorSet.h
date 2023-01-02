/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#pragma once

#include <functional>
#include <iterator>
#include <vector>

#include "boost/range/algorithm/find_if.hpp"
#include "boost/range/distance.hpp"

namespace support {
    namespace duplicates {
        enum class Policy {
            OVERWRITE_OLD_WITH_NEW,
            REMOVE_OLD_AND_APPEND_NEW
        };
    }  // namespace duplicates

    /* Defines a set that keeps the insertion order. Dealing with duplicates is handled through a comparison function
     * in combination with a policy */
    template <typename T, duplicates::Policy PolicyType = duplicates::Policy::OVERWRITE_OLD_WITH_NEW>
    class VectorSet {
    public:
        using iterator = typename std::vector<T>::iterator;
        using const_iterator = typename std::vector<T>::const_iterator;
        iterator begin() { return _v.begin(); }
        iterator end() { return _v.end(); }
        const_iterator begin() const { return _v.begin(); }
        const_iterator end() const { return _v.end(); }
        const T& front() const { return _v.front(); }
        const T& back() const {return _v.back(); }
        bool empty() const { return _v.empty(); }
        std::size_t size() const { return _v.size(); }

        explicit VectorSet(const std::function<bool(T, T)>& comparison_function = [](T t1, T t2) { return t1 == t2; })
                : _comparison_function(comparison_function)
        {}

        // Linear insertion
        void insert(const T& item) {
            using boost::range::find_if;

            auto is_duplicate_lambda = [&_comparison_function = this->_comparison_function, &item](const T& elem) {
                return _comparison_function(item, elem);
            };

            auto it = find_if(_v, is_duplicate_lambda);
            if (it != _v.end()) {
                if (PolicyType == duplicates::Policy::OVERWRITE_OLD_WITH_NEW) {
                    _v.at(std::distance(_v.begin(), it)) = item;
                } else {
                    _v.erase(_v.begin() + std::distance(_v.begin(), it));
                    _v.push_back(item);
                }
            } else {
                _v.push_back(item);
            }
        }

        std::vector<T> get() const {
            return _v;
        }

    private:
        std::vector<T> _v;
        std::function<bool(T, T)> _comparison_function;
    };
}  // namespace support
