#include <fmt/core.h>
#include <emu/core/opcodeinfo.hpp>

namespace Core {

#define op(X) \
    X(0x00, brk, impld) \
    X(0x01, ora, indx) \
    X(0x05, ora, zero) \
    X(0x06, asl, zero) \
    X(0x08, php, impld) \
    X(0x09, ora, imm) \
    X(0x0A, asl, accum) \
    X(0x0D, ora, abs) \
    X(0x0E, asl, abs) \
    X(0x10, bpl, branch) \
    X(0x11, ora, indy) \
    X(0x15, ora, zerox) \
    X(0x16, asl, zerox) \
    X(0x18, clc, impld) \
    X(0x19, ora, absy) \
    X(0x1D, ora, absx) \
    X(0x1E, asl, absx) \
    X(0x20, jsr, abs) \
    X(0x21, and, indx) \
    X(0x24, bit, zero) \
    X(0x25, and, zero) \
    X(0x26, rol, zero) \
    X(0x28, plp, impld) \
    X(0x29, and, imm) \
    X(0x2A, rol, accum) \
    X(0x2C, bit, abs) \
    X(0x2D, and, abs) \
    X(0x2E, rol, abs) \
    X(0x30, bmi, branch) \
    X(0x31, and, indy) \
    X(0x35, and, zerox) \
    X(0x36, rol, zerox) \
    X(0x38, sec, impld) \
    X(0x39, and, absy) \
    X(0x3D, and, absx) \
    X(0x3E, rol, absx) \
    X(0x40, rti, impld) \
    X(0x41, eor, indx) \
    X(0x45, eor, zero) \
    X(0x46, lsr, zero) \
    X(0x48, pha, impld) \
    X(0x49, eor, imm) \
    X(0x4A, lsr, accum) \
    X(0x4C, jmp, abs) \
    X(0x4D, eor, abs) \
    X(0x4E, lsr, abs) \
    X(0x50, bvc, branch) \
    X(0x51, eor, indy) \
    X(0x55, eor, zerox) \
    X(0x56, lsr, zerox) \
    X(0x58, cli, impld) \
    X(0x59, eor, absy) \
    X(0x5D, eor, absx) \
    X(0x5E, lsr, absx) \
    X(0x60, rts, impld) \
    X(0x61, adc, indx) \
    X(0x65, adc, zero) \
    X(0x66, ror, zero) \
    X(0x68, pla, impld) \
    X(0x69, adc, imm) \
    X(0x6A, ror, accum) \
    X(0x6C, jmp, ind) \
    X(0x6D, adc, abs) \
    X(0x6E, ror, abs) \
    X(0x70, bvs, branch) \
    X(0x71, adc, indy) \
    X(0x75, adc, zerox) \
    X(0x76, ror, zerox) \
    X(0x78, sei, impld) \
    X(0x79, adc, absy) \
    X(0x7D, adc, absx) \
    X(0x7E, ror, absx) \
    X(0x81, sta, indx) \
    X(0x84, sty, zero) \
    X(0x85, sta, zero) \
    X(0x86, stx, zero) \
    X(0x88, dey, impld) \
    X(0x8A, txa, impld) \
    X(0x8C, sty, abs) \
    X(0x8D, sta, abs) \
    X(0x8E, stx, abs) \
    X(0x90, bcc, branch) \
    X(0x91, sta, indy) \
    X(0x94, sta, zerox) \
    X(0x95, sta, zerox) \
    X(0x96, stx, zeroy) \
    X(0x98, tya, impld) \
    X(0x99, sta, absy) \
    X(0x9A, txs, impld) \
    X(0x9D, sta, absx) \
    X(0xA0, ldy, imm) \
    X(0xA1, lda, indx) \
    X(0xA2, ldx, imm) \
    X(0xA4, ldy, zero) \
    X(0xA5, lda, zero) \
    X(0xA6, ldx, zero) \
    X(0xA8, tay, impld) \
    X(0xA9, lda, imm) \
    X(0xAA, tax, impld) \
    X(0xAC, ldy, abs) \
    X(0xAD, lda, abs) \
    X(0xAE, ldx, abs) \
    X(0xB0, bcs, branch) \
    X(0xB1, lda, indy) \
    X(0xB4, ldy, zerox) \
    X(0xB5, lda, zerox) \
    X(0xB6, ldx, zeroy) \
    X(0xB8, clv, impld) \
    X(0xB9, lda, absy) \
    X(0xBA, tsx, impld) \
    X(0xBC, ldy, absx) \
    X(0xBD, lda, absx) \
    X(0xBE, ldx, absy) \
    X(0xC0, cpy, imm) \
    X(0xC1, cmp, indx) \
    X(0xC4, cpy, zero) \
    X(0xC5, cmp, zero) \
    X(0xC6, dec, zero) \
    X(0xC8, iny, impld) \
    X(0xC9, cmp, imm) \
    X(0xCA, dex, impld) \
    X(0xCC, cpy, abs) \
    X(0xCD, cmp, abs) \
    X(0xCE, dec, abs) \
    X(0xD0, bne, branch) \
    X(0xD1, cmp, indy) \
    X(0xD5, cmp, zerox) \
    X(0xD6, dec, zerox) \
    X(0xD8, cld, impld) \
    X(0xD9, cmp, absy) \
    X(0xDD, cmp, absx) \
    X(0xDE, dec, absx) \
    X(0xE0, cpx, imm) \
    X(0xE1, sbc, indx) \
    X(0xE4, cpx, zero) \
    X(0xE5, sbc, zero) \
    X(0xE6, inc, zero) \
    X(0xE8, inx, impld) \
    X(0xE9, sbc, imm) \
    X(0xEA, nop, impld) \
    X(0xEC, cpx, abs) \
    X(0xED, sbc, abs) \
    X(0xEE, inc, abs) \
    X(0xF0, beq, branch) \
    X(0xF1, sbc, indy) \
    X(0xF5, sbc, zerox) \
    X(0xF6, inc, zerox) \
    X(0xF9, sbc, absy) \
    X(0xFD, sbc, absx) \
    X(0xFE, inc, absx) \

#define opdesc(X) \
    X(adc, "", "") \
    X(and, "", "") \
    X(asl, "", "") \
    X(bcc, "", "") \
    X(bcs, "", "") \
    X(beq, "", "") \
    X(beq, "", "") \
    X(bit, "", "") \
    X(bmi, "", "") \
    X(bne, "", "") \
    X(bpl, "", "") \
    X(brk, "", "") \
    X(bvc, "", "") \
    X(bvs, "", "") \
    X(clc, "", "") \
    X(cld, "", "") \
    X(cli, "", "") \
    X(clv, "", "") \
    X(cmp, "", "") \
    X(cpx, "", "") \
    X(cpy, "", "") \
    X(dec, "", "") \
    X(dex, "", "") \
    X(dey, "", "") \
    X(eor, "", "") \
    X(inc, "", "") \
    X(inx, "", "") \
    X(iny, "", "") \
    X(jmp, "", "") \
    X(jsr, "", "") \
    X(lda, "", "") \
    X(ldx, "", "") \
    X(ldy, "", "") \
    X(lsr, "", "") \
    X(nop, "", "") \
    X(ora, "", "") \
    X(pha, "", "") \
    X(php, "", "") \
    X(pla, "", "") \
    X(plp, "", "") \
    X(rol, "", "") \
    X(ror, "", "") \
    X(rti, "", "") \
    X(rts, "", "") \
    X(sbc, "", "") \
    X(sec, "", "") \
    X(sed, "", "") \
    X(sei, "", "") \
    X(sta, "", "") \
    X(stx, "", "") \
    X(sty, "", "") \
    X(tax, "", "") \
    X(tay, "", "") \
    X(tsx, "", "") \
    X(txa, "", "") \
    X(txs, "", "") \
    X(tya, "", "") \

OpcodeInfo disassemble(const uint8 opcode, const uint8 arglow, const uint8 arghigh)
{
    const auto disass_impld = [&](const char name[4]) -> OpcodeInfo { return { std::string(name),                                        1 }; };
    const auto disass_accum = [&](const char name[4]) -> OpcodeInfo { return { fmt::format("{} A", name),                                1 }; };
    const auto disass_branch= [&](const char name[4]) -> OpcodeInfo { return { fmt::format("{} {}", name, (int8_t) arglow),              2 }; };
    const auto disass_imm   = [&](const char name[4]) -> OpcodeInfo { return { fmt::format("{} #${:02X}",        name, arglow),          2 }; };
    const auto disass_zero  = [&](const char name[4]) -> OpcodeInfo { return { fmt::format("{} ${:02X}",         name, arglow),          2 }; };
    const auto disass_zerox = [&](const char name[4]) -> OpcodeInfo { return { fmt::format("{} ${:02X},x",       name, arglow),          2 }; };
    const auto disass_zeroy = [&](const char name[4]) -> OpcodeInfo { return { fmt::format("{} ${:02X},y",       name, arglow),          2 }; };
    const auto disass_indx  = [&](const char name[4]) -> OpcodeInfo { return { fmt::format("{} (${:02X},x)",     name, arglow),          2 }; };
    const auto disass_indy  = [&](const char name[4]) -> OpcodeInfo { return { fmt::format("{} (${:02X}),y",     name, arglow),          2 }; };
    const auto disass_abs   = [&](const char name[4]) -> OpcodeInfo { return { fmt::format("{} ${:02X}{:02X}",   name, arghigh, arglow), 3 }; };
    const auto disass_absx  = [&](const char name[4]) -> OpcodeInfo { return { fmt::format("{} ${:02X}{:02X},x", name, arghigh, arglow), 3 }; };
    const auto disass_absy  = [&](const char name[4]) -> OpcodeInfo { return { fmt::format("{} ${:02X}{:02X},y", name, arghigh, arglow), 3 }; };
    const auto disass_ind   = [&](const char name[4]) -> OpcodeInfo { return { fmt::format("{} (${:02X}{:02X})", name, arghigh, arglow), 3 }; };

#define X(id, name, mode) case id: return disass_##mode(#name);
    switch(opcode) {
        op(X)
        default:
            return { .str = "[Unknown]", .numb = 0 };
    }
#undef X
}

} // namespace Core

