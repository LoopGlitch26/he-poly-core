\
#include <bits/stdc++.h>
#include "../include/ntt.h"
using namespace std;
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

int main() {
    cout << "Running lightweight NTT tests..." << endl;
    const u64 mod = 2013265921; // 15 * 2^27 + 1, common NTT prime
    const size_t n = 8;
    // primitive root for this mod (known): 31 is primitive root for some such primes; compute suitable root
    u64 root = 31;
    // compute n-th root: root^{(mod-1)/n}
    u64 root_n = mod_pow(root, (mod - 1) / n, mod);
    vector<u64> roots = compute_roots(root_n, n, mod);
    vector<u64> a = {1,2,3,4,0,0,0,0};
    auto A = a;
    ntt(A, roots, mod);
    // pointwise square then inverse
    vector<u64> B = A;
    for (size_t i = 0; i < n; ++i) B[i] = ( (__uint128_t)A[i] * A[i] ) % mod;
    intt(B, roots, mod);
    // naive square convolution
    vector<u64> C(2*n-1);
    for (size_t i = 0; i < n; ++i)
        for (size_t j = 0; j < n; ++j)
            C[i+j] = (C[i+j] + ( (__uint128_t)a[i] * a[j] ) % mod ) % mod;
    // compare first n coefficients (with wrap or truncation)
    bool ok = true;
    for (size_t i = 0; i < n; ++i) {
        if (B[i] != C[i]) { ok = false; break; }
    }
    if (!ok) {
        cerr << "NTT roundtrip/convolution mismatch" << endl;
        return 1;
    }
    cout << "Lightweight NTT tests passed." << endl;
    return 0;
}
