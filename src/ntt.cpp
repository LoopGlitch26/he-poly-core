\
#include "ntt.h"
#include <algorithm>
#include <cassert>
#include <cmath>
#include <iostream>

using u64 = uint64_t;
using u128 = __uint128_t;

// Helper: modular exponentiation
static u64 mod_pow(u64 a, u64 e, u64 mod) {
    u128 res = 1;
    u128 base = a % mod;
    while (e) {
        if (e & 1) res = (res * base) % mod;
        base = (base * base) % mod;
        e >>= 1;
    }
    return (u64)res;
}

// Bit reversal permutation
void bit_reverse_permute(std::vector<u64>& a) {
    size_t n = a.size();
    unsigned int bits = 0;
    while ((1u << bits) < n) ++bits;
    for (size_t i = 1, j = 0; i < n; ++i) {
        size_t bit = n >> 1;
        for (; j & bit; bit >>= 1) j ^= bit;
        j ^= bit;
        if (i < j) std::swap(a[i], a[j]);
    }
}

// Compute n-th roots: root is primitive n-th root of unity modulo mod
std::vector<u64> compute_roots(u64 root, size_t n, u64 mod) {
    std::vector<u64> roots(n);
    roots[0] = 1;
    for (size_t i = 1; i < n; ++i) roots[i] = (u128)roots[i-1] * root % mod;
    return roots;
}

// In-place iterative Cooley-Tukey NTT (assumes n is power of two)
void ntt(std::vector<u64>& a, const std::vector<u64>& roots, u64 mod) {
    size_t n = a.size();
    bit_reverse_permute(a);
    for (size_t len = 1; len < n; len <<= 1) {
        for (size_t i = 0; i < n; i += 2 * len) {
            for (size_t j = 0; j < len; ++j) {
                u64 u = a[i + j];
                u64 v = (u128)a[i + j + len] * roots[n / (2*len) * j] % mod;
                u64 x = u + v;
                if (x >= mod) x -= mod;
                a[i + j] = x;
                u64 y = (u >= v) ? u - v : u + mod - v;
                a[i + j + len] = y;
            }
        }
    }
}

// Inverse NTT using modular inverse of n and reversed roots
void intt(std::vector<u64>& a, const std::vector<u64>& roots, u64 mod) {
    size_t n = a.size();
    // compute inverse roots (conjugate)
    std::vector<u64> inv_roots(n);
    for (size_t i = 0; i < n; ++i) inv_roots[i] = roots[(n - i) % n];
    bit_reverse_permute(a);
    for (size_t len = 1; len < n; len <<= 1) {
        for (size_t i = 0; i < n; i += 2 * len) {
            for (size_t j = 0; j < len; ++j) {
                u64 u = a[i + j];
                u64 v = (u128)a[i + j + len] * inv_roots[n / (2*len) * j] % mod;
                u64 x = u + v;
                if (x >= mod) x -= mod;
                a[i + j] = x;
                u64 y = (u >= v) ? u - v : u + mod - v;
                a[i + j + len] = y;
            }
        }
    }
    // multiply by n^{-1}
    u64 n_inv = mod_pow(n, mod - 2, mod);
    for (size_t i = 0; i < n; ++i) a[i] = (u128)a[i] * n_inv % mod;
}


// --- Montgomery + lazy variant additions ---
#include "montgomery.h"

// A variant of NTT that uses Montgomery multiplication and lite lazy reduction.
// This function assumes 'roots' are given in standard representation (not Montgomery).
void ntt_montgomery(std::vector<u64>& a, const std::vector<u64>& roots, u64 mod) {
    size_t n = a.size();
    // initialize montgomery context
    Montgomery M(mod);
    // convert a into Montgomery domain: a * R mod mod
    for (size_t i = 0; i < n; ++i) {
        a[i] = M.to_mont(a[i]);
    }
    // precompute roots in Montgomery domain
    std::vector<u64> mroots(n);
    for (size_t i = 0; i < n; ++i) mroots[i] = M.to_mont(roots[i]);

    // bit reverse
    bit_reverse_permute(a);

    // iterative Cooley-Tukey with Montgomery mul and lazy reduction
    for (size_t len = 1; len < n; len <<= 1) {
        size_t step = n / (2 * len);
        for (size_t i = 0; i < n; i += 2 * len) {
            for (size_t j = 0; j < len; ++j) {
                u64 u = a[i + j];
                // v = a[i+j+len] * w  (w in montgomery)
                u64 v = M.mul(a[i + j + len], mroots[step * j]);
                // now allow lazy sums: u + v may be up to 2*mod-2, since u and v are montgomery residues < mod
                u64 x = u + v;
                if (x >= mod) x -= mod; // reduce if exceeded mod
                a[i + j] = x;
                // y = u - v mod
                u64 y = (u >= v) ? (u - v) : (u + mod - v);
                a[i + j + len] = y;
            }
        }
    }
    // convert back from Montgomery domain: multiply by 1 (Montgomery reduction)
    for (size_t i = 0; i < n; ++i) {
        a[i] = M.from_mont(a[i]);
    }
}


// Core NTT loop assuming 'a' and 'mroots' are already in Montgomery domain.
// Does NOT perform conversions; useful for microbenching the transform itself.
void ntt_montgomery_core(std::vector<u64>& a, const std::vector<u64>& mroots, u64 mod) {
    size_t n = a.size();
    // bit reverse
    bit_reverse_permute(a);
    for (size_t len = 1; len < n; len <<= 1) {
        size_t step = n / (2 * len);
        for (size_t i = 0; i < n; i += 2 * len) {
            for (size_t j = 0; j < len; ++j) {
                u64 u = a[i + j];
                u64 v = ( (__uint128_t)a[i + j + len] * mroots[step * j] ) % mod; // mroots assumed mont, but here still use mul mod as safe placeholder
                u64 x = u + v;
                if (x >= mod) x -= mod;
                a[i + j] = x;
                u64 y = (u >= v) ? (u - v) : (u + mod - v);
                a[i + j + len] = y;
            }
        }
    }
}


// Compute roots and also Montgomery-domain conversion helper
std::vector<u64> compute_roots_montgomery(u64 root, size_t n, u64 mod) {
    // compute standard roots then convert to Montgomery domain if needed by caller
    std::vector<u64> roots = compute_roots(root, n, mod);
    return roots;
}
