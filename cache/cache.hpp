#ifndef SCE_CACHE_HPP
#define SCE_CACHE_HPP

#define MSZ_CL 56
#define MSZ_PG 4088

typedef struct elem {
  struct elem *next;
  struct elem *prev;
  int set;      // >0 : set id; -1: picked; -2: initialized
  unsigned int delta;
} elem_t;

// get the latency threshold
extern int calibrate(char *victim);

#endif
