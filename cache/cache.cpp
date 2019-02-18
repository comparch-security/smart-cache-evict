
#include "cache/cache.hpp"
#include "util/assembly.hpp"
#include "util/random.hpp"

#include <cassert>
#include <cstdlib>
#include <cstdint>
#include <set>
#include <vector>

#include <cstdio>

#define SCE_CACHE_CALIBRATE_HISTO

#ifdef SCE_CACHE_CALIBRATE_HISTO
  #include "util/statistics.hpp"
  #include <iomanip>
  #include <fstream>
#endif

void calibrate(elem_t *victim) {
  float unflushed = 0.0;
  float flushed = 0.0;

#ifdef SCE_CACHE_CALIBRATE_HISTO
  uint32_t stat_histo_unflushed = init_histo_stat(20, CFG.calibrate_repeat);
  uint32_t stat_histo_flushed = init_histo_stat(40, CFG.calibrate_repeat);
#endif
  
  maccess (victim);
  maccess (victim);
  maccess (victim);
  maccess_fence (victim);

  for (int i=0; i<CFG.calibrate_repeat; i++) {
    maccess (victim);
    maccess (victim);
    maccess (victim);
    maccess (victim);

    uint64_t time = rdtscfence();
    maccess_fence (victim);
    uint64_t delta = rdtscfence() - time;
    unflushed += delta;

#ifdef SCE_CACHE_CALIBRATE_HISTO
    record_histo_stat(stat_histo_unflushed, (float)(delta));
#endif
  }
  unflushed /= CFG.calibrate_repeat;

  for (int i=0; i<CFG.calibrate_repeat; i++) {
    maccess (victim);
    maccess (victim);
    maccess (victim);
    maccess_fence (victim);

    flush (victim);
    uint64_t time = rdtscfence();
    maccess_fence (victim);
    uint64_t delta = rdtscfence() - time;
    flushed += delta;

#ifdef SCE_CACHE_CALIBRATE_HISTO
    record_histo_stat(stat_histo_flushed, (float)(delta));
#endif
  }
  flushed /= CFG.calibrate_repeat;

#ifdef SCE_CACHE_CALIBRATE_HISTO
  {
    std::ofstream outfile("unflushed.data", std::ofstream::app);
    auto hist = get_histo_density(stat_histo_unflushed);
    outfile << "=================" << std::endl;
    for(int i=0; i<hist.size(); i++)
      outfile << hist[i].first << "\t0\t0\t" << hist[i].second << "\t0" << std::endl;
    outfile.close();
  }

  {
    std::ofstream outfile("flushed.data", std::ofstream::app);
    auto hist = get_histo_density(stat_histo_flushed);
    outfile << "=================" << std::endl;
    for(int i=0; i<hist.size(); i++)
      outfile << hist[i].first << "\t0\t0\t" << hist[i].second << "\t0" << std::endl;
    outfile.close();
  }
#endif

  assert(flushed > unflushed);
  CFG.flush_low = (int)((2.5*flushed + 1.0*unflushed) / 3.5);
  CFG.flush_high  = (int)(flushed * 1.5);
  printf("calibrate: (%f, %f) -> [%d : %d]\n", flushed, unflushed, CFG.flush_high, CFG.flush_low);

#ifdef SCE_CACHE_CALIBRATE_HISTO
  {
    std::ofstream outfile("flushed.data", std::ofstream::app);
    outfile << "[" << CFG.flush_low <<"," << CFG.flush_high << "]" << std::endl;
    outfile.close();
  }
#endif

}

bool test_tar(elem_t *ptr, elem_t *victim) {
  float latency = 0.0;
  int i=0, t=0;

  while(i<CFG.trials && t<CFG.trials*16) {
	maccess (victim);
	maccess (victim);
	maccess (victim);
	maccess_fence (victim);

	for(int j=0; j<CFG.scans; j++)
      CFG.traverse(ptr);

    if((char *)victim > CFG.pool_root + 2*CFG.elem_size)
      maccess_fence((char *)victim - 2*CFG.elem_size );

    if((char *)victim < CFG.pool_roof - 2*CFG.elem_size)
      maccess_fence((char *)victim + 2*CFG.elem_size);

	uint64_t delay = rdtscfence();
	maccess_fence (victim);
	delay = rdtscfence() - delay;
    printf("%ld ", delay);
    if(delay < CFG.flush_high) {
      latency += (float)(delay);
      i++;
    }
    t++;
  }

  printf("\n");

  if(i == CFG.trials) {
    latency /= i;
    return latency > (float)CFG.flush_low;
  } else {
    return false;
  }
}

bool test_tar_lists(std::vector<elem_t *> &lists, elem_t *victim, int skip) {
  float latency = 0.0;
  int i=0, t=0;

  while(i<CFG.trials && t<CFG.trials*16) {
	maccess (victim);
	maccess (victim);
	maccess (victim);
	maccess_fence (victim);

	for(int j=0; j<CFG.scans; j++)
      for(int k=0; k<lists.size(); k++)
        if(k!=skip) CFG.traverse(lists[k]);

    if((char *)victim > CFG.pool_root + 2*CFG.elem_size)
      maccess_fence((char *)victim - 2*CFG.elem_size );

    if((char *)victim < CFG.pool_roof - 2*CFG.elem_size)
      maccess_fence((char *)victim + 2*CFG.elem_size);

	uint64_t delay = rdtscfence();
	maccess_fence (victim);
	delay = rdtscfence() - delay;
    if(delay < CFG.flush_high) {
      latency += (float)(delay);
      i++;
    }
    t++;
  }

  if(i == CFG.trials) {
    latency /= i;
    return latency > (float)CFG.flush_low;
  } else {
    return false;
  }
}

