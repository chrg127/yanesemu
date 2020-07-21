#include "cpu.h"

#include <cstdio>
#include <cassert>
#include <cstring>
#include <cctype>
#include "memorymap.h"

static void printopcode(uint8_t op)
{
    static const char *table[] = {
        "BRK", "ORA", nullptr, nullptr, nullptr, "ORA", "ASL", nullptr, "PHP",
        "ORA", "ASL", nullptr, nullptr, "ORA", "ASL", nullptr, "BPL", "ORA",
        nullptr, nullptr, nullptr, "ORA", "ASL", nullptr, "CLC", "ORA", nullptr,
        nullptr, nullptr, "ORA", "ASL", nullptr, "JSR", "AND", nullptr, nullptr,
        "BIT", "AND", "ROL", nullptr, "PLP", "AND", "ROL", nullptr, "BIT",
        "AND", "ROL", nullptr, "BMI", "AND", nullptr, nullptr, nullptr, "AND",
        "ROL", nullptr, "SEC", "AND", nullptr, nullptr, nullptr, "AND", "ROL",
        nullptr, "RTI", "EOR", nullptr, nullptr, nullptr, "EOR", "LSR", nullptr,
        "PHA", "EOR", "LSR", nullptr, "JMP", "EOR", "LSR", nullptr, "BVC",
        "EOR", nullptr, nullptr, nullptr, "EOR", "LSR", nullptr, "CLI", "EOR",
        nullptr, nullptr, nullptr, "EOR", "LSR", nullptr, "RTS", "ADC", nullptr,
        nullptr, nullptr, "ADC", "ROR", nullptr, "PLA", "ADC", "ROR", nullptr,
        "JMP", "ADC", "ROR", nullptr, "BVS", "ADC", nullptr, nullptr, nullptr,
        "ADC", "ROR", nullptr, "SEI", "ADC", nullptr, nullptr, nullptr, "ADC",
        "ROR", nullptr, nullptr, "STA", nullptr, nullptr, "STY", "STA", "STX",
        nullptr, "DEY", nullptr, "TXA", nullptr, "STY", "STA", "STX", nullptr,
        "BCC", "STA", nullptr, nullptr, "STY", "STA", "STX", nullptr, "TYA",
        "STA", "TXS", nullptr, nullptr, "STA", nullptr, nullptr, "LDY", "LDA",
        "LDX", nullptr, "LDY", "LDA", "LDX", nullptr, "TAY", "LDA", "TAX",
        nullptr, "LDY", "LDA", "LDX", nullptr, "BCS", "LDA", nullptr, nullptr,
        "LDY", "LDA", "LDX", nullptr, "CLV", "LDA", "TSX", nullptr, "LDY",
        "LDA", "LDX", nullptr, "CPY", "CMP", nullptr, nullptr, "CPY", "CMP",
        "DEC", nullptr, "INY", "CMP", "DEX", nullptr, "CPY", "CMP", "DEC",
        nullptr, "BNE", "CMP", nullptr, nullptr, nullptr, "CMP", "DEC", nullptr,
        "CLD", "CMP", nullptr, nullptr, nullptr, "CMP", "DEC", nullptr, "CPX",
        "SBC", nullptr, nullptr, "CPX", "SBC", "INC", nullptr, "INX", "SBC",
        "NOP", nullptr, "CPX", "SBC", "INC", nullptr, "BEQ", "SBC", nullptr,
        nullptr, nullptr, "SBC", "INC", nullptr, nullptr, "SBC", nullptr, nullptr,
        nullptr, "SBC", "INC",
    };
    std::printf("Instruction: [%02X] %s", op, table[op]);
}

void CPU::initmem()
{
    std::memset(memory, 0x55, CARTRIDGE_SPACE_START-1);
    std::memcpy(memory+CARTRIDGE_SPACE_START, rom.get_prgrom(), MEMSIZE-CARTRIDGE_SPACE_START);
    pc = 0x4020;
}

uint8_t CPU::fetch()
{
    assert(pc != 0xFFFF);
    return memory[pc++];
}

#define INSTR_CASE(id, name, mode) \
    case id: addrmode_##mode(&CPU::instr_##name); break;

