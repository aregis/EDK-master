/*******************************************************************************
Copyright (C) 2019 Signify Holding
All Rights Reserved.
********************************************************************************/

#pragma once

#include <vector>
#include <map>
#include <utility>
#include <chrono>

namespace support {
    namespace detail {
        class TaskSchedule {
        public:
            using Task = std::pair<std::function<void()>, bool>;
            using TaskContainer = std::vector<Task>;
            using TaskContainerMap = std::map<std::chrono::steady_clock::time_point, TaskContainer>;

            void add_task(std::chrono::steady_clock::time_point time_point, std::function<void()> invocable, bool is_cancelable = true) {
                _tasks[time_point].push_back(std::make_pair(invocable, is_cancelable));
            }

            TaskContainer filter_and_erase_tasks(std::chrono::steady_clock::time_point time_point) {
                TaskContainer return_value;

                const auto upper_bound = _tasks.upper_bound(time_point);
                for (auto iter = _tasks.begin(); iter != upper_bound; ++iter) {
                    return_value.insert(std::end(return_value), std::begin(iter->second), std::end(iter->second));
                }
                _tasks.erase(std::begin(_tasks), upper_bound);
                return return_value;
            }

            size_t num_of_tasks() const {
                size_t return_value = 0;

                for (auto&& item : _tasks) {
                    return_value += item.second.size();
                }

                return return_value;
            }

        private:
            TaskContainerMap _tasks;
        };
    }  // namespace detail
}  // namespace support