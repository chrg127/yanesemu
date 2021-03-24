#ifndef CORE_INSTRINFO_HPP_INCLUDED
#define CORE_INSTRINFO_HPP_INCLUDED

#include <string>
#include <emu/util/unsigned.hpp>

namespace Core {

/* These structures are used inside the CPU -- but I define
 * them here to avoid a circular dependency */
union Reg16 {
    struct { uint8 low, high; };
    uint16 full = 0;

    Reg16() = default;
    Reg16(uint16 val)                                     { operator=(val); }
    Reg16 & operator=(const uint16 val)                   { full = val;  return *this; }
    template <typename T> Reg16 & operator&=(const T val) { full &= val; return *this; }
    template <typename T> Reg16 & operator|=(const T val) { full |= val; return *this; }
};

struct ProcStatus {
    bool carry   = 0;
    bool zero    = 0;
    bool intdis  = 0;
    bool decimal = 0;
    bool breakf  = 0;
    bool unused  = 1;
    bool ov      = 0;
    bool neg     = 0;

    operator uint8() const
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

struct Instruction {
    // uint16 pc;
    uint8 id;
    Reg16 op;
    unsigned numb;
    std::string str;
};

std::string disassemble(const uint8 instr, const uint8 oplow, const uint8 ophigh);
unsigned num_bytes(uint8 id);

void disassemble_block(uint16 start, uint16 end, auto &&readval, auto &&process)
{
    while (start <= end) {
        uint8 id   = readval(start);
        uint8 low  = readval(start + 1);
        uint8 high = readval(start + 2);
        process(start, disassemble(id, low, high));
        start += num_bytes(id);
    }
}

/* It is interesting to note that branch instructions follow a specific pattern in
 * their corresponding id:
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
constexpr inline bool is_branch(const uint8 instr)
{
    return (instr & 0x1F) == 0x10;
}

constexpr inline bool took_branch(uint8 instr, const ProcStatus &ps)
{
    switch (instr) {
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

constexpr inline uint16 branch_pointer(const uint8 branch_arg, const uint16 pc)
{
    return pc + 2 + (int8_t) branch_arg;
}

constexpr inline bool is_jump(const uint8 instr)
{
    return instr == 0x20 || instr == 0x4C || instr == 0x6C;
}

} // namespace Core

#endif
