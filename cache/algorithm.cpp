#include "cache/algorithm.hpp"
#include "cache/cache.hpp"

#include <vector>

bool trim_tar(elem_t **candidate, elem_t **evict, elem_t *victim) {
  std::vector<elem_t *> stack(CFG.rblimit, NULL);
  int stack_read = 0, stack_write = 0;
  bool stack_valid = false;
  int retry = 0;
  int way = 32;
  while((!CFG.retry    || retry < CFG.rtlimit ) &&
        (!CFG.rollback || !stack_valid || stack_read != stack_write)
        ) {
    
  }
}
