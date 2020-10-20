#include <cstdio>
#include <cstdlib>
#include <emu/utils/cmdargs.hpp>
#include <emu/core/bus.hpp>
#include <emu/core/cpu.hpp>
#include <emu/io/file.hpp>
#include <emu/io/nesrom.hpp>
#define DEBUG
#include <emu/utils/debug.hpp>

#include <emu/core/ppu.hpp>
#include <emu/core/ppubus.hpp>
#include <emu/video/video.hpp>

#include <cmath>

static const char *version_str = "0.1";
static const int NUM_FLAGS = 5;

enum {
    ARG_BREAK_ON_BRK = 0x01,
    ARG_LOG_FILE     = 0x02,
    ARG_DUMP_FILE    = 0x04,
    ARG_HELP         = 0x20000000,
    ARG_VERSION      = 0x40000000,
};

static Utils::ArgOption cmdflags[] = {
    { 'b', ARG_BREAK_ON_BRK, "break-on-brk", "Stops emulation when BRK is encountered.", false, false, {} },
    { 'l', ARG_LOG_FILE,     "log-file",     "The file where to log instructions. "
                                             "Pass \"stdout\" to print to stdout, "
                                             "\"stderr\" to print to stderr.",           true,  true,  {} },
    { 'd', ARG_DUMP_FILE,    "dump-file",    "The file where to dump memory."
                                             "Pass \"stdout\" to print to stdout, "
                                             "\"stderr\" to print to stderr.",           true,  true,  {} },
    { 'h', ARG_HELP,         "help",         "Print this help text and quit",            false, false, {} },
    { 'v', ARG_VERSION,      "version",      "Shows the program's version",              false, false, {} },
};

void logopen(IO::File &f, Utils::ArgFlags &flags, const uint32_t arg);
void dump(IO::File &df, const uint8_t *const mem, const std::size_t size);
void write_chrrom(Video::Video::Screen sc, uint8_t *rom, size_t size);

void logopen(IO::File &f, Utils::ArgFlags &flags, const uint32_t arg)
{
    if ((flags.bits & arg) == 0)
        return;

    std::string &s = flags.get_choice(arg);
    if (s == "stdout")
        f.assoc(stdout, IO::Mode::WRITE);
    else if (s == "stderr")
        f.assoc(stderr, IO::Mode::WRITE);
    else if (s == "")
        return;
    else {
        if (!f.open(s, IO::Mode::WRITE))
            error("can't open %s for writing\n", s.c_str());
    }
}

void dump(IO::File &df, const uint8_t *const mem, const std::size_t size)
{
    std::size_t i, j;

    for (i = 0; i < size; i++) {
        df.printf("%04lX: ", i);
        for (j = 0; j < 16; j++)
            df.printf("%02X ", mem[i++]);
        df.putc('\n');
    }
    df.putc('\n');
}

void write_chrrom(Video::Video::Screen sc, uint8_t *rom, size_t size)
{
    static const uint32_t colortab[] = { 0x000000FF, 0x888888FF, 0xBBBBBBFF, 0xFFFFFFFF };
    static const int rowsize = 128;

    for (size_t i = 0; i < size; i += 16) {
        std::printf("tile %ld [%ld], x = %ld, y = %ld\n", i/16, i, (i/2)%rowsize, ((i/2)/rowsize)*8);
        for (int j = 0; j < 8; j++) {
            for (int b = 0b10000000; b != 0; b >>= 1) {
                bool bit1 = rom[i+j] & b;
                bool bit2 = rom[i+j+8] & b;
                int tabi = bit1+bit2;
                uint32_t pixel = colortab[tabi];
                int x = ((i/2) % rowsize) + (7-std::log2(b));
                int y = ((i/2)/rowsize)*8 + j;

                std::printf("tabi = %d, pixel = %08X, x = %d, y = %d\n", tabi, pixel, x, y);

                sc.buf[y*sc.w + x] = pixel;
            }
        }
    }
}

int main(int argc, char *argv[])
{
    Utils::ArgParser parser(*argv, cmdflags, NUM_FLAGS);
    IO::File logfile, dumpfile, fout(stdout, IO::Mode::WRITE);
    IO::ROM rom;
    Video::Video v;
    Core::PPUBus ppubus;
    Core::PPU ppu(&ppubus);

    if (argc < 2) {
        parser.print_usage();
        return 1;
    }

    Utils::ArgFlags flags = parser.parse_args(argc, argv);
    logopen(logfile, flags, ARG_LOG_FILE);
    logopen(dumpfile, flags, ARG_DUMP_FILE);

    if (flags.bits & ARG_HELP) {
        parser.print_usage();
        return 0;
    } else if (flags.bits & ARG_VERSION) {
        fout.printf("%s\n", version_str);
        return 0;
    } else if (flags.item == "") {
        error("ROM file not specified\n");
        return 1;
    } else if (!rom.open(flags.item)) {
        error("can't open rom\n");
        return 1;
    } else if (!v.create()) {
        error("can't initialize video subsytem\n");
        return 1;
    }

    rom.printinfo(fout);
    // dump(dumpfile, rom.get_chrrom(), rom.get_chrrom_size());
    // Video::Video::Screen sc = v.getpixels();
    // write_chrrom(sc, rom.get_chrrom(), rom.get_chrrom_size());
    // while (!v.closed()) {
    //     v.poll();
    //     v.render();
    // }
    ppu.power(rom.get_chrrom());
    int counter = 0;
    for (;;) {
        ppu.main();
        ppu.printinfo(logfile);
        if (++counter == 100)
            break;
    }
    return 0;
}

    // cpu.power(rom.get_prgrom(), rom.get_prgrom_size());
    // counter = 20;
    // while (!done) {
    //     cpu.main();
    //     logfile.printf("Instruction [%02X] ", cpu.peek_opcode());
    //     logfile.putstr(cpu.disassemble().c_str());
    //     logfile.putc('\n');
    //     cpu.printinfo(logfile);
    //     if (flags.bits & ARG_BREAK_ON_BRK && cpu.peek_opcode() == 0) {
    //         DBGPRINT("got BRK, stopping emulation\n");
    //         bus.memdump(dumpfile);
    //         done = true;
    //     }
    //     if (--counter < 0) {
    //         cpu.reset();
    //         bus.memdump(dumpfile);
    //         done = true;
    //     }
    // }

    // Core::Bus bus;
    // Core::CPU cpu(&bus);
    // bool done = false;
    // int counter;
    // Core::PPUBus pbus;
    // Core::PPU ppu(&pbus);
    // bool done = false;

    // ppu.writereg(0x2000, 0b10000001);
    // ppu.writereg(0x2001, 0b00000000);
    // ppu.writereg(0x2005, 0);
    // ppu.writereg(0x2005, 0);
    // ppu.writereg(0x2006, 0);
    // ppu.writereg(0x2006, 0);
    // ppu.power(rom.get_chrrom());
    // while (!done) {
    //     ppu.main();
    // }