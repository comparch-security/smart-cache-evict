#include "cache/cache.hpp"
#include "util/random.hpp"
#include <cstdio>

int main() {
  init_cfg();
  randomize_seed();
  CFG.candidate_size = 90000;
  CFG.cache_size = 8 * (1 << 20);  // 8M for i7-3770 (ws-office)
  
}
