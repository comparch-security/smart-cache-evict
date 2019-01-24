#include "cache/cache.hpp"
#include "util/assembly.hpp"
#include "util/statistics.hpp"

#include <iomanip>
#include <fstream>

int calibrate(char *victim) {
  uint32_t repeat = 500;
  double unflushed = 0.0;
  double flushed = 0.0;
  uint32_t stat_histo_unflushed = init_histo_stat(20, repeat);
  uint32_t stat_histo_flushed = init_histo_stat(20, repeat);
  
  maccess (victim);
  maccess (victim);
  maccess (victim);
  maccess (victim);

  for (int i=0; i<repeat; i++) {
    maccess (victim);
    maccess (victim);
    maccess (victim);
    maccess (victim);

    uint64_t time = rdtscfence();
    maccess (victim);
    uint64_t delta = rdtscfence() - time;
    record_histo_stat(stat_histo_unflushed, (double)(delta));
  }

  for (int i=0; i<repeat; i++) {
    maccess (victim);
    maccess (victim);
    maccess (victim);
    maccess (victim);

    flush (victim);
    uint64_t time = rdtscfence();
    maccess (victim);
    uint64_t delta = rdtscfence() - time;
    record_histo_stat(stat_histo_flushed, (double)(delta));
  }

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
  
  return 0;
}
