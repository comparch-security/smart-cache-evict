#ifndef SCE_CACHE_HPP
#define SCE_CACHE_HPP

#include "common/definitions.hpp"
#include <vector>

extern void calibrate(elem_t *ptr);

extern bool test_tar(elem_t *ptr, elem_t *victim);
extern bool test_tar_lists(std::vector<elem_t *> &lists, elem_t *victim, int skip);
extern bool test_arb(elem_t *ptr);

extern void traverse_list_1(elem_t *ptr);
extern void traverse_list_2(elem_t *ptr);
extern void traverse_list_3(elem_t *ptr);
extern void traverse_list_4(elem_t *ptr);
extern void traverse_list_rr(elem_t *ptr);
typedef void (*traverse_func)(elem_t *);
extern traverse_func choose_traverse_func(int);

extern int list_size(elem_t *ptr);
extern elem_t *pick_from_list(elem_t **ptr, int pksz);
extern elem_t *append_list(elem_t *lptr, elem_t *rptr);
extern std::vector<elem_t *> split_list(elem_t *lptr, int way);
extern elem_t *combine_lists(std::vector<elem_t *>lists);
extern float evict_rate(int ltsz, int trial);

extern void print_set(elem_t *ptr);

#endif
