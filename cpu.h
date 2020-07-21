#ifndef NESCPU_H_INCLUDED
#define NESCPU_H_INCLUDED

#include <cstdio>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include "nesrom.h"
#include "memorymap.h"

#define DEBUG

#ifdef DEBUG
#define DBGPRINT(fmt) std::printf(fmt)
#define DBGPRINTHEX8(val) std::printf("%02X", val)
#define DBGPRINTHEX16(val) std::printf("%04X", val)
#else
#define DBGPRINT(fmt) ;
#define DBGPRINTHEX8(val) ;
#define DBGPRINTHEX16(val) ;
#endif

class CPU {
    RomFile &rom;

    uint8_t memory[MEMSIZE];

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

    uint8_t fetch_op();
    uint8_t read_mem(uint16_t addr);
    void write_mem(uint16_t addr, uint8_t val);
    void push(uint8_t val);
    uint8_t pull();
    inline uint16_t buildval16(uint8_t low, uint8_t hi)
    {
        uint16_t addr = (hi << 8);
        addr |= low;
        return addr;
    }

public:
    CPU(RomFile &f)
        : rom(f), accum(0), xreg(0), yreg(0), sp(0)
    {
        procstatus.reg = 0;
    }

    ~CPU() { }

    void initmem();
    uint8_t fetch();
    void execute(uint8_t opcode);
    void printinfo();
    void memdump(FILE *f);

// Definitions of all opcodes and addressing modes.
#include "opcodes.h"

};

#endif

