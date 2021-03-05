#include "gtest.h"

#include "find_if.h"

bool IsOne(int64_t value) {
  return value == 1;
}

bool TruePredicate(int64_t) {
  return true;
}

TEST(CheckValue, MutexUnlock) {
  std::mutex mutex;
  std::vector<int64_t> result;
  CheckValue(1, TruePredicate, &result, &mutex);
  ASSERT_TRUE(mutex.try_lock());
}

TEST(CheckValue, AddValue) {
  std::mutex mutex;
  std::vector<int64_t> result;

  CheckValue(0, IsOne, &result, &mutex);
  ASSERT_TRUE(result.empty());

  CheckValue(1, IsOne, &result, &mutex);
  ASSERT_EQ(1, result.size());
  ASSERT_EQ(1, result[0]);
}

TEST(CheckValue, VectorAccess) {
  /// Predicate returns true -> add to vector(nullptr) -> Death
  // Sanitizer doesn't like this because of *nullptr :(
  // ASSERT_DEATH(CheckValue(1, IsOne, nullptr, nullptr), ".*");

  /// Predicate returns false -> no access to vector -> OK
  CheckValue(0, IsOne, nullptr, nullptr);
}

TEST(CheckSegment, IncorrectSegment) {
  bool is_predicate_called = false;
  auto predicate = [&is_predicate_called](int64_t value) {
    is_predicate_called = true;
    return false;
  };

  std::mutex mutex;
  std::vector<int64_t> result;
  CheckSegment(1, 0, predicate, &result, &mutex);
  ASSERT_FALSE(is_predicate_called);
}

TEST(CheckSegment, AllSegmentChecked) {
  std::vector<int64_t> predicate_arguments;
  auto predicate = [&predicate_arguments](int64_t value) {
    predicate_arguments.push_back(value);
    return false;
  };

  int64_t begin = -10;
  int64_t end = 10;

  std::mutex mutex;
  std::vector<int64_t> result;
  CheckSegment(begin, end, predicate, &result, &mutex);

  ASSERT_EQ(end - begin + 1, predicate_arguments.size());
  for (int i = begin; i <= end; i++) {
    ASSERT_EQ(i, predicate_arguments[i - begin]);
  }
}

auto ToSegments(std::initializer_list<std::pair<int64_t, int64_t>> segments) {
  return std::vector<std::pair<int64_t, int64_t>>(segments);
}

TEST(SplitIntoSegments, EqualSegments) {
  ASSERT_EQ(SplitIntoSegments(1, 10, 1), ToSegments({{1, 10}}));
  ASSERT_EQ(SplitIntoSegments(1, 10, 2), ToSegments({{1, 5}, {6, 10}}));
  ASSERT_EQ(SplitIntoSegments(1, 10, 5),
            ToSegments({{1, 2}, {3, 4}, {5, 6}, {7, 8}, {9, 10}}));
  ASSERT_EQ(SplitIntoSegments(1, 5, 5),
            ToSegments({{1, 1}, {2, 2}, {3, 3}, {4, 4}, {5, 5}}));
}

TEST(SplitIntoSegments, NonEqualSegments) {
  ASSERT_EQ(SplitIntoSegments(1, 10, 3), ToSegments({{1, 3}, {4, 6}, {7, 10}}));
  ASSERT_EQ(SplitIntoSegments(1, 10, 4),
            ToSegments({{1, 2}, {3, 4}, {5, 6}, {7, 10}}));
  ASSERT_EQ(SplitIntoSegments(1, 10, 6),
            ToSegments({{1, 1}, {2, 2}, {3, 3}, {4, 4}, {5, 5}, {6, 10}}));
  ASSERT_EQ(SplitIntoSegments(1, 13, 2), ToSegments({{1, 6}, {7, 13}}));
}

