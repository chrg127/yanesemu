#ifndef NESCPU_H_INCLUDED
#define NESCPU_H_INCLUDED

#include <cstdio>
#include <cstddef>
#include <cstdint>
#include "nesrom.h"

#define OPCODELEN 1

enum OPCODES : int {
    LDA = 0,
    STA = 1,
    LDX = 2,
    STX = 3,
};

class CPU {
    RomFile &file;

    int areg;
    int xreg;
    int yreg;

    uint8_t rambuf[8192];

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

