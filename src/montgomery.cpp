\
#include "montgomery.h"
#include <cassert>
#include <stdint.h>

// R = 2^64 implicitly; operations use 128-bit intermediates
Montgomery::Montgomery(u64 m) {
    if (m) init(m);
}
void Montgomery::init(u64 m) {
    mod = m;
    // compute ninv such that mod * ninv ≡ -1 (mod R)
    // i.e., mod * ninv + 1 ≡ 0 (mod 2^64) => ninv ≡ -mod^{-1} mod R
    // compute using Newton-Raphson for inverse modulo 2^64 or extended gcd; we'll use algorithm to find inv of mod mod 2^64
    u64 inv = 1;
    // compute modular inverse of mod modulo 2^64 using Newton iterations
    for (int i = 0; i < 6; ++i) {
        // inv = inv * (2 - mod * inv) mod 2^64
        inv = inv * (2 - mod * inv);
    }
    ninv = (~inv) + 1; // ninv = -inv mod 2^64
    rmask = ~0ULL;
    // quick sanity
    // (mod * ninv) % (1ULL<<64) should be 2^64 - 1
    // but we'll skip assert to avoid UB
}

u64 Montgomery::to_mont(u64 a) const {
    // returns (a * R) mod mod; since R = 2^64, (a * R) % mod == (a << 64) % mod -> use mul by R%mod
    // compute (a * R) mod mod = (a * (2^64 mod mod)) mod mod
    // 2^64 mod mod can be computed as ( ( (u128)1 << 64 ) % mod ), but that's zero in 64-bit arithmetic.
    // Simpler: to convert, compute (a * (R % mod)) % mod, with R%mod = (-mod) % mod ??? To avoid complexity, use regular multiplication by R via u128 then mod
    __uint128_t t = ( __uint128_t)a * (__uint128_t(1) << 64);
    return (u64)(t % mod);
}

u64 Montgomery::from_mont(u64 a) const {
    // montgomery reduction of a with multiplier 1
    return mul(a, 1);
}

u64 Montgomery::mul(u64 a, u64 b) const {
    __uint128_t t = (__uint128_t)a * (__uint128_t)b;
    u64 m = (u64)t * ninv;
    __uint128_t u = (t + (__uint128_t)m * (__uint128_t)mod) >> 64;
    u64 res = (u64)u;
    if (res >= mod) res -= mod;
    return res;
}

u64 Montgomery::add(u64 a, u64 b) const {
    u64 s = a + b;
    if (s >= mod || s < a) s -= mod;
    return s;
}
u64 Montgomery::sub(u64 a, u64 b) const {
    return (a >= b) ? a - b : mod - (b - a);
}
