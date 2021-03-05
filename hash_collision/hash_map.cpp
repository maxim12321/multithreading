#include "hash_map.h"

HashMap::HashMap()
    : buckets_(kBucketCount),
      bucket_mutexes_(kMutexCount) {}

void HashMap::Insert(const HashString& value) {
  int64_t bucket_index = value.GetHash() % buckets_.size();
  int64_t mutex_index = bucket_index % bucket_mutexes_.size();

  std::lock_guard lock_guard(bucket_mutexes_[mutex_index]);
  buckets_[bucket_index].push_back(value);
}

std::string HashMap::Find(int64_t target_hash) {
  int64_t index = target_hash % buckets_.size();

  for (const auto& value : buckets_[index]) {
    if (value.GetHash() == target_hash) {
      return value.Get();
    }
  }
  return "";
}

void HashMap::Clear() {
  for (int index = 0; index < buckets_.size(); index++) {
    int64_t mutex_index = index % bucket_mutexes_.size();
    std::lock_guard lock_guard(bucket_mutexes_[mutex_index]);
    buckets_[index].clear();
  }
}
