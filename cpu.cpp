#include "cpu.h"

#include <cstdio>

unsigned char CPU::fetch(RomFile &rom)
{
    unsigned char buf[2];
    std::fread(buf, 1, 1, rom.file);
    return *buf;
}

int CPU::decode(unsigned char b)
{
    return b;
}

void CPU::execute(int opcode)
{
    switch(opcode) {
    case LDA:
        ////cpu::fetchoperands1(opcode);
        break;
    case STA:
        break;
    case LDX:
        break;
    case STX:
        break;
    default:
        fprintf(stderr, "error: unknown opcode: %02X\n", opcode);
    }
}

int CPU::fetchoperands1(char opcode)
{
    return opcode;
}

