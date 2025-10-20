\
# Stage-2: NTT Engine (Explanation & Usage)

This document explains the NTT implementation provided in `src/ntt.cpp` and how to run tests and benchmarks.

## What is NTT?
The Number Theoretic Transform (NTT) is a modular analogue of the Fast Fourier Transform (FFT). It replaces complex roots of unity with primitive roots modulo a prime `q` where `q = k * 2^m + 1` to allow power-of-two transforms.

We implement:
- iterative Cooley-Tukey radix-2 NTT (`ntt`)
- inverse NTT (`intt`)
- helper functions: `compute_roots`, `bit_reverse_permute`

## How to build (recommended)
We provide a `CMakeLists.txt` that will attempt to FetchContent Catch2 and GoogleBenchmark.
If your environment allows internet access, CMake will download these automatically.

Basic commands:
```bash
mkdir build && cd build
cmake ..
make -j
```

Run lightweight tests (works without external downloads):
```bash
./test_runner
```

Run Catch2 unit tests (requires Catch2 to be fetched by CMake):
```bash
./unit_tests
```

Run the simple chrono benchmark:
```bash
./bench_ntt
```

## Notes on parameters
The implementation assumes:
- `n` is a power of two.
- `mod` is a prime where a primitive `n`-th root exists (i.e., `mod = k*n + 1`).
- For serious HE work, you will need specific NTT-friendly primes used by SEAL/OpenFHE.

## Next steps
- Replace naive modular multiplications with Montgomery multiplication for speed.
- Implement lazy reduction in the butterfly to reduce modular reduction count.
- Vectorize inner loops with AVX2/AVX-512 intrinsics.
- Implement CUDA batched NTT kernel.

