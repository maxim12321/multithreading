#include "benchmark/benchmark.h"

#include "hash.h"

const int64_t kPower = 31;

static void BM_Hash(benchmark::State& state) {
  static std::random_device random_device;
  static std::mt19937_64 generator(random_device());

  std::string target(1000, 'a');

  for (auto _ : state) {
    state.PauseTiming();
    RandomizeString(&generator, &target);
    state.ResumeTiming();
    auto result = FindCollision(target,
                                kPower,
                                state.range(1),
                                state.range(0));
  }
}
BENCHMARK(BM_Hash)->Unit(benchmark::kMillisecond)->MinTime(5)
    ->ArgsProduct({{1, 2, 4, 6, 8, 12},
                   {10'000'019, 1'000'000'411, 10'000'000'033,
                    100'000'000'003, 1'000'000'000'039, 10'000'000'000'019}});

BENCHMARK_MAIN();
