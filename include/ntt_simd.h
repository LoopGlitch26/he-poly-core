\
#pragma once
#include <vector>
#include <cstdint>
using u64 = uint64_t;

#ifdef __AVX2__
void ntt_avx2_core(std::vector<u64>& a, const std::vector<u64>& mroots, u64 mod);
#endif
