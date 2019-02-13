
MAKE = make
CXX = g++
CXXFLAGS = --std=c++11 -O2 -g -I.

TARGETS = \
	run/test-evict-ratio \
	run/test-evict \

OBJECTS = \
	cache/cache.o \
	cache/algorithm.o \
	common/definitions.o \
	util/random.o \
	util/statistics.o \

HEADERS = $(wildcard cache/*.hpp) $(wildcard database/*.hpp) $(wildcard util/*.hpp)

all: $(TARGETS)

$(OBJECTS): %.o:%.cpp $(HEADERS)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(TARGETS): run/% : test/%.cpp $(OBJECTS)
	$(CXX) $(CXXFLAGS) $^ -o $@

clean:
	-rm $(TARGETS) $(OBJECTS)
