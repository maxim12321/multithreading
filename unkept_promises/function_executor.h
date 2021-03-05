#pragma once

#include <condition_variable>
#include <memory>
#include <thread>

template<typename ResultType>
class FunctionExecutor {
 public:
  ~FunctionExecutor();

  void Execute(std::function<ResultType()> function);

  ResultType Wait();

 private:
  std::shared_ptr<ResultType> result_;
  std::mutex result_mutex_;
  std::condition_variable cv_;

  std::exception_ptr exception_;

  bool is_result_ready_ = false;
  std::thread thread_;
};

template<typename ResultType>
void
FunctionExecutor<ResultType>::Execute(std::function<ResultType()> function) {
  thread_ = std::thread([this, function]() {
    std::unique_lock<std::mutex> unique_lock(result_mutex_);

    try {
      result_ = std::make_shared<ResultType>(function());
    } catch (const std::exception& exception) {
      exception_ = std::current_exception();
    }
    is_result_ready_ = true;

    unique_lock.unlock();
    cv_.notify_one();
  });
}

template<>
void FunctionExecutor<void>::Execute(std::function<void()> function) {
  thread_ = std::thread([this, function]() {
    std::unique_lock<std::mutex> unique_lock(result_mutex_);

    try {
      function();
    } catch (const std::exception& exception) {
      exception_ = std::current_exception();
    }
    is_result_ready_ = true;

    unique_lock.unlock();
    cv_.notify_one();
  });
}

template<typename ResultType>
ResultType FunctionExecutor<ResultType>::Wait() {
  std::unique_lock<std::mutex> unique_lock(result_mutex_);
  cv_.wait(unique_lock, [this]() { return is_result_ready_; });

  if (exception_ != nullptr) {
    std::rethrow_exception(exception_);
  }
  return *result_;
}

template<>
void FunctionExecutor<void>::Wait() {
  std::unique_lock<std::mutex> unique_lock(result_mutex_);
  cv_.wait(unique_lock, [this]() { return is_result_ready_; });

  if (exception_ != nullptr) {
    std::rethrow_exception(exception_);
  }
}

template<typename ResultType>
FunctionExecutor<ResultType>::~FunctionExecutor() {
  if (thread_.joinable()) {
    thread_.join();
  }
}
