
#include "ntt_simd.h"
#include "ntt.h"
#include <immintrin.h>
#include <vector>
#include <cstdint>
#include <cassert>
#include <iostream>

// Improved AVX2-assisted 4-lane 64x64->128 multiply using _mm_mul_epu32 partial products,
// then reconstructing full 128-bit results per lane and reducing modulo 'mod' using 128-bit ops.
// This balances vector loads with scalar 128-bit combines for correctness.

using u64 = uint64_t;
using u128 = __uint128_t;

#ifdef __AVX2__

// Helper: load 4 u64 into array (no alignment assumption)
static inline void load4_u64(const uint64_t* p, uint64_t out[4]) {
    out[0] = p[0]; out[1] = p[1]; out[2] = p[2]; out[3] = p[3];
}

// Vector-assisted partial multiply: compute (a[i] * b[i]) % mod for i=0..3
static inline void vec_mul64_mod_4(const uint64_t *a, const uint64_t *b, uint64_t mod, uint64_t out[4]) {
    // Use 32-bit decomposition to compute partial products in parallel using _mm_mul_epu32.
    // This produces low and some cross terms vectorized; final reconstruction uses 128-bit arithmetic per lane.
    // Note: correctness prioritized over absolute vector-only implementation.

    // Extract 32-bit parts
    uint32_t a_lo[4], a_hi[4], b_lo[4], b_hi[4];
    for (int i = 0; i < 4; ++i) {
        a_lo[i] = (uint32_t)(a[i] & 0xFFFFFFFFu);
        a_hi[i] = (uint32_t)(a[i] >> 32);
        b_lo[i] = (uint32_t)(b[i] & 0xFFFFFFFFu);
        b_hi[i] = (uint32_t)(b[i] >> 32);
    }

    // Use _mm_mul_epu32 to compute pairs of 32-bit products (lanes 0&2 then 1&3).
    __m128i alo = _mm_setr_epi32(a_lo[0], a_lo[1], a_lo[2], a_lo[3]);
    __m128i ahi = _mm_setr_epi32(a_hi[0], a_hi[1], a_hi[2], a_hi[3]);
    __m128i blo = _mm_setr_epi32(b_lo[0], b_lo[1], b_lo[2], b_lo[3]);
    __m128i bhi = _mm_setr_epi32(b_hi[0], b_hi[1], b_hi[2], b_hi[3]);

    // low*low products
    __m128i p_lo = _mm_mul_epu32(alo, blo); // produces products for lanes 0 and 2 in 64-bit slots
    __m128i alo_sh = _mm_shuffle_epi32(alo, _MM_SHUFFLE(2,3,0,1));
    __m128i blo_sh = _mm_shuffle_epi32(blo, _MM_SHUFFLE(2,3,0,1));
    __m128i p_lo_sh = _mm_mul_epu32(alo_sh, blo_sh); // lanes 1 and 3

    // cross terms a_lo*b_hi and a_hi*b_lo
    __m128i p_x1 = _mm_mul_epu32(alo, bhi);
    __m128i p_x1_sh = _mm_mul_epu32(alo_sh, _mm_shuffle_epi32(bhi, _MM_SHUFFLE(2,3,0,1)));
    __m128i p_x2 = _mm_mul_epu32(ahi, blo);
    __m128i p_x2_sh = _mm_mul_epu32(_mm_shuffle_epi32(ahi, _MM_SHUFFLE(2,3,0,1)), blo_sh);

    // high*high
    __m128i p_hi = _mm_mul_epu32(ahi, bhi);
    __m128i p_hi_sh = _mm_mul_epu32(_mm_shuffle_epi32(ahi, _MM_SHUFFLE(2,3,0,1)), _mm_shuffle_epi32(bhi, _MM_SHUFFLE(2,3,0,1)));

    // Extract 64-bit lanes
    uint64_t low00 = (uint64_t)_mm_cvtsi128_si64(p_lo);
    uint64_t low02 = (uint64_t)_mm_cvtsi128_si64(_mm_srli_si128(p_lo, 8));
    uint64_t low10 = (uint64_t)_mm_cvtsi128_si64(p_lo_sh);
    uint64_t low12 = (uint64_t)_mm_cvtsi128_si64(_mm_srli_si128(p_lo_sh, 8));

    uint64_t x10 = (uint64_t)_mm_cvtsi128_si64(p_x1);
    uint64_t x12 = (uint64_t)_mm_cvtsi128_si64(_mm_srli_si128(p_x1, 8));
    uint64_t x11 = (uint64_t)_mm_cvtsi128_si64(p_x1_sh);
    uint64_t x13 = (uint64_t)_mm_cvtsi128_si64(_mm_srli_si128(p_x1_sh, 8));

    uint64_t y10 = (uint64_t)_mm_cvtsi128_si64(p_x2);
    uint64_t y12 = (uint64_t)_mm_cvtsi128_si64(_mm_srli_si128(p_x2, 8));
    uint64_t y11 = (uint64_t)_mm_cvtsi128_si64(p_x2_sh);
    uint64_t y13 = (uint64_t)_mm_cvtsi128_si64(_mm_srli_si128(p_x2_sh, 8));

    uint64_t hi00 = (uint64_t)_mm_cvtsi128_si64(p_hi);
    uint64_t hi02 = (uint64_t)_mm_cvtsi128_si64(_mm_srli_si128(p_hi, 8));
    uint64_t hi10 = (uint64_t)_mm_cvtsi128_si64(p_hi_sh);
    uint64_t hi12 = (uint64_t)_mm_cvtsi128_si64(_mm_srli_si128(p_hi_sh, 8));

    // Now reconstruct 128-bit results per lane using 128-bit arithmetic (portable and correct)
    __uint128_t r0 = ( (__uint128_t)low00 ) + ( (__uint128_t)(x10 + y10) << 32 ) + ( (__uint128_t)hi00 << 64 );
    __uint128_t r1 = ( (__uint128_t)low10 ) + ( (__uint128_t)(x11 + y11) << 32 ) + ( (__uint128_t)hi10 << 64 );
    __uint128_t r2 = ( (__uint128_t)low02 ) + ( (__uint128_t)(x12 + y12) << 32 ) + ( (__uint128_t)hi02 << 64 );
    __uint128_t r3 = ( (__uint128_t)low12 ) + ( (__uint128_t)(x13 + y13) << 32 ) + ( (__uint128_t)hi12 << 64 );

    out[0] = (uint64_t)(r0 % mod);
    out[1] = (uint64_t)(r1 % mod);
    out[2] = (uint64_t)(r2 % mod);
    out[3] = (uint64_t)(r3 % mod);
}

