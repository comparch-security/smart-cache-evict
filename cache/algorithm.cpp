#include "cache/algorithm.hpp"
#include "cache/cache.hpp"

#include <vector>

#include <cstdio>

bool trim_tar(elem_t **candidate, elem_t *victim) {
  std::vector<elem_t *> stack(CFG.rblimit, 0);
  int stack_read = 0, stack_write = 0;
  bool started = false;
  int retry = 0;
  int way = 32;
  int ltsz = list_size(*candidate);
  while(true) {
    int step = ltsz > way ? ltsz / way : 1;
    stack[stack_write] = pick_from_list(candidate, ltsz, step);
    if(test_tar(*candidate, victim)) {
      printf("%d (%d) \n", ltsz, retry); fflush(stdout);
      retry = 0;
      ltsz -= step;
      stack_write = (stack_write + 1) % CFG.rblimit;
      if(stack_read == stack_write) {
        free_list(stack[stack_read]);
        stack_read = (stack_read + 1) % CFG.rblimit;
      }
    } else {
      *candidate = append_list(*candidate, stack[stack_write]);
      if(CFG.retry && retry < CFG.rtlimit) retry++;
      else if(ltsz > way && CFG.rollback && stack_read != stack_write) {
        printf("%d (%d)\n", ltsz, retry); fflush(stdout);
        stack_write = (stack_write + CFG.rblimit - 1) % CFG.rblimit;
        ltsz += list_size(stack[stack_write]);
        *candidate = append_list(*candidate, stack[stack_write]);
        retry = 0;
      } else
        break;
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
