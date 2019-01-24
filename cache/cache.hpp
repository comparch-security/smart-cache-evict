#ifndef SCE_CACHE_HPP
#define SCE_CACHE_HPP

#include <cstdint>

#define SZ_CL  64
#define ESZ_CL 56
#define ESZ_PG 4088

typedef struct elem {
  struct elem *next;
  struct elem *prev;
} elem_t;

// get the latency threshold
extern void calibrate(elem_t *victim);

// test an eviction set for a targeted address
extern bool test_tar(elem_t ptr, char *victim);

extern void traverse_list_1(elem_t *ptr);
extern void traverse_list_2(elem_t *ptr);
extern void traverse_list_3(elem_t *ptr);
extern void traverse_list_4(elem_t *ptr);
typedef void (*traverse_func)(elem_t *);
extern traverse_func choose_traverse_func(int);

#endif
