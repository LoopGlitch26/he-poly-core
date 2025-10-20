#pragma once
#include <cstdint>
using u64 = uint64_t;
using u128 = __uint128_t;

struct Montgomery {
    u64 mod;    // modulus N
    u64 ninv;   // -N^{-1} mod R (R = 2^64)
    u64 rmask;  // mask for R-1 (if R power of two) not used explicitly but kept for clarity

    Montgomery(u64 m=0);
    void init(u64 m);
    u64 to_mont(u64 a) const;
    u64 from_mont(u64 a) const;
    u64 mul(u64 a, u64 b) const;
    u64 add(u64 a, u64 b) const;
    u64 sub(u64 a, u64 b) const;
};
