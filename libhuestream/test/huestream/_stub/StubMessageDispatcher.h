/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#ifndef LIBHUESTREAM_MESSAGEDISPATCHERSTUB_H
#define LIBHUESTREAM_MESSAGEDISPATCHERSTUB_H

#include <huestream/connect/IMessageDispatcher.h>
#include <vector>

using std::vector;
using huestream::IMessageDispatcher;


class StubMessageDispatcher : public IMessageDispatcher {
 public:
  StubMessageDispatcher():
      _queue(), _isProcessingQueue(false) {
  }

  void Execute(bool) override {

  }

  void Stop() override {

  }

  vector<DispatchAction> _queue;

  void Queue(DispatchAction action) override {
    _queue.insert(_queue.begin(), action);
  }

  void ExecuteOnePendingAction() {
    if (!_queue.empty()) {
      _isProcessingQueue = true;
      auto action = _queue.back();
      _queue.pop_back();
      action();
    }
    _isProcessingQueue = false;
  }
    void ExecutePendingActions(){
    while (!_queue.empty()) {
      _isProcessingQueue = true;
      auto action = _queue.back();
      _queue.pop_back();
      action();
    }
    _isProcessingQueue = false;
  }

  bool _isProcessingQueue;
};


#endif  // LIBHUESTREAM_MESSAGEDISPATCHERSTUB_H
