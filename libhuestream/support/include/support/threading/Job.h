/*******************************************************************************
Copyright (C) 2019 Signify Holding
All Rights Reserved.
********************************************************************************/

#pragma once

#include <algorithm>
#include <condition_variable>
#include <vector>
#include <memory>
#include <mutex>
#include <type_traits>
#include <utility>

#include "support/threading/QueueExecutor.h"

namespace support {

    template <typename TTask>
    class Job;

    template <typename TTask>
    class NestedJob;

    /**
     * Task abstracts an asynchronous operation.
     * User should extend JobTask and implement task logic inside execute() method.
     * Actual task execution is handled by the Job object. @see create_job()
     * Optionally, stop() method can be implemented to support task cancellation.
     */
    class JobTask {
    public:
        using CompletionHandler = std::function<void()>;
        virtual ~JobTask() = default;

        /**
         * User should override method to implement task logic.
         * @param completion_handler Callback to be called by the implementation when task has been completed
         */
        virtual void execute(CompletionHandler completion_handler) = 0;
        /**
         * User should override method to implement task cancellation logic.
         * Note, that method is called on the same thread as execute(). Thus long running task should use
         * external thread pool or own thread.
         * There will be no affect if completion_handler is called after stop() has been requested.
         */
        virtual void stop() {}

    protected:
        /**
         * Creates the job to process nested task inside the execute() implementation.
         * @tparam TTask type of the task to be processed
         * @tparam TArgs argument types required for the task creation
         * @param args arguments required for the task creation
         * @return pointer to the created job
         */
        template <typename TTask, typename... TArgs>
        std::unique_ptr<NestedJob<TTask>> create_job(TArgs&&... args);

        /**
         * Creates the job to process nested task inside the execute() implementation.
         * @tparam TTask type of the task to be processed
         * @param task task to be controlled by the Job
         * @return pointer to the created job
         */
        template <typename TTask>
        std::unique_ptr<NestedJob<TTask>> create_job(std::unique_ptr<TTask> task);

    private:
        template <typename TTask>
        friend class Job;

        using BaseCallback = std::function<void(JobTask*)>;
        using StartTaskFunction = std::function<void(std::shared_ptr<JobTask>, BaseCallback)>;

        void set_task_runner(const StartTaskFunction& task_runner);
        StartTaskFunction _task_runner;
    };

    /**
     * Creates the task and the job which will control the task execution process.
     * @tparam TTask type of the task to be processed
     * @tparam TArgs argument types required for the task creation
     * @param args arguments required for the task creation
     * @return pointer to the created job
     */
    template <typename TTask, typename... TArgs>
    std::unique_ptr<Job<TTask>> create_job(TArgs&&... args);

    /**
     * Creates the task and the job which will control the task execution process.
     * @tparam TTask type of the task to be processed
     * @tparam TTaskOut type of the task to be used for the Job interface. TTask should be derived from TTaskOut
     * @tparam TArgs argument types required for the task creation
     * @param args arguments required for the task creation
     * @return pointer to the created job
     */
    template <typename TTask, typename TTaskOut, typename... TArgs>
    std::unique_ptr<Job<TTaskOut>> create_job(TArgs&&... args);

    /**
     * Wraps the task into the job which will control the task execution process.
     * @tparam TTask type of the task to be processed
     * @param task task to be controlled by the Job
     * @return pointer to the created job
     */
    template <typename TTask>
    std::unique_ptr<Job<TTask>> create_job(std::unique_ptr<TTask> task);

    enum class JobState {
        ReadyToRun,
        Running,
        Dispatching,
        Completed,
        Canceling,
        Canceled
    };

    /**
     * Job controls the execution of the Task object.
     * Job wraps the task and provides subset of the methods to control the task execution.
     * @tparam TTask type of the task to be processed
     */
    template <typename TTask>
    class Job {
    public:
        /**
         * Destructor. Cancels the task if it is running.
         */
        ~Job();

        Job(const Job&) = delete;
        Job& operator=(const Job&) = delete;

        /**
         * Starts async task execution. Job can be started only once.
         * @return true if call to run caused job to start
         */
        bool run();
        /**
         * Starts async task execution. Job can be started only once.
         * @param callback Callback to be called when task is completed. Executed on the same thread as the task.
         * @return true if call to run caused job to start
         */
        bool run(std::function<void(TTask*)> callback);

