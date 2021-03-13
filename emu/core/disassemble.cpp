#ifndef INSIDE_CPU_CPP
#error "Only emu/core/cpu.cpp may #include this file."
#else

inline static std::string disass_implied(const char name[4])
{
    return std::string(name);
}

enum DisassTag { imm = 0, zero, zerox, zeroy, indx, indy };
enum DisassTag16 { abs = 0, absx, absy, ind };

template <DisassTag Tag>
constexpr inline static std::string disass(const char name[4], uint8 op)
{
    constexpr const char *fmts[] = {
        "{} #${:02X}",  "{} ${:02X}",     "{} ${:02X},x",
        "{} ${:02X},y", "{} (${:02X},x)", "{} (${:02X}),y",
    };
    return fmt::format(fmts[Tag], name, op);
}

template <DisassTag16 Tag>
constexpr inline static std::string disass16(const char name[4], uint8 low, uint8 hi)
{
    constexpr const char *fmts[] = {
        "{} ${:02X}{:02X}", "{} ${:02X}{:02X},x", "{} ${:02X}{:02X},y", "{} (${:02X}{:02X})",
    };
    return fmt::format(fmts[Tag], name, hi, low);
}

inline static std::string disass_branch(const char name[4], int8_t disp, uint16 addr, bool took)
{
    return fmt::format("{} {} [${:02X}] {}", name, disp, addr, (took) ? "[Branch taken]" : "[Branch not taken]");
}

CPU::InstrInfo CPU::disassemble() const
{
    uint8 instruction = bus->read(pc.reg);
    uint8 oplow       = bus->read(pc.reg+1);
    uint8 ophigh      = bus->read(pc.reg+2);
    return disassemble_internal(instruction, oplow, ophigh);
}

