# Variables to override
#
# CC            C compiler
# CROSSCOMPILE  crosscompiler prefix, if any
# CFLAGS  compiler flags for compiling all C files
# ERL_CFLAGS  additional compiler flags for files using Erlang header files
# ERL_EI_LIBDIR path to libei.a
# LDFLAGS linker flags for linking all binaries
# ERL_LDFLAGS additional linker flags for projects referencing Erlang libraries

LDFLAGS +=
CFLAGS ?= -O2 -Wall -Wextra -Wno-unused-parameter
CFLAGS += -std=c99 -D_GNU_SOURCE
CC ?= $(CROSSCOMPILER)gcc

#CFLAGS += -DDEBUG

SRC=$(wildcard src/*.cpp)
SRC1=src/riseset.cpp
SRC2=src/moonphase.cpp

$(UNAME_S)

# -lrt is needed for clock_gettime() on linux with glibc before version 2.17
# (for example raspbian wheezy)
UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Linux)
  LDFLAGS += -lrt
endif

CFLAGS = -I"/usr/local/include/"
ERL_ROOT_DIR = $(ERLHOME)

# Look for the EI library and header files
# For crosscompiled builds, ERL_EI_INCLUDE_DIR and ERL_EI_LIBDIR must be
# passed into the Makefile.
ifeq ($(ERL_EI_INCLUDE_DIR),)
ERL_ROOT_DIR = $(shell erl -eval "io:format(\"~s~n\", [code:root_dir()])" -s init stop -noshell)
ifeq ($(ERL_ROOT_DIR),)
   $(error Could not find the Erlang installation. Check to see that 'erl' is in your PATH)
endif
ERL_EI_INCLUDE_DIR = "$(ERL_ROOT_DIR)/usr/include"
ERL_EI_LIBDIR = "$(ERL_ROOT_DIR)/usr/lib"
endif

# Set Erlang-specific compile and linker flags
# ERL_CFLAGS ?= -I$(ERL_EI_INCLUDE_DIR) -L$(/usr/lib)
ERL_CFLAGS ?= -I$(ERL_EI_INCLUDE_DIR)
ERL_LDFLAGS ?= -L$(ERL_EI_LIBDIR) -lei 

# ERL_LDFLAGS += -lc++
ERL_CFLAGS += -I$(/usr/local/lib/erlang/usr/include)


# If compiling on OSX and not crosscompiling, include CoreFoundation and IOKit
ifeq ($(CROSSCOMPILE),)
ifeq ($(shell uname),Darwin)
LDFLAGS += -framework CoreFoundation -framework IOKit
endif
endif

host-type := $(shell arch)
dest_lib := priv/${MIX_TARGET}
LIB = $(dest_lib)/lunar2.o  -lm

.PHONY: all clean

all: src $(dest_lib) $(dest_lib)/lunar2.o $(dest_lib)/ps_1996.dat $(dest_lib)/elp82.dat $(dest_lib)/vsop.bin $(dest_lib)/planet_loc $(dest_lib)/riseset $(dest_lib)/moonphase

$(dest_lib)/vsop.bin:
	cp priv/vsop.bin $(dest_lib)/vsop.bin

$(dest_lib)/elp82.dat:
	cp priv/elp82.dat $(dest_lib)/elp82.dat

$(dest_lib)/ps_1996.dat:
	cp priv/ps_1996.dat $(dest_lib)/ps_1996.dat

$(dest_lib):
	mkdir -p $(dest_lib)

$(dest_lib)/%.o: src/lunar-master/%.cpp
	$(CC) -c $(ERL_CFLAGS) $(CFLAGS) -o $@ $<

$(dest_lib)/riseset3.o: src/lunar-master/riseset3.cpp src/lunar-master/riseset3.h
	$(CC) -c $(ERL_CFLAGS) $(CFLAGS) -o $@ $<

$(dest_lib)/%.o: src/%.cpp
	$(CC) -c $(ERL_CFLAGS) $(CFLAGS) -o $@ $<

$(dest_lib)/lunar2.o: src/lunar-master/lunar2.cpp src/lunar-master/lunar.h
	$(CC) -c $(ERL_CFLAGS) $(CFLAGS) -o $@ $<

$(dest_lib)/planet_loc: $(dest_lib)/planet_loc.o $(dest_lib)/cospar.o $(dest_lib)/nutation.o  $(dest_lib)/support.o $(dest_lib)/delta_t.o $(dest_lib)/date.o $(dest_lib)/miscell.o $(dest_lib)/de_plan.o $(dest_lib)/precess.o $(dest_lib)/mpc_code.o  $(dest_lib)/obliquit.o $(dest_lib)/elp82dat.o
	$(CXX)  -g $(CFLAGS) $^ $(ERL_LDFLAGS) $(LDFLAGS) -o $@ $(LIB)

$(dest_lib)/riseset: $(dest_lib)/riseset.o $(dest_lib)/riseset3.o $(dest_lib)/miscell.o $(dest_lib)/date.o $(dest_lib)/obliquit.o $(dest_lib)/vsopson.o $(dest_lib)/support.o
	$(CXX)  -g $(CFLAGS) $^ $(ERL_LDFLAGS) $(LDFLAGS) -o $@ $(LIB)

$(dest_lib)/moonphase: $(dest_lib)/moonphase.o $(dest_lib)/vsopson.o $(dest_lib)/miscell.o $(dest_lib)/date.o $(dest_lib)/obliquit.o $(dest_lib)/delta_t.o $(dest_lib)/support.o
	$(CXX) $^ $(ERL_LDFLAGS) $(LDFLAGS) -o $@ $(LIB)

clean:
	rm -f $(dest_lib)/riseset $(dest_lib)/moonphase $(dest_lib)/*.o
