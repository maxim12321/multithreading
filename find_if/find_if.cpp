#include "find_if.h"

void CheckValue(int64_t value, const std::function<bool(int64_t)>& predicate,
                std::vector<int64_t>* result, std::mutex* result_mutex) {
  if (predicate(value)) {
    std::lock_guard guard(*result_mutex);
    result->push_back(value);
  }
}

void CheckSegment(int64_t lower_bound, int64_t upper_bound,
                  const std::function<bool(int64_t)>& predicate,
                  std::vector<int64_t>* result, std::mutex* result_mutex) {
  for (; lower_bound <= upper_bound; lower_bound++) {
    CheckValue(lower_bound, predicate, result, result_mutex);
  }
}

void CheckValuesBySegments(int64_t lower_bound, int64_t upper_bound,
                           const std::function<bool(int64_t)>& predicate,
                           uint8_t concurrency, std::vector<int64_t>* result) {
  auto segments = SplitIntoSegments(lower_bound, upper_bound, concurrency);

  std::mutex result_mutex;
  std::vector<std::thread> threads;

  threads.reserve(segments.size());
  for (const auto& segment : segments) {
    threads.emplace_back(CheckSegment, segment.first, segment.second,
                         predicate, result, &result_mutex);
  }

  for (auto& thread : threads) {
    thread.join();
  }
}

std::vector<int64_t> FindIf(int64_t lower_bound, int64_t upper_bound,
                            const std::function<bool(int64_t)>& predicate,
                            uint8_t concurrency) {
  if (concurrency == 0) {
    return {};
  }
  std::vector<int64_t> result;
  CheckValuesBySegments(lower_bound, upper_bound, predicate,
                        concurrency, &result);
  return result;
}
