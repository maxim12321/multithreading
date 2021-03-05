#include "benchmark/benchmark.h"

#include "find_if.h"

const int64_t kFastPredicateSegmentSize = 5000000;
const int64_t kSlowPredicateSegmentSize = 20000;

bool FastTruePredicate(int64_t value) {
  return true;
}

bool FastHalfPredicate(int64_t value) {
  return value % 2ll == 1ll;
}

bool FastFalsePredicate(int64_t value) {
  return false;
}

bool SlowTruePredicate(int64_t value) {
  std::this_thread::sleep_for(std::chrono::nanoseconds(value / 100));
  return true;
}

bool SlowHalfPredicate(int64_t value) {
  std::this_thread::sleep_for(std::chrono::nanoseconds(value / 100));
  return value % 2ll == 1ll;
}

bool SlowFalsePredicate(int64_t value) {
  std::this_thread::sleep_for(std::chrono::nanoseconds(value / 100));
  return false;
}

static void BM_FindIf_FastTruePredicate(benchmark::State& state) {
  for (auto _ : state) {
    FindIf(1, kFastPredicateSegmentSize, FastTruePredicate, state.range(0));
  }
}
BENCHMARK(BM_FindIf_FastTruePredicate)->Unit(benchmark::kMillisecond)
    ->Arg(1)->Arg(2)->Arg(4)->Arg(6)->Arg(8)->Arg(12);

static void BM_FindIf_FastHalfPredicate(benchmark::State& state) {
  for (auto _ : state) {
    FindIf(1, kFastPredicateSegmentSize, FastHalfPredicate, state.range(0));
  }
}
BENCHMARK(BM_FindIf_FastHalfPredicate)->Unit(benchmark::kMillisecond)
    ->Arg(1)->Arg(2)->Arg(4)->Arg(6)->Arg(8)->Arg(12);

static void BM_FindIf_FastFalsePredicate(benchmark::State& state) {
  for (auto _ : state) {
    FindIf(1, kFastPredicateSegmentSize, FastFalsePredicate, state.range(0));
  }
}
BENCHMARK(BM_FindIf_FastFalsePredicate)->Unit(benchmark::kMillisecond)
    ->Arg(1)->Arg(2)->Arg(4)->Arg(6)->Arg(8)->Arg(12);


static void BM_FindIf_SlowTruePredicate(benchmark::State& state) {
  for (auto _ : state) {
    FindIf(1, kSlowPredicateSegmentSize, SlowTruePredicate, state.range(0));
  }
}
BENCHMARK(BM_FindIf_SlowTruePredicate)->Unit(benchmark::kMillisecond)
    ->Arg(1)->Arg(2)->Arg(4)->Arg(6)->Arg(8)->Arg(12);

static void BM_FindIf_SlowHalfPredicate(benchmark::State& state) {
  for (auto _ : state) {
    FindIf(1, kSlowPredicateSegmentSize, SlowHalfPredicate, state.range(0));
  }
}
BENCHMARK(BM_FindIf_SlowHalfPredicate)->Unit(benchmark::kMillisecond)
    ->Arg(1)->Arg(2)->Arg(4)->Arg(6)->Arg(8)->Arg(12);

static void BM_FindIf_SlowFalsePredicate(benchmark::State& state) {
  for (auto _ : state) {
    FindIf(1, kSlowPredicateSegmentSize, SlowFalsePredicate, state.range(0));
  }
}
BENCHMARK(BM_FindIf_SlowFalsePredicate)->Unit(benchmark::kMillisecond)
    ->Arg(1)->Arg(2)->Arg(4)->Arg(6)->Arg(8)->Arg(12);

BENCHMARK_MAIN();
