#include "cpu_features.h"
#include "ntt_simd.h"
#pragma once
#include <vector>
#include <cstdint>
#include <cstddef>
using u64 = uint64_t;

void bit_reverse_permute(std::vector<u64>& a);
std::vector<u64> compute_roots(u64 root, size_t n, u64 mod);
void ntt(std::vector<u64>& a, const std::vector<u64>& roots, u64 mod);
void intt(std::vector<u64>& a, const std::vector<u64>& roots, u64 mod);

void ntt_montgomery(std::vector<u64>& a, const std::vector<u64>& roots, u64 mod);

void ntt_montgomery_core(std::vector<u64>& a, const std::vector<u64>& mroots, u64 mod);

#ifdef __AVX2__
void ntt_avx2_core(std::vector<u64>& a, const std::vector<u64>& mroots, u64 mod);
#endif
