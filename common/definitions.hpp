#ifndef SCE_DEFINITIONS_HPP
#define SCE_DEFINITIONS_HPP

struct elem;

struct config {
  int candidate_size;               // number of candidate cache lines
  int cache_size;                   // size if the cache in bytes
  int cache_way;                    // number of ways
  int cache_slices;                 // number of LLC slices
  int flush_low;                    // the latency lower bound of considering as evicted
  int flush_high;                   // the latency higher bound of considering as evicted
  int trials;                       // number of trials for each iteration
  int scans;                        // number of scans for each trial
  int calibrate_repeat;             // repeat in the calibration process
  bool retry;
  bool backtracking;
  bool ignoreslice;
  bool findallcolors;
  bool findallcongruent;
  bool verify;
  void (*traverse)(struct elem*);   // list traver function
  int pool_size;                    // the size of the element pool
  int elem_size;                    // size of an element
  char *pool_root;                  // base memory address of the pool
  char *pool_roof;                  // max memory address of the pool
  elem *pool;                       // pointer of the pool
};

extern struct config CFG;
extern void init_cfg();
extern void dump_cfg();
#endif
