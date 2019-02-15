#include "cache/algorithm.hpp"
#include "cache/cache.hpp"

#include <vector>
#include <cstdint>

#include <cstdio>

bool trim_tar_ran(elem_t **candidate, elem_t *victim, int &way) {
  std::vector<elem_t *> stack(CFG.rblimit, 0);
  int stack_read = 0, stack_write = 0;
  bool started = false;
  int retry = 0;
  int ltsz = (*candidate)->ltsz;
  int ltsz_min = ltsz;
  long long iter = 0;
  while(true) {
    int step = ltsz > way ? ltsz / way : 1;
    iter += ltsz - step;
    stack[stack_write] = pick_from_list(candidate, step);
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
        printf("%d (0x%016llx) %d\n", ltsz, iter, retry);
      }
    } else {
      *candidate = append_list(*candidate, stack[stack_write]);
      if(CFG.retry && retry < (step > 1 ? CFG.rtlimit : 4*way)) retry++;
      else if(ltsz > way + 1 && CFG.rollback && stack_read != stack_write) {
        stack_write = (stack_write + CFG.rblimit - 1) % CFG.rblimit;
        ltsz += stack[stack_write]->ltsz;
        *candidate = append_list(*candidate, stack[stack_write]);
        retry = 0;
      } else {
        if(ltsz <= way +1) way = ltsz - 1;
        break;
      }
    }
  }

  // housekeeping
  while(CFG.rollback && stack_read != stack_write) {
    free_list(stack[stack_read]);
    stack_read = (stack_read + 1) % CFG.rblimit;
  }

  printf("CFG.pool->size = %d\n", CFG.pool->ltsz);
  return ltsz <= way;
}

bool trim_tar_split(elem_t **candidate, elem_t *victim, int &way) {
  std::vector<elem_t *> stack(CFG.rblimit, 0);
  int stack_read = 0, stack_write = 0;
  bool started = false;
  int retry = 0;
  int ltsz = (*candidate)->ltsz;
  int ltsz_min = ltsz;
  int iter = 0, max_iter = way;
  int level = 0, rblevel = 0;
  while(true) {
    std::vector<elem_t *> lists = split_list(*candidate, way*2);
    int vsz = lists.size();
    int i;
    for(i=0; i<vsz; i++) {
      iter++;
      if(test_tar_lists(lists, victim, i)) {
        ltsz -= lists[i]->ltsz;
        stack[stack_write] = lists[i];
        lists[i] = NULL;
        stack_write = (stack_write + 1) % CFG.rblimit;
        level++;
        if(stack_read == stack_write) {
          free_list(stack[stack_read]);
          stack_read = (stack_read + 1) % CFG.rblimit;
        }
        if(ltsz < ltsz_min) {
          printf("%d (%d,%d,%d) %d\n", ltsz, level, iter, level-rblevel-1, i);
          max_iter += level;
          rblevel = level;
          iter = 0;
          ltsz_min = ltsz;
        }
        break;
      }
    }
    *candidate = combine_lists(lists);
    if(i == vsz) {
      if(iter > max_iter) {
        printf("failed with iteration %d > %d !\n", iter, max_iter);
        break;
      } else if(ltsz > way + 1 && CFG.rollback && stack_read != stack_write) {
        int max_rb = (stack_write < stack_read)
          ? stack_write + CFG.rblimit - stack_read
          : stack_write - stack_read;
        for(int r=0; r < 1 + (max_rb/4); r++) {
          stack_write = (stack_write + CFG.rblimit - 1) % CFG.rblimit;
          level--;
          ltsz += stack[stack_write]->ltsz;
          *candidate = append_list(*candidate, stack[stack_write]);
        }
        if(rblevel > level) rblevel = level;
      } else {
        break;
      }
    }
  }

  // housekeeping
  while(CFG.rollback && stack_read != stack_write) {
    free_list(stack[stack_read]);
    stack_read = (stack_read + 1) % CFG.rblimit;
  }

  if(ltsz <= way + 1) {
    printf("targeted victim: 0x%016lx\n", (uint64_t)victim);
    print_set(*candidate);
    if(way > ltsz)      way = (ltsz + way)/2;
    else if(way < ltsz) way = ltsz;
    return true;
  } else
    return false;
}
