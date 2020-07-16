#include <cstdio>
#include "nescpu.h"

int main(int argc, char **argv)
{
    FILE *romfile;

    if (argc < 2) {
        std::fprintf(stderr, "%s: error: rom file not specified", *argv);
        return 1;
    }

    romfile = std::fopen(argv[1], "rb");
    if (!romfile) {
        std::fprintf(stderr, "%s: error: rom file couldn't be opened", *argv);
        return 1;

    }
    
    bool done = false;
    char buf[BUFSIZ];
    while (!done) {
        //fetch, decode and execute
        std::fread(buf, 1, 1, romfile);
        int opcode = cpu::decode(buf[0]);
        switch(opcode) {
        case LDA:
            ////cpu::fetchoperands1(opcode);
            break;
        case STA:
            break;
        case LDX:
            break;
        case STX:
            break;
        default:
            fprintf(stderr, "%s: error: unknown opcode: %02X\n", *argv, opcode);
        }

        if (std::feof(romfile))
            done = true;
    }
    return 0;
}
