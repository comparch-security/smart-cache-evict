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

extern void calibrate(elem_t *victim);

extern bool test_tar(elem_t ptr, char *victim);
extern bool test_arb(elem_t ptr, char *victim);

extern void traverse_list_1(elem_t *ptr);
extern void traverse_list_2(elem_t *ptr);
extern void traverse_list_3(elem_t *ptr);
extern void traverse_list_4(elem_t *ptr);
typedef void (*traverse_func)(elem_t *);
extern traverse_func choose_traverse_func(int);

extern elem_t *init_list(uint32_t ltsz, uint32_t emsz);
extern uint32_t list_size(elem_t *ptr);
extern elem_t *pick_from_list(elem_t *ptr, uint32_t ltsz, uint32_t pksz);

#endif
