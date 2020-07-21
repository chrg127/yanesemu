CXX = g++
CFLAGS = -Wall -Wextra
DEBDIR = debug
RELDIR = release
OBJDIR = obj
PRGNAME = emu

HEADERS = cpu.h nesrom.h memorymap.h
_OBJS = main.o cpu.o nesrom.o
OBJS = $(patsubst %,$(OBJDIR)/%,$(_OBJS))

default: directories debug

directories:
	mkdir -p $(DEBDIR) $(RELDIR) $(DEBDIR)/$(OBJDIR) $(RELDIR)/$(OBJDIR) 

# OUTDIR defined in these two rules
debug: CFLAGS += -g
debug: OUTDIR = $(DEBDIR)
debug: OBJS_WITHDIR = $(patsubst %,$(OUTDIR)/%,$(OBJS))
debug: $(PRGNAME)

release: CFLAGS += -O2
release: OUTDIR = $(RELDIR)
release: OBJS_WITHDIR = $(patsubst %,$(OUTDIR)/%,$(OBJS))
release: $(PRGNAME)

$(OBJDIR)/%.o: %.cpp $(HEADERS)
	$(CXX) $(CFLAGS) -c $< -o $(OUTDIR)/$@

$(PRGNAME): $(OBJS)
	$(CXX) $(OBJS_WITHDIR) -o $(OUTDIR)/$(PRGNAME)

.PHONY: clean
clean:
	rm -f $(DEBDIR)/$(OBJDIR)/*.o $(RELDIR)/$(OBJDIR)/*.o $(DEBDIR)/$(PRGNAME)

