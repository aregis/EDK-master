/*******************************************************************************
Copyright © 2016 Philips Lighting Holding B.V.
All Rights Reserved.
********************************************************************************/

#ifndef EDTLS_SERVER_MESSAGEDISPATCHER_H_
#define EDTLS_SERVER_MESSAGEDISPATCHER_H_

#include <edtls/server/Queue.h>
#include <edtls/server/Semaphore.h>

#include <thread>
#include <memory>

struct MessageInfo {
  MessageInfo(): function_(), stopped_() {}
  std::function<void()> function_;
  Semaphore stopped_;
};

class MessageDispatcher {
 private:
  SafeQueue<std::shared_ptr<MessageInfo>> messageQueue_;
  bool isRunning_;
  std::thread *workerThread_;

 public:
  MessageDispatcher();
  virtual ~MessageDispatcher();
  MessageDispatcher(const MessageDispatcher&) = delete;
  MessageDispatcher& operator=(const MessageDispatcher&) = delete;
  void execBlocked(std::function<void()> function);
  void exec(std::function<void()> function);
  void dispatch();
};

#endif  // EDTLS_SERVER_MESSAGEDISPATCHER_H_
