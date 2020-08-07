#include <cstdio>
#include <cstdlib>
#include "bus.h"
#include "cpu.h"
#include "nesrom.h"

static char *romname = nullptr;

enum Args : int {
    BREAK_ON_BRK    = 0x01,
    EXEC_FOR        = 0x02,
};

int parse_args(int argc, char *argv[], int &flags)
{
    int c;

    flags = 0;
    while (++argv, --argc > 0) {
        if (c = *argv[0], c == '-') {
            c = (*argv)[1];
            switch(c) {
            case 'b':
                flags |= Args::BREAK_ON_BRK;
                break;
            default:
                std::fprintf(stderr, "error: invalid argument: -%c\n", c);
                return 1;
            }
        } else {
            if (romname == nullptr)
                romname = *argv;
            else
                std::fputs("warning: rom file already specified, the first one found will be used\n", stderr);
        }
    }
    if (romname == nullptr) {
        std::fputs("error: rom file not specified\n", stderr);
        return 1;
    }
    return 0;
}

int main(int argc, char *argv[])
{
    nesrom::ROM rom;
    Processor::Bus bus;
    Processor::CPU cpu(bus);
    bool done = false;
    int flags, counter = 20;

    if (argc < 2) {
        std::fprintf(stderr, "usage: %s [ARGS...] ROMFILE\nerror: rom file not specified\n", *argv);
        return 1;
    }

    if (parse_args(argc, argv, flags) != 0)
        return 1;

    if (rom.open(romname) != 0) {
        std::fprintf(stderr, "%s: error: %s\n", *argv, rom.get_errormsg());
        return 1;
    }

    rom.printinfo();
    cpu.power(rom.get_prgrom(), rom.get_prgrom_size());
    while (!done) {
        cpu.main();
        cpu.printinfo();
        if (flags & Args::BREAK_ON_BRK) {
            if (cpu.peek_opcode() == 0) {
                std::printf("got BRK, stopping emulation\n");
                bus.memdump("other/memdump.log");
                done = true;
            }
        }
        if (--counter < 0) {
            cpu.reset();
            bus.memdump("other/memdump.log");
            done = true;
        }
    }
    puts("");

    return 0;
}