        /**
         * Cancels tasks execution.
         * Calls stop on each of the running tasks and blocks until processing is finished.
         * @return true if call caused the job to enter canceled state.
         */
        bool cancel();
        /**
         * Blocks until all processing is finished.
         * @return state of the job after method has completed
         */
        JobState wait();

        /**
         * Gets the processed task. Call waits for the job to be finished.
         * If job was not previously started, starts the job.
         * @return pointer to the processed task
         */
        std::shared_ptr<TTask> get();

        /**
         * Returns current state of the job
         * @return state of the job
         */
        JobState state() const;

    private:
        template<typename T, typename... TArgs>
        friend std::unique_ptr<Job<T>> create_job(TArgs&&... args);

        template<typename T>
        friend std::unique_ptr<Job<T>> create_job(std::unique_ptr<T>);

        template<typename T, typename U, typename... TArgs>
        friend std::unique_ptr<Job<U>> create_job(TArgs&&... args);


        explicit Job(std::shared_ptr<TTask> task, std::shared_ptr<OperationalQueue> queue);

        bool run(std::shared_ptr<JobTask> task, std::function<void(JobTask*)> callback);
        std::function<void()> wrap_task_execution(std::shared_ptr<JobTask> task, std::function<void(JobTask*)> callback);


        std::shared_ptr<QueueExecutor> _executor;

        struct TaskDetails {
            std::weak_ptr<JobTask> task;
            bool done = false;
            bool execute_finished = false;
            bool started = false;
        };

        struct OperationState {
            std::vector<TaskDetails> task_details;
            std::mutex mutex;
            std::condition_variable cv;
            JobState job_state = JobState::ReadyToRun;

            void set_task_done(std::shared_ptr<JobTask> task);
            void set_task_execute_finished(std::shared_ptr<JobTask> task);
        };

        std::shared_ptr<OperationState> _state = std::make_shared<OperationState>();
        std::vector<std::shared_ptr<JobTask>> _tasks;
        std::shared_ptr<JobTask> _main_task;
    };

    /**
     * Job controls the execution of the task created withing a context of another task (nested).
     * @tparam TTask type of the nested task to be executed
     */
    template <typename TTask>
    class NestedJob {
    public:
        /**
         * Starts async task execution. Job can be started only once.
         * @param callback Callback to be called when task is completed. Executed on the same thread as the task.
         */
        void run(std::function<void(TTask*)> callback);

    private:
        friend class JobTask;

        using BaseCallback = std::function<void(JobTask*)>;
        using StartTaskFunction = std::function<void(std::shared_ptr<JobTask>, BaseCallback)>;

        explicit NestedJob(std::shared_ptr<TTask> task, StartTaskFunction task_runner);

        StartTaskFunction _task_runner;
        std::shared_ptr<TTask> _task;
    };



    template<typename TTask, typename... TArgs>
    std::unique_ptr<Job<TTask>> create_job(TArgs&&... args) {
        auto task = std::make_shared<TTask>(std::forward<TArgs>(args)...);
        return std::unique_ptr<Job<TTask>>(new Job<TTask>(task, GlobalQueueExecutor::get()->get_operational_queue()));
    }

    template<typename TTask>
    std::unique_ptr<Job<TTask>> create_job(std::unique_ptr<TTask> task) {
        std::shared_ptr<TTask> shared_task = std::move(task);
        return std::unique_ptr<Job<TTask>>(new Job<TTask>(shared_task, GlobalQueueExecutor::get()->get_operational_queue()));
    }

    template<typename TTask, typename TTaskOut, typename... TArgs>
    std::unique_ptr<Job<TTaskOut>> create_job(TArgs&&... args) {
        static_assert(std::is_base_of<TTaskOut, TTask>::value, "Job result task type should derive from worker task type");
        auto task = std::make_shared<TTask>(std::forward<TArgs>(args)...);
        return std::unique_ptr<Job<TTaskOut>>(new Job<TTaskOut>(task, GlobalQueueExecutor::get()->get_operational_queue()));
    }

    template<typename TTask>
    void Job<TTask>::OperationState::set_task_done(std::shared_ptr<JobTask> task) {
        std::lock_guard<decltype(mutex)> lock(mutex);

        auto it = std::find_if(std::begin(task_details), std::end(task_details), [task](const TaskDetails& details){
            return details.task.lock() == task;
        });

        if (it != std::end(task_details)) {
            it->done = true;
        }

        bool all_completed = std::all_of(std::begin(task_details), std::end(task_details), [] (const TaskDetails& t) {
            return t.done;
        });

        if (all_completed) {
            job_state = JobState::Completed;
        }

        cv.notify_all();
    }

