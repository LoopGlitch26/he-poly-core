\
#include <bits/stdc++.h>
#include "../include/ntt.h"
#include "../include/montgomery.h"
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

double time_fn(function<void()> fn, int runs=3) {
    vector<double> times;
    for (int i=0;i<runs;i++) {
        auto t0 = clk::now();
        fn();
        auto t1 = clk::now();
        times.push_back(chrono::duration<double, milli>(t1-t0).count());
    }
    sort(times.begin(), times.end());
    return times[times.size()/2];
}

int main() {
    const u64 mod = 2013265921;
    size_t n = 1<<12; // default 4096
    const char* envn = getenv("BENCH_N"); if (envn) n = (size_t)atoi(envn);
    u64 root = 31;
    u64 root_n = mod_pow(root, (mod - 1) / n, mod);
    auto roots = compute_roots(root_n, n, mod);

    vector<u64> a(n);
    for (size_t i=0;i<n/8;i++) a[i]=i+1;

    auto a_baseline = a;
    auto a_full = a;
    auto a_core = a;

    // Full transform timings (includes conversions)
    double t_baseline = time_fn([&](){ auto t = a_baseline; ntt(t, roots, mod); }, 5);
    double t_full = time_fn([&](){ auto t = a_full; ntt_montgomery(t, roots, mod); }, 5);

    // Now prepare montgomery preconverted arrays for core timing
    Montgomery M(mod);
    // convert roots to mont domain once
    vector<u64> mroots = compute_roots(root_n, n, mod);
    for (size_t i = 0; i < n; ++i) mroots[i] = M.to_mont(mroots[i]);
    // convert input to mont domain
    for (size_t i = 0; i < n; ++i) a_core[i] = M.to_mont(a_core[i]);

    double t_core = time_fn([&](){ auto t = a_core; ntt_montgomery_core(t, mroots, mod); }, 5);


    auto a_avx = a_core;
    double t_avx = time_fn([&](){ auto t = a_avx; ntt_avx2_core(t, mroots, mod); }, 5);
    cout << "NTT montgomery avx2 core median ms: " << t_avx << endl;
    if (t_avx>0) cout << "AVX2 Core Speedup: " << t_baseline / t_avx << "x" << endl;


    cout << "NTT baseline median ms: " << t_baseline << endl;
    cout << "NTT montgomery (full) median ms: " << t_full << endl;
    cout << "NTT montgomery core median ms (preconverted): " << t_core << endl;
    if (t_core>0) cout << "Core Speedup: " << t_baseline / t_core << "x" << endl;
    if (t_full>0) cout << "Full Speedup: " << t_baseline / t_full << "x" << endl;
    return 0;
}
