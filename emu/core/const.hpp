#pragma once

namespace Core {

enum {
    // Effective bus sizes. These aren't the REAL bus sizes,
    // since most parts of it are mirrored.
    CPUBUS_SIZE     = 0x10000,
    PPUBUS_SIZE     = 0x4000,

    // RAM bus constants, defining real size and regions.
    RAM_SIZE        = 0x800,
    RAM_START       = 0,
    PPUREG_START    = 0x2000,
    APU_START       = 0x4000,
    CARTRIDGE_START = 0x4020,
    NMI_VEC         = 0xFFFA,
    RESET_VEC       = 0xFFFC,
    IRQ_BRK_VEC     = 0xFFFE,
    STACK_BASE      = 0x0100,

    // Constants for PPU memories (VRAM and OAM).
    VRAM_SIZE       = 0x800,
    PAL_SIZE        = 0x20,
    OAM_SIZE        = 0x40 * 4,
    PT_START        = 0,
    NT_START        = 0x2000,
    PAL_START       = 0x3F00,

    // Screen contants, used by the PPU.
    SCREEN_WIDTH    = 256,
    SCREEN_HEIGHT   = 240,
    PPU_MAX_LINES   = 262,
    PPU_MAX_LCYCLE  = 341,
};

// OTHER is usually mapper defined.
enum class Mirroring {
    VERT,
    HORZ,
    FOUR_SCREEN,
    OTHER,
};

} // namespace Core
