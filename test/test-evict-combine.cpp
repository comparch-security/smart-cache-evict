#include "cache/cache.hpp"
#include "cache/algorithm.hpp"
#include "util/random.hpp"
#include <cstdio>

int main() {
  init_cfg();
  randomize_seed();
  int way = 32;
  int succ = 0, iter = 0, keep = 0;
  int csize = 8000;
  int way_pre = way;
  while (keep < 3 && iter < 200) {
    elem_t *victim = allocate_list(1);
    elem_t *candidate = NULL;
    calibrate(victim);
    bool rv = trim_tar_combined_ran(&candidate, victim, way, csize, 600, 10);
    int m_way = way;
    if(rv) {
      printf("verify result");
      do {
        printf(".");
        m_way = way;
        rv = trim_tar_ran(&candidate, victim, way);
      } while(rv && m_way != way);
      printf("\n");
    }
    free_list(victim);
    if(rv) {
      succ++;
      if(way_pre == way)
        keep++;
      else {
        keep = 0;
        way_pre = way;
      }
    } else {
      way = way_pre;
    }
    iter++;
    printf("trials %d sucesses: %d keep: %d result: %d, way=%d\n", iter, succ, keep, rv, way);
  }
  printf("the predicted way = %d\n", way);
  return 0;
}
