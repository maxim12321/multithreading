#pragma once

#include <string>

#include "hash_collision_searcher.h"

std::string FindCollision(const std::string& a, int64_t p, int64_t m,
                          uint8_t concurrency);
