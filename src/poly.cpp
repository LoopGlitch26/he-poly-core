\
#include "poly.h"
#include <vector>
using u64 = uint64_t;
using u128 = __uint128_t;

std::vector<u64> poly_mul_naive(const std::vector<u64>& A, const std::vector<u64>& B, u64 mod) {
    int n = (int)A.size(), m = (int)B.size();
    std::vector<u64> C(n + m - 1, 0);
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < m; ++j) {
            u128 t = (u128)A[i] * (u128)B[j] + (u128)C[i + j];
            C[i + j] = (u64)(t % mod);
        }
    }
    return C;
}