    template<typename TTask>
    void Job<TTask>::OperationState::set_task_execute_finished(std::shared_ptr<JobTask> task) {
        std::lock_guard<decltype(mutex)> lock(mutex);

        auto it = std::find_if(std::begin(task_details), std::end(task_details), [task](const TaskDetails& details){
            return details.task.lock() == task;
        });

        if (it != std::end(task_details))
            it->execute_finished = true;
    }

    template<typename TTask>
    Job<TTask>::Job(std::shared_ptr<TTask> task, std::shared_ptr<OperationalQueue> queue)
            : _executor(std::make_shared<QueueExecutor>(queue))
            , _main_task(task) {
        _tasks.push_back(task);

        TaskDetails details;
        details.task = std::shared_ptr<JobTask>(task);
        _state->task_details.push_back(details);
    }

    template<typename TTask>
    Job<TTask>::~Job() {
        cancel();
    }

    template<typename TTask>
    JobState Job<TTask>::state() const {
        std::lock_guard<decltype(_state->mutex)> state_lock(_state->mutex);
        return _state->job_state;
    }

    template<typename TTask>
    bool Job<TTask>::cancel() {
        {
            std::lock_guard<decltype(_state->mutex)> state_lock(_state->mutex);
            if (_state->job_state != JobState::Running && _state->job_state != JobState::Canceling && _state->job_state != JobState::Dispatching) {
                return false;
            }

            if (_state->job_state != JobState::Canceling) {
                auto state = _state;
                _executor->execute([state]() {
                    std::vector<std::shared_ptr<JobTask>> tasks_to_cancel;

                    {
                        std::lock_guard<decltype(state->mutex)> state_lock(state->mutex);
                        for (const auto& d : state->task_details) {
                            if (!d.done && d.execute_finished) {
                                tasks_to_cancel.push_back(d.task.lock());
                            }
                        }
                    }

                    for (auto t : tasks_to_cancel) {
                        t->stop();
                    }
                }, QueueExecutor::OperationType::NON_CANCELABLE);
            }

            _state->job_state = JobState::Canceling;
            _state->cv.notify_all();
        }

        _executor->cancel_all();

        bool actually_canceled = false;
        {
            std::lock_guard<decltype(_state->mutex)> state_lock(_state->mutex);
            if (_state->job_state == JobState::Canceling) {
                _state->job_state = JobState::Canceled;
                actually_canceled = true;
                _executor->execute([]{}, QueueExecutor::OperationType::NON_CANCELABLE);  // notify about required break from recursive operational loop inside Job::Wait
            }

            _state->cv.notify_all();
        }

        _executor->cancel_all();
        return actually_canceled;
    }

    template<typename TTask>
    JobState Job<TTask>::wait() {
        {
            const auto is_idle = [this]() -> bool {
                return (_state->job_state != JobState::Running && _state->job_state != JobState::Canceling && _state->job_state != JobState::Dispatching);
            };

            auto queue = _executor->get_operational_queue();

            bool recursion_denied = !queue->run_recursive_process_tickets_loop([this, is_idle] () -> bool {
                std::lock_guard<decltype(_state->mutex)> state_lock(_state->mutex);
                return is_idle();
            });

            if (recursion_denied) {
                std::unique_lock<decltype(_state->mutex)> state_lock(_state->mutex);
                _state->cv.wait(state_lock, [is_idle] () -> bool {
                    return is_idle();
                });
            }
        }

        _executor->wait_all();

        std::lock_guard<decltype(_state->mutex)> state_lock(_state->mutex);
        return _state->job_state;
    }

    template<typename TTask>
    std::shared_ptr<TTask> Job<TTask>::get() {
        run();
        wait();
        return std::static_pointer_cast<TTask>(_tasks.front());
    }

