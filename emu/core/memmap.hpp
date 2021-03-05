#ifndef CORE_MEMORYMAP_HPP_INCLUDED
#define CORE_MEMORYMAP_HPP_INCLUDED

#include <cstdint>

namespace Core {

enum {
    CPUBUS_SIZE     = 0x10000,
    RAM_SIZE        = 0x2000,
    RAM_START       = 0,
    PPUREG_START    = 0x2000,
    APU_START       = 0x4000,
    CARTRIDGE_START = 0x4020,
    NMI_VEC         = 0xFFFA,
    RESET_VEC       = 0xFFFC,
    IRQ_BRK_VEC     = 0xFFFE,
    STACK_BASE      = 0x0100,

    SCREEN_WIDTH  = 256,
    SCREEN_HEIGHT = 240,

    LINE_MAX = 262,
    CYCLE_MAX = 341,

    VRAM_SIZE = 0x800,
    PAL_SIZE = 0x20,
    PT_START  = 0,
    NT_START  = 0x2000,
    PAL_START = 0x3F00,

    OAM_SIZE = 0x0100,
};

enum class Mirroring {
    VERT,
    HORZ,
    OTHER,
};

} // namespace Core

#endif
