VPATH := emu:emu/core:emu/util:emu/io:emu/video:tests

headers := bus.hpp cartridge.hpp cpu.hpp memmap.hpp ppu.hpp types.hpp \
		  bits.hpp cmdline.hpp debug.hpp easyrandom.hpp file.hpp heaparray.hpp settings.hpp stringops.hpp unsigned.hpp settings.hpp \
		  video.hpp opengl.hpp \
		  external/glad/glad.h external/glad/khrplatform.h

_objs := emulator.o \
	   bus.o cartridge.o cpu.o ppu.o \
	   cmdline.o easyrandom.o file.o stringops.o settings.o \
	   video.o opengl.o \
	   glad.o

libs := -lm -lSDL2 -lfmt -lpthread -ldl -lGL

CC = gcc
CXX = g++
CFLAGS = -I. -std=c11
CXXFLAGS = -I. -std=c++17 -Wall -Wextra -pipe \
		 -Wcast-align -Wcast-qual -Wpointer-arith \
		 -Wformat=2 -Wmissing-include-dirs -Wno-unused-parameter \
		 -fno-rtti

programname := emu
profile := debug
platform := linux

ifeq ($(profile),debug)
    outdir := debug
    CFLAGS += -g -DDEBUG
    CXXFLAGS += -g -DDEBUG
else
    outdir := release
    CFLAGS += -O3
    CXXFLAGS += -O3
endif

objs.main := $(outdir)/main.o
objs.test1 := $(outdir)/test1.o
objs := $(patsubst %,$(outdir)/%,$(_objs))

all: $(outdir)/$(programname)

$(outdir)/cpu.o: emu/core/cpu.cpp emu/core/opcodes.cpp emu/core/disassemble.cpp $(headers)
$(outdir)/ppu.o: emu/core/ppu.cpp emu/core/ppumain.cpp $(headers)
$(outdir)/glad.o: external/glad/glad.c $(headers)
	$(info Compiling $< ...)
	@$(CC) $(CFLAGS) -c $< -o $@

$(outdir)/%.o: %.cpp $(headers)
	$(info Compiling $< ...)
	@$(CXX) $(CXXFLAGS) -c $< -o $@

# main
$(outdir)/$(programname): $(objs.main) $(objs)
	$(info Linking $@ ...)
	@$(CXX) $(objs.main) $(objs) -o $@ $(libs)
# tests
$(outdir)/test1: $(objs.test1) $(objs)
	$(info Linking $@ ...)
	$(CXX) $(objs.test1) $(objs) -o $@ $(libs)

.PHONY: clean directories tests

directories:
	mkdir -p $(outdir)

tests: directories $(outdir)/test1

clean:
	rm -rf $(outdir)/*.o $(outdir)/$(programname)

