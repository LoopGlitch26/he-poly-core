# Design Overview

## Core Components
- mod_arith: modular operations
- montgomery: Montgomery reduction
- ntt: radix-2 NTT (scalar)
- ntt_simd: AVX2 vectorized butterfly

## Data Flow
Polynomial → bit-reversal → butterflies → reduction → output

## SIMD
Uses _mm256_add_epi64/_mm256_sub_epi64 for adds/subs, partial 64x64→128 via _mm_mul_epu32.
