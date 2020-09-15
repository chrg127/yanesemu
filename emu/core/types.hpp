#ifndef CORE_TYPES_HPP_INCLUDED
#define CORE_TYPES_HPP_INCLUDED

#include <cstdint>

namespace Core {

union Reg16 {
    struct {
        uint8_t low, high;
    };
    uint16_t reg;

    Reg16() : reg(0) { }
    Reg16(uint16_t val) : reg(val) { }
    inline void operator=(uint16_t val)
    {
        reg = val;
    }
};

} // namespace Core

#endif
