VPATH=emu:emu/core:emu/utils
HEADERS = nesrom.hpp \
		  bus.hpp cpu.hpp memorymap.hpp \
		  cmdargs.hpp debug.hpp
OBJS = main.o nesrom.o \
	   cpu.o bus.o \
	   cmdargs.o
LIBS = -lm

CXX = g++
CFLAGS = -I. -std=c++17 -Wall -Wextra -pipe \
		 -Wuninitialized -Wcast-align -Wcast-qual -Wpointer-arith \
		 -Wformat=2 -Wmissing-declarations -Wmissing-include-dirs \

DEBDIR = debug
DEBOBJDIR = $(DEBDIR)/obj
DEBPRGNAME = emu
DEBCFLAGS = -g
#RELDIR = release
#RELOBJDIR = $(RELDIR)/obj
#RELPRGNAME = emu-release
#RELCFLAGS = -O2

DEBOBJS = $(patsubst %,$(DEBOBJDIR)/%,$(OBJS))
#RELOBJS = $(patsubst %,$(RELOBJDIR)/%,$(OBJS))

default: directories debug

print:
	@echo "using objects:" $(DEBOBJS) "and headers:" $(HEADERS)

directories:
	mkdir -p $(DEBDIR) $(DEBOBJDIR)
#$(RELDIR) $(RELOBJDIR)

debug: CFLAGS += $(DEBCFLAGS)
debug: $(DEBPRGNAME)

$(DEBOBJDIR)/%.o: %.cpp $(HEADERS)
	$(CXX) $(CFLAGS) -c $< -o $@

$(DEBPRGNAME): $(DEBOBJS)
	$(CXX) $(DEBOBJS) -o $(DEBDIR)/$(DEBPRGNAME) $(LIBS)

.PHONY: clean
clean:
	rm -f $(DEBDIR)/$(OBJDIR)/*.o $(DEBDIR)/$(PRGNAME)

