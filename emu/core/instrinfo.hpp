#ifndef CORE_INSTRINFO_HPP_INCLUDED
#define CORE_INSTRINFO_HPP_INCLUDED

#include <string>
#include <emu/util/unsigned.hpp>

namespace Core {

// Given 3 bytes, return a string representing an instruction.
std::string disassemble(const uint8 id, const uint8 oplow, const uint8 ophigh);

// Return the number of bytes an instruction uses.
unsigned num_bytes(uint8 id);

/* Disassemble a whole block of memory.
 * Memory values are received from the readval function, which has the
 * signature (uint16) -> uint8.
 * String representation of instructions are sent the process, which
 * has the signature (std::string &&) -> void. */
inline void disassemble_block(uint16 start, uint16 end, auto &&readval, auto &&process)
{
    while (start <= end) {
        uint8 id   = readval(start);
        uint8 low  = readval(start + 1);
        uint8 high = readval(start + 2);
        process(start, disassemble(id, low, high));
        start += num_bytes(id);
    }
}

// Return if an instruction is a branch instruction.
constexpr inline bool is_branch(const uint8 id)
{
    return (id & 0x1F) == 0x10;
}

// Given the argument to a branch instruction and a pc value, get the real
// offset the of branch instruction.
constexpr inline uint16 branch_pointer(const uint8 branch_arg, const uint16 pc)
{
    return pc + 2 + (int8_t) branch_arg;
}

// Check if id is a jump instruction, which is one of jsr, jmp, jmp (ind)
constexpr inline bool is_jump(const uint8 id)
{
    return id == 0x20 || id == 0x4C || id == 0x6C;
}

} // namespace Core

#endif
