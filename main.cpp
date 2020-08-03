#include <cstdio>
#include <cstdlib>
#include "bus.h"
#include "cpu.h"
#include "nesrom.h"

using nesrom::RomFile;

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

    if (rom.file_format() == nesrom::NesFmt::NES20) {
        std::fprintf(stderr, "%s: error: NES 2.0 format not yet supported.\n", *argv);
        return 1;
    }

    rom.printinfo();

    int counter = 10;
    cpu.power();
    while (!done) {
        cpu.main();
        if (--counter < 0) {
            bus.memdump("other/memdump.log");
            cpu.reset();
            done = true;
        }
    }
    puts("");

    return 0;
}

