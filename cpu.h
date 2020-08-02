#ifndef NESCPU_H_INCLUDED
#define NESCPU_H_INCLUDED

#include <cstdint>
#include "bus.h"

#define DEBUG
#include "debug.h"

class RomFile;

class CPU {
    // component system
    RomFile &rom;
    Bus &bus;

    uint16_t pc;
    uint8_t accum;
    uint8_t xreg;
    uint8_t yreg;
    uint8_t sp;

    struct {
        bool carry      = 0;
        bool zero       = 0;
        bool intdis     = 0;
        bool decimal    = 0;
        bool breakc     = 0;
        bool ov         = 0;
        bool neg        = 0;
        bool unused     = 0;

        uint8_t reg()
        {
            return carry << 0  | zero << 1 | intdis << 2 | decimal << 3 |
                   breakc << 4 | ov << 5   | neg << 6    | unused << 7;
        }

        inline void operator=(const uint8_t data)
        {
            carry   = data & 0x01;
            zero    = data & 0x02;
            intdis  = data & 0x04;
            decimal = data & 0x08;
            breakc  = data & 0x10;
            ov      = data & 0x20;
            neg     = data & 0x40;
            unused  = data & 0x80;
        }

        void reset()
        {
            carry = zero = intdis = decimal = breakc = ov = neg = unused = 0;
        }
    } procstatus;

    // interrupt signals
    int cycles;
    bool nmipending = false;

    uint8_t fetch();
    void execute(uint8_t opcode);
    uint8_t fetch_op();
    void push(uint8_t val);
    uint8_t pull();
    inline uint16_t buildval16(uint8_t low, uint8_t hi)
    {
        return (hi << 8) | low;
    }
    void cycle(uint8_t n);

// Definitions of all opcodes and addressing modes.
#include "opcodes.h"

public:
    CPU(RomFile &f, Bus &b)
        : rom(f), bus(b),
          accum(0), xreg(0), yreg(0), sp(0),
          cycles(0)
    {
        procstatus.reset();
    }

    ~CPU() { }

    void main();
    void initemu();
    void printinfo();
    void memdump(const char * const fname);

};

#endif