static inline __m256i load_u64x4(const uint64_t* p) {
    return _mm256_setr_epi64x((long long)p[0], (long long)p[1], (long long)p[2], (long long)p[3]);
}
static inline void store_u64x4(uint64_t* p, __m256i v) {
    alignas(32) uint64_t tmp[4];
    _mm256_storeu_si256((__m256i*)tmp, v);
    p[0]=tmp[0]; p[1]=tmp[1]; p[2]=tmp[2]; p[3]=tmp[3];
}

void ntt_avx2_core(std::vector<u64>& a, const std::vector<u64>& mroots, u64 mod) {
    size_t n = a.size();
    for (size_t len = 1; len < n; len <<= 1) {
        size_t step = n / (2 * len);
        for (size_t i = 0; i < n; i += 2 * len) {
            size_t j = 0;
            for (; j + 4 <= len; j += 4) {
                uint64_t uvals[4], bvals[4], wvals[4];
                for (int t = 0; t < 4; ++t) {
                    uvals[t] = a[i + j + t];
                    bvals[t] = a[i + j + len + t];
                    wvals[t] = mroots[step * (j + t)];
                }
                uint64_t vtmp[4];
                vec_mul64_mod_4(bvals, wvals, mod, vtmp);
                __m256i uvec = load_u64x4(uvals);
                __m256i vvec = load_u64x4(vtmp);
                __m256i xvec = _mm256_add_epi64(uvec, vvec);
                // handle reduction: x >= mod
                __m256i modvec = _mm256_set1_epi64x((long long)mod);
                __m256i bias = _mm256_set1_epi64x((long long)0x8000000000000000ULL);
                __m256i xs = _mm256_xor_si256(xvec, bias);
                __m256i ms = _mm256_xor_si256(modvec, bias);
                __m256i cmp = _mm256_cmpgt_epi64(xs, _mm256_sub_epi64(ms, _mm256_set1_epi64x(1)));
                __m256i xsub = _mm256_sub_epi64(xvec, modvec);
                __m256i xres = _mm256_blendv_epi8(xvec, xsub, cmp);
                store_u64x4(&a[i + j], xres);
                for (int t=0;t<4;++t) {
                    uint64_t u = uvals[t];
                    uint64_t v = vtmp[t];
                    uint64_t y = (u >= v) ? u - v : u + mod - v;
                    a[i + j + len + t] = y;
                }
            }
            for (; j < len; ++j) {
                u64 u = a[i + j];
                u64 v = (u128)a[i + j + len] * mroots[step * j] % mod;
                u64 x = u + v;
                if (x >= mod) x -= mod;
                a[i + j] = x;
                u64 y = (u >= v) ? u - v : u + mod - v;
                a[i + j + len] = y;
            }
        }
    }
}

#else
void ntt_avx2_core(std::vector<u64>& a, const std::vector<u64>& mroots, u64 mod) {
    (void)a; (void)mroots; (void)mod;
}
#endif