void CPU::execute(uint8_t opcode)
{
    printopcode(opcode);
    switch(opcode) {
        INSTR_CASE(0x00, brk, impl)
        INSTR_CASE(0x01, ora, indx)
        INSTR_CASE(0x05, ora, zero)
        INSTR_CASE(0x06, asl, zero)
        INSTR_CASE(0x08, php, impl)
        INSTR_CASE(0x09, ora, imm)
        INSTR_CASE(0x0A, asl, accum)
        INSTR_CASE(0x0D, ora, abs)
        INSTR_CASE(0x0E, asl, abs)
        INSTR_CASE(0x10, bpl, rel)
        INSTR_CASE(0x11, ora, indy)
        INSTR_CASE(0x15, ora, zerox)
        INSTR_CASE(0x16, asl, zerox)
        INSTR_CASE(0x18, clc, impl)
        INSTR_CASE(0x19, ora, absy)
        INSTR_CASE(0x1D, ora, absx)
        INSTR_CASE(0x1E, asl, absx)
        INSTR_CASE(0x20, jsr, absjmp)
        INSTR_CASE(0x21, and, indx)
        INSTR_CASE(0x24, bit, zero)
        INSTR_CASE(0x25, and, zero)
        INSTR_CASE(0x26, rol, zero)
        INSTR_CASE(0x28, plp, impl)
        INSTR_CASE(0x29, and, imm)
        INSTR_CASE(0x2A, rol, accum)
        INSTR_CASE(0x2C, bit, abs)
        INSTR_CASE(0x2D, and, abs)
        INSTR_CASE(0x2E, rol, abs)
        INSTR_CASE(0x30, bmi, rel)
        INSTR_CASE(0x31, and, indy)
        INSTR_CASE(0x35, and, zerox)
        INSTR_CASE(0x36, rol, zerox)
        INSTR_CASE(0x38, sec, impl)
        INSTR_CASE(0x39, and, absy)
        INSTR_CASE(0x3D, and, absx)
        INSTR_CASE(0x3E, rol, absx)
        INSTR_CASE(0x40, rti, impl)
        INSTR_CASE(0x41, eor, indx)
        INSTR_CASE(0x45, eor, zero)
        INSTR_CASE(0x46, lsr, zero)
        INSTR_CASE(0x48, pha, impl)
        INSTR_CASE(0x49, eor, imm)
        INSTR_CASE(0x4A, lsr, accum)
        INSTR_CASE(0x4C, jmp, absjmp)
        INSTR_CASE(0x4D, eor, abs)
        INSTR_CASE(0x4E, lsr, abs)
        INSTR_CASE(0x50, bvc, rel)
        INSTR_CASE(0x51, eor, indy)
        INSTR_CASE(0x55, eor, zerox)
        INSTR_CASE(0x56, lsr, zerox)
        INSTR_CASE(0x58, cli, impl)
        INSTR_CASE(0x59, eor, absy)
        INSTR_CASE(0x5D, eor, absx)
        INSTR_CASE(0x5E, lsr, absx)
        INSTR_CASE(0x60, rts, impl)
        INSTR_CASE(0x61, adc, indx)
        INSTR_CASE(0x65, adc, zero)
        INSTR_CASE(0x66, ror, zero)
        INSTR_CASE(0x68, pla, impl)
        INSTR_CASE(0x69, adc, imm)
        INSTR_CASE(0x6A, ror, accum)
        INSTR_CASE(0x6C, jmp, indjmp)
        INSTR_CASE(0x6D, adc, abs)
        INSTR_CASE(0x6E, ror, abs)
        INSTR_CASE(0x70, bvs, rel)
        INSTR_CASE(0x71, adc, indy)
        INSTR_CASE(0x75, adc, zerox)
        INSTR_CASE(0x76, ror, zerox)
        INSTR_CASE(0x78, sei, impl)
        INSTR_CASE(0x79, adc, absy)
        INSTR_CASE(0x7D, adc, absx)
        INSTR_CASE(0x7E, ror, absx)
        INSTR_CASE(0x81, sta, indx)
        INSTR_CASE(0x84, sty, zero)
        INSTR_CASE(0x85, sta, zero)
        INSTR_CASE(0x86, stx, zero)
        INSTR_CASE(0x88, dey, impl)
        INSTR_CASE(0x8A, txa, impl)
        INSTR_CASE(0x8C, sty, abs)
        INSTR_CASE(0x8D, sta, abs)
        INSTR_CASE(0x8E, stx, abs)
        INSTR_CASE(0x90, bcc, rel)
        INSTR_CASE(0x91, sta, indy)
        INSTR_CASE(0x94, sty, zerox)
        INSTR_CASE(0x95, sta, zerox)
        INSTR_CASE(0x96, stx, zeroy)
        INSTR_CASE(0x98, tya, impl)
        INSTR_CASE(0x99, sta, absy)
        INSTR_CASE(0x9A, txs, impl)
        INSTR_CASE(0x9D, sta, absx)
        INSTR_CASE(0xA0, ldy, imm)
        INSTR_CASE(0xA1, lda, indx)
        INSTR_CASE(0xA2, ldx, imm)
        INSTR_CASE(0xA4, ldy, zero)
        INSTR_CASE(0xA5, lda, zero)
        INSTR_CASE(0xA6, ldx, zero)
        INSTR_CASE(0xA8, tay, impl)
        INSTR_CASE(0xA9, lda, imm)
        INSTR_CASE(0xAA, tax, impl)
        INSTR_CASE(0xAC, ldy, abs)
        INSTR_CASE(0xAD, lda, abs)
        INSTR_CASE(0xAE, ldx, abs)
        INSTR_CASE(0xB0, bcs, rel)
        INSTR_CASE(0xB1, lda, indy)
        INSTR_CASE(0xB4, ldy, zerox)
        INSTR_CASE(0xB5, lda, zerox)
        INSTR_CASE(0xB6, ldx, zeroy)
        INSTR_CASE(0xB8, clv, impl)
        INSTR_CASE(0xB9, lda, absy)
        INSTR_CASE(0xBA, tsx, impl)
        INSTR_CASE(0xBC, ldy, absx)
        INSTR_CASE(0xBD, lda, absx)
        INSTR_CASE(0xBE, ldx, absy)
        INSTR_CASE(0xC0, cpy, imm)
        INSTR_CASE(0xC1, cmp, indx)
        INSTR_CASE(0xC4, cpy, zero)
        INSTR_CASE(0xC5, cmp, zero)
        INSTR_CASE(0xC6, dec, zero)
        INSTR_CASE(0xC8, iny, impl)
        INSTR_CASE(0xC9, cmp, imm)
        INSTR_CASE(0xCA, dex, impl)
        INSTR_CASE(0xCC, cpy, abs)
        INSTR_CASE(0xCD, cmp, abs)
        INSTR_CASE(0xCE, dec, abs)
        INSTR_CASE(0xD0, bne, rel)
        INSTR_CASE(0xD1, cmp, indy)
        INSTR_CASE(0xD5, cmp, zerox)
        INSTR_CASE(0xD6, dec, zerox)
        INSTR_CASE(0xD8, cld, impl)
        INSTR_CASE(0xD9, cmp, absy)
        INSTR_CASE(0xDD, cmp, absx)
        INSTR_CASE(0xDE, dec, absx)
        INSTR_CASE(0xE0, cpx, imm)
        INSTR_CASE(0xE1, sbc, indx)
        INSTR_CASE(0xE4, cpx, zero)
        INSTR_CASE(0xE5, sbc, zero)
        INSTR_CASE(0xE6, inc, zero)
        INSTR_CASE(0xE8, inx, impl)
        INSTR_CASE(0xE9, sbc, imm)
        INSTR_CASE(0xEA, nop, impl)
        INSTR_CASE(0xEC, cpx, abs)
        INSTR_CASE(0xED, sbc, abs)
        INSTR_CASE(0xEE, inc, abs)
        INSTR_CASE(0xF0, beq, rel)
        INSTR_CASE(0xF1, sbc, indy)
        INSTR_CASE(0xF5, sbc, zerox)
        INSTR_CASE(0xF6, inc, zerox)
        INSTR_CASE(0xF9, sbc, absy)
        INSTR_CASE(0xFD, sbc, absx)
        INSTR_CASE(0xFE, inc, absx)
        default:
            std::fprintf(stderr, "error: unknown opcode: %02X\n", opcode);
    }
    DBGPRINT("\n");
}

