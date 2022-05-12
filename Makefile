programname := emu

# can be: debug, release
build := debug

_objs := \
	emulator.cpp cartridge.cpp cpu.cpp ppu.cpp screen.cpp controller.cpp mapper.cpp \
	debugger.cpp clidebugger.cpp cpudebugger.cpp ppudebugger.cpp disassemble.cpp \
	conf.cpp easyrandom.cpp \
	backend.cpp opengl.cpp \
	program.cpp \
	stb_image.c
_objs_main := main.cpp
_tests := cpu_test

VPATH := emu:emu/core:emu/util:emu/io:emu/backend:emu/debugger:external/stb:test
CC := gcc
CXX := g++
CFLAGS := -I. -std=c11
CXXFLAGS := -I. -std=c++20 -Wall -Wextra -pipe \
		 -Wcast-align -Wcast-qual -Wpointer-arith \
		 -Wformat=2 -Wmissing-include-dirs -Wno-unused-parameter \
		 -fno-rtti -fconcepts
LDLIBS := -lm -lSDL2 -lfmt
flags_deps = -MMD -MP -MF $(@:.o=.d)
libs_test := -lCatch2WithMain

# can be: linux, mingw64
platform := linux

ifeq ($(build),debug)
    outdir := debug
    CFLAGS += -g -DDEBUG
    CXXFLAGS += -g -DDEBUG
else ifeq ($(build),release)
    outdir := release
    CFLAGS += -O3
    CXXFLAGS += -O3
else
	$(error error: invalid value for profile)
endif

ifeq ($(platform),linux)
    LDLIBS += -lpthread -ldl -lGL
else ifeq ($(platform),mingw64)
    LDLIBS += -lopengl32 -lmingw32 -lSDL2main -lSDL2
else
    $(error error: platform not supported)
endif

objs 	      := $(patsubst %,$(outdir)/%.o,$(_objs))
objs_main 	  := $(patsubst %,$(outdir)/%.o,$(_objs_main))
test_programs := $(patsubst %,debug/test/%,$(_tests))

all: $(outdir)/$(programname)

$(outdir)/$(programname): $(outdir) $(objs) $(objs_main)
	$(info Linking $@ ...)
	@$(CXX) $(objs) $(objs_main) -o $@ $(LDLIBS)

debug/test/%_test: debug/%_test.cpp.o debug/test $(objs)
	$(info Linking test $@ ...)
	$(CXX) $< $(objs) -o $@ $(LDLIBS) $(libs_test)

-include $(outdir)/*.d

$(outdir)/%.cpp.o: %.cpp
	$(info Compiling $< ...)
	@$(CXX) $(CXXFLAGS) $(flags_deps) -c $< -o $@

$(outdir)/%.c.o: %.c
	$(info Compiling $< ...)
	@$(CC) $(CFLAGS) $(flags_deps) -c $< -o $@

$(outdir):
	mkdir -p $(outdir)

debug/test:
	mkdir -p debug
	mkdir -p debug/test

.PHONY: clean tests

tests: $(test_programs)

clean:
	rm -rf debug release
