#include "gtest.h"

#include "hash.h"
#include "hash_map.h"

const int64_t kPower = 31;
const int64_t kModule09 = 1'000'000'007;

int64_t Hash(const std::string& s) {
  return Hash(s, kPower, kModule09);
}

TEST(HashMap, Simple) {
  HashMap hash_map;
  HashString s1(kPower, kModule09, 0);
  HashString s2(kPower, kModule09, 0);
  s1.Load("hello");
  s2.Load("world");
  hash_map.Insert(s1);
  hash_map.Insert(s2);

  ASSERT_EQ("hello", hash_map.Find(Hash("hello")));
  ASSERT_EQ("world", hash_map.Find(Hash("world")));
  ASSERT_EQ("", hash_map.Find(Hash("hell")));
}

void Check(const std::string& s, uint8_t concurrency,
           int64_t power = kPower, int64_t module = kModule09) {
  std::string result = FindCollision(s, power, module, concurrency);
  ASSERT_NE(s, result);
  ASSERT_EQ(Hash(s, power, module), Hash(result, power, module));
}

TEST(FindCollision, ShortStrings) {
  std::string target;
  for (int i = 0; i <= 10; i++, target += 'a') {
    Check(target, 1);
  }
}

TEST(FindCollision, ShortStringsBigPower) {
  std::string target;
  for (int i = 0; i <= 5; i++, target += 'a') {
    Check(target, 1, 1069);
  }
}

TEST(FindCollision, ShortStringManyThreads) {
  uint32_t max_threads = std::thread::hardware_concurrency();
  for (int threads = 2; threads <= max_threads; threads++) {
    Check("aaaaaaaa", threads);
  }
}

TEST(FindCollision, Simple) {
  std::string target = "this""is""test""string";

  uint32_t max_threads = std::thread::hardware_concurrency();
  for (int threads = 1; threads <= max_threads; threads++) {
    Check(target, threads);
  }
}

TEST(FindCollision, ShortStringBigModule) {
  int64_t power = kPower;
  int64_t module = 1'000'000'000'007;
  static std::random_device random_device;
  static std::mt19937_64 generator(random_device());

  std::string target(10, 'a');
  RandomizeString(&generator, &target);

  Check(target, 1, power, module);
}

TEST(FindCollision, BigModuleMultiThread) {
  int64_t power = kPower;
  int64_t module = 1'000'000'000'007;
  static std::random_device random_device;
  static std::mt19937_64 generator(random_device());

  std::string target(100, 'a');
  RandomizeString(&generator, &target);

  Check(target, 4, power, module);
}
