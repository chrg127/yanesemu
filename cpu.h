#ifndef NESCPU_H_INCLUDED
#define NESCPU_H_INCLUDED

#include <cstdio>
#include <cstdint>
#include <cstddef>
#include "bus.h"

namespace Processor {

union reg16 {
    struct {
        uint8_t low, high;
    };
    uint16_t reg;

    reg16() : reg(0) { }
    reg16(uint16_t val) : reg(val) { }
    inline void operator=(uint16_t val)
    {
        reg = val;
    }
};

class CPU {
    Bus &bus;

    uint8_t curropcode;
    reg16 op;       // operand
    reg16 result;   // for results in addrmode_* functions

    reg16 pc;
    uint8_t accum;
    uint8_t xreg;
    uint8_t yreg;
    uint8_t sp;

    struct {
        bool carry      = 0;
        bool zero       = 0;
        bool intdis     = 0;
        bool decimal    = 0;
        bool breakf     = 0;
        bool unused     = 0;
        bool ov         = 0;
        bool neg        = 0;

        uint8_t reg()
        {
            return carry  << 0  | zero   << 1  | intdis << 2 | decimal << 3 |
                   breakf << 4  | unused << 5  | ov     << 6 | neg     << 7;
        }

        inline void operator=(const uint8_t data)
        {
            carry   = data & 0x01;
            zero    = data & 0x02;
            intdis  = data & 0x04;
            decimal = data & 0x08;
            breakf  = data & 0x10;
            unused  = data & 0x20;
            ov      = data & 0x40;
            neg     = data & 0x80;
        }

        void reset()
        {
            carry = zero = intdis = decimal = breakf = ov = neg = 0;
            unused = 1;
        }
    } procstatus;

    // interrupt signals
    int cycles;
    bool nmipending = false;
    bool execnmi = false;
    bool irqpending = false;
    bool execirq = false;

    uint8_t fetch();
    void execute(uint8_t opcode);
    void interrupt(bool reset = false);
    void push(uint8_t val);
    uint8_t pull();
    void cycle();
    void last_cycle();
    void irqpoll();
    void nmipoll();

    inline uint16_t buildval16(uint8_t low, uint8_t hi)
    {
        return (hi << 8) | low;
    }

    /* Increment cycles and forwards the address to bus.read() */
    inline uint8_t readmem(uint16_t addr)
    {
        cycle();
        return bus.read(addr);
    }

    inline void writemem(uint16_t addr, uint8_t val)
    {
        cycle();
        bus.write(addr, val);
    }

// Definitions of all opcodes and addressing modes.
#include "opcodes.h"

public:
    CPU(Bus &b)
        : bus(b), pc(0), accum(0),
          xreg(0), yreg(0), sp(0), cycles(0)
    {
        bus.write_enable = false;
        procstatus.reset();
    }

    ~CPU() { }

    void main();
    void power(uint8_t *prgrom, size_t romsize);
    void fire_irq();
    void fire_nmi();
    void reset();
    void disassemble(uint8_t op1, uint8_t op2, FILE *f);
    void printinfo(FILE *logfile);

    uint8_t peek_opcode()
    { return curropcode; }
};

}

#endif

