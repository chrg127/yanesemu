CXX = g++
CFLAGS = -Wall -Wextra
DEBDIR = debug
RELDIR = rel
OBJDIR = obj
PRGNAME = emu

HEADERS = nescpu.h
_OBJS = main.o nescpu.o
OBJS = $(patsubst %,$(OBJDIR)/%,$(_OBJS))

default: debug

debug: CFLAGS += -g
debug: $(PRGNAME)

release: CFLAGS += O2
release: $(PRGNAME)

$(OBJDIR)/%.o: %.cpp $(HEADERS)
	$(CXX) $(CFLAGS) -c $< -o $@

$(PRGNAME): $(OBJS)
	$(CXX) $(OBJS) -o $(DEBDIR)/$(PRGNAME)

.PHONY: clean
clean:
	rm -f $(OBJDIR)/*.o $(DEBDIR)/$(PRGNAME)

