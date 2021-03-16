#include <fmt/core.h>
#include <emu/core/opcodeinfo.hpp>

namespace Core {

/* It is interesting to note that branch opcodes follow a specific pattern in
 * their corresponding code:
 *    ffsmmmmm
 * mmmmm = mask. must be exactly 0x1F (0b10000).
 * s     = the corresponding processor flag must be equal to this bit.
 * ff    = which flag it affects?
 *
 * Possible values for ff:
 *   00 -> neg
 *   01 -> ov
 *   10 -> carry
 *   11 -> zero
 *
 * For example: 0x70 -> 0b01110000
 *   ff = 01 -> overflow flag
 *   s  =  1 -> overflow flag == 1
 *
 * The ProcStatus struct doesn't really follow this layout so almost none
 * of this info is useful.
 */
bool took_branch(const uint8 opcode, const ProcStatus &ps)
{
    switch (opcode) {
    case 0x10: return ps.neg == 0;
    case 0x30: return ps.neg == 1;
    case 0x50: return ps.ov == 0;
    case 0x70: return ps.ov == 1;
    case 0x90: return ps.carry == 0;
    case 0xB0: return ps.carry == 1;
    case 0xD0: return ps.zero == 0;
    case 0xF0: return ps.zero == 1;
    default:   return false;
    }
}

uint16 branch_next_addr(const Opcode &opcode, const Reg16 pc, const ProcStatus &ps)
{
    return took_branch(opcode.code, ps) ? pc.reg + 2 + (int8_t) opcode.args.low
                                        : pc.reg + opcode.info.numb;
}

// disassemble() stuff
enum DisassTag { IMM = 0, ZERO, ZEROX, ZEROY, INDX, INDY };
enum DisassTag16 { ABS = 0, ABSX, ABSY, IND };

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

OpcodeInfo disassemble(uint8 opcode, uint8 arglow, uint8 arghigh)
{
    uint8 oplow = arglow;
    uint8 ophigh = arghigh;
#define INSTR_IMPLD(id, name) case id: return { .str = std::string(#name), .numb = 1 };
#define INSTR_ACCUM(id, name) case id: return { .str = fmt::format("{} A", #name), .numb = 1 };
#define INSTR_ADDRMODE8(id, name, mode, op) case id: return { .str = disass<DisassTag::mode>(#name, op), .numb = 2 };
#define INSTR_ADDRMODE16(id, name, mode, oplow, ophigh) case id: return { .str = disass16<DisassTag16::mode>(#name, oplow, ophigh), .numb = 3 };
#define INSTR_BRNCH(id, name) case id: return { .str = fmt::format("{} {}", #name, (int8_t) oplow), .numb = 2 };

    switch(opcode) {
        INSTR_IMPLD(0x00, BRK)
        INSTR_ADDRMODE8(0x01, ORA, INDX, oplow)
        INSTR_ADDRMODE8(0x05, ORA, ZERO, oplow)
        INSTR_ADDRMODE8(0x06, ASL, ZERO, oplow)
        INSTR_IMPLD(0x08, PHP)
        INSTR_ADDRMODE8(0x09, ORA, IMM, oplow)
        INSTR_ACCUM(0x0A, ASL)
        INSTR_ADDRMODE16(0x0D, ORA, ABS, oplow, ophigh)
        INSTR_ADDRMODE16(0x0E, ASL, ABS, oplow, ophigh)
        INSTR_BRNCH(0x10, BPL)//, ps.neg == 0)
        INSTR_ADDRMODE8(0x11, ORA, INDY, oplow)
        INSTR_ADDRMODE8(0x15, ORA, ZEROX, oplow)
        INSTR_ADDRMODE8(0x16, ASL, ZEROX, oplow)
        INSTR_IMPLD(0x18, CLC)
        INSTR_ADDRMODE16(0x19, ORA, ABSY, oplow, ophigh)
        INSTR_ADDRMODE16(0x1D, ORA, ABSX, oplow, ophigh)
        INSTR_ADDRMODE16(0x1E, ASL, ABSX, oplow, ophigh)
        INSTR_ADDRMODE16(0x20, JSR, ABS, oplow, ophigh)
        INSTR_ADDRMODE8(0x21, AND, INDX, oplow)
        INSTR_ADDRMODE8(0x24, BIT, ZERO, oplow)
        INSTR_ADDRMODE8(0x25, AND, ZERO, oplow)
        INSTR_ADDRMODE8(0x26, ROL, ZERO, oplow)
        INSTR_IMPLD(0x28, PLP)
        INSTR_ADDRMODE8(0x29, AND, IMM, oplow)
        INSTR_ACCUM(0x2A, ROL)
        INSTR_ADDRMODE16(0x2C, BIT, ABS, oplow, ophigh)
        INSTR_ADDRMODE16(0x2D, AND, ABS, oplow, ophigh)
        INSTR_ADDRMODE16(0x2E, ROL, ABS, oplow, ophigh)
        INSTR_BRNCH(0x30, BMI)//, ps.neg == 1)
        INSTR_ADDRMODE8(0x31, AND, INDY, oplow)
        INSTR_ADDRMODE8(0x35, AND, ZEROX, oplow)
        INSTR_ADDRMODE8(0x36, ROL, ZEROX, oplow)
        INSTR_IMPLD(0x38, SEC)
        INSTR_ADDRMODE16(0x39, AND, ABSY, oplow, ophigh)
        INSTR_ADDRMODE16(0x3D, AND, ABSX, oplow, ophigh)
        INSTR_ADDRMODE16(0x3E, ROL, ABSX, oplow, ophigh)
        INSTR_IMPLD(0x40, RTI)
        INSTR_ADDRMODE8(0x41, EOR, INDX, oplow)
        INSTR_ADDRMODE8(0x45, EOR, ZERO, oplow)
        INSTR_ADDRMODE8(0x46, LSR, ZERO, oplow)
        INSTR_IMPLD(0x48, PHA)
        INSTR_ADDRMODE8(0x49, EOR, IMM, oplow)
        INSTR_ACCUM(0x4A, LSR)
        INSTR_ADDRMODE16(0x4C, JMP, ABS, oplow, ophigh)
        INSTR_ADDRMODE16(0x4D, EOR, ABS, oplow, ophigh)
        INSTR_ADDRMODE16(0x4E, LSR, ABS, oplow, ophigh)
        INSTR_BRNCH(0x50, BVC)//, ps.ov == 0)
        INSTR_ADDRMODE8(0x51, EOR, INDY, oplow)
        INSTR_ADDRMODE8(0x55, EOR, ZEROX, oplow)
        INSTR_ADDRMODE8(0x56, LSR, ZEROX, oplow)
        INSTR_IMPLD(0x58, CLI)
        INSTR_ADDRMODE16(0x59, EOR, ABSY, oplow, ophigh)
        INSTR_ADDRMODE16(0x5D, EOR, ABSX, oplow, ophigh)
        INSTR_ADDRMODE16(0x5E, LSR, ABSX, oplow, ophigh)
        INSTR_IMPLD(0x60, RTS)
        INSTR_ADDRMODE8(0x61, ADC, INDX, oplow)
        INSTR_ADDRMODE8(0x65, ADC, ZERO, oplow)
        INSTR_ADDRMODE8(0x66, ROR, ZERO, oplow)
        INSTR_IMPLD(0x68, PLA)
        INSTR_ADDRMODE8(0x69, ADC, IMM, oplow)
        INSTR_ACCUM(0x6A, ROR)
        INSTR_ADDRMODE16(0x6C, JMP, IND, oplow, ophigh)
        INSTR_ADDRMODE16(0x6D, ADC, ABS, oplow, ophigh)
        INSTR_ADDRMODE16(0x6E, ROR, ABS, oplow, ophigh)
        INSTR_BRNCH(0x70, BVS)//, ps.ov == 1)
        INSTR_ADDRMODE8(0x71, ADC, INDY, oplow)
        INSTR_ADDRMODE8(0x75, ADC, ZEROX, oplow)
        INSTR_ADDRMODE8(0x76, ROR, ZEROX, oplow)
        INSTR_IMPLD(0x78, SEI)
        INSTR_ADDRMODE16(0x79, ADC, ABSY, oplow, ophigh)
        INSTR_ADDRMODE16(0x7D, ADC, ABSX, oplow, ophigh)
        INSTR_ADDRMODE16(0x7E, ROR, ABSX, oplow, ophigh)
        INSTR_ADDRMODE8(0x81, STA, INDX, oplow)
        INSTR_ADDRMODE8(0x84, STY, ZERO, oplow)
        INSTR_ADDRMODE8(0x85, STA, ZERO, oplow)
        INSTR_ADDRMODE8(0x86, STX, ZERO, oplow)
        INSTR_IMPLD(0x88, DEY)
        INSTR_IMPLD(0x8A, TXA)
        INSTR_ADDRMODE16(0x8C, STY, ABS, oplow, ophigh)
        INSTR_ADDRMODE16(0x8D, STA, ABS, oplow, ophigh)
        INSTR_ADDRMODE16(0x8E, STX, ABS, oplow, ophigh)
        INSTR_BRNCH(0x90, BCC)//, ps.carry == 0)
        INSTR_ADDRMODE8(0x91, STA, INDY, oplow)
        INSTR_ADDRMODE8(0x94, STA, ZEROX, oplow)
        INSTR_ADDRMODE8(0x95, STA, ZEROX, oplow)
        INSTR_ADDRMODE8(0x96, STX, ZEROY, oplow)
        INSTR_IMPLD(0x98, TYA)
        INSTR_ADDRMODE16(0x99, STA, ABSY, oplow, ophigh)
        INSTR_IMPLD(0x9A, TXS)
        INSTR_ADDRMODE16(0x9D, STA, ABSX, oplow, ophigh)
        INSTR_ADDRMODE8(0xA0, LDY, IMM, oplow)
        INSTR_ADDRMODE8(0xA1, LDA, INDX, oplow)
        INSTR_ADDRMODE8(0xA2, LDX, IMM, oplow)
        INSTR_ADDRMODE8(0xA4, LDY, ZERO, oplow)
        INSTR_ADDRMODE8(0xA5, LDA, ZERO, oplow)
        INSTR_ADDRMODE8(0xA6, LDX, ZERO, oplow)
        INSTR_IMPLD(0xA8, TAY)
        INSTR_ADDRMODE8(0xA9, LDA, IMM, oplow)
        INSTR_IMPLD(0xAA, TAX)
        INSTR_ADDRMODE16(0xAC, LDY, ABS, oplow, ophigh)
        INSTR_ADDRMODE16(0xAD, LDA, ABS, oplow, ophigh)
        INSTR_ADDRMODE16(0xAE, LDX, ABS, oplow, ophigh)
        INSTR_BRNCH(0xB0, BCS)//, ps.carry == 1)
        INSTR_ADDRMODE8(0xB1, LDA, INDY, oplow)
        INSTR_ADDRMODE8(0xB4, LDY, ZEROX, oplow)
        INSTR_ADDRMODE8(0xB5, LDA, ZEROX, oplow)
        INSTR_ADDRMODE8(0xB6, LDX, ZEROY, oplow)
        INSTR_IMPLD(0xB8, CLV)
        INSTR_ADDRMODE16(0xB9, LDA, ABSY, oplow, ophigh)
        INSTR_IMPLD(0xBA, TSX)
        INSTR_ADDRMODE16(0xBC, LDY, ABSX, oplow, ophigh)
        INSTR_ADDRMODE16(0xBD, LDA, ABSX, oplow, ophigh)
        INSTR_ADDRMODE16(0xBE, LDX, ABSY, oplow, ophigh)
        INSTR_ADDRMODE8(0xC0, CPY, IMM, oplow)
        INSTR_ADDRMODE8(0xC1, CMP, INDX, oplow)
        INSTR_ADDRMODE8(0xC4, CPY, ZERO, oplow)
        INSTR_ADDRMODE8(0xC5, CMP, ZERO, oplow)
        INSTR_ADDRMODE8(0xC6, DEC, ZERO, oplow)
        INSTR_IMPLD(0xC8, INY)
        INSTR_ADDRMODE8(0xC9, CMP, IMM, oplow)
        INSTR_IMPLD(0xCA, DEX)
        INSTR_ADDRMODE16(0xCC, CPY, ABS, oplow, ophigh)
        INSTR_ADDRMODE16(0xCD, CMP, ABS, oplow, ophigh)
        INSTR_ADDRMODE16(0xCE, DEC, ABS, oplow, ophigh)
        INSTR_BRNCH(0xD0, BNE)//, ps.ZERO == 0)
        INSTR_ADDRMODE8(0xD1, CMP, INDY, oplow)
        INSTR_ADDRMODE8(0xD5, CMP, ZEROX, oplow)
        INSTR_ADDRMODE8(0xD6, DEC, ZEROX, oplow)
        INSTR_IMPLD(0xD8, CLD)
        INSTR_ADDRMODE16(0xD9, CMP, ABSY, oplow, ophigh)
        INSTR_ADDRMODE16(0xDD, CMP, ABSX, oplow, ophigh)
        INSTR_ADDRMODE16(0xDE, DEC, ABSX, oplow, ophigh)
        INSTR_ADDRMODE8(0xE0, CPX, IMM, oplow)
        INSTR_ADDRMODE8(0xE1, SBC, INDX, oplow)
        INSTR_ADDRMODE8(0xE4, CPX, ZERO, oplow)
        INSTR_ADDRMODE8(0xE5, SBC, ZERO, oplow)
        INSTR_ADDRMODE8(0xE6, INC, ZERO, oplow)
        INSTR_IMPLD(0xE8, INX)
        INSTR_ADDRMODE8(0xE9, SBC, IMM, oplow)
        INSTR_IMPLD(0xEA, NOP)
        INSTR_ADDRMODE16(0xEC, CPX, ABS, oplow, ophigh)
        INSTR_ADDRMODE16(0xED, SBC, ABS, oplow, ophigh)
        INSTR_ADDRMODE16(0xEE, INC, ABS, oplow, ophigh)
        INSTR_BRNCH(0xF0, BEQ)//, ps.ZERO == 1)
        INSTR_ADDRMODE8(0xF1, SBC, INDY, oplow)
        INSTR_ADDRMODE8(0xF5, SBC, ZEROX, oplow)
        INSTR_ADDRMODE8(0xF6, INC, ZEROX, oplow)
        INSTR_ADDRMODE16(0xF9, SBC, ABSY, oplow, ophigh)
        INSTR_ADDRMODE16(0xFD, SBC, ABSX, oplow, ophigh)
        INSTR_ADDRMODE16(0xFE, INC, ABSX, oplow, ophigh)
        default:
            return { .str = "[Unknown]", .numb = 0 };
    }
    // unreachable

#undef INSTR_IMPLD
#undef INSTR_ADDRMODE8
#undef INSTR_ADDRMODE16
#undef INSTR_IMPL
}

} // namespace Core

