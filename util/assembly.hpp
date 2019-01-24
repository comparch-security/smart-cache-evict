#ifndef SCE_ASSEMBLY_HPP
#define SCE_ASSEMBLY_HPP

#include <cstdint>

inline void flush(void *p) {
  __asm__ volatile ("clflush 0(%0)" : : "c" (p) : "rax");
}

inline uint64_t rdtscfence() {
  uint64_t a, d;
  __asm__ volatile ("lfence");
  __asm__ volatile ("rdtsc" : "=a" (a), "=d" (d) : :);
  __asm__ volatile ("lfence");
  return ((d<<32) | a);
}

inline void maccess(void* p) {
  __asm__ volatile ("movq (%0), %%rax\n" : : "c" (p) : "rax");
}

#endif
