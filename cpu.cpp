#include "cpu.h"

#include <cstdio>

uint8_t CPU::fetch(RomFile &rom)
{
    uint8_t buf[2];
    rom.read(1, buf);
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
        printf("\rerror: unknown opcode: %02X\n", opcode);
    }
}

int CPU::fetchoperands1(char opcode)
{
    return opcode;
}

