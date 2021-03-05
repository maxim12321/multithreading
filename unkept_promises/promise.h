#pragma once

#define PROMISE_SUPPORTS_EXCEPTIONS
#define PROMISE_SUPPORTS_THEN

#include "function_executor.h"

#include <condition_variable>
#include <memory>
#include <thread>

template<typename ResultType>
class Promise : public std::enable_shared_from_this<Promise<ResultType>> {
 public:
  explicit Promise(std::function<ResultType()> function);

  ResultType Wait();

  template<typename Function>
  auto Then(Function function);

  template<typename HandlerType>
  std::shared_ptr<Promise<ResultType>> Catch(HandlerType handler);

 private:
  std::shared_ptr<FunctionExecutor<ResultType>> function_executor_;
};

template<>
class Promise<void> : public std::enable_shared_from_this<Promise<void>> {
 public:
  explicit Promise(std::function<void()> function);

  void Wait();

  template<typename Function>
  auto Then(Function function);

  template<typename HandlerType>
  std::shared_ptr<Promise<void>> Catch(HandlerType handler);

 private:
  std::shared_ptr<FunctionExecutor<void>> function_executor_;
};

template<typename Function>
auto MakePromise(Function function) {
  return std::make_shared<Promise<std::result_of_t<Function&()>>>(function);
}

template<typename ResultType>
Promise<ResultType>::Promise(std::function<ResultType()> function)
    : function_executor_(std::make_shared<FunctionExecutor<ResultType>>()) {
  function_executor_->Execute(function);
}

Promise<void>::Promise(std::function<void()> function)
    : function_executor_(std::make_shared<FunctionExecutor<void>>()) {
  function_executor_->Execute(function);
}

template<typename ResultType>
ResultType Promise<ResultType>::Wait() {
  return function_executor_->Wait();
}

void Promise<void>::Wait() {
  function_executor_->Wait();
}

template<typename ResultType>
template<typename Function>
auto Promise<ResultType>::Then(Function function) {
  auto promise = this->shared_from_this();
  return MakePromise([promise, function]() {
    return function(promise->Wait());
  });
}

template<typename Function>
auto Promise<void>::Then(Function function) {
  auto promise = this->shared_from_this();
  return MakePromise([promise, function]() {
    promise->Wait();
    return function();
  });
}

template<typename ResultType>
template<typename HandlerType>
std::shared_ptr<Promise<ResultType>>
Promise<ResultType>::Catch(HandlerType handler) {
  auto promise = this->shared_from_this();
  return MakePromise([promise, handler]() -> ResultType {
    try {
      return promise->Wait();
    } catch (const std::exception& e) {
      return handler(e);
    }
  });
}

template<typename HandlerType>
std::shared_ptr<Promise<void>> Promise<void>::Catch(HandlerType handler) {
  auto promise = this->shared_from_this();
  return MakePromise([promise, handler]() -> void {
    try {
      promise->Wait();
    } catch (const std::exception& e) {
      handler(e);
    }
  });
}
