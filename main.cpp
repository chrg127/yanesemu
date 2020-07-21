#include <cstdio>
#include <cstdlib>
#include <unistd.h>
#include "cpu.h"
#include "nesrom.h"

int main(int argc, char *argv[])
{
    RomFile rom;
    CPU cpu(rom);
    bool done = false;
    int opcode;

    if (argc < 2) {
        std::fprintf(stderr, "%s: error: rom file not specified\n", *argv);
        return 1;
    }

    if (rom.open(argv[1]) != 0) {
        std::fprintf(stderr, "%s: error: rom file couldn't be opened\n", *argv);
        return 1;
    }
    rom.printinfo();
    if (rom.file_format() == NesFmt::NES20) {
        std::fprintf(stderr, "%s: error: NES 2.0 format not yet supported.\n", *argv);
        return 1;
    }

    cpu.initmem();

    //fetch, decode and execute cycle
    int counter = 100;
    while (!done) {
        //system("clear");
        opcode = cpu.fetch();
        cpu.execute(opcode);
        cpu.printinfo();
        usleep(10000);
        if (--counter < 0)
            done = true;
        //if (rom.eof())
            //done = true;
    }
    puts("");
    return 0;
}
