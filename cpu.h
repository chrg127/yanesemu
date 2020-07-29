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

    union {
        struct {
            uint8_t carry   : 1;
            uint8_t zero    : 1;
            uint8_t intdis  : 1;
            uint8_t breakc  : 1;
            uint8_t ov      : 1;
            uint8_t neg     : 1;
            uint8_t decimal : 1;
        };
        uint8_t reg;
    } procstatus;

    // interrupt signals
    int cycles;
    bool nmipending = false;

    uint8_t fetch_op();
    void push(uint8_t val);
    uint8_t pull();
    inline uint16_t buildval16(uint8_t low, uint8_t hi)
    {
        return (hi << 8) | low;
    }

// Definitions of all opcodes and addressing modes.
#include "opcodes.h"

public:
    CPU(RomFile &f, Bus &b)
        : rom(f), bus(b),
          accum(0), xreg(0), yreg(0), sp(0),
          cycles(0)
    {
        procstatus.reg = 0;
    }

    ~CPU() { }

    void main();
    void initemu();
    uint8_t fetch();
    void execute(uint8_t opcode);
    void printinfo();
    void memdump(const char * const fname);

};

#endif

