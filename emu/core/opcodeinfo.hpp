#ifndef CORE_INSTRINFO_HPP_INCLUDED
#define CORE_INSTRINFO_HPP_INCLUDED

#include <string>
#include <emu/util/unsigned.hpp>

namespace Core {

/* These structures are used inside the CPU -- but I define
 * them here to avoid a circular dependency */
union Reg16 {
    struct {
        uint8 low, high;
    };
    uint16 reg = 0;

    Reg16() = default;
    Reg16(uint16 val)                                     { operator=(val); }
    Reg16 & operator=(const uint16 val)                   { reg = val;  return *this; }
    template <typename T> Reg16 & operator&=(const T val) { reg &= val; return *this; }
    template <typename T> Reg16 & operator|=(const T val) { reg |= val; return *this; }
};
/* Can be different:
union Reg16 {
    uint16 value = 0;
    Util::BitField<uint16, 0, 8> low;
    Util::BitField<uint16, 8, 8> high;
};
But I still don't feel like changing so much code. */

struct ProcStatus {
    bool carry   = 0;
    bool zero    = 0;
    bool intdis  = 0;
    bool decimal = 0;
    bool breakf  = 0;
    bool unused  = 1;
    bool ov      = 0;
    bool neg     = 0;

    uint8 reg()
    {
        return carry  << 0  | zero   << 1  | intdis << 2 | decimal << 3 |
               breakf << 4  | unused << 5  | ov     << 6 | neg     << 7;
    }

    void operator=(const uint8 data)
    {
        carry   = data & 0x01;
        zero    = data & 0x02;
        intdis  = data & 0x04;
        decimal = data & 0x08;
        breakf  = data & 0x10;
        unused  = data & 0x20;
        ov      = data & 0x40;
        neg     = data & 0x80;
    }

    void reset()
    {
        carry = zero = intdis = decimal = breakf = ov = neg = 0;
        unused = 1;
    }
};

struct OpcodeInfo {
    std::string str;
    unsigned numb;
};

struct Opcode {
    uint8 code;
    Reg16 args;
    OpcodeInfo info;
};

OpcodeInfo disassemble(uint8 opcode, uint8 arglow, uint8 arghigh);
bool took_branch(uint8 opcode, const ProcStatus &ps);
uint16 branch_next_addr(const Opcode &opcode, const Reg16 pc, const ProcStatus &ps);

inline bool is_branch(uint8 opcode)
{
    return (opcode & 0x1F) == 0x10;
}

inline bool is_jump(uint8 opcode)
{
    return opcode == 0x20 || opcode == 0x4C || opcode == 0x6C;
}

} // namespace Core

#endif
