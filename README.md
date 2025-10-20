he-poly-core
============

Homomorphic Encryption Polynomial Arithmetic Core

he-poly-core is a modular arithmetic and NTT acceleration library built for
Homomorphic Encryption (HE) systems. It implements core primitives like
Montgomery reduction, modular multiplication, and forward/inverse NTT — all
optimized with AVX2 SIMD and lazy reduction techniques.

Features
--------

- Montgomery multiplication with lazy reduction
- AVX2-assisted NTT butterfly with vectorized add/sub
- Benchmark harness for timing comparisons
- Deterministic modular arithmetic across hardware
- Clean API, embeddable into FHE/ML frameworks

Build and Run
-------------

Prerequisites:
- GCC >= 9 or Clang >= 10
- CMake >= 3.12
- x86_64 CPU with AVX2 support (for SIMD path)

Build:
```bash
mkdir build && cd build
cmake ..
make -j$(nproc)
```

Run Benchmarks:
```bash
export BENCH_N=8192
./bench_compare_avx_full
```

Architecture Overview
---------------------

| Component | Description |
|-----------|-------------|
| `mod_arith.*` | Core modular arithmetic (add, sub, mul) |
| `montgomery.*` | Montgomery domain conversion and reduction |
| `ntt.*` | Scalar NTT and polynomial transforms |
| `ntt_simd.*` | AVX2 vectorized butterflies |
| `bench_compare.cpp` | Timing harness for all variants |
| `cpu_features.h` | Runtime AVX2 detection |
| `docs/*` | Developer documentation and design notes |

Performance Notes
-----------------

Performance depends on CPU microarchitecture and compiler flags. Example results
from a tuned environment:

| Transform Size | Baseline | AVX2 | Speedup |
|----------------|----------|------|---------|
| N=4096  | 0.29 ms | 0.25 ms | 1.16× |
| N=8192  | 0.82 ms | 0.69 ms | 1.18× |
| N=16384 | 1.81 ms | 1.52 ms | 1.19× |

License
-------

MIT © 2025 he-poly-core Contributors

Contributing
------------

See `docs/Contributing.md` for guidelines on code style, testing, and PRs.
