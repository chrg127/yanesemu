/* a small module for disassembling an instruction, completely separate from the cpu module. */

#ifndef DISASSEMBLE_H_INCLUDED
#define DISASSEMBLE_H_INCLUDED

enum AddrMode {
    INVALID,
    IMM, ACCUM, ZERO, ZEROX, ZEROY,
    ABS, ABSX, ABSY, IND, INDX, INDY,
    IMPL, REL,
};

static struct {
    const char *str;
    AddrMode mode;
} optab[] = {
    { "BRK", IMPL },        { "ORA", INDX },        { nullptr, INVALID }, 
    { nullptr, INVALID },   { nullptr, INVALID },   { "ORA", ZERO },
    { "ASL", ZERO },        { nullptr, INVALID },   { "PHP", IMPL },
    { "ORA", IMM },         { "ASL", ACCUM },       { nullptr, INVALID }, 
    { nullptr, INVALID },   { "ORA", ABS },         { "ASL", ABS },
    { nullptr, INVALID },   { "BPL", REL },         { "ORA", INDY },
    { nullptr, INVALID },   { nullptr, INVALID },   { nullptr, INVALID }, 
    { "ORA", ZEROX },       { "ASL", ZEROX },       { nullptr, INVALID }, 
    { "CLC", IMPL },        { "ORA", ABSY },        { nullptr, INVALID }, 
    { nullptr, INVALID },   { nullptr, INVALID },   { "ORA", ABSX },
    { "ASL", ABSX },        { nullptr, INVALID },   { "JSR", IMPL },
    { "AND", INDX },        { nullptr, INVALID },   { nullptr, INVALID }, 
    { "BIT", ZERO },        { "AND", ZERO },        { "ROL", ZERO },
    { nullptr, INVALID },   { "PLP", IMPL },        { "AND", IMM },
    { "ROL", ACCUM },       { nullptr, INVALID },   { "BIT", ABS },
    { "AND", ABS },         { "ROL", ABS },         { nullptr, INVALID }, 
    { "BMI", REL },         { "AND", INDY },        { nullptr, INVALID }, 
    { nullptr, INVALID },   { nullptr, INVALID },   { "AND", ZEROX },
    { "ROL", ZEROX },       { nullptr, INVALID },   { "SEC", IMPL },
    { "AND", ABSY },        { nullptr, INVALID },   { nullptr, INVALID }, 
    { nullptr, INVALID },   { "AND", ABSX },        { "ROL", ABSX },
    { nullptr, INVALID },   { "RTI", IMPL },        { "EOR", INDX },
    { nullptr, INVALID },   { nullptr, INVALID },   { nullptr, INVALID }, 
    { "EOR", ZERO },        { "LSR", ZERO },        { nullptr, INVALID }, 
    { "PHA", IMPL },        { "EOR", IMM },         { "LSR", ACCUM },
    { nullptr, INVALID },   { "JMP", IMPL },        { "EOR", ABS },
    { "LSR", ABS },         { nullptr, INVALID },   { "BVC", REL },
    { "EOR", INDY },        { nullptr, INVALID },   { nullptr, INVALID }, 
    { nullptr, INVALID },   { "EOR", ZEROX },       { "LSR", ZEROX },
    { nullptr, INVALID },   { "CLI", IMPL },        { "EOR", ABSY },
    { nullptr, INVALID },   { nullptr, INVALID },   { nullptr, INVALID }, 
    { "EOR", ABSX },        { "LSR", ABSX },        { nullptr, INVALID }, 
    { "RTS", IMPL },        { "ADC", INDX },        { nullptr, INVALID }, 
    { nullptr, INVALID },   { nullptr, INVALID },   { "ADC", ZERO },
    { "ROR", ZERO },        { nullptr, INVALID },   { "PLA", IMPL },
    { "ADC", IMM },         { "ROR", ACCUM },       { nullptr, INVALID }, 
    { "JMP_IND", IMPL },    { "ADC", ABS },         { "ROR", ABS },
    { nullptr, INVALID },   { "BVS", REL },         { "ADC", INDY },
    { nullptr, INVALID },   { nullptr, INVALID },   { nullptr, INVALID }, 
    { "ADC", ZEROX },       { "ROR", ZEROX },       { nullptr, INVALID }, 
    { "SEI", IMPL },        { "ADC", ABSY },        { nullptr, INVALID }, 
    { nullptr, INVALID },   { nullptr, INVALID },   { "ADC", ABSX },
    { "ROR", ABSX },        { nullptr, INVALID },   { nullptr, INVALID }, 
    { "STA", INDX },        { nullptr, INVALID },   { nullptr, INVALID }, 
    { "STY", ZERO },        { "STA", ZERO },        { "STX", ZERO },
    { nullptr, INVALID },   { "DEY", IMPL },        { nullptr, INVALID }, 
    { "TXA", IMPL },        { nullptr, INVALID },   { "STY", ABS },
    { "STA", ABS },         { "STX", ABS },         { nullptr, INVALID }, 
    { "BCC", REL },         { "STA", INDY },        { nullptr, INVALID }, 
    { nullptr, INVALID },   { "STY", ZEROX },       { "STA", ZEROX },
    { "STX", ZEROY },       { nullptr, INVALID },   { "TYA", IMPL },
    { "STA", ABSY },        { "TXS", IMPL },        { nullptr, INVALID }, 
    { nullptr, INVALID },   { "STA", ABSX },        { nullptr, INVALID }, 
    { nullptr, INVALID },   { "LDY", IMM },         { "LDA", INDX },
    { "LDX", IMM },         { nullptr, INVALID },   { "LDY", ZERO },
    { "LDA", ZERO },        { "LDX", ZERO },        { nullptr, INVALID }, 
    { "TAY", IMPL },        { "LDA", IMM },         { "TAX", IMPL },
    { nullptr, INVALID },   { "LDY", ABS },         { "LDA", ABS },
    { "LDX", ABS },         { nullptr, INVALID },   { "BCS", REL },
    { "LDA", INDY },        { nullptr, INVALID },   { nullptr, INVALID }, 
    { "LDY", ZEROX },       { "LDA", ZEROX },       { "LDX", ZEROY },
    { nullptr, INVALID },   { "CLV", IMPL },        { "LDA", ABSY },
    { "TSX", IMPL },        { nullptr, INVALID },   { "LDY", ABSX },
    { "LDA", ABSX },        { "LDX", ABSY },        { nullptr, INVALID }, 
    { "CPY", IMM },         { "CMP", INDX },        { nullptr, INVALID }, 
    { nullptr, INVALID },   { "CPY", ZERO },        { "CMP", ZERO },
    { "DEC", ZERO },        { nullptr, INVALID },   { "INY", IMPL },
    { "CMP", IMM },         { "DEX", IMPL },        { nullptr, INVALID }, 
    { "CPY", ABS },         { "CMP", ABS },         { "DEC", ABS },
    { nullptr, INVALID },   { "BNE", REL },         { "CMP", INDY },
    { nullptr, INVALID },   { nullptr, INVALID },   { nullptr, INVALID }, 
    { "CMP", ZEROX },       { "DEC", ZEROX },       { nullptr, INVALID }, 
    { "CLD", IMPL },        { "CMP", ABSY },        { nullptr, INVALID }, 
    { nullptr, INVALID },   { nullptr, INVALID },   { "CMP", ABSX },
    { "DEC", ABSX },        { nullptr, INVALID },   { "CPX", IMM },
    { "SBC", INDX },        { nullptr, INVALID },   { nullptr, INVALID }, 
    { "CPX", ZERO },        { "SBC", ZERO },        { "INC", ZERO },
    { nullptr, INVALID },   { "INX", IMPL },        { "SBC", IMM },
    { "NOP", IMPL },        { nullptr, INVALID },   { "CPX", ABS },
    { "SBC", ABS },         { "INC", ABS },         { nullptr, INVALID }, 
    { "BEQ", REL },         { "SBC", INDY },        { nullptr, INVALID }, 
    { nullptr, INVALID },   { nullptr, INVALID },   { "SBC", ZEROX },
    { "INC", ZEROX },       { nullptr, INVALID },   { nullptr, INVALID }, 
    { "SBC", ABSY },        { nullptr, INVALID },   { nullptr, INVALID }, 
    { nullptr, INVALID },   { "SBC", ABSX },        { "INC", ABSX },
};

