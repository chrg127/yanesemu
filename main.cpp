#include <cstdio>
#include "cpu.h"
#include "nesrom.h"

int main(int argc, char **argv)
{
    CPU cpu;
    RomFile rom;
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

    //fetch, decode and execute cycle
    while (!done) {
        opcodeb = cpu.fetch(rom);
        opcode = cpu.decode(opcodeb);
        cpu.execute(opcode);

        if (rom.eof())
            done = true;
    }
    return 0;
}