    template<typename TTask>
    bool Job<TTask>::run(std::shared_ptr<JobTask> task, std::function<void(JobTask*)> callback) {
        std::lock_guard<decltype(_state->mutex)> state_lock(_state->mutex);
        if (_state->job_state != JobState::ReadyToRun && _state->job_state != JobState::Running) {
            return false;
        }

        auto it_existing = std::find_if(std::begin(_state->task_details), std::end(_state->task_details), [task](const TaskDetails& d){
            return d.task.lock() == task;
        });

        if (it_existing == std::end(_state->task_details)) {
            TaskDetails details;
            details.task = task;
            details.started = true;
            _state->task_details.push_back(details);
            _tasks.push_back(task);
        } else if (!it_existing->started) {
            it_existing->started = true;
        } else {
            return false;
        }

        _state->job_state = JobState::Running;

        const auto wrapped_task_execution = wrap_task_execution(task, std::move(callback));
        _executor->execute(std::move(wrapped_task_execution));

        _state->cv.notify_all();

        return true;
    }

    template <typename TTask>
    bool Job<TTask>::run() {
        std::function<void(TTask*)> callback;
        return run(callback);
    }

    template <typename TTask>
    bool Job<TTask>::run(std::function<void(TTask*)> callback) {
        const auto casting_callback = [callback](JobTask* task) {
            if (callback) {
                callback(static_cast<TTask*>(task));
            }
        };

        return run(_main_task, std::move(casting_callback));
    }

    template <typename TTask>
    std::function<void()> Job<TTask>::wrap_task_execution(std::shared_ptr<JobTask> task, std::function<void(JobTask*)> callback) {
        std::weak_ptr<QueueExecutor> weak_executor = _executor;
        std::weak_ptr<JobTask> weak_task = task;

        const bool is_main_task = _main_task == task;

        auto state = _state;

        const auto notify_done = [callback, weak_task, state, is_main_task] {
            auto task = weak_task.lock();
            if (task) {
                if (callback) {
                    if (is_main_task) {
                        std::lock_guard<decltype(state->mutex)> state_lock(state->mutex);
                        if (state->job_state != JobState::Canceling) {
                            state->job_state = JobState::Dispatching;
                            state->cv.notify_all();
                        }
                    }

                    callback(task.get());
                }

                state->set_task_done(task);
            }
        };

        const auto repost_callback = [weak_executor, notify_done, state] {
            std::lock_guard<decltype(state->mutex)> state_lock(state->mutex);
            if (state->job_state == JobState::Running || state->job_state == JobState::Dispatching) {
                auto executor = weak_executor.lock();
                if (executor) {
                    // using not cancelable execute here allows all the posted 'done' to be processed before the cancellation
                    // and avoid calling stop on tasks, that posted done before Job::cancel was requested
                    executor->execute([notify_done, state] {
                        notify_done();
                    }, QueueExecutor::OperationType::NON_CANCELABLE);
                }
            }
        };

        const auto task_runner_delegate = [this](std::shared_ptr<JobTask> t, const std::function<void(JobTask*)>& cb) {
            run(t, cb);
        };

        return [task, repost_callback, task_runner_delegate, state] {
            task->set_task_runner(task_runner_delegate);
            task->execute(repost_callback);
            state->set_task_execute_finished(task);
        };
    }

    template<typename TTask, typename... TArgs>
    std::unique_ptr<NestedJob<TTask>> JobTask::create_job(TArgs&& ... args) {
        const auto task = std::make_shared<TTask>(std::forward<TArgs>(args)...);
        return std::unique_ptr<NestedJob<TTask>>(new NestedJob<TTask>(task, _task_runner));
    }

    template<typename TTask>
    std::unique_ptr<NestedJob<TTask>> JobTask::create_job(std::unique_ptr<TTask> task) {
        std::shared_ptr<TTask> shared_task = std::move(task);
        return std::unique_ptr<NestedJob<TTask>>(new NestedJob<TTask>(shared_task, _task_runner));
    }

    inline
    void JobTask::set_task_runner(const StartTaskFunction& task_runner) {
        _task_runner = task_runner;
    }

    template <typename TTask>
    NestedJob<TTask>::NestedJob(std::shared_ptr<TTask> task, StartTaskFunction task_runner)
            : _task_runner(std::move(task_runner))
            , _task(task) {
    }

    template <typename TTask>
    void NestedJob<TTask>::run(std::function<void(TTask*)> callback) {
        if (!_task_runner) {
            return;
        }

        const auto casting_callback = [callback](JobTask* task) {
            if (callback) {
                callback(static_cast<TTask*>(task));
            }
        };

        _task_runner(_task, std::move(casting_callback));
    }

}  // namespace support

