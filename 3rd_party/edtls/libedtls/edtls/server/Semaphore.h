/*******************************************************************************
Copyright © 2016 Philips Lighting Holding B.V.
All Rights Reserved.
********************************************************************************/

#ifndef EDTLS_SERVER_SEMAPHORE_H_
#define EDTLS_SERVER_SEMAPHORE_H_

#include <mutex>
#include <condition_variable>

class Semaphore {
 private:
  std::mutex mtx_;
  std::condition_variable cv_;
  int count_;

 public:
  explicit Semaphore(int count = 0) : mtx_(), cv_(), count_{count} {}

  void notify() {
    {
      std::unique_lock<std::mutex> lck(mtx_);
      ++count_;
    }
    cv_.notify_one();
  }

  void wait() {
    std::unique_lock<std::mutex> lck(mtx_);
    while (count_ == 0) {
      cv_.wait(lck);
    }
    --count_;
  }
};

#endif  // EDTLS_SERVER_SEMAPHORE_H_
