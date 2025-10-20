\
#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>
#include "ntt.h"
#include <vector>
using u64 = uint64_t;

u64 mod_pow(u64 a, u64 e, u64 mod) {
    __uint128_t res = 1;
    __uint128_t base = a % mod;
    while (e) {
        if (e & 1) res = (res * base) % mod;
        base = (base * base) % mod;
        e >>= 1;
    }
    return (u64)res;
}

TEST_CASE("NTT roundtrip and convolution small", "[ntt]") {
    const u64 mod = 2013265921;
    const size_t n = 8;
    u64 root = 31;
    u64 root_n = mod_pow(root, (mod - 1) / n, mod);
    auto roots = compute_roots(root_n, n, mod);
    std::vector<u64> a = {1,2,3,4,0,0,0,0};
    auto A = a;
    ntt(A, roots, mod);
    auto B = A;
    for (size_t i = 0; i < n; ++i) B[i] = ( (__uint128_t)A[i] * A[i] ) % mod;
    intt(B, roots, mod);
    // naive
    std::vector<u64> C(2*n-1,0);
    for (size_t i=0;i<n;i++)
        for (size_t j=0;j<n;j++)
            C[i+j] = (C[i+j] + (__uint128_t)a[i]*a[j]) % mod;
    for (size_t i=0;i<n;i++) REQUIRE(B[i] == C[i]);
}
