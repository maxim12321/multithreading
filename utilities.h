#pragma once

#include <random>
#include <string>
#include <utility>
#include <vector>

const int kAlphabetSize = 26;

int64_t Hash(const std::string& s, int64_t p, int64_t m);

int64_t BinaryPow(int64_t value, uint64_t power);

void RandomizeString(std::mt19937_64* generator, std::string* string);

std::vector<std::pair<int64_t, int64_t>> SplitIntoSegments(int64_t from,
                                                           int64_t to,
                                                           uint8_t amount);