static void print_branch(uint8_t opcode, uint8_t procstatus, FILE *f)
{
    bool took;

#define BRANCH_CASE(id, flagbit, set) \
    case id: took = (procstatus & flagbit) == set; break;
    switch(opcode) {
        BRANCH_CASE(0x10, 0x80, 0)
        BRANCH_CASE(0x30, 0x80, 1)
        BRANCH_CASE(0x50, 0x40, 0)
        BRANCH_CASE(0x70, 0x40, 1)
        BRANCH_CASE(0x90, 0x01, 0)
        BRANCH_CASE(0xB0, 0x01, 1)
        BRANCH_CASE(0xD0, 0x02, 0)
        BRANCH_CASE(0xF0, 0x02, 1)
    }
#undef BRANCH_CASE
    if (took)
        std::fprintf(f, " [Branch taken]");
    else
        std::fprintf(f, " [Branch not taken]");
}

static void print_opcode(uint8_t opcode, uint8_t op1, uint8_t op2, uint8_t procstatus, FILE *f)
{
    std::fprintf(f, "Instruction: [%02X] %s", opcode, optab[opcode].str);

#define PRINT_CASE(mode, fmt, ...) \
    case mode: std::fprintf(f, fmt, __VA_ARGS__); break;

    switch (optab[opcode].mode) {
    case IMPL: break;
        PRINT_CASE(IMM, " #$%02X", op1)
    case ACCUM:
        std::fputs(" A", f);
        break;
        PRINT_CASE(ZERO,    " $%02X",       op1)
        PRINT_CASE(ZEROX,   " $%02X,x",     op1)
        PRINT_CASE(ZEROY,   " $%02X,y",     op1)
        PRINT_CASE(ABS,     " $%02X%02X",   op2, op1)
        PRINT_CASE(ABSX,    " $%02X%02X,x", op2, op1)
        PRINT_CASE(ABSY,    " $%02X%02X,y", op2, op1)
        PRINT_CASE(IND,     " ($%02X%02X)", op2, op1)
        PRINT_CASE(INDX,    " ($%02X,x)",   op1)
        PRINT_CASE(INDY,    " ($%02X),y",   op1)
    case REL:
        std::fprintf(f, " $%02X", op1);
        print_branch(opcode, procstatus, f);
        break;
    default:
        std::fprintf(stderr, "error: mode not valid\n");
        break;
    }
#undef PRINT_CASE
    std::fputs("\n", f);
}

