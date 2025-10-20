\
#include <bits/stdc++.h>
using u64 = uint64_t;
using u128 = __uint128_t;

u64 add_mod(u64 a, u64 b, u64 mod) {
    u64 r = a + b;
    if (r >= mod || r < a) r -= mod;
    if (r >= mod) r %= mod;
    return r;
}
u64 sub_mod(u64 a, u64 b, u64 mod) {
    return (a >= b) ? a - b : (mod - (b - a));
}
u64 mul_mod(u64 a, u64 b, u64 mod) {
    u128 t = (u128)a * (u128)b;
    return (u64)(t % mod);
}