#undef INSTR_CASE

#define WRITEFLAG(f, c) putchar( (f == 1) ? std::toupper(c) : std::tolower(c) )
void CPU::printinfo()
{
    std::printf("PC: %04X A: %02X X: %02X Y: %02X S: %02X\n", pc, accum, xreg, yreg, sp);
    WRITEFLAG(procstatus.carry,     'c');
    WRITEFLAG(procstatus.zero,      'z');
    WRITEFLAG(procstatus.intdis,    'i');
    WRITEFLAG(procstatus.breakc,    'b');
    WRITEFLAG(procstatus.ov,        'v');
    WRITEFLAG(procstatus.neg,       'n');
    WRITEFLAG(procstatus.decimal,   'd');
    puts("");
}
#undef WRITEFLAG

uint8_t CPU::fetch_op()
{
    return memory[pc++];
}

uint8_t CPU::read_mem(uint16_t addr)
{
    return memory[addr];
}

void CPU::write_mem(uint16_t addr, uint8_t val)
{
    memory[addr] = val;
}

void CPU::push(uint8_t val)
{
    memory[sp--] = val;
}

uint8_t CPU::pull()
{
    return memory[sp++];
}

void CPU::memdump(FILE *f)
{
    int i, j;
    for (i = 0; i < MEMSIZE; i++) {
        for (j = 0; j < 16; j++, i++)
            std::fprintf(f, "%04X = %02X\n", i, memory[i]);
    }
}
