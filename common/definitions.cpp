#include "common/definitions.hpp"
#include "cache/cache.hpp"
#include "database/json.hpp"

#include <fstream>

struct config CFG;
using json = nlohmann::json;
static json db;

#define CFG_SET_ENTRY(name, var, dvalue) \
  if(db.count(name)) var = db[name];     \
  else {                                 \
    var = dvalue;                        \
    db[name] = dvalue;                   \
  }                                      \

bool init_cfg() {
  std::ifstream db_file("configure.json");
  if(!db_file.fail()) {
    db_file >> db;
    db_file.close();
  }

  CFG_SET_ENTRY("candidate_size",   CFG.candidate_size,   0               )
  CFG_SET_ENTRY("cache_size",       CFG.cache_size,       0               )
  CFG_SET_ENTRY("cache_way",        CFG.cache_way,        0               )
  CFG_SET_ENTRY("cache_slices",     CFG.cache_slices,     0               )
  CFG_SET_ENTRY("threshold",        CFG.threshold,        0               )
  CFG_SET_ENTRY("trials",           CFG.trials,           4               )
  CFG_SET_ENTRY("scans",            CFG.scans,            4               )
  CFG_SET_ENTRY("calibrate_repeat", CFG.calibrate_repeat, 20000           )
  CFG_SET_ENTRY("retry",            CFG.retry,            true            )
  CFG_SET_ENTRY("backtracking",     CFG.backtracking,     true            )
  CFG_SET_ENTRY("ignoreslice",      CFG.ignoreslice,      true            )
  CFG_SET_ENTRY("findallcolors",    CFG.findallcolors,    false           )
  CFG_SET_ENTRY("findallcongruent", CFG.findallcongruent, false           )
  CFG_SET_ENTRY("verify",           CFG.verify,           true            )

  if(db.count("traverse")) {
    int t = db["traverse"];
    CFG.traverse = choose_traverse_func(t);
  } else {
    CFG.traverse = traverse_list_4;
    db["traverse"] = 4;
  }
}

static bool db_init = init_cfg();

void dump_cfg() {
  std::ofstream db_file("configure.json");
  if(!db_file.fail()) {
    db_file << db.dump(4);
    db_file.close();
  }
}


