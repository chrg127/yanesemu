#ifndef NESCPU_H_INCLUDED
#define NESCPU_H_INCLUDED

#include <cstdio>
#include <cstddef>
#include <cstdint>
#include "nesrom.h"

#define OPCODELEN 1

enum Mode {
    IMM,
    ZERO,
    ZERO_X,
    ABS,
    ABS_X,
    ABS_Y,
    IND_X,
    IND_Y,
}

class CPU {
    RomFile &file;

    uint16_t pc;
    uint8_t accum;
    uint8_t sp;
    uint8_t xreg;
    uint8_t yreg;

    union {
        struct {
            char carry  : 1;
            char zero   : 1;
            char intdis : 1;
            char breakc : 1;
            char ov     : 1;
            char neg    : 1;
            char unused : 1;
        }
        uint8_t reg;
    } procstatus;

    void lda(uint8_t val);
    void sta(uint16_t ptr);

    void ldx();
    void stx();
    void stz();

    void andop();
    void ora();
    void bitop();
    void eor();

    void adc();
    void sbc();

    void cmp();
    void cpx();
    void cpy();

    void decop();
    void inc();
    void asl(uint16_t ptr, Mode m);

    void lsr();
    void rol();
    void ror();

    void jmp();
    void jsr();
    void rts();
    void rti();

    void brk();
    void nop();

public:
    CPU(RomFile &f)
        : file(f)
    { }
    
    ~CPU() { }

    unsigned char fetch(RomFile &rom);
    int decode(unsigned char opcode);
    void execute(int opcode);
    int fetchoperands1(char opcode);
};

#endif

