#include "cache/cache.hpp"
#include "cache/algorithm.hpp"
#include "util/random.hpp"
#include <cstdio>

int main() {
  init_cfg();
  randomize_seed();
  int way = 32;
  int succ = 0, iter = 0;
  while (succ < 20 && iter < 200) {
    elem_t *candidate = allocate_list(90000);
    elem_t *victim = allocate_list(1);
    calibrate(victim);
      free_list(candidate);
      free_list(victim);
      candidate = allocate_list(90000);
      victim = allocate_list(1);
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
    if(rv) succ++;
    iter++;
    printf("trials %d sucesses: %d result: %d, way=%d\n", iter, succ, rv, way);
  }
  printf("the predicted way = %d\n", way);
  return 0;
}