TEST(SplitIntoSegments, NegativeValues) {
  ASSERT_EQ(SplitIntoSegments(-10, 10, 1), ToSegments({{-10, 10}}));
  ASSERT_EQ(SplitIntoSegments(-10, 10, 2), ToSegments({{-10, -1}, {0, 10}}));
  ASSERT_EQ(SplitIntoSegments(-10, 10, 3),
            ToSegments({{-10, -4}, {-3, 3}, {4, 10}}));
  ASSERT_EQ(SplitIntoSegments(-10, 10, 5),
            ToSegments({{-10, -7}, {-6, -3}, {-2, 1}, {2, 5}, {6, 10}}));
}

TEST(SplitIntoSegments, MoreSegmentsThanValues) {
  ASSERT_EQ(SplitIntoSegments(1, 1, 1), ToSegments({{1, 1}}));
  ASSERT_EQ(SplitIntoSegments(1, 1, 100), ToSegments({{1, 1}}));
  ASSERT_EQ(SplitIntoSegments(1, 3, 10), ToSegments({{1, 1}, {2, 2}, {3, 3}}));
  ASSERT_EQ(SplitIntoSegments(1, 3, 100), ToSegments({{1, 1}, {2, 2}, {3, 3}}));
}

TEST(SplitIntoSegments, IncorrectSegment) {
  ASSERT_EQ(SplitIntoSegments(10, 1, 10), ToSegments({}));
  ASSERT_EQ(SplitIntoSegments(3, -3, 1), ToSegments({}));
}

TEST(CheckValuesBySegments, IncorrectSegment) {
  std::mutex mutex;
  std::vector<int64_t> result;
  CheckSegment(1, 0, TruePredicate, &result, &mutex);
  ASSERT_TRUE(result.empty());
}

TEST(CheckValuesBySegments, AllValuesAdded) {
  std::mutex mutex;
  std::vector<int64_t> result;

  int64_t begin = -10;
  int64_t end = 10;
  CheckSegment(begin, end, TruePredicate, &result, &mutex);

  ASSERT_EQ(end - begin + 1, result.size());
  for (int i = begin; i <= end; i++) {
    ASSERT_EQ(i, result[i - begin]);
  }
}

TEST(CheckValuesBySegments, AllCorrectValuesAdded) {
  std::mutex mutex;
  std::vector<int64_t> result;

  CheckSegment(-100, 100, IsOne, &result, &mutex);
  ASSERT_EQ(1, result.size());
  ASSERT_EQ(1, result[0]);
}

TEST(FindIf, ZeroConcurrency) {
  ASSERT_TRUE(FindIf(1, 100, TruePredicate, 0).empty());
}

TEST(FindIf, NoCorrectValues) {
  ASSERT_TRUE(FindIf(2, 100, IsOne, 0).empty());
}

TEST(FindIf, OneCorrectValue) {
  auto result = FindIf(-100, 100, IsOne);
  ASSERT_EQ(1, result.size());
  ASSERT_EQ(1, result[0]);
}

TEST(FindIf, AllValuesAreCorrect) {
  int64_t begin = -10;
  int64_t end = 10;
  auto result = FindIf(begin, end, TruePredicate);
  std::sort(result.begin(), result.end());

  ASSERT_EQ(end - begin + 1, result.size());
  for (int i = begin; i <= end; i++) {
    ASSERT_EQ(i, result[i - begin]);
  }
}

TEST(FindIf, ConcurrencyMoreThanSegment) {
  int64_t begin = -3;
  int64_t end = 3;
  auto result = FindIf(begin, end, TruePredicate, 10);
  std::sort(result.begin(), result.end());

  ASSERT_EQ(end - begin + 1, result.size());
  for (int i = begin; i <= end; i++) {
    ASSERT_EQ(i, result[i - begin]);
  }
}

TEST(FindIf, MultipleThreads) {
  int64_t begin = -10;
  int64_t end = 10;

  std::vector<int64_t> result;
  for (uint8_t concurrency = 1; concurrency <= 10; concurrency++) {
    result = FindIf(begin, end, TruePredicate, concurrency);
    std::sort(result.begin(), result.end());

    ASSERT_EQ(end - begin + 1, result.size());
    for (int i = begin; i <= end; i++) {
      ASSERT_EQ(i, result[i - begin]);
    }
  }
}