/*
void print_implied(uint8_t id, const char *name)
{
    std::fprintf(f, "Instruction [%02X] %s\n", id, #name);
}

void disassemble(uint8_t opcode, uint8_t op1, uint8_t op2, uint8_t procstatus, FILE *f)
{
#define INSTR_IMPLD(id, name) \
    case id: print_implied(id, #name); break;
#define INSTR_AMODE(id, name, mode, type) \
    case id: break;
#define INSTR_WRITE(id, mode, val) \
    case id: break;
#define INSTR_OTHER(id, name, ...) \
    case id: break;
    switch(opcode) {
        INSTR_IMPLD(0x00, brk)
        INSTR_AMODE(0x01, ora, indx, read)
        INSTR_AMODE(0x05, ora, zero, read)
        INSTR_AMODE(0x06, asl, zero, modify)
        INSTR_IMPLD(0x08, php)
        INSTR_AMODE(0x09, ora, imm, read)
        INSTR_AMODE(0x0A, asl, accum, modify)
        INSTR_AMODE(0x0D, ora, abs, read)
        INSTR_AMODE(0x0E, asl, abs, modify)
        INSTR_OTHER(0x10, branch, procstatus.neg == 0)     // bpl
        INSTR_AMODE(0x11, ora, indy, read)
        INSTR_AMODE(0x15, ora, zerox, read)
        INSTR_AMODE(0x16, asl, zerox, modify)
        INSTR_OTHER(0x18, flag, procstatus.carry, false)   // clc
        INSTR_AMODE(0x19, ora, absy, read)
        INSTR_AMODE(0x1D, ora, absx, read)
        INSTR_AMODE(0x1E, asl, absx, modify)
        INSTR_IMPLD(0x20, jsr)
        INSTR_AMODE(0x21, and, indx, read)
        INSTR_AMODE(0x24, bit, zero, read)
        INSTR_AMODE(0x25, and, zero, read)
        INSTR_AMODE(0x26, rol, zero, modify)
        INSTR_IMPLD(0x28, plp)
        INSTR_AMODE(0x29, and, imm, read)
        INSTR_AMODE(0x2A, rol, accum, modify)
        INSTR_AMODE(0x2C, bit, abs, read)
        INSTR_AMODE(0x2D, and, abs, read)
        INSTR_AMODE(0x2E, rol, abs, modify)
        INSTR_OTHER(0x30, branch, procstatus.neg == 1)     // bmi
        INSTR_AMODE(0x31, and, indy, read)
        INSTR_AMODE(0x35, and, zerox, read)
        INSTR_AMODE(0x36, rol, zerox, modify)
        INSTR_OTHER(0x38, flag, procstatus.carry, true)    //sec
        INSTR_AMODE(0x39, and, absy, read)
        INSTR_AMODE(0x3D, and, absx, read)
        INSTR_AMODE(0x3E, rol, absx, modify)
        INSTR_IMPLD(0x40, rti)
        INSTR_AMODE(0x41, eor, indx, read)
        INSTR_AMODE(0x45, eor, zero, read)
        INSTR_AMODE(0x46, lsr, zero, modify)
        INSTR_IMPLD(0x48, pha)
        INSTR_AMODE(0x49, eor, imm, read)
        INSTR_AMODE(0x4A, lsr, accum, modify)
        INSTR_IMPLD(0x4C, jmp)
        INSTR_AMODE(0x4D, eor, abs, read)
        INSTR_AMODE(0x4E, lsr, abs, modify)
        INSTR_OTHER(0x50, branch, procstatus.ov == 0)      // bvc
        INSTR_AMODE(0x51, eor, indy, read)
        INSTR_AMODE(0x55, eor, zerox, read)
        INSTR_AMODE(0x56, lsr, zerox, modify)
        INSTR_OTHER(0x58, flag, procstatus.intdis, false)  //cli
        INSTR_AMODE(0x59, eor, absy, read)
        INSTR_AMODE(0x5D, eor, absx, read)
        INSTR_AMODE(0x5E, lsr, absx, modify)
        INSTR_IMPLD(0x60, rts)
        INSTR_AMODE(0x61, adc, indx, read)
        INSTR_AMODE(0x65, adc, zero, read)
        INSTR_AMODE(0x66, ror, zero, modify)
        INSTR_IMPLD(0x68, pla)
        INSTR_AMODE(0x69, adc, imm, read)
        INSTR_AMODE(0x6A, ror, accum, modify)
        INSTR_IMPLD(0x6C, jmp_ind)
        INSTR_AMODE(0x6D, adc, abs, read)
        INSTR_AMODE(0x6E, ror, abs, modify)
        INSTR_OTHER(0x70, branch, procstatus.ov == 1)      // bvs
        INSTR_AMODE(0x71, adc, indy, read)
        INSTR_AMODE(0x75, adc, zerox, read)
        INSTR_AMODE(0x76, ror, zerox, modify)
        INSTR_OTHER(0x78, flag, procstatus.intdis, true)   //sei
        INSTR_AMODE(0x79, adc, absy, read)
        INSTR_AMODE(0x7D, adc, absx, read)
        INSTR_AMODE(0x7E, ror, absx, modify)
        INSTR_WRITE(0x81, indx, accum)                      // sta
        INSTR_WRITE(0x84, zero, yreg)                       // sty
        INSTR_WRITE(0x85, zero, accum)                      // sta
        INSTR_WRITE(0x86, zero, xreg)                       // stx
        INSTR_IMPLD(0x88, dey)
        INSTR_OTHER(0x8A, transfer, xreg, accum)            // txa
        INSTR_WRITE(0x8C, abs, yreg)                        // sty
        INSTR_WRITE(0x8D, abs, accum)                       // sta
        INSTR_WRITE(0x8E, abs, xreg)                        // stx
        INSTR_OTHER(0x90, branch, procstatus.carry == 0)    // bcc
        INSTR_WRITE(0x91, indy, accum)                      // sta
        INSTR_WRITE(0x94, zerox, yreg)                      // sty
        INSTR_WRITE(0x95, zerox, accum)                     // sta
        INSTR_WRITE(0x96, zeroy, xreg)                      // stx
        INSTR_OTHER(0x98, transfer, yreg, accum)            // tya
        INSTR_WRITE(0x99, absy, accum)                      // sta
        INSTR_OTHER(0x9A, transfer, xreg, sp)               // txs
        INSTR_WRITE(0x9D, absx, accum)                      // sta
        INSTR_AMODE(0xA0, ldy, imm, read)
        INSTR_AMODE(0xA1, lda, indx, read)
        INSTR_AMODE(0xA2, ldx, imm, read)
        INSTR_AMODE(0xA4, ldy, zero, read)
        INSTR_AMODE(0xA5, lda, zero, read)
        INSTR_AMODE(0xA6, ldx, zero, read)
        INSTR_OTHER(0xA8, transfer, accum, yreg)            // tay
        INSTR_AMODE(0xA9, lda, imm, read)
        INSTR_OTHER(0xAA, transfer, accum, xreg)            // tax
        INSTR_AMODE(0xAC, ldy, abs, read)
        INSTR_AMODE(0xAD, lda, abs, read)
        INSTR_AMODE(0xAE, ldx, abs, read)
        INSTR_OTHER(0xB0, branch, procstatus.carry == 1)    // bcs
        INSTR_AMODE(0xB1, lda, indy, read)
        INSTR_AMODE(0xB4, ldy, zerox, read)
        INSTR_AMODE(0xB5, lda, zerox, read)
        INSTR_AMODE(0xB6, ldx, zeroy, read)
        INSTR_OTHER(0xB8, flag, procstatus.ov, false)       // clv
        INSTR_AMODE(0xB9, lda, absy, read)
        INSTR_OTHER(0xBA, transfer, sp, xreg)               // tsx
        INSTR_AMODE(0xBC, ldy, absx, read)
        INSTR_AMODE(0xBD, lda, absx, read)
        INSTR_AMODE(0xBE, ldx, absy, read)
        INSTR_AMODE(0xC0, cpy, imm, read)
        INSTR_AMODE(0xC1, cmp, indx, read)
        INSTR_AMODE(0xC4, cpy, zero, read)
        INSTR_AMODE(0xC5, cmp, zero, read)
        INSTR_AMODE(0xC6, dec, zero, modify)
        INSTR_IMPLD(0xC8, iny)
        INSTR_AMODE(0xC9, cmp, imm, read)
        INSTR_IMPLD(0xCA, dex)
        INSTR_AMODE(0xCC, cpy, abs, read)
        INSTR_AMODE(0xCD, cmp, abs, read)
        INSTR_AMODE(0xCE, dec, abs, modify)
        INSTR_OTHER(0xD0, branch, procstatus.zero == 0)    // bne
        INSTR_AMODE(0xD1, cmp, indy, read)
        INSTR_AMODE(0xD5, cmp, zerox, read)
        INSTR_AMODE(0xD6, dec, zerox, modify)
        INSTR_OTHER(0xD8, flag, procstatus.decimal, false) //cld
        INSTR_AMODE(0xD9, cmp, absy, read)
        INSTR_AMODE(0xDD, cmp, absx, read)
        INSTR_AMODE(0xDE, dec, absx, modify)
        INSTR_AMODE(0xE0, cpx, imm, read)
        INSTR_AMODE(0xE1, sbc, indx, read)
        INSTR_AMODE(0xE4, cpx, zero, read)
        INSTR_AMODE(0xE5, sbc, zero, read)
        INSTR_AMODE(0xE6, inc, zero, modify)
        INSTR_IMPLD(0xE8, inx)
        INSTR_AMODE(0xE9, sbc, imm, read)
        INSTR_IMPLD(0xEA, nop)
        INSTR_AMODE(0xEC, cpx, abs, read)
        INSTR_AMODE(0xED, sbc, abs, read)
        INSTR_AMODE(0xEE, inc, abs, modify)
        INSTR_OTHER(0xF0, branch, procstatus.zero == 1)    // beq
        INSTR_AMODE(0xF1, sbc, indy, read)
        INSTR_AMODE(0xF5, sbc, zerox, read)
        INSTR_AMODE(0xF6, inc, zerox, modify)
        INSTR_AMODE(0xF9, sbc, absy, read)
        INSTR_AMODE(0xFD, sbc, absx, read)
        INSTR_AMODE(0xFE, inc, absx, modify)
        default:
            DBGPRINTF("error: unknown opcode: %02X\n", opcode);
            return;
    }
#undef INSTR_IMPLD
#undef INSTR_AMODE
#undef INSTR_WRITE
#undef INSTR_OTHER
}*/

#endif

