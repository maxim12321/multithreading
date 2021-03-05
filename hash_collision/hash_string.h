#pragma once

#include <string>

#include "../utilities.h"

class HashString {
 public:
  HashString(int64_t power, int64_t module, int length,
             int64_t value_to_load = 0);

  void Load(int64_t value);
  void Load(const std::string& value);

  std::string Get() const;
  int64_t GetHash() const;

  HashString& operator++();

 private:
  void RepairString();

 private:
  std::string value_;

  int64_t power_;
  int64_t module_;
  int64_t hash_;
};
