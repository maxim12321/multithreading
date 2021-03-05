#include "gtest.h"

#include "promise.h"

TEST(PromiseConstructor, FunctionCalled) {
  std::atomic<bool> is_function_called(false);
  auto function = [&is_function_called]() {
    is_function_called.store(true);
  };
  Promise<void> promise(function);
  std::this_thread::sleep_for(std::chrono::milliseconds(100));
  ASSERT_TRUE(is_function_called.load());
}

TEST(PromiseConstructor, NotWaitingForResult) {
  std::atomic<bool> has_function_returned(false);
  auto function = [&has_function_returned]() {
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    has_function_returned.store(true);
  };
  Promise<void> promise(function);
  std::this_thread::sleep_for(std::chrono::milliseconds(100));
  ASSERT_FALSE(has_function_returned.load());
}

TEST(PromiseWait, WaitingForResult) {
  std::atomic<bool> has_function_returned(false);
  auto function = [&has_function_returned]() {
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    has_function_returned.store(true);
  };
  Promise<void> promise(function);
  promise.Wait();
  ASSERT_TRUE(has_function_returned.load());
}

TEST(PromiseWait, ResultIsReturned) {
  auto function = []() -> int {
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    return 42;
  };

  Promise<int> promise(function);
  ASSERT_EQ(42, promise.Wait());
}

TEST(PromiseWait, ResultBeforeWait) {
  auto function = []() -> int {
    return 42;
  };

  Promise<int> promise(function);
  std::this_thread::sleep_for(std::chrono::milliseconds(300));
  ASSERT_EQ(42, promise.Wait());
}

TEST(PromiseWait, StrangeResult) {
  auto function = []() {
    auto value = std::vector<std::string>({"hell", "the", "What"});
    std::swap(value[0], value[2]);
    return value;
  };

  Promise<std::vector<std::string>> promise(function);
  std::vector<std::string> result = promise.Wait();
  ASSERT_EQ(std::vector<std::string>({"What", "the", "hell"}), result);
}

TEST(MakePromise, SimpleVoid) {
  auto function = []() {
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  };
  auto promise = MakePromise(function);
  promise->Wait();
}

TEST(MakePromise, Simple) {
  auto function = []() -> int {
    return 42;
  };
  auto promise = MakePromise(function);
  ASSERT_EQ(42, promise->Wait());
}

TEST(MakePromise, StrangeType) {
  auto function = []() {
    return std::make_tuple(std::vector<char>('1', '2'), std::string("hi"), 42);
  };
  auto promise = MakePromise(function);
  ASSERT_EQ(std::make_tuple(std::vector<char>('1', '2'), "hi", 42),
            promise->Wait());
}

TEST(Promise, CopyConstructorFlex) {
  auto promise = Promise<Promise<Promise<int>>>([] {
    return Promise<Promise<int>>([] {
      return Promise<int>([] {
        return 42;
      });
    });
  });
  ASSERT_EQ(42, promise.Wait().Wait().Wait());
}

TEST(Promise, CopyConstructorFlex2) {
  auto promise = MakePromise([] {
    return MakePromise([] {
      return MakePromise([] {
        return 42;
      });
    });
  });
  ASSERT_EQ(42, promise->Wait()->Wait()->Wait());
}

TEST(Promise, CopyConstructorFlex3) {
  auto promise = MakePromise([] {
    return MakePromise([] {
      return MakePromise([] {
        return 42;
      })->Wait();
    });
  });
  ASSERT_EQ(42, promise->Wait()->Wait());
}

TEST(PromiseConstructor, ExceptionCaught) {
  auto function = []() {
    throw std::runtime_error("Oops");
  };
  ASSERT_NO_THROW(MakePromise(function));
}

TEST(PromiseWait, ExceptionThrown) {
  auto function = []() {
    throw std::runtime_error("Oops");
  };
  auto promise = MakePromise(function);
  ASSERT_THROW(promise->Wait(), std::runtime_error);
}

TEST(PromiseWait, CorrectExceptionCaught) {
  auto function = []() {
    throw std::logic_error("Logic error: no logic found");
  };
  auto promise = MakePromise(function);

  bool is_exception_caught = false;
  try {
    promise->Wait();
    ASSERT_TRUE(false);
  } catch (const std::logic_error& exception) {
    ASSERT_STREQ("Logic error: no logic found", exception.what());
    is_exception_caught = true;
  }
  ASSERT_TRUE(is_exception_caught);
}

TEST(PromiseThen, NotWaitingForResult) {
  std::atomic<bool> has_function_returned(false);
  auto function = [&has_function_returned]() {
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    has_function_returned.store(true);
  };
  auto promise = MakePromise(function)
      ->Then(function);

  std::this_thread::sleep_for(std::chrono::milliseconds(100));
  ASSERT_FALSE(has_function_returned.load());
  promise->Wait();
}

TEST(PromiseThen, CorrectReturn) {
  auto promise = MakePromise([]() -> int { return 42; })
      ->Then([](int x) { return x / 2; })
      ->Then([](int x) { return std::string(std::to_string(x)); });

  ASSERT_STREQ("21", promise->Wait().c_str());
}

