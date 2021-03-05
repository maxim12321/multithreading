#include "hash_string.h"

HashString::HashString(int64_t power, int64_t module,
                       int length, int64_t value_to_load)
    : value_(length, 'a'),
      power_(power),
      module_(module),
      hash_(Hash(value_, power, module)) {
  Load(value_to_load);
}

void HashString::Load(int64_t value) {
  for (auto it = value_.rbegin(); it != value_.rend(); it++) {
    *it = 'a' + value % kAlphabetSize;
    value /= kAlphabetSize;
  }
  hash_ = Hash(value_, power_, module_);
}

void HashString::Load(const std::string& value) {
  value_ = value;
  hash_ = Hash(value_, power_, module_);
}

std::string HashString::Get() const {
  return value_;
}

int64_t HashString::GetHash() const {
  return hash_;
}

HashString& HashString::operator++() {
  if (value_.empty()) {
    return *this;
  }

  value_.back()++;
  hash_++;
  if (hash_ == module_) {
    hash_ = 0;
  }

  if (value_.back() > 'z') {
    RepairString();
  }
  return *this;
}

void HashString::RepairString() {
  int index = value_.size();
  while (--index > 0) {
    if (value_[index] <= 'z') {
      break;
    }
    value_[index] = 'a';
    value_[index - 1]++;
  }
  hash_ = Hash(value_, power_, module_);
}
