#include <cstdio>
#include "cpu.h"
#include "nesrom.h"

int main(int argc, char *argv[])
{
    RomFile rom;
    CPU cpu(rom);
    bool done = false;
    int opcodeb, opcode;

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

    //rom.printinfo();

    //fetch, decode and execute cycle
    /*while (!done) {
        opcodeb = cpu.fetch(rom);
        opcode = cpu.decode(opcodeb);
        cpu.execute(opcode);

        if (rom.eof())
            done = true;
    }
    return 0;*/
}
