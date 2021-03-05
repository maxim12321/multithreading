#pragma once

#include <mutex>
#include <string>
#include <utility>
#include <vector>

#include "hash_string.h"

class HashMap {
 public:
  HashMap();

  void Insert(const HashString& value);
  std::string Find(int64_t target_hash);

  void Clear();

 private:
  const int kBucketCount = 300'000;
  const int kMutexCount = 300;

 private:
  std::vector<std::vector<HashString>> buckets_;
  std::vector<std::mutex> bucket_mutexes_;
};
