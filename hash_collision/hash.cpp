#include "hash.h"

std::string FindCollision(const std::string& a, int64_t p, int64_t m,
                          uint8_t concurrency) {
  HashCollisionSearcher hash_collision_searcher(p, m);

  int length = 0;
  std::string result;
  while (result.empty()) {
    result = hash_collision_searcher.FindCollision(a, length, concurrency);
    length++;
  }

  return result;
}
