programname := emu

# can be: debug, release
profile := release

_objs_main := \
	emulator.o cartridge.o cpu.o ppu.o instrinfo.o memory.o screen.o \
	debugger.o clidbg.o cpudebugger.o ppudebugger.o \
	cmdline.o easyrandom.o file.o stringops.o \
	video.o opengl.o \
	glad.o \
	main.o
_objs_video_test := video_test.o video.o opengl.o glad.o
_objs_ppu_test := ppu_test.o cpu.o ppu.o bus.o video.o opengl.o glad.o cartridge.o file.o easyrandom.o
libs := -lm -lSDL2 -lfmt

VPATH := emu:emu/core:emu/util:emu/io:emu/video:emu/debugger:tests
CC := gcc
CXX := g++
CFLAGS := -I. -std=c11
CXXFLAGS := -I. -std=c++20 -Wall -Wextra -pipe \
		 -Wcast-align -Wcast-qual -Wpointer-arith \
		 -Wformat=2 -Wmissing-include-dirs -Wno-unused-parameter \
		 -fno-exceptions -fno-rtti -fconcepts
flags_deps = -MMD -MP -MF $(@:.o=.d)

# can be: linux, mingw64
platform := linux

ifeq ($(profile),debug)
    outdir := debug
    CFLAGS += -g -DDEBUG
    CXXFLAGS += -g -DDEBUG
else
    outdir := release
    CFLAGS += -O3
    CXXFLAGS += -O3
endif

ifeq ($(platform),linux)
    libs += -lpthread -ldl -lGL
else ifeq ($(platform),mingw64)
    libs += -lopengl32 -lmingw32 -lSDL2main -lSDL2
else
    $(error error: platform not supported)
endif

# setup objs variables with outdir
objs_main := $(patsubst %,$(outdir)/%,$(_objs_main))
objs_video_test := $(patsubst %,$(outdir)/%,$(_objs_video_test))
objs_ppu_test := $(patsubst %,$(outdir)/%,$(_objs_ppu_test))

# rules
all: $(outdir) $(outdir)/$(programname)

$(outdir)/$(programname): $(objs_main)
	$(info Linking $@ ...)
	$(CXX) $(objs_main) -o $@ $(libs)

# tests
$(outdir)/video_test: $(objs_video_test)
	$(info Linking $@ ...)
	$(CXX) $(objs_video_test) -o $@ $(libs)

$(outdir)/ppu_test: $(objs_ppu_test)
	$(info Linking $@ ...)
	$(CXX) $(objs_ppu_test) -o $@ $(libs)

# This should require a bit of explanation.
# At first pass, no *.d exist, so we just build all files
# (every *.cpp file always match the %.o: %.cpp file). Note that
# the general matching rule does not depend on anything other than
# the corresponding .cpp file. That means, with no other rule,
# we would simply never re-compile anything unless a .cpp file
# changed.
# But there are other rules! it's those .d files. When they're
# created and included, we automatically get header file information,
# thus we are able to re-compile even when some header file changes,
# and we re-compile only what's needed.
-include $(outdir)/*.d

$(outdir)/%.o: %.cpp
	$(info Compiling $< ...)
	@$(CXX) $(CXXFLAGS) $(flags_deps) -c $< -o $@

$(outdir)/glad.o: external/glad/glad.c
	$(info Compiling $< ...)
	@$(CC) $(CFLAGS) $(flags_deps) -c $< -o $@

$(outdir):
	mkdir -p $(outdir)

.PHONY: clean tests

tests: $(outdir)/video_test $(outdir)/ppu_test

clean:
	rm -rf $(outdir)

