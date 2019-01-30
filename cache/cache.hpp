#ifndef SCE_CACHE_HPP
#define SCE_CACHE_HPP

#include "common/definitions.hpp"

extern void calibrate(elem_t *ptr);

extern bool test_tar(elem_t *ptr, elem_t *victim);
extern bool test_arb(elem_t *ptr);

extern void traverse_list_1(elem_t *ptr);
extern void traverse_list_2(elem_t *ptr);
extern void traverse_list_3(elem_t *ptr);
extern void traverse_list_4(elem_t *ptr);
typedef void (*traverse_func)(elem_t *);
extern traverse_func choose_traverse_func(int);

extern int list_size(elem_t *ptr);
extern elem_t *pick_from_list(elem_t **ptr, int ltsz, int pksz);
extern elem_t *append_list(elem_t *lptr, elem_t *rptr);
extern float evict_rate(int ltsz, int trial);

#endif
