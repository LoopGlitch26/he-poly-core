# Contributing Guidelines

Contributions are welcome.

## Workflow
1. Fork and clone the repository.
2. Create a branch:
   ```
   git checkout -b feature/avx512-ntt
   ```
3. Build and test:
   ```
   mkdir build && cd build
   cmake ..
   make -j$(nproc)
   ctest
   ```
4. Run benchmarks:
   ```
   export BENCH_N=8192
   ./bench_compare_avx_full
   ```
5. Submit a PR with benchmark results and documentation updates.

## Style
- C++17
- 2-space indentation
- Comment on intent, not syntax
