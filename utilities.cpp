#include "utilities.h"

int64_t Hash(const std::string& s, int64_t p, int64_t m) {
  int64_t result = 0;
  for (char ch : s) {
    result = (__int128_t(result) * p + (ch - 'a' + 1)) % m;
  }
  return result;
}

int64_t BinaryPow(int64_t value, uint64_t power) {
  int64_t result = 1;
  while (power > 0) {
    if (power & 1ull) {
      result = (__int128_t(result) * value);
    }
    value = (__int128_t(value) * value);
    power >>= 1ull;
  }
  return result;
}

void RandomizeString(std::mt19937_64* generator, std::string* string) {
  std::uniform_int_distribution<char> random_char('a', 'z');
  for (char& c : *string) {
    c = random_char(*generator);
  }
}

std::vector<std::pair<int64_t, int64_t>> SplitIntoSegments(int64_t from,
                                                           int64_t to,
                                                           uint8_t amount) {
  std::vector<std::pair<int64_t, int64_t>> segments;
  int64_t segment_size = (to - from + 1) / amount;
  if (segment_size <= 0) {
    segment_size = 1;
  }

  int64_t left = from;
  while (amount > 0 && left <= to) {
    int64_t right = left + segment_size - 1;
    if (amount == 1 || right > to) {
      right = to;
    }

    segments.emplace_back(left, right);

    amount--;
    left += segment_size;
  }
  return segments;
}
