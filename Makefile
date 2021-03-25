VPATH := emu:emu/core:emu/util:emu/io:emu/video:tests

headers := emulator.hpp bus.hpp cartridge.hpp cpu.hpp const.hpp ppu.hpp debugger.hpp instrinfo.hpp clidbg.hpp \
		  bits.hpp cmdline.hpp debug.hpp easyrandom.hpp file.hpp heaparray.hpp settings.hpp stringops.hpp unsigned.hpp settings.hpp circularbuffer.hpp \
		  video.hpp opengl.hpp \
		  external/glad/glad.h external/glad/khrplatform.h

_objs := emulator.o bus.o cartridge.o cpu.o ppu.o debugger.o instrinfo.o clidbg.o \
	   cmdline.o easyrandom.o file.o stringops.o settings.o \
	   video.o opengl.o \
	   glad.o

libs := -lm -lSDL2 -lfmt

CC = gcc
CXX = g++
CFLAGS = -I. -std=c11
CXXFLAGS = -I. -std=c++20 -Wall -Wextra -pipe \
		 -Wcast-align -Wcast-qual -Wpointer-arith \
		 -Wformat=2 -Wmissing-include-dirs -Wno-unused-parameter \
		 -fno-rtti -fconcepts

programname := emu
profile := debug

# can be: linux, mingw64
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

ifeq ($(platform),linux)
    libs += -lpthread -ldl -lGL
else ifeq ($(platform),mingw64)
    libs += -lopengl32 -lmingw32 -lSDL2main -lSDL2
else
    $(error error: platform not supported)
endif

all: $(outdir)/$(programname)

$(outdir)/cpu.o: emu/core/cpu.cpp emu/core/instructions.cpp $(headers)
$(outdir)/ppu.o: emu/core/ppu.cpp emu/core/ppumain.cpp $(headers)
$(outdir)/glad.o: external/glad/glad.c $(headers)
	$(info Compiling $< ...)
	@$(CC) $(CFLAGS) -c $< -o $@

$(outdir)/%.o: %.cpp $(headers)
	$(info Compiling $< ...)
	@$(CXX) $(CXXFLAGS) -c $< -o $@

# main
objs := $(patsubst %,$(outdir)/%,$(_objs))
objs.main := $(outdir)/main.o
$(outdir)/$(programname): $(objs.main) $(objs)
	$(info Linking $@ ...)
	$(CXX) $(objs.main) $(objs) -o $@ $(libs)

# tests
objs.video_test := $(outdir)/video_test.o $(outdir)/video.o $(outdir)/opengl.o $(outdir)/glad.o
$(outdir)/video_test: $(objs.video_test) emu/video/video.hpp emu/video/opengl.hpp 
	$(info Linking $@ ...)
	$(CXX) $(objs.video_test) -o $@ $(libs)

_objs.ppu_test := ppu_test.o cpu.o ppu.o bus.o video.o opengl.o glad.o cartridge.o file.o easyrandom.o
objs.ppu_test := $(patsubst %,$(outdir)/%,$(_objs.ppu_test))
$(outdir)/ppu_test: $(objs.ppu_test)
	$(info Linking $@ ...)
	$(CXX) $(objs.ppu_test) -o $@ $(libs)

.PHONY: clean directories tests

directories:
	mkdir -p $(outdir)

tests: directories $(outdir)/video_test $(outdir)/ppu_test

clean:
	rm -rf $(outdir)/*

