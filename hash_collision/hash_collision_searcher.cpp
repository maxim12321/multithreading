#include "hash_collision_searcher.h"

HashCollisionSearcher::HashCollisionSearcher(int64_t power, int64_t module)
    : power_(power), module_(module), is_answer_found_(false) {}

std::string HashCollisionSearcher::FindCollision(const std::string& target,
                                                 int64_t string_length,
                                                 uint8_t concurrency) {
  target_ = target;
  hash_maps_.emplace_back();
  target_hash_ = Hash(target, power_, module_);
  is_answer_found_.store(false);

  GenerateAllStrings(string_length, concurrency);
  SearchForCollision(string_length, concurrency);

  return is_answer_found_.load() ? result_ : "";
}

bool HashCollisionSearcher::CheckString(const HashString& string) {
  int64_t shifted_hash = string.GetHash();

  for (int length = 0; length < hash_maps_.size(); length++) {
    int64_t right_hash = (target_hash_ - shifted_hash + module_) % module_;
    std::string right_string = hash_maps_[length].Find(right_hash);

    if (!right_string.empty() && string.Get() + right_string != target_) {
      if (!is_answer_found_.exchange(true)) {
        std::lock_guard lock_guard(result_mutex_);
        result_ = string.Get() + right_string;
      }
      return true;
    }

    shifted_hash = (__int128_t(shifted_hash) * power_) % module_;
  }
  return false;
}

void HashCollisionSearcher::CheckStrings(int64_t length,
                                         int64_t from, int64_t to) {
  HashString string(power_, module_, length, from);
  for (; from <= to; ++from, ++string) {
    if (CheckString(string) || is_answer_found_.load()) {
      return;
    }
  }
}

void HashCollisionSearcher::SearchForCollision(int64_t length,
                                               uint8_t concurrency) {
  int64_t max_value = BinaryPow(kAlphabetSize, length) - 1;

  auto segments = SplitIntoSegments(0, max_value, concurrency);

  std::vector<std::thread> threads;

  threads.reserve(segments.size());
  for (const auto& segment : segments) {
    threads.emplace_back([this, length, segment] {
      CheckStrings(length, segment.first, segment.second);
    });
  }

  for (auto& thread : threads) {
    thread.join();
  }
}

void HashCollisionSearcher::CreateStrings(int64_t length,
                                          int64_t from, int64_t to) {
  HashString string(power_, module_, length, from);
  for (; from <= to; ++from, ++string) {
    hash_maps_[length].Insert(string);
  }
}

void HashCollisionSearcher::GenerateAllStrings(int64_t length,
                                               uint8_t concurrency) {
  int64_t max_value = BinaryPow(kAlphabetSize, length) - 1;

  auto segments = SplitIntoSegments(0, max_value, concurrency);

  std::vector<std::thread> threads;

  threads.reserve(segments.size());
  for (const auto& segment : segments) {
    threads.emplace_back([this, length, segment] {
      CreateStrings(length, segment.first, segment.second);
    });
  }

  for (auto& thread : threads) {
    thread.join();
  }
}
