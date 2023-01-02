/*******************************************************************************
Copyright © 2016 Philips Lighting Holding B.V.
All Rights Reserved.
********************************************************************************/

#include <edtls/server/MessageDispatcher.h>

#include <memory>

MessageDispatcher::MessageDispatcher() :
    messageQueue_(),
    isRunning_(true),
    workerThread_(new std::thread([=]() {
      while (isRunning_) {
        dispatch();
      }
    })) {}

MessageDispatcher::~MessageDispatcher() {
  isRunning_ = false;
  exec([=] {});
  workerThread_->join();
  delete workerThread_;
}

void MessageDispatcher::execBlocked(std::function<void()> function) {
  auto info = std::make_shared<MessageInfo>();

  info->function_ = function;

  if (std::this_thread::get_id() == workerThread_->get_id()) {
    info->function_();
  } else {
    messageQueue_.enqueue(info);
    info->stopped_.wait();
  }
}

void MessageDispatcher::exec(std::function<void()> function) {
  auto info = std::make_shared<MessageInfo>();

  info->function_ = function;
  messageQueue_.enqueue(info);
}

void MessageDispatcher::dispatch() {
  auto info = messageQueue_.dequeue();

  info->function_();
  info->stopped_.notify();
}
