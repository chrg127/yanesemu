VPATH=emu:emu/core:emu/utils:emu/io:emu/video
HEADERS = bus.hpp cpu.hpp memorymap.hpp ppu.hpp types.hpp ppubus.hpp \
		  nesrom.hpp file.hpp \
		  cmdargs.hpp debug.hpp stringops.hpp \
		  video.hpp videosdl.hpp

OBJS = main.o \
	   cpu.o bus.o ppu.o ppubus.o \
	   nesrom.o file.o \
	   cmdargs.o stringops.o \
	   video.o

LIBS = -lm -lSDL2

CXX = g++
CFLAGS = -I. -std=c++17 -Wall -Wextra -pipe \
		 -Wuninitialized -Wcast-align -Wcast-qual -Wpointer-arith \
		 -Wformat=2 -Wmissing-declarations -Wmissing-include-dirs \

DEBDIR = debug
DEBOBJDIR = $(DEBDIR)/obj
DEBPRGNAME = emu
DEBCFLAGS = -g
RELDIR = release
RELOBJDIR = $(RELDIR)/obj
RELPRGNAME = emu-release
RELCFLAGS = -O2

DEBOBJS = $(patsubst %,$(DEBOBJDIR)/%,$(OBJS))
RELOBJS = $(patsubst %,$(RELOBJDIR)/%,$(OBJS))

default: debug

directories:
	mkdir -p $(DEBDIR) $(DEBOBJDIR) $(RELDIR) $(RELOBJDIR)

debug: directories
debug: CFLAGS += $(DEBCFLAGS)
debug: $(DEBPRGNAME)
	@echo "using objects:" $(DEBOBJS) "and headers:" $(HEADERS)

$(DEBOBJDIR)/%.o: %.cpp $(HEADERS)
	$(CXX) $(CFLAGS) -c $< -o $@

$(DEBPRGNAME): $(DEBOBJS)
	$(CXX) $(DEBOBJS) -o $(DEBDIR)/$(DEBPRGNAME) $(LIBS)

release: directories
release: CFLAGS += $(RELCFLAGS)
release: $(RELPRGNAME)
	@echo "using objects:" $(DEBOBJS) "and headers:" $(HEADERS)

$(RELOBJDIR)/%.o: %.cpp $(HEADERS)
	$(CXX) $(CFLAGS) -c $< -o $@

$(RELPRGNAME): $(RELOBJS)
	$(CXX) $(RELOBJS) -o $(RELDIR)/$(RELPRGNAME) $(LIBS)

.PHONY: clean
clean:
	rm -f $(DEBOBJDIR)/*.o $(DEBDIR)/$(DEBPRGNAME) $(RELOBJDIR)/*.o $(RELDIR)/$(RELPRGNAME)

