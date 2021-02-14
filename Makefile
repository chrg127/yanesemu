VPATH=emu:emu/core:emu/util:emu/io:emu/video

HEADERS = bits.hpp bus.hpp cartridge.hpp cmdline.hpp cpu.hpp debug.hpp easyrandom.hpp \
		emulator.hpp file.hpp heaparray.hpp memmap.hpp ppu.hpp \
		stringops.hpp types.hpp unsigned.hpp video.hpp
# settings.hpp

OBJS = bus.o cartridge.o cmdline.o cpu.o easyrandom.o \
	   emulator.o file.o main.o ppu.o \
	   stringops.o video.o
# settings.o

LIBS = -lm -lSDL2 -lfmt -lpthread

CXX = g++
CFLAGS = -I. -std=c++17 -Wall -Wextra -pipe \
		 -Wcast-align -Wcast-qual -Wpointer-arith \
		 -Wformat=2 -Wmissing-include-dirs -Wno-unused-parameter \
		 -fno-rtti


PRGNAME = emu

all: debug

DEBDIR = debug
DEBOBJS = $(patsubst %,$(DEBDIR)/%,$(OBJS))
$(DEBDIR)/cpu.o: emu/core/cpu.cpp emu/core/opcodes.cpp emu/core/disassemble.cpp $(HEADERS)
	$(CXX) $(CFLAGS) -c $< -o $@
$(DEBDIR)/ppu.o: emu/core/ppu.cpp emu/core/ppumain.cpp $(HEADERS)
	$(CXX) $(CFLAGS) -c $< -o $@
$(DEBDIR)/video.o: emu/video/video.cpp emu/video/videosdl.cpp $(HEADERS)
	$(CXX) $(CFLAGS) -c $< -o $@
$(DEBDIR)/%.o: %.cpp $(HEADERS)
	$(CXX) $(CFLAGS) -c $< -o $@
$(DEBDIR)/$(PRGNAME): $(DEBOBJS)
	$(CXX) $(DEBOBJS) -o $(DEBDIR)/$(PRGNAME) $(LIBS)

# this duplication is unfortunately necessary.
RELDIR = release
RELOBJS = $(patsubst %,$(RELDIR)/%,$(OBJS))
$(RELDIR)/cpu.o: emu/core/cpu.cpp emu/core/opcodes.cpp emu/core/disassemble.cpp $(HEADERS)
	$(CXX) $(CFLAGS) -c $< -o $@
$(RELDIR)/ppu.o: emu/core/ppu.cpp emu/core/ppumain.cpp $(HEADERS)
	$(CXX) $(CFLAGS) -c $< -o $@
$(RELDIR)/video.o: emu/video/video.cpp emu/video/videosdl.cpp $(HEADERS)
	$(CXX) $(CFLAGS) -c $< -o $@
$(RELDIR)/%.o: %.cpp $(HEADERS)
	$(CXX) $(CFLAGS) -c $< -o $@
$(RELDIR)/$(PRGNAME): $(RELOBJS)
	$(CXX) $(RELOBJS) -o $(RELDIR)/$(PRGNAME) $(LIBS)

.PHONY: clean directories debug release

directories:
	mkdir -p $(DEBDIR) $(RELDIR)

clean:
	rm -rf $(DEBDIR)/*.o $(DEBDIR)/$(DEBPRGNAME) $(RELDIR)/*.o $(RELDIR)/$(RELPRGNAME)

debug: CFLAGS += -g
debug: directories $(DEBDIR)/$(PRGNAME)

release: CFLAGS += -O3
release: directories $(RELDIR)/$(PRGNAME)
