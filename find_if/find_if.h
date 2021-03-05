#pragma once

#include <algorithm>
#include <cstdint>
#include <functional>
#include <mutex>
#include <thread>
#include <vector>

#include "../utilities.h"

void CheckValue(int64_t value, const std::function<bool(int64_t)>& predicate,
                std::vector<int64_t>* result, std::mutex* result_mutex);

void CheckSegment(int64_t lower_bound, int64_t upper_bound,
                  const std::function<bool(int64_t)>& predicate,
                  std::vector<int64_t>* result, std::mutex* result_mutex);

void CheckValuesBySegments(int64_t lower_bound, int64_t upper_bound,
                           const std::function<bool(int64_t)>& predicate,
                           uint8_t concurrency, std::vector<int64_t>* result);

std::vector<int64_t> FindIf(int64_t lower_bound, int64_t upper_bound,
                            const std::function<bool(int64_t)>& predicate,
                            uint8_t concurrency = 1);
