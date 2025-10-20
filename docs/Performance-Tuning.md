# Performance Tuning

Flags:
```
-O3 -march=native -mavx2 -mfma -funroll-loops -flto
```
Align data to 32B for AVX2. Use BENCH_N to control test size.
