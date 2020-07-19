#ifndef NESCPU_H_INCLUDED
#define NESCPU_H_INCLUDED

#include <cstdio>
#include <cstddef>
#include <cstdint>
#include "nesrom.h"
#include "memorymap.h"

#define OPCODELEN 1

/*enum AddrMode {
    SINGLE,
    ACCUM,
    IMM,
    ZERO,
    ZERO_X,
    ABS,
    ABS_X,
    ABS_Y,
    IND,
    IND_X,
    IND_Y,
};

enum Opcode {
    ADC,
    AND,
    ASL,
    BIT,
    BPL,
    BMI,
    BVC,
    BVS,
    BCC,
    BCS,
    BNE,
    BEQ,
    BRK,
    CMP,
    CPX,
    CPY,
    DEC,
    DEX,
    DEY,
    EOR,
    CLC,
    SEC,
    CLI,
    SEI,
    CLV,
    INC,
    JMP,
    JSR,
    LDA,
    LDX,
    LDY,
    LSR,
    NOP,
    ORA,
    PHA,
    PLA,
    PHP,
    PLP,
    ROL,
    ROR,
    RTI,
    RTS,
    SBC,
    STA,
    STX,
    STY,
    TAX,
    TXA,
    TAY,
    TYA,
    TXS,
    TSX,
};*/

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
    uint8_t read_mem(uint16_t addr);
    void cmp(uint8_t reg, uint8_t val);
    void jmp(uint16_t addr);
    void push(uint8_t val);
    uint8_t pull();
    inline uint16_t build_addr(uint8_t low, uint8_t hi)
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
    
#include "opcodes.h"

};

#endif

