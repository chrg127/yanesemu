#include "cpu.h"

#include <cstdio>
#include "memorymap.h"

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

void CPU::execute(uint8_t opcode, Mode m)
{
/*    switch(m) {
    case IMM:
        fetch_imm();
        break;
    case ZERO:
        fetch_zero_ptr(0);
        break;
    case ZERO_X:
        fetch_zero_ptr(xreg);
    case ABS:
        fetch_abs_ptr(0);
        break;
    case ASB_X:
        fetch_abs_ptr(xreg);
        break;
    case ABS_Y:
        fetch_abs_ptr(yreg);
        break;
    case IND_X:
        fetch_ind_x(xreg);
        break;
    case IND_Y:
        fetch_ind_y(yreg);
        break;
    default:
        std::fprintf(stderr, "error: unknown mode\n");
    }*/
    Mode mode;
    uint8_t op1, op2, value;
    uint16_t addr;

    switch (opcode) {
    case ADC:
        accum += value;
        break;
    case AND:
        accum &= value;
        break;
    case ASL:
        if (mode == Mode::ACCUM)
            accum <<= 1;
        else
            ram[addr] <<= 1;
    case BIT:
        break;
    case BPL:
        if (procstatus.neg == 0) {
            if (op1 >= 0x80 && op1 <= 0xFF)
                op1 = -(op1^0xFF)+1;
            pc += op1;
        }
    case BMI:
        if (
            if (op1 >= 0x80 && op1 <= 0xFF)
                op1 = -(op1^0xFF)+1;
            pc += op1;
        }

    }
}

int CPU::fetchoperands1(char opcode)
{
    return opcode;
}

void CPU::lda()
{
    accum = val;
}

void CPU::sta()
{
    ram[ptr] = accum;
}

void CPU::ldx()
{

}

void CPU::stx()
{

}

void CPU::stz()
{

}

void CPU::andop()
{

}

void CPU::ora()
{

}

void CPU::bitop()
{

}

void CPU::eor()
{

}

void CPU::adc()
{

}

void CPU::sbc()
{

}

void CPU::cmp()
{

}

void CPU::cpx()
{

}

void CPU::cpy()
{

}

void CPU::decop()
{

}

void CPU::inc()
{

}

void CPU::asl()
{
    if (m == ACCUM)
        accum <<= 1;
    else
        ram[ptr] <<= 1;
}

void CPU::lsr()
{

}

void CPU::rol()
{

}

void CPU::ror()
{

}

void CPU::jmp(uint16_t addr)
{
    pc = addr;
}

void CPU::jsr()
{

}

void CPU::rts()
{

}

void CPU::rti()
{

}

void CPU::brk()
{

}

void CPU::nop()
{

}


