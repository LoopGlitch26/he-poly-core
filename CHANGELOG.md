## v0.1.1 - Montgomery + Lazy NTT variant

- Added: Montgomery helper (include/montgomery.h, src/montgomery.cpp)
- Added: ntt_montgomery variant and microbenchmark (bench/bench_compare.cpp)
- Added: docs/Montgomery-Lazy.md with tests and reproduction instructions
- Tests: lightweight runner and Catch2 test for validation
- Notes: conservative implementation prioritizing correctness; further
  performance work (SIMD, precomputation) planned.
