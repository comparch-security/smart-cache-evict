// #define SCE_CACHE_CALIBRATE_HISTO

#include "common/definitions.hpp"
#include "cache/cache.hpp"
#include "util/assembly.hpp"
#include "util/random.hpp"

#include <cassert>
#include <cstdlib>
#include <set>

#ifdef SCE_CACHE_CALIBRATE_HISTO
  #include "util/statistics.hpp"
  #include <iomanip>
  #include <fstream>
#endif

void calibrate(elem_t *victim) {
  double unflushed = 0.0;
  double flushed = 0.0;

#ifdef SCE_CACHE_CALIBRATE_HISTO
  uint32_t stat_histo_unflushed = init_histo_stat(20, CFG.calibrate_repeat);
  uint32_t stat_histo_flushed = init_histo_stat(20, CFG.calibrate_repeat);
#endif
  
  maccess (victim);
  maccess (victim);
  maccess (victim);
  maccess (victim);

  for (int i=0; i<CFG.calibrate_repeat; i++) {
    maccess (victim);
    maccess (victim);
    maccess (victim);
    maccess (victim);

    uint64_t time = rdtscfence();
    maccess (victim);
    uint64_t delta = rdtscfence() - time;
    unflushed += delta;

#ifdef SCE_CACHE_CALIBRATE_HISTO
    record_histo_stat(stat_histo_unflushed, (double)(delta));
#endif
  }
  unflushed /= CFG.calibrate_repeat;

  for (int i=0; i<CFG.calibrate_repeat; i++) {
    maccess (victim);
    maccess (victim);
    maccess (victim);
    maccess (victim);

    flush (victim);
    uint64_t time = rdtscfence();
    maccess (victim);
    uint64_t delta = rdtscfence() - time;
    flushed += delta;

#ifdef SCE_CACHE_CALIBRATE_HISTO
    record_histo_stat(stat_histo_flushed, (double)(delta));
#endif
  }
  flushed /= CFG.calibrate_repeat;

#ifdef SCE_CACHE_CALIBRATE_HISTO
  {
    std::ofstream outfile("unflushed.data", std::ofstream::app);
    auto hist = get_histo_density(stat_histo_unflushed);
    for(uint32_t i=0; i<hist.size(); i++)
      outfile << hist[i].first << "\t0\t0\t" << hist[i].second << "\t0" << std::endl;
    outfile.close();
  }

  {
    std::ofstream outfile("flushed.data", std::ofstream::app);
    auto hist = get_histo_density(stat_histo_flushed);
    for(uint32_t i=0; i<hist.size(); i++)
      outfile << hist[i].first << "\t0\t0\t" << hist[i].second << "\t0" << std::endl;
    outfile.close();
  }
#endif

  assert(flushed > unflushed);
  CFG.threshold = (int)((flushed + 2 * unflushed) / 2.0);
}

bool test_tar(elem_t *ptr, elem_t *victim) {
  double latency = 0.0;
  for(int i=0; i<CFG.trials; i++) {
	maccess (victim);
	maccess (victim);
	maccess (victim);
	maccess (victim);

	for(int j=0; j<CFG.scans; j++)
      CFG.traverse(ptr);

	// page walk
    maccess (victim + 8);
    maccess (victim - 8);

	uint64_t time = rdtscfence();
	maccess (victim);
	latency += (double)(rdtscfence() - time);
  }
  return (latency / CFG.trials) > (double)CFG.threshold;
}

bool test_arb(elem_t *ptr) {
  int count = 0;
  for(int i=0; i<CFG.trials; i++) {
	for(int j=0; j<CFG.scans; j++)
      CFG.traverse(ptr);

    elem_t *p = ptr;
    while(p) {
      uint64_t time = rdtscfence();
      maccess(p);
      if(rdtscfence() - time > CFG.threshold)
        count++;
      p = p->next;
    }
  }
  return count / CFG.trials  > CFG.cache_way;
}

void traverse_list_1(elem_t *ptr) {
  while(ptr) {
    maccess(ptr);
    ptr = ptr->next;
  }
}

void traverse_list_2(elem_t *ptr) {
  while(ptr) {
    maccess(ptr);
    maccess(ptr);
    ptr = ptr->next;
  }
}

void traverse_list_3(elem_t *ptr) {
  while(ptr) {
    maccess(ptr);
    maccess(ptr);
    maccess(ptr);
    ptr = ptr->next;
  }
}

void traverse_list_4(elem_t *ptr) {
  while(ptr) {
    maccess(ptr);
    maccess(ptr);
    maccess(ptr);
    maccess(ptr);
    ptr = ptr->next;
  }
}

traverse_func choose_traverse_func(int t) {
  switch(t) {
  case 1:  return traverse_list_1;
  case 2:  return traverse_list_2;
  case 3:  return traverse_list_3;
  default: return traverse_list_4;
  }
}

elem_t *init_list(uint32_t ltsz, uint32_t emsz) {
  elem_t *ptr = (elem_t *)malloc(emsz);
  elem_t *rv = ptr;
  ptr->prev = NULL;
  for(uint32_t i=1; i<ltsz; i++) {
    ptr->next = (elem_t *)malloc(emsz);
    ptr->next->prev = ptr;
    ptr = ptr->next;
  }
  ptr->next = NULL;
  return rv;
}

uint32_t list_size(elem_t *ptr) {
  int32_t rv = 0;
  while(ptr) {
    rv++;
    ptr = ptr->next;
  }
}

elem_t *pick_from_list(elem_t **pptr, uint32_t ltsz, uint32_t pksz) {
  std::set<uint32_t> pick_set;
  while(pick_set.size() < pksz) {
    pick_set.insert(random_fast() % ltsz);
  }

  uint32_t index = 0;
  elem_t *rv, *pick = NULL, *ptr = *pptr;
  while(ptr) {
    if(pick_set.count(index)) {
      elem_t *p = ptr;
      ptr = ptr->next;

      if(p->prev != NULL) p->prev->next = p->next; else *pptr = p->next;
      if(p->next != NULL) p->next->prev = p->prev;

      if(pick == NULL) rv = p;
      else             pick->next = p;
      p->prev = pick;
      p->next = NULL;
      pick = p;
    } else {
      ptr = ptr->next;
    }
    index++;
  }
  return rv;
}
