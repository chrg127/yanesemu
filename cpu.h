#ifndef NESCPU_H_INCLUDED
#define NESCPU_H_INCLUDED

#include <cstdio>
#include <cstddef>
#include <cstdint>
#include "nesrom.h"
#include "memorymap.h"

#define OPCODELEN 1

class CPU {
    RomFile &rom;

    uint8_t memory[MEMSIZE];

    uint16_t pc;
    uint8_t accum;
    uint8_t sp;
    uint8_t xreg;
    uint8_t yreg;

    union {
        struct {
            uint8_t carry  : 1;
            uint8_t zero   : 1;
            uint8_t intdis : 1;
            uint8_t breakc : 1;
            uint8_t ov     : 1;
            uint8_t neg    : 1;
            uint8_t unused : 1;
        };
        uint8_t reg;
    } procstatus;

    uint8_t fetch_op();
    uint8_t read_mem(uint16_t addr)
    {
        return 0;
    }
    void write_mem(uint16_t addr)
    {
    }
    void push(uint8_t val);
    uint8_t pull();
    inline uint16_t buildval16(uint8_t low, uint8_t hi)
    {
        uint16_t addr = hi << 8;
        addr |= low;
        return addr;
    }

public:
    CPU(RomFile &f)
        : rom(f)
    { }

    ~CPU() { }

    uint8_t fetch();
    //void decode(uint8_t byte, uint8_t &opcode, AddrMode &mode);
    void execute(uint8_t opcode);
    //int fetchoperands1(char opcode);

// Definitions of all opcodes and addressing modes.
#include "opcodes.h"

};

#endif

