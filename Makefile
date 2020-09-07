CXX = g++
CFLAGS = -Wall -Wextra -pipe
DEBDIR = debug
RELDIR = release
DEBOBJDIR = $(DEBDIR)/obj
RELOBJDIR = $(RELDIR)/obj
DEBPRGNAME = emu
RELPRGNAME = emu-release

HEADERS = cpu.h nesrom.h memorymap.h bus.h cmdargs.h
OBJS = main.o cpu.o nesrom.o bus.o cmdargs.o

DEBOBJS = $(patsubst %,$(DEBOBJDIR)/%,$(OBJS))
RELOBJS = $(patsubst %,$(RELOBJDIR)/%,$(OBJS))
LIBS = -lm

default: directories debug

directories:
	mkdir -p $(DEBDIR) $(RELDIR) $(DEBDIR)/$(OBJDIR) $(RELDIR)/$(OBJDIR) 

# different rules for debug and release
debug: CFLAGS += -g
debug: $(DEBPRGNAME)

$(DEBOBJDIR)/%.o: %.cpp $(HEADERS)
	$(CXX) $(CFLAGS) -c $< -o $@

$(DEBPRGNAME): $(DEBOBJS)
	$(CXX) $(DEBOBJS) -o $(DEBDIR)/$(DEBPRGNAME) $(LIBS)


release: CFLAGS += -O2
release: $(RELPRGNAME)

$(RELOBJDIR)/%.o: %.cpp $(HEADERS)
	$(CXX) $(CFLAGS) -c $< -o $@

$(RELPRGNAME): $(RELOBJS)
	$(CXX) $(RELOBJS) -o $(RELDIR)/$(RELPRGNAME) $(LIBS)

.PHONY: clean
clean:
	rm -f $(DEBDIR)/$(OBJDIR)/*.o $(RELDIR)/$(OBJDIR)/*.o $(DEBDIR)/$(PRGNAME)

