\
#include <bits/stdc++.h>
#include "../include/ntt.h"
using namespace std;
using u64 = uint64_t;
using clk = chrono::high_resolution_clock;

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
    const u64 mod = 2013265921;
    size_t n = 1<<14; // 16384 - may be large for this environment; adjust as needed
    // find primitive root (we'll use 31 and compute nth root)
    u64 root = 31;
    u64 root_n = mod_pow(root, (mod - 1) / n, mod);
    auto roots = compute_roots(root_n, n, mod);
    vector<u64> a(n);
    for (size_t i=0;i<n/4;i++) a[i]=i+1;
    auto t0 = clk::now();
    ntt(a, roots, mod);
    auto t1 = clk::now();
    double ms = chrono::duration<double, std::milli>(t1-t0).count();
    cout << "NTT of size " << n << " took " << ms << " ms (single run)" << endl;
    return 0;
}
