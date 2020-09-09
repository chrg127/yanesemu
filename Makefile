CXX = g++
CFLAGS = -std=c++17 -Wall -Wextra -pipe -Wwrite-strings -Winit-self -Wcast-align -Wcast-qual -Wpointer-arith -Wstrict-aliasing -Wformat=2 -Wmissing-declarations -Wmissing-include-dirs -Wno-unused-parameter -Wuninitialized
DEBDIR = debug
RELDIR = release
DEBOBJDIR = $(DEBDIR)/obj
RELOBJDIR = $(RELDIR)/obj
DEBPRGNAME = emu
RELPRGNAME = emu-release
DEBCFLAGS = -g
RELCFLAGS = -O2

HEADERS = cpu.h nesrom.h memorymap.h bus.h cmdargs.h debug.h
OBJS = main.o cpu.o nesrom.o bus.o cmdargs.o

DEBOBJS = $(patsubst %,$(DEBOBJDIR)/%,$(OBJS))
RELOBJS = $(patsubst %,$(RELOBJDIR)/%,$(OBJS))
LIBS = -lm

default: directories debug

directories:
	mkdir -p $(DEBDIR) $(RELDIR) $(DEBDIR)/$(OBJDIR) $(RELDIR)/$(OBJDIR) 

# different rules for debug and release
debug: CFLAGS += $(DEBCFLAGS)
debug: $(DEBPRGNAME)

$(DEBOBJDIR)/%.o: %.cpp $(HEADERS)
	$(CXX) $(CFLAGS) -c $< -o $@

$(DEBPRGNAME): $(DEBOBJS)
	$(CXX) $(DEBOBJS) -o $(DEBDIR)/$(DEBPRGNAME) $(LIBS)


release: CFLAGS += $(RELCFLAGS)
release: $(RELPRGNAME)

$(RELOBJDIR)/%.o: %.cpp $(HEADERS)
	$(CXX) $(CFLAGS) -c $< -o $@

$(RELPRGNAME): $(RELOBJS)
	$(CXX) $(RELOBJS) -o $(RELDIR)/$(RELPRGNAME) $(LIBS)

.PHONY: clean
clean:
	rm -f $(DEBDIR)/$(OBJDIR)/*.o $(RELDIR)/$(OBJDIR)/*.o $(DEBDIR)/$(PRGNAME)

