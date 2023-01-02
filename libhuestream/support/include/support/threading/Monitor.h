/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#pragma once

#include <functional>
#include <mutex>
#include <memory>
#include <utility>

namespace support {

    /**
     Generic Monitor. Monitor ensures exclusive access to the object resources
     @param Class class of the instance that should be monitored
     @param Mutex mutex type that should be used for monitoring of the object
     */
    template <typename Class, typename Mutex = std::recursive_mutex>
    class Monitor {
    private:
        std::unique_ptr<Class> _obj;
        Class& _ref;
        mutable Mutex _mutex;
        
        // operator() locks critical section and calls function taken as input argument
        template <typename F>
        auto operator()(F f) const -> decltype(f(_ref)) {
            std::lock_guard<Mutex> hold{_mutex};
            return f(_ref);
        }

    public:
        /**
         Constructs monitor for new instance from monitored class.
         */
        Monitor() : _obj(new Class), _ref(*_obj.get()) {}

        /**
         Constructs monitor for certain instance of monitored class.
         Monitor takes ownership to the passed instance
         @param instance instance that should be monitored
         */
        explicit Monitor(Class* instance) : _obj(instance), _ref(*_obj.get()) {}

        /**
         Constructs monitor for referenced object.
         Ownership in this case is left to the user.
         @param ref The object that needs to be monitored
         */
        explicit Monitor(Class& ref) : _ref(ref) {}

        /**
         Locks mutex and calls method from monitored object.
         @param method pointer to the method that should be called
         */
        template <typename Method>
        auto call(Method method) -> decltype(std::bind(method, std::placeholders::_1)(_ref)) {
            return (*this)(std::bind(method, std::placeholders::_1));
        }

        /**
         Locks mutex and calls method from monitored object.
         @param method pointer to the method that should be called
         @param args method arguments
         */
        template <typename Method, typename... Args>
        auto call(Method method, Args&&... args) -> decltype(std::bind(method, std::placeholders::_1, std::forward<Args>(args)...)(_ref)) {
            return (*this)(std::bind(method, std::placeholders::_1, std::forward<Args>(args)...));
        }
        
        /**
         Locks mutex and calls a function passed as input argument.
         @param task runnable object that gets reference to the Class object
         */
        void exec(std::function<void(Class&)> task) {
            std::lock_guard<Mutex> hold{_mutex};
            task(_ref);
        }
    };

}  // namespace support

