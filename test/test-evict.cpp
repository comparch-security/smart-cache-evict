#include "cache/cache.hpp"
#include "cache/algorithm.hpp"
#include "util/random.hpp"
#include <cstdio>

int main() {
  init_cfg();
  randomize_seed();
  int way = 32;
  for (int i=0; i<20; i++) {
    printf("(main 0) CFG.pool->size = %d\n", CFG.pool->ltsz);
    elem_t *candidate = allocate_list(90000);
    elem_t *victim = allocate_list(1);
    calibrate(victim);
    printf("(main 1) CFG.pool->size = %d\n", CFG.pool->ltsz);
    while(!test_tar(candidate, victim)) {
      free_list(candidate);
      free_list(victim);
      candidate = allocate_list(90000);
      victim = allocate_list(1);
      calibrate(victim);
      printf("(main 6) CFG.pool->size = %d\n", CFG.pool->ltsz);
    }
    printf("just test: %d\n", test_tar(candidate, victim));
    printf("(main 3) CFG.pool->size = %d\n", CFG.pool->ltsz);
    bool rv = trim_tar_split(&candidate, victim, way);
    printf("(main 4) CFG.pool->size = %d\n", CFG.pool->ltsz);
    printf("%d: %d\n", rv, list_size(candidate));
    free_list(candidate);
    free_list(victim);
    printf("(main 5) CFG.pool->size = %d\n", CFG.pool->ltsz);
  }
  return 0;
}
