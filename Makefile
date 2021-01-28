VPATH=emu:emu/core:emu/util:emu/io:emu/video

HEADERS = bits.hpp bus.hpp cartridge.hpp cmdline.hpp cpu.hpp debug.hpp easyrandom.hpp \
		emulator.hpp file.hpp heaparray.hpp memmap.hpp ppu.hpp \
		stringops.hpp types.hpp unsigned.hpp video.hpp
# settings.hpp

OBJS = bus.o cartridge.o cmdline.o cpu.o easyrandom.o \
	   emulator.o file.o main.o ppu.o \
	   stringops.o video.o
# settings.o

LIBS = -lm -lSDL2 -lfmt

CXX = g++
CFLAGS = -I. -std=c++17 -Wall -Wextra -pipe \
		 -Wcast-align -Wcast-qual -Wpointer-arith \
		 -Wformat=2 -Wmissing-include-dirs -Wno-unused-parameter

DEBDIR = debug
DEBPRGNAME = emu
DEBCFLAGS = -g
DEBOBJS = $(patsubst %,$(DEBDIR)/%,$(OBJS))
RELDIR = release
RELPRGNAME = emu-release

all: debug

# these are special - they #include other .cpp files
$(DEBDIR)/cpu.o: emu/core/cpu.cpp emu/core/opcodes.cpp emu/core/disassemble.cpp $(HEADERS)
	$(CXX) $(CFLAGS) -c $< -o $@

$(DEBDIR)/ppu.o: emu/core/ppu.cpp emu/core/ppumain.cpp $(HEADERS)
	$(CXX) $(CFLAGS) -c $< -o $@

$(DEBDIR)/video.o: emu/video/video.cpp emu/video/videosdl.cpp $(HEADERS)
	$(CXX) $(CFLAGS) -c $< -o $@

$(DEBDIR)/%.o: %.cpp $(HEADERS)
	$(CXX) $(CFLAGS) -c $< -o $@

directories:
	mkdir -p $(DEBDIR) $(RELDIR)

debug: CFLAGS += $(DEBCFLAGS)
debug: directories $(DEBPRGNAME)

$(DEBPRGNAME): $(DEBOBJS)
	$(CXX) $(DEBOBJS) -o $(DEBDIR)/$(DEBPRGNAME) $(LIBS)

.PHONY: clean
clean:
	rm -rf $(DEBDIR)/*.o $(DEBDIR)/$(DEBPRGNAME) $(RELDIR)/*.o $(RELDIR)/$(RELPRGNAME)

