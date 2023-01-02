/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#include <unordered_set>
#include <utility>

#include "support/network/http/HttpMonitor.h"
#include "support/network/http/IHttpRequest.h"
#include "support/network/http/IHttpResponse.h"
#include "support/network/http/HttpRequestError.h"

#include "support/threading/Monitor.h"

using support::Monitor;
using support::HttpMonitorObserver;
using support::HttpMonitor;
using support::IHttpRequest;
using support::IHttpResponse;
using support::HttpRequestError;

using Observers = std::unordered_set<HttpMonitorObserver*>;
using InsertObserverMethod = std::pair<Observers::iterator, bool>(Observers::*)(const Observers::value_type&);
using EraseObserverMethod = Observers::size_type(Observers::*)(const Observers::key_type& __k);
static Monitor<Observers> _observers;

void HttpMonitor::add_observer(HttpMonitorObserver* observer) {
    _observers.call(static_cast<InsertObserverMethod>(&Observers::insert), observer);
}

void HttpMonitor::remove_observer(HttpMonitorObserver* observer) {
    _observers.call(static_cast<EraseObserverMethod>(&Observers::erase), observer);
}

void HttpMonitor::notify_request_performed(const IHttpRequest& request, int status) {
    _observers.exec([&](Observers& observers){
        if (!observers.empty()) {
            const auto observers_copy = observers;
            for (auto observer : observers_copy) {
                observer->on_request_performed(request, status);
            }
        }
    });
}

void HttpMonitor::notify_response_received(const HttpRequestError& error, const IHttpResponse& response) {
    _observers.exec([&](Observers& observers){
        if (!observers.empty()) {
            const auto observers_copy = observers;
            for (auto observer : observers_copy) {
                observer->on_response_received(response, error);
            }
        }
    });
}

