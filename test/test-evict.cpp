#include "cache/cache.hpp"
#include "cache/algorithm.hpp"
#include "util/random.hpp"
#include <cstdio>

int main() {
  init_cfg();
  randomize_seed();
  for (int i=0; i<10; i++) {
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
    printf("just test: %d\n", test_tar(candidate, victim));
    bool rv = trim_tar(&candidate, victim);
    printf("%d: %d\n", rv, list_size(candidate));
    free_list(candidate);
    free_list(victim);
  }
  return 0;
}
