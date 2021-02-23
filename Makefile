VPATH=emu:emu/core:emu/util:emu/io:emu/video

HEADERS = bus.hpp cartridge.hpp cpu.hpp memmap.hpp ppu.hpp types.hpp \
		  bits.hpp cmdline.hpp debug.hpp easyrandom.hpp file.hpp heaparray.hpp settings.hpp stringops.hpp unsigned.hpp \
		  video.hpp opengl.hpp \
		  external/glad/glad.h external/glad/khrplatform.h
		  # settings.hpp

OBJS = main.o emulator.o \
	   bus.o cartridge.o cpu.o ppu.o \
	   cmdline.o easyrandom.o file.o stringops.o \
	   video.o opengl.o \
	   glad.o
	   # settings.o

LIBS = -lm -lSDL2 -lfmt -lpthread -ldl -lGL

CC = gcc
CXX = g++
CFLAGS = -I. -std=c11
CXXFLAGS = -I. -std=c++17 -Wall -Wextra -pipe \
		 -Wcast-align -Wcast-qual -Wpointer-arith \
		 -Wformat=2 -Wmissing-include-dirs -Wno-unused-parameter \
		 -fno-rtti

PRGNAME = emu

all: debug

DEBDIR = debug
DEBOBJS = $(patsubst %,$(DEBDIR)/%,$(OBJS))
$(DEBDIR)/glad.o: external/glad/glad.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@
$(DEBDIR)/cpu.o: emu/core/cpu.cpp emu/core/opcodes.cpp emu/core/disassemble.cpp $(HEADERS)
	$(CXX) $(CXXFLAGS) -c $< -o $@
$(DEBDIR)/ppu.o: emu/core/ppu.cpp emu/core/ppumain.cpp $(HEADERS)
	$(CXX) $(CXXFLAGS) -c $< -o $@
$(DEBDIR)/%.o: %.cpp $(HEADERS)
	$(CXX) $(CXXFLAGS) -c $< -o $@
$(DEBDIR)/$(PRGNAME): $(DEBOBJS)
	$(CXX) $(DEBOBJS) -o $(DEBDIR)/$(PRGNAME) $(LIBS)

# this duplication is unfortunately necessary.
RELDIR = release
RELOBJS = $(patsubst %,$(RELDIR)/%,$(OBJS))
$(RELDIR)/cpu.o: emu/core/cpu.cpp emu/core/opcodes.cpp emu/core/disassemble.cpp $(HEADERS)
	$(CXX) $(CXXFLAGS) -c $< -o $@
$(RELDIR)/ppu.o: emu/core/ppu.cpp emu/core/ppumain.cpp $(HEADERS)
	$(CXX) $(CXXFLAGS) -c $< -o $@
$(RELDIR)/%.o: %.cpp $(HEADERS)
	$(CXX) $(CXXFLAGS) -c $< -o $@
$(RELDIR)/$(PRGNAME): $(RELOBJS)
	$(CXX) $(RELOBJS) -o $(RELDIR)/$(PRGNAME) $(LIBS)

.PHONY: clean directories debug release

directories:
	mkdir -p $(DEBDIR) $(RELDIR)

clean:
	rm -rf $(DEBDIR)/*.o $(DEBDIR)/$(DEBPRGNAME) $(RELDIR)/*.o $(RELDIR)/$(RELPRGNAME)

debug: CXXFLAGS += -g
debug: directories $(DEBDIR)/$(PRGNAME)

release: CXXFLAGS += -O3
release: directories $(RELDIR)/$(PRGNAME)
