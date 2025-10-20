\
#pragma once
#include <vector>
#include <cstdint>
using u64 = uint64_t;

std::vector<u64> poly_mul_naive(const std::vector<u64>& A, const std::vector<u64>& B, u64 mod);
