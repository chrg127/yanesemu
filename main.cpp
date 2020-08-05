#include <cstdio>
#include <cstdlib>
#include "bus.h"
#include "cpu.h"
#include "nesrom.h"

int main(int argc, char *argv[])
{
    nesrom::ROM rom;
    Processor::Bus bus;
    Processor::CPU cpu(rom, bus);
    bool done = false;
    int counter = 10;

    if (argc < 2) {
        std::fprintf(stderr, "%s: error: rom file not specified\n", *argv);
        return 1;
    }

    if (rom.open(argv[1]) != 0) {
        std::fprintf(stderr, "%s: error: %s\n", *argv, rom.errormsg());
        return 1;
    }

    rom.printinfo();

    cpu.power();
    while (!done) {
        cpu.main();
        cpu.printinfo();
        if (--counter < 0) {
            bus.memdump("other/memdump.log");
            cpu.reset();
            done = true;
        }
    }
    puts("");

    return 0;
}

