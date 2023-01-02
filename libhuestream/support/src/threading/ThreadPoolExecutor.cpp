/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#include "support/threading/ThreadPoolExecutor.h"
#include "support/threading/ThreadPool.h"
#include "support/util/ExceptionUtil.h"

using support::ThreadPoolExecutor;
using support::ThreadPool;
using support::GlobalThreadPool;

class ThreadPoolExecutor::Operation : public support::IOperation {
public:
    enum class State {
        Idle,
        Running,
        Canceled
    };

    Operation(std::shared_ptr<std::mutex> mutex, ThreadPoolExecutor::OperationType operation_type)
        : _mutex(mutex)
        , _operation_type(operation_type) {}

    void wait() override {
        _future.wait();
    }

    bool is_cancelable() const override {
        return _operation_type == ThreadPoolExecutor::OperationType::CANCELABLE;
    }

    void cancel() override  {
        if (is_cancelable()) {
            std::lock_guard<std::mutex> lock{*_mutex};
            _state = State::Canceled;
        } else {
            support::throw_exception<std::runtime_error>("This operation is not cancelable.");
        }

        wait();
    }

    void set_future(std::future<void> future) {
        _future = std::move(future);
    }

    State get_state() const {
        return _state;
    }

    void set_state(State state) {
        _state = state;
    }

private:
    std::shared_ptr<std::mutex> _mutex;
    State _state = State::Idle;
    std::shared_future<void> _future;
    ThreadPoolExecutor::OperationType _operation_type;
};

ThreadPoolExecutor::ThreadPoolExecutor(ShutdownPolicy shutdown_policy)
        : ThreadPoolExecutor(support::GlobalThreadPool::get(), shutdown_policy) {}

ThreadPoolExecutor::ThreadPoolExecutor(std::shared_ptr<ThreadPool> thread_pool, ShutdownPolicy shutdown_policy)
        : _shared_data{std::make_shared<SharedData>()}
        , _thread_pool{std::move(thread_pool)}
        , _shutdown_policy{shutdown_policy} {}

ThreadPoolExecutor::~ThreadPoolExecutor() {
    shutdown(_shutdown_policy);
}

support::Operation ThreadPoolExecutor::execute(std::function<void()> invocable, OperationType operation_type) {
    std::lock_guard<std::mutex> lock{*_shared_data->_mutex};

    if (_is_shutdown) {
        return support::Operation{};
    }

    auto shared_data = _shared_data;
    auto operation = std::make_shared<ThreadPoolExecutor::Operation>(shared_data->_mutex, operation_type);
    operation->set_future(_thread_pool->add_task([shared_data, operation, invocable = std::move(invocable)] {
        {
            std::lock_guard<std::mutex> lock{*shared_data->_mutex};
            if (operation->get_state() == ThreadPoolExecutor::Operation::State::Canceled) return;
            operation->set_state(ThreadPoolExecutor::Operation::State::Running);
        }

        call_and_ignore_exception(invocable);

        {
            std::lock_guard<std::mutex> lock{*shared_data->_mutex};
            auto iter = std::find_if(shared_data->_operations.begin(), shared_data->_operations.end(), [operation](auto current) {
                return operation == current;
            });

            if (iter != shared_data->_operations.end()) {
                shared_data->_operations.erase(iter);
            }
        }
    }));

    _shared_data->_operations.emplace_back(std::move(operation));

    return support::Operation{_shared_data->_operations.back()};
}

std::vector<std::shared_ptr<ThreadPoolExecutor::Operation>> ThreadPoolExecutor::take_operations() {
    std::lock_guard<std::mutex> lock{*_shared_data->_mutex};
    std::vector<std::shared_ptr<ThreadPoolExecutor::Operation>> return_value;
    _shared_data->_operations.swap(return_value);
    return return_value;
}

void ThreadPoolExecutor::wait_all() {
    _waiting_condition.perform(support::operations::increment<int>);
    auto operations = take_operations();
    while (operations.size()) {
        for (auto&& operation : operations) {
            operation->wait();
        }
        operations = take_operations();
    }
    _waiting_condition.perform(support::operations::decrement<int>);
    _waiting_condition.wait(0);
}

void ThreadPoolExecutor::cancel_all() {
    _waiting_condition.perform(support::operations::increment<int>);
    auto operations = take_operations();
    while (operations.size()) {
        for (auto&& operation : operations) {
            if (operation->is_cancelable()) {
                operation->cancel();
            } else {
                operation->wait();
            }
        }
        operations = take_operations();
    }
    _waiting_condition.perform(support::operations::decrement<int>);
    _waiting_condition.wait(0);
}

void ThreadPoolExecutor::shutdown() {
    shutdown(ShutdownPolicy::CANCEL_ALL);
}

void ThreadPoolExecutor::shutdown(ShutdownPolicy shutdown_policy) {
    support::Subscription task_executed_subscription;
    {
        std::lock_guard<std::mutex> lock{*_shared_data->_mutex};
        task_executed_subscription = std::move(_task_executed_subscription);
        _is_shutdown = true;
    }
    task_executed_subscription = {};

    switch (shutdown_policy) {
        case ShutdownPolicy::CANCEL_ALL:
            cancel_all();
            break;
        case ShutdownPolicy::WAIT_ALL:
            wait_all();
            break;
        case ShutdownPolicy::KEEP_RUNNING:
            // do nothing
            break;
    }
}

std::shared_ptr<support::ThreadPool> ThreadPoolExecutor::get_thread_pool() const {
    return _thread_pool;
}

void ThreadPoolExecutor::schedule(std::chrono::steady_clock::time_point time_point, std::function<void()> invocable, OperationType operation_type /* = OperationType::CANCELABLE */) {
    std::lock_guard<std::mutex> lock{*_shared_data->_mutex};
    _shared_data->_task_schedule.add_task(std::move(time_point), std::move(invocable), operation_type == OperationType::CANCELABLE);

    if (_task_executed_subscription || _is_shutdown) return;

    _task_executed_subscription = _thread_pool->subscribe_for_tick_events([this] {
        {
            support::detail::TaskSchedule::TaskContainer tasks;
            {
                std::lock_guard<std::mutex> lock(*_shared_data->_mutex);
                tasks = _shared_data->_task_schedule.filter_and_erase_tasks(std::chrono::steady_clock::now());
            }
            for (auto&& task : tasks) {
                this->execute(task.first, task.second ? OperationType::CANCELABLE : OperationType::NON_CANCELABLE);
            }
        }
    });
}