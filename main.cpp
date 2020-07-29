#include <cstdio>
#include <cstdlib>
#include "bus.h"
#include "cpu.h"
#include "nesrom.h"

int main(int argc, char *argv[])
{
    RomFile rom;
    Bus bus;
    CPU cpu(rom, bus);
    bool done = false;

    if (argc < 2) {
        std::fprintf(stderr, "%s: error: rom file not specified\n", *argv);
        return 1;
    }

    if (rom.open(argv[1]) != 0) {
        std::fprintf(stderr, "%s: error: rom file couldn't be opened\n", *argv);
        return 1;
    }

    if (rom.file_format() == NesFmt::NES20) {
        std::fprintf(stderr, "%s: error: NES 2.0 format not yet supported.\n", *argv);
        return 1;
    }

    rom.printinfo();

    cpu.initemu();

    //fetch, decode and execute cycle
    int counter = 10;
    while (!done) {
        cpu.main();
        if (--counter < 0) {
            bus.memdump("other/memdump.log");
            done = true;
        }
    }
    puts("");

    return 0;
}

