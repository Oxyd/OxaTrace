##
## Using this Makefile:
##   1) Release build:     make
##   2) Debug build:       make mode=debug
##   3) Clean everything:  make clean
##

#
# Configuration
#

CXX = g++

include_paths = /usr/include/eigen3
libraries = m boost_program_options

# -Wno-unused-local-typedefs silences warnings from Eigen.
CXXFLAGS = -Wall -Wextra -Wno-unused-local-typedefs -std=c++11 -pedantic \
           -msse4.2 \
           $(foreach path,$(include_paths),-I$(path))

# We want LDFLAGS to come before all object files and libs to come after them.
LDFLAGS = -pthread
libs = $(foreach lib,$(libraries),-l$(lib))

mode ?= release

ifeq ($(mode), release)
	CXXFLAGS += -O3 -DNDEBUG
else ifeq ($(mode), profile)
	CXXFLAGS += -O3 -ggdb
else
	CXXFLAGS += -Og -ggdb
endif

#
# Rules
#

srcdir	   = src
objdir     = $(mode)
docdir     = doc
target     = $(objdir)/oxatrace
cxxsources = $(wildcard $(srcdir)/*.cpp)
cxxobjects = $(patsubst $(srcdir)/%.cpp,$(objdir)/%.o,$(cxxsources))
depfiles   = $(patsubst $(srcdir)/%.cpp,$(objdir)/%.d,$(cxxsources))

-include $(depfiles)

.PHONY: all clean doc
.DEFAULT_GOAL = all

all: $(target)

clean:
	rm -f $(target) $(cxxobjects) $(depfiles)
	rm -rf $(docdir)

doc:
	doxygen Doxyfile

$(target) : $(objdir) $(cxxobjects) Makefile
	$(CXX) $(LDFLAGS) $(cxxobjects) $(libs) -o $@

$(cxxobjects) : $(objdir)/%.o : $(srcdir)/%.cpp
	$(CXX) $(CXXFLAGS) $< -c -o $@ -MD -MF $(objdir)/$*.d

$(objdir):
	mkdir $@
