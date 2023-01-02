/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#ifndef LIBHUESTREAM_MOCKSCHEDULER_H
#define LIBHUESTREAM_MOCKSCHEDULER_H

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <memory>

#include "support/scheduler/Scheduler.h"

namespace huestream {
    class MockScheduler : public support::Scheduler {
    public:
        MockScheduler() : MockScheduler(1000) {}
        MockScheduler(unsigned int interval) : Scheduler(interval) {
            auto replace_task = [this](support::SchedulerTask task){
                _task = std::move(task);
            };

            ON_CALL(*this, add_task(_)).WillByDefault(testing::Invoke(replace_task));
            ON_CALL(*this, add_task(_, _)).WillByDefault(testing::WithArgs<0>(testing::Invoke(replace_task)));
        }

        void execute_scheduled_callback() {
            if (_task.get_method()) {
                _task();
            }
        }

        MOCK_METHOD1(add_task, void(support::SchedulerTask task));

        MOCK_METHOD2(add_task, void(support::SchedulerTask task, milliseconds next_occurence_ms));

        MOCK_METHOD1(get_task, const support::SchedulerTask*(int id));

        MOCK_CONST_METHOD0(get_task_count, size_t());

        MOCK_METHOD1(remove_task, void(int id));

        MOCK_METHOD0(remove_all_tasks, void());

        MOCK_METHOD0(start, void());

        MOCK_METHOD0(stop, void());

        MOCK_CONST_METHOD0(is_running, bool());

    private:
        support::SchedulerTask _task;
    };

    class MockWrapperScheduler : public support::Scheduler {
    public:
        MockWrapperScheduler(unsigned int interval, std::shared_ptr<MockScheduler> mock) : Scheduler(interval), _mock(std::move(mock)) {}

        void add_task(support::SchedulerTask task) override {
            _mock->add_task(std::move(task));
        }

        void add_task(support::SchedulerTask task, milliseconds next_occurence_ms) override {
            _mock->add_task(std::move(task), next_occurence_ms);
        }

        const support::SchedulerTask *get_task(int id) override {
            return _mock->get_task(id);
        }

        size_t get_task_count() const override {
            return _mock->get_task_count();
        }

        void remove_task(int id) override {
            _mock->remove_task(id);
        }

        void remove_all_tasks() override {
            _mock->remove_all_tasks();
        }

        void start() override {
            _mock->start();
        }

        void stop() override {
            _mock->stop();
        }

        bool is_running() const override {
            return Scheduler::is_running();
        }

    private:
        std::shared_ptr<MockScheduler> _mock;
    };
}

#endif  // LIBHUESTREAM_MOCKSCHEDULER_H
