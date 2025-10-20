# Modular Arithmetic and Montgomery Reduction

Montgomery representation replaces division by modulus with bit shifts.

Algorithm:
```
montgomery_mul(a,b):
    t = a * b
    u = (t * m') mod R
    x = (t + u*m) / R
    if x >= m: x -= m
```
Lazy reduction postpones final mod until needed.
