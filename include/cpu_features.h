\
#pragma once
#if defined(__GNUC__) || defined(__clang__)
#include <cpuid.h>
#include <cstddef>
static inline bool cpu_has_avx2() {
#if defined(__x86_64__) || defined(_M_X64) || defined(__i386)
    unsigned int eax, ebx, ecx, edx;
    if (!__get_cpuid_max(0, nullptr)) return false;
    if (!__get_cpuid_count(7, 0, &eax, &ebx, &ecx, &edx)) return false;
    return (ebx & (1 << 5)) != 0; // AVX2 bit in EBX of leaf 7
#else
    return false;
#endif
}
#else
static inline bool cpu_has_avx2(){ return false; }
#endif