bool test_arb(elem_t *ptr) {
  int count = 0;
  for(int i=0; i<CFG.trials; i++) {
	for(int j=0; j<CFG.scans; j++)
      CFG.traverse(ptr);

    elem_t *p = ptr;
    while(p) {
      uint64_t time = rdtscfence();
      maccess_fence(p);
      if(rdtscfence() - time > CFG.flush_low)
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
  while(ptr && ptr->next && ptr->next->next) {
    maccess(ptr);
    maccess(ptr->next);
    maccess(ptr->next->next);
    maccess(ptr);
    maccess(ptr->next);
    maccess(ptr->next->next);
    maccess(ptr);
    maccess(ptr->next);
    maccess(ptr->next->next);
    maccess(ptr);
    maccess(ptr->next);
    maccess(ptr->next->next);
    ptr = ptr->next;
  }
  if(ptr && ptr->next) {
    maccess(ptr);
    maccess(ptr->next);
    maccess(ptr);
    maccess(ptr->next);
    maccess(ptr);
    maccess(ptr->next);
    maccess(ptr);
    maccess(ptr->next);
    ptr = ptr->next;
  }
  if(ptr) {
    maccess(ptr);
    maccess(ptr);
    maccess(ptr);
    maccess(ptr);
    ptr = ptr->next;
  }
}


void traverse_list_rr(elem_t *ptr) {
  while(ptr->next) {
    maccess(ptr);
    ptr = ptr->next;
  }
  while(ptr->prev) {
    maccess(ptr);
    ptr = ptr->prev;
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

int list_size(elem_t *ptr) {
  int rv = 0;
  while(ptr) { rv++; ptr = ptr->next; }
  return rv;
}

elem_t *pick_from_list(elem_t **pptr, int pksz) {
  int ltsz = (*pptr)->ltsz;
  std::set<int> pick_set;
  while(pick_set.size() < pksz) {
    pick_set.insert(random_fast() % ltsz);
  }

  int index = 0;
  elem_t *rv, *pick = NULL, *ptr = *pptr, *pend = NULL;
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
      pend = ptr;
      ptr = ptr->next;
    }
    index++;
  }
  rv->ltsz = pksz;
  rv->tail = pick;
  (*pptr)->ltsz = ltsz - pksz;
  (*pptr)->tail = pend;
  return rv;
}

elem_t *append_list(elem_t *lptr, elem_t *rptr) {
  if(lptr == NULL) return rptr;
  if(rptr != NULL) {
    rptr->prev = lptr->tail;
    lptr->tail->next = rptr;
    lptr->ltsz += rptr->ltsz;
    lptr->tail = rptr->tail;
  }
  //printf("append into list size: %d-%d <- %d\n", lptr->ltsz, list_size(lptr), rptr->ltsz);
  return lptr;
}

std::vector<elem_t *> split_list(elem_t *ptr, int way) {
  int vsz = ptr->ltsz > way ? way : ptr->ltsz;
  std::vector<elem_t *> rv(vsz, NULL);
  std::vector<elem_t *> ltp(vsz, NULL);
  int index = 0;
  while(ptr) {
    if(rv[index] == NULL) {
      rv[index] = ptr;
      ltp[index] = ptr;
      ptr->prev = NULL;
      rv[index]->ltsz = 1;
    } else {
      rv[index]->ltsz++;
      ltp[index]->next = ptr;
      ptr->prev = ltp[index];
      ltp[index] = ptr;
    }
    index = (index + 1) % vsz;
    ptr = ptr->next;
  }
  for(int i=0; i<vsz; i++) {
    //printf("%d ", rv[i]->ltsz);
    ltp[i]->next = NULL;
    rv[i]->tail = ltp[i];
  }
  //printf("\n");
  return rv;
}

elem_t *combine_lists(std::vector<elem_t *>lists) {
  int vsz = lists.size();
  elem_t *rv = NULL, *ptr = NULL;
  int ltsz = 0;
  for(int i=0; i<vsz; i++) {
    if(lists[i] != NULL) {
      ltsz += lists[i]->ltsz;
      if(rv == NULL) {
        rv = lists[i];
      } else {
        ptr->next = lists[i];
        lists[i]->prev = ptr;
      }
      ptr = lists[i]->tail;
    }
  }
  rv->tail = ptr;
  rv->ltsz = ltsz;
  //printf("combine into list size: %d\n", ltsz);
  return rv;
}

float evict_rate(int ltsz, int trial) {
  float rate = 0.0;
  for(int i=0; i<trial; i++) {
    elem_t *ev_list = allocate_list(ltsz);
    calibrate(ev_list);
    bool res = test_tar(ev_list->next, ev_list);
    if(res) rate += 1.0;
    free_list(ev_list);
  }
  return rate / trial;
}

void print_set(elem_t *ptr) {
  printf("Set with %d elements:\n", ptr->ltsz);
  int i = 0;
  while(ptr) {
    printf("0x%016lx ", (uint64_t)ptr);
    ptr = ptr->next;
    i++;
    if(i==4) { printf("\n"); i=0;}
  }
  printf("\n");
}
