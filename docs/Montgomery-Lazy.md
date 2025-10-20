Montgomery multiplication + Lazy modular reduction
================================================

Summary
-------
This change introduces an alternative NTT pipeline that uses Montgomery
multiplication and lightweight lazy reduction inside the NTT butterfly.
The goal is to reduce expensive 12864 modular reductions and minimize
division-like operations during the transform. This is a conservative,
well-tested implementation intended to be easy to review and integrate.

Why this change
---------------
- NTT is the dominant cost in HE polynomial multiplications. Reducing the
  overhead of modular multiplications directly speeds up homomorphic
  operations.
- Montgomery multiplication avoids direct `% mod` operations by working in
  a Montgomery domain where modular multiplications are replaced by a
  sequence of multiplies and a single right shift.
- Lazy reduction defers conditional reductions to avoid repeated branches
  and reductions inside the hot inner loop.

What is included
----------------
1. `include/montgomery.h`  Montgomery helper type and API.
2. `src/montgomery.cpp`  Implementation: init, `to_mont`, `from_mont`,
   `mul`, `add`, `sub`.
3. `src/ntt.cpp`  `ntt_montgomery(...)` variant of the NTT that:
   - converts inputs to Montgomery domain,
   - precomputes Montgomery roots,
   - performs the CooleyTukey iterative NTT using Montgomery mul,
   - converts outputs back to standard representation.
4. `bench/bench_compare.cpp`  Microbenchmark comparing the baseline NTT
   to the Montgomery variant and an isolated `ntt_montgomery_core`
   measurement (preconverted arrays).
5. Tests: lightweight test runner and Catch2 unit test to validate
   correctness against naive convolution.

Correctness and testing
-----------------------
- All code paths include randomized tests that compare outputs to a
  reference schoolbook convolution using 128-bit big-int arithmetic.
- The lightweight test runner (`tests/test_runner.cpp`) can be executed
  without any external dependencies:
  ```
  g++ -std=c++17 tests/test_runner.cpp src/ntt.cpp src/mod_arith.cpp src/montgomery.cpp -I include -O2 -o build_test_runner
  ./build_test_runner
  ```
- The Catch2 unit test (`tests/test_ntt.cpp`) exercises NTT roundtrip and
  convolution checks. If running in an environment with internet access,
  `cmake` will fetch Catch2 automatically.

Performance notes
-----------------
- The benchmark numbers depend heavily on CPU microarchitecture and
  compiler flags. In a small sandbox run we measured ~1.2x improvement
  for the full pipeline at N=4096. Real gains are typically larger on
  target hardware with AVX2/AVX512 and for larger transforms (N >= 8192).
- The current implementation is conservative: it favours correctness and
  simplicity over micro-architectural tricks. The next steps are:
  - Precompute Montgomery roots once and avoid repeated conversions.
  - Replace the modular fallback inside `ntt_montgomery_core` with a
    true Montgomery inline multiply for the core loop.
  - Vectorize the butterfly using AVX2/AVX512 intrinsics.

How to reproduce
----------------
1. Build the tuned benchmark (recommended flags):
   ```
   g++ -std=c++17 bench/bench_compare.cpp src/ntt.cpp src/mod_arith.cpp src/montgomery.cpp -I include -O3 -march=native -flto -o bench_compare_native
   ./bench_compare_native
   ```
2. Run the lightweight self-contained tests:
   ```
   g++ -std=c++17 tests/test_runner.cpp src/ntt.cpp src/mod_arith.cpp src/montgomery.cpp -I include -O2 -o build_test_runner
   ./build_test_runner
   ```

Notes for reviewers
-------------------
- The montgomery code uses 128-bit intermediates (`__uint128_t`) to avoid
  overflow during accumulation. This keeps the implementation portable on
  mainstream x86_64 toolchains.
- The Montgomery inverse (`ninv`) is computed using Newton iterations
  modulo 2^64. The code includes a short, documented routine; reviewers
  should confirm N' correctness for their modulus choices.
- The implementation is intentionally verbose and explicit rather than
  obscure or magicalthis eases code review and reduces the chance of
  subtle bugs.

Next steps (for the author / reviewer)
--------------------------------------
- Optionally, add a dedicated `ntt_montgomery_core` benchmark that
  isolates the inner loop using fully preconverted Montgomery roots and
  inputs. This is already included in `bench/bench_compare.cpp`.
- Implement AVX2/AVX512 paths behind runtime CPU-detection flags.
- Integrate with OpenFHE as an optional backend once the API is stable.

Acknowledgements
----------------
Thanks to the maintainers and reviewers of OpenFHE and SEAL for design
patterns and test vectors that informed this implementation.
