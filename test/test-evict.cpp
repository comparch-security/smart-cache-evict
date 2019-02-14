#include "cache/cache.hpp"
#include "cache/algorithm.hpp"
#include "util/random.hpp"
#include <cstdio>

int main() {
  init_cfg();
  randomize_seed();
  int way = 32;
  for (int i=0; i<20; i++) {
    elem_t *candidate = allocate_list(90000);
    elem_t *victim = allocate_list(1);
    calibrate(victim);
    while(!test_tar(candidate, victim)) {
      free_list(candidate);
      free_list(victim);
      candidate = allocate_list(90000);
      victim = allocate_list(1);
      calibrate(victim);
    }
    bool rv = trim_tar_split(&candidate, victim, way);
    free_list(candidate);
    free_list(victim);
  }
  printf("the predicted way = %d\n", way);
  return 0;
}
