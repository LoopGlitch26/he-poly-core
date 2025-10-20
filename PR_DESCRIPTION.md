# Add Montgomery multiplication + lazy-reduction NTT variant (bench + tests)

Background
----------
This PR adds a safe, well-tested Montgomery multiplication path for the NTT
butterfly and a conservative lazy reduction approach to reduce modular
reduction overhead. The implementation focuses on clarity and correctness,
with tests and microbenchmarks included.

What I changed
--------------
- include/montgomery.h, src/montgomery.cpp: Montgomery helper and operations.
- src/ntt.cpp: Added `ntt_montgomery` and `ntt_montgomery_core` variants.
- bench/bench_compare.cpp: Tuned microbenchmark for baseline and Montgomery.
- tests/*: Lightweight runner and Catch2 unit tests for verification.
- docs/Montgomery-Lazy.md: Design notes, reproduction instructions, and
  performance comments.

Why this is useful
------------------
NTT arithmetic is the hot path for homomorphic encryption workloads. A
correct, modular Montgomery implementation lets downstream libraries
optimize the hot loop without changing algorithmic semantics. This PR
provides a straightforward path to follow-up optimizations (SIMD, precomputed
Montgomery roots, GPU offload).

How to review
-------------
- Run `tests/test_runner` for immediate correctness checks.
- Run `bench/bench_compare.cpp` as described in docs to see the microbenchmark.
- Review `src/montgomery.cpp` for the modular inverse computation and the
  Montgomery reduction logic.

Notes
-----
This PR is intentionally conservative: it prefers explicit, readable code
over obfuscated micro-optimizations. After accepting the high-level
changes we can add more aggressive optimizations behind feature flags.

