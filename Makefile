##
## Using this Makefile:
##   1) Release build:     make
##   2) Debug build:       make mode=debug
##   3) Clean everything:  make clean
##

#
# Configuration
#

include_paths = /usr/include/eigen3
CXXFLAGS = -Wall -Wextra -std=c++11 -pedantic -msse4.2 \
           $(foreach path,$(include_paths),-I$(path))
LDFLAGS = -lm -pthread

mode ?= release

ifeq ($(mode), release)
	CXXFLAGS += -O3 -DNDEBUG
else
	CXXFLAGS += -ggdb
endif

#
# Rules
#

srcdir	   = src
objdir     = $(mode)
target     = $(objdir)/oxatrace
cxxsources = $(wildcard $(srcdir)/*.cpp)
cxxobjects = $(patsubst $(srcdir)/%.cpp,$(objdir)/%.o,$(cxxsources))
depfiles   = $(patsubst $(srcdir)/%.cpp,$(objdir)/%.d,$(cxxsources))

-include $(depfiles)

.PHONY: all clean
.DEFAULT_GOAL = all

all: $(target)

clean:
	rm -f $(target) $(cxxobjects) $(depfiles)

$(target) : $(objdir) $(cxxobjects) Makefile
	$(CXX) $(LDFLAGS) $(cxxobjects) -o $@

$(cxxobjects) : $(objdir)/%.o : $(srcdir)/%.cpp
	$(CXX) $(CXXFLAGS) $< -c -o $@ -MD -MF $(objdir)/$*.d

$(objdir):
	mkdir $@