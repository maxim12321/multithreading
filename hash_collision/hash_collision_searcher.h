#pragma once

#include <atomic>
#include <string>
#include <thread>
#include <vector>

#include "hash_map.h"
#include "../utilities.h"

class HashCollisionSearcher {
 public:
  HashCollisionSearcher(int64_t power, int64_t module);

  std::string FindCollision(const std::string& target, int64_t string_length,
                            uint8_t concurrency);

 private:
  bool CheckString(const HashString& string);
  void CheckStrings(int64_t length, int64_t from, int64_t to);
  void SearchForCollision(int64_t length, uint8_t concurrency);

  void CreateStrings(int64_t length, int64_t from, int64_t to);
  void GenerateAllStrings(int64_t length, uint8_t concurrency);

 private:
  int64_t power_;
  int64_t module_;

  std::vector<HashMap> hash_maps_;

  std::string target_;
  int64_t target_hash_ = 0;

  std::string result_;
  std::mutex result_mutex_;

  std::atomic<bool> is_answer_found_;
};