CPU::InstrInfo CPU::disassemble_internal(uint8 instr, uint8 oplow, uint8 ophigh) const
{
#define INSTR_IMPLD(id, name) \
    case id: return { .code = instr, .lowop = oplow, .highop = ophigh, .num_bytes = 1, .to_str = std::string(#name) };
#define INSTR_ACCUM(id, name) \
    case id: return { .code = instr, .lowop = oplow, .highop = ophigh, .num_bytes = 1, .to_str = fmt::format("{} A", #name) };
#define INSTR_ADDRMODE8(id, name, mode, op) \
    case id: return { .code = instr, .lowop = oplow, .highop = ophigh, .num_bytes = 2, .to_str = disass<DisassTag::mode>(#name, op) };
#define INSTR_ADDRMODE16(id, name, mode, oplow, ophigh) \
    case id: return { .code = instr, .lowop = oplow, .highop = ophigh, .num_bytes = 3, .to_str = disass16<DisassTag16::mode>(#name, oplow, ophigh) };
    // i will leave the reason for that 2 to someone else.
#define INSTR_BRNCH(id, name, expr) \
    case id: return { .code = instr, .lowop = oplow, .highop = ophigh, .num_bytes = 2, \
                      .to_str = disass_branch(#name, (int8_t) oplow, pc.reg + 2 + (int8_t) oplow, expr) };

    switch(instr) {
        INSTR_IMPLD(0x00, BRK)
        INSTR_ADDRMODE8(0x01, ORA, indx, oplow)
        INSTR_ADDRMODE8(0x05, ORA, zero, oplow)
        INSTR_ADDRMODE8(0x06, ASL, zero, oplow)
        INSTR_IMPLD(0x08, PHP)
        INSTR_ADDRMODE8(0x09, ORA, imm, oplow)
        INSTR_ACCUM(0x0A, ASL)
        INSTR_ADDRMODE16(0x0D, ORA, abs, oplow, ophigh)
        INSTR_ADDRMODE16(0x0E, ASL, abs, oplow, ophigh)
        INSTR_BRNCH(0x10, BPL, procstatus.neg == 0)
        INSTR_ADDRMODE8(0x11, ORA, indy, oplow)
        INSTR_ADDRMODE8(0x15, ORA, zerox, oplow)
        INSTR_ADDRMODE8(0x16, ASL, zerox, oplow)
        INSTR_IMPLD(0x18, CLC)
        INSTR_ADDRMODE16(0x19, ORA, absy, oplow, ophigh)
        INSTR_ADDRMODE16(0x1D, ORA, absx, oplow, ophigh)
        INSTR_ADDRMODE16(0x1E, ASL, absx, oplow, ophigh)
        INSTR_ADDRMODE16(0x20, JSR, abs, oplow, ophigh)
        INSTR_ADDRMODE8(0x21, AND, indx, oplow)
        INSTR_ADDRMODE8(0x24, BIT, zero, oplow)
        INSTR_ADDRMODE8(0x25, AND, zero, oplow)
        INSTR_ADDRMODE8(0x26, ROL, zero, oplow)
        INSTR_IMPLD(0x28, PLP)
        INSTR_ADDRMODE8(0x29, AND, imm, oplow)
        INSTR_ACCUM(0x2A, ROL)
        INSTR_ADDRMODE16(0x2C, BIT, abs, oplow, ophigh)
        INSTR_ADDRMODE16(0x2D, AND, abs, oplow, ophigh)
        INSTR_ADDRMODE16(0x2E, ROL, abs, oplow, ophigh)
        INSTR_BRNCH(0x30, BMI, procstatus.neg == 1)
        INSTR_ADDRMODE8(0x31, AND, indy, oplow)
        INSTR_ADDRMODE8(0x35, AND, zerox, oplow)
        INSTR_ADDRMODE8(0x36, ROL, zerox, oplow)
        INSTR_IMPLD(0x38, SEC)
        INSTR_ADDRMODE16(0x39, AND, absy, oplow, ophigh)
        INSTR_ADDRMODE16(0x3D, AND, absx, oplow, ophigh)
        INSTR_ADDRMODE16(0x3E, ROL, absx, oplow, ophigh)
        INSTR_IMPLD(0x40, RTI)
        INSTR_ADDRMODE8(0x41, EOR, indx, oplow)
        INSTR_ADDRMODE8(0x45, EOR, zero, oplow)
        INSTR_ADDRMODE8(0x46, LSR, zero, oplow)
        INSTR_IMPLD(0x48, PHA)
        INSTR_ADDRMODE8(0x49, EOR, imm, oplow)
        INSTR_ACCUM(0x4A, LSR)
        INSTR_ADDRMODE16(0x4C, JMP, abs, oplow, ophigh)
        INSTR_ADDRMODE16(0x4D, EOR, abs, oplow, ophigh)
        INSTR_ADDRMODE16(0x4E, LSR, abs, oplow, ophigh)
        INSTR_BRNCH(0x50, BVC, procstatus.ov == 0)
        INSTR_ADDRMODE8(0x51, EOR, indy, oplow)
        INSTR_ADDRMODE8(0x55, EOR, zerox, oplow)
        INSTR_ADDRMODE8(0x56, LSR, zerox, oplow)
        INSTR_IMPLD(0x58, CLI)
        INSTR_ADDRMODE16(0x59, EOR, absy, oplow, ophigh)
        INSTR_ADDRMODE16(0x5D, EOR, absx, oplow, ophigh)
        INSTR_ADDRMODE16(0x5E, LSR, absx, oplow, ophigh)
        INSTR_IMPLD(0x60, RTS)
        INSTR_ADDRMODE8(0x61, ADC, indx, oplow)
        INSTR_ADDRMODE8(0x65, ADC, zero, oplow)
        INSTR_ADDRMODE8(0x66, ROR, zero, oplow)
        INSTR_IMPLD(0x68, PLA)
        INSTR_ADDRMODE8(0x69, ADC, imm, oplow)
        INSTR_ACCUM(0x6A, ROR)
        INSTR_ADDRMODE16(0x6C, JMP, ind, oplow, ophigh)
        INSTR_ADDRMODE16(0x6D, ADC, abs, oplow, ophigh)
        INSTR_ADDRMODE16(0x6E, ROR, abs, oplow, ophigh)
        INSTR_BRNCH(0x70, BVS, procstatus.ov == 1)
        INSTR_ADDRMODE8(0x71, ADC, indy, oplow)
        INSTR_ADDRMODE8(0x75, ADC, zerox, oplow)
        INSTR_ADDRMODE8(0x76, ROR, zerox, oplow)
        INSTR_IMPLD(0x78, SEI)
        INSTR_ADDRMODE16(0x79, ADC, absy, oplow, ophigh)
        INSTR_ADDRMODE16(0x7D, ADC, absx, oplow, ophigh)
        INSTR_ADDRMODE16(0x7E, ROR, absx, oplow, ophigh)
        INSTR_ADDRMODE8(0x81, STA, indx, oplow)
        INSTR_ADDRMODE8(0x84, STY, zero, oplow)
        INSTR_ADDRMODE8(0x85, STA, zero, oplow)
        INSTR_ADDRMODE8(0x86, STX, zero, oplow)
        INSTR_IMPLD(0x88, DEY)
        INSTR_IMPLD(0x8A, TXA)
        INSTR_ADDRMODE16(0x8C, STY, abs, oplow, ophigh)
        INSTR_ADDRMODE16(0x8D, STA, abs, oplow, ophigh)
        INSTR_ADDRMODE16(0x8E, STX, abs, oplow, ophigh)
        INSTR_BRNCH(0x90, BCC, procstatus.carry == 0)
        INSTR_ADDRMODE8(0x91, STA, indy, oplow)
        INSTR_ADDRMODE8(0x94, STA, zerox, oplow)
        INSTR_ADDRMODE8(0x95, STA, zerox, oplow)
        INSTR_ADDRMODE8(0x96, STX, zeroy, oplow)
        INSTR_IMPLD(0x98, TYA)
        INSTR_ADDRMODE16(0x99, STA, absy, oplow, ophigh)
        INSTR_IMPLD(0x9A, TXS)
        INSTR_ADDRMODE16(0x9D, STA, absx, oplow, ophigh)
        INSTR_ADDRMODE8(0xA0, LDY, imm, oplow)
        INSTR_ADDRMODE8(0xA1, LDA, indx, oplow)
        INSTR_ADDRMODE8(0xA2, LDX, imm, oplow)
        INSTR_ADDRMODE8(0xA4, LDY, zero, oplow)
        INSTR_ADDRMODE8(0xA5, LDA, zero, oplow)
        INSTR_ADDRMODE8(0xA6, LDX, zero, oplow)
        INSTR_IMPLD(0xA8, TAY)
        INSTR_ADDRMODE8(0xA9, LDA, imm, oplow)
        INSTR_IMPLD(0xAA, TAX)
        INSTR_ADDRMODE16(0xAC, LDY, abs, oplow, ophigh)
        INSTR_ADDRMODE16(0xAD, LDA, abs, oplow, ophigh)
        INSTR_ADDRMODE16(0xAE, LDX, abs, oplow, ophigh)
        INSTR_BRNCH(0xB0, BCS, procstatus.carry == 1)
        INSTR_ADDRMODE8(0xB1, LDA, indy, oplow)
        INSTR_ADDRMODE8(0xB4, LDY, zerox, oplow)
        INSTR_ADDRMODE8(0xB5, LDA, zerox, oplow)
        INSTR_ADDRMODE8(0xB6, LDX, zeroy, oplow)
        INSTR_IMPLD(0xB8, CLV)
        INSTR_ADDRMODE16(0xB9, LDA, absy, oplow, ophigh)
        INSTR_IMPLD(0xBA, TSX)
        INSTR_ADDRMODE16(0xBC, LDY, absx, oplow, ophigh)
        INSTR_ADDRMODE16(0xBD, LDA, absx, oplow, ophigh)
        INSTR_ADDRMODE16(0xBE, LDX, absy, oplow, ophigh)
        INSTR_ADDRMODE8(0xC0, CPY, imm, oplow)
        INSTR_ADDRMODE8(0xC1, CMP, indx, oplow)
        INSTR_ADDRMODE8(0xC4, CPY, zero, oplow)
        INSTR_ADDRMODE8(0xC5, CMP, zero, oplow)
        INSTR_ADDRMODE8(0xC6, DEC, zero, oplow)
        INSTR_IMPLD(0xC8, INY)
        INSTR_ADDRMODE8(0xC9, CMP, imm, oplow)
        INSTR_IMPLD(0xCA, DEX)
        INSTR_ADDRMODE16(0xCC, CPY, abs, oplow, ophigh)
        INSTR_ADDRMODE16(0xCD, CMP, abs, oplow, ophigh)
        INSTR_ADDRMODE16(0xCE, DEC, abs, oplow, ophigh)
        INSTR_BRNCH(0xD0, BNE, procstatus.zero == 0)
        INSTR_ADDRMODE8(0xD1, CMP, indy, oplow)
        INSTR_ADDRMODE8(0xD5, CMP, zerox, oplow)
        INSTR_ADDRMODE8(0xD6, DEC, zerox, oplow)
        INSTR_IMPLD(0xD8, CLD)
        INSTR_ADDRMODE16(0xD9, CMP, absy, oplow, ophigh)
        INSTR_ADDRMODE16(0xDD, CMP, absx, oplow, ophigh)
        INSTR_ADDRMODE16(0xDE, DEC, absx, oplow, ophigh)
        INSTR_ADDRMODE8(0xE0, CPX, imm, oplow)
        INSTR_ADDRMODE8(0xE1, SBC, indx, oplow)
        INSTR_ADDRMODE8(0xE4, CPX, zero, oplow)
        INSTR_ADDRMODE8(0xE5, SBC, zero, oplow)
        INSTR_ADDRMODE8(0xE6, INC, zero, oplow)
        INSTR_IMPLD(0xE8, INX)
        INSTR_ADDRMODE8(0xE9, SBC, imm, oplow)
        INSTR_IMPLD(0xEA, NOP)
        INSTR_ADDRMODE16(0xEC, CPX, abs, oplow, ophigh)
        INSTR_ADDRMODE16(0xED, SBC, abs, oplow, ophigh)
        INSTR_ADDRMODE16(0xEE, INC, abs, oplow, ophigh)
        INSTR_BRNCH(0xF0, BEQ, procstatus.zero == 1)
        INSTR_ADDRMODE8(0xF1, SBC, indy, oplow)
        INSTR_ADDRMODE8(0xF5, SBC, zerox, oplow)
        INSTR_ADDRMODE8(0xF6, INC, zerox, oplow)
        INSTR_ADDRMODE16(0xF9, SBC, absy, oplow, ophigh)
        INSTR_ADDRMODE16(0xFD, SBC, absx, oplow, ophigh)
        INSTR_ADDRMODE16(0xFE, INC, absx, oplow, ophigh)
        default:
            return { .code = instr, .lowop = oplow, .highop = ophigh, .num_bytes = 1, .to_str = "[Unknown]" };
    }
    // unreachable

#undef INSTR_IMPLD
#undef INSTR_ADDRMODE8
#undef INSTR_ADDRMODE16
#undef INSTR_IMPL
}

#endif
