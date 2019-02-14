#include "cache/algorithm.hpp"
#include "cache/cache.hpp"

#include <vector>

#include <cstdio>

bool trim_tar(elem_t **candidate, elem_t *victim, int &way) {
  std::vector<elem_t *> stack(CFG.rblimit, 0);
  int stack_read = 0, stack_write = 0;
  bool started = false;
  int retry = 0;
  int ltsz = list_size(*candidate);
  int ltsz_min = ltsz;
  long long iter = 0;
  while(true) {
    int step = ltsz > way ? ltsz / way : 1;
    iter += ltsz - step;
    stack[stack_write] = pick_from_list(candidate, ltsz, step);
    if(test_tar(*candidate, victim)) {
      retry = 0;
      ltsz -= step;
      stack_write = (stack_write + 1) % CFG.rblimit;
      if(stack_read == stack_write) {
        free_list(stack[stack_read]);
        stack_read = (stack_read + 1) % CFG.rblimit;
      }
      if(ltsz < ltsz_min) {
        ltsz_min = ltsz;
        printf("%d (0x%016llx)\n", ltsz, iter);
      }
    } else {
      *candidate = append_list(*candidate, stack[stack_write]);
      if(CFG.retry && retry < (step > 1 ? CFG.rtlimit : 4*way)) retry++;
      else if(ltsz > way + 1 && CFG.rollback && stack_read != stack_write) {
        stack_write = (stack_write + CFG.rblimit - 1) % CFG.rblimit;
        ltsz += list_size(stack[stack_write]);
        *candidate = append_list(*candidate, stack[stack_write]);
        retry = 0;
      } else {
        if(ltsz <= way +1) way = ltsz;
        break;
      }
    }
  }

  // housekeeping
  while(CFG.rollback && stack_read != stack_write) {
    free_list(stack[stack_read]);
    stack_read = (stack_read + 1) % CFG.rblimit;
  }

  printf("\n");
  return ltsz <= way;
}
