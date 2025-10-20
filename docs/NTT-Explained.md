# Number Theoretic Transform (NTT) Explained

NTT performs FFT-like convolution in modular arithmetic.

Each stage:
```
u = a[j]
v = a[j + len] * roots[k] mod q
a[j] = (u + v) mod q
a[j+len] = (u - v) mod q
```
Inverse NTT multiplies by N^{-1} mod q.