TEST(PromiseThen, Wait) {
  std::atomic<bool> has_function_returned(false);
  auto function = [&has_function_returned](int x) {
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    has_function_returned.store(true);
  };
  auto promise = MakePromise([]() -> int { return 42; })
      ->Then(function);

  std::this_thread::sleep_for(std::chrono::milliseconds(100));
  ASSERT_FALSE(has_function_returned.load());
  promise->Wait();
  ASSERT_TRUE(has_function_returned.load());
}

TEST(PromiseThen, VoidFunction) {
  std::atomic<bool> has_function_returned(false);
  auto promise = MakePromise([&has_function_returned]() {
    has_function_returned.store(true);
  })
      ->Then([]() { return 42; });

  ASSERT_EQ(42, promise->Wait());
  ASSERT_TRUE(has_function_returned.load());
}

TEST(PromiseThen, Chain) {
  auto promise = MakePromise([] {
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    return 12;
  })
      ->Then([](int value) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
      })
      ->Then([] {
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        return 42;
      })
      ->Then([](int value) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        return value;
      });

  ASSERT_EQ(42, promise->Wait());
}

TEST(PromiseThen, VoidChain) {
  auto promise = MakePromise([] { return 12; })
      ->Then([](int value) {})
      ->Then([] { return 42; })
      ->Then([](int value) { return value; });

  ASSERT_EQ(42, promise->Wait());
}

TEST(PromiseCatch, NotWaitingForResult) {
  std::atomic<bool> has_function_returned(false);
  auto function = [&has_function_returned]() {
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    has_function_returned.store(true);
  };
  auto promise = MakePromise(function)
      ->Catch([](const std::exception& e) {});

  std::this_thread::sleep_for(std::chrono::milliseconds(100));
  ASSERT_FALSE(has_function_returned.load());
}

TEST(PromiseCatch, ExceptionStopped) {
  auto promise = MakePromise([] {
    throw std::runtime_error("Error");
  })
      ->Catch([](const std::exception& e) {});
  ASSERT_NO_THROW(promise->Wait());
}

TEST(PromiseCatch, CatchNotCalled) {
  std::atomic<bool> has_exception_handled(false);
  auto promise = MakePromise([] {})
      ->Catch([&has_exception_handled](const std::exception& e) {
        has_exception_handled.store(true);
      });

  ASSERT_NO_THROW(promise->Wait());
  ASSERT_FALSE(has_exception_handled.load());
}

TEST(PromiseCatch, ExceptionHandled) {
  std::atomic<bool> has_exception_handled(false);
  auto promise = MakePromise([] {
    throw std::runtime_error("Error");
  })
      ->Catch([&has_exception_handled](const std::exception& e) {
        has_exception_handled.store(true);
      });

  promise->Wait();
  ASSERT_TRUE(has_exception_handled.load());
}

std::shared_ptr<Promise<bool>> TestPromise(const std::string& value) {
  auto promise = MakePromise([value]() -> std::string {
    return value;
  })
      ->Then([](const std::string& str) -> int {
        if (str == "Hello") {
          throw std::invalid_argument("No hello here!");
        }
        if (str == "World") {
          throw std::logic_error("Something is wrong");
        }
        return 0;
      })
      ->Catch([](const std::exception& e) -> int {
        return 401;
      })
      ->Catch([](const std::exception& e) -> int {
        return 404;
      })
      ->Then([](int exit_code) -> std::string {
        if (exit_code == 0) {
          return "OK";
        }
        if (exit_code == 401) {
          return "401 Error!";
        }
        return "Some other error";
      })
      ->Then([](const std::string& str) -> bool {
        if (str == "OK") {
          return true;
        }
        throw std::runtime_error(str);
      })
      ->Catch([](const std::exception& e) -> bool {
        std::cout << "Exception: " << e.what() << std::endl;
        return false;
      });
  return promise;
}

TEST(PromiseThenCatch, StrangeChain) {
  ASSERT_FALSE(TestPromise("Hello")->Wait());
  ASSERT_FALSE(TestPromise("World")->Wait());
  ASSERT_TRUE(TestPromise("Else")->Wait());
}

int ParseInt(const std::string& s) {
  int result = 0;
  for (char ch : s) {
    if (ch < '0' || ch > '9') {
      throw std::runtime_error("Couldn't parse int: " + s);
    }
    result = result * 10 + (ch - '0');
  }
  return result;
}

int64_t ExpensiveComputation(int x) {
  return x * (x - 1) * (x - 2);
}

auto PromiseCreator(const std::string& value) {
  return MakePromise([value] {
    return value;
  })
      ->Then(ParseInt)
      ->Then(ExpensiveComputation)
      ->Catch([](const std::exception& e) {
        return 0;
      });
}

TEST(PromiseThenCatch, Simple) {
  ASSERT_EQ(0, PromiseCreator("123a")->Wait());
  ASSERT_EQ(0, PromiseCreator("123O")->Wait());
  ASSERT_EQ(0, PromiseCreator("-7")->Wait());
  ASSERT_EQ(ExpensiveComputation(1230), PromiseCreator("1230")->Wait());
  ASSERT_EQ(ExpensiveComputation(0), PromiseCreator("0")->Wait());
  ASSERT_EQ(ExpensiveComputation(0), PromiseCreator("")->Wait());
}
