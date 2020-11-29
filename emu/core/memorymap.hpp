#ifndef CORE_MEMORYMAP_HPP_INCLUDED
#define CORE_MEMORYMAP_HPP_INCLUDED

#include <cstdint>

namespace Core {

namespace CPUMap {

/* General defines for memory map sections. */
const int MEMSIZE              = 0xFFFF+1;

const uint16 RAM_START            = 0;
const uint16 RAM_END              = 0x07FF;
const uint16 RAM_SIZE             = 0x0800;

const uint16 STACK_START          = 0x0100;
const uint16 STACK_END            = 0x01FF;
const uint16 STACK_SIZE           = 0xFF;

const uint16 RAM_MIRROR1_START    = 0x0800;
const uint16 RAM_MIRROR1_END      = 0x0FFF;
const uint16 RAM_MIRROR2_START    = 0x1000;
const uint16 RAM_MIRROR2_END      = 0x17FF;
const uint16 RAM_MIRROR3_START    = 0x1800;
const uint16 RAM_MIRROR3_END      = 0x17FF;

enum {
    PPU_CTRL    = 0x2000,
    PPU_MASK    = 0x2001,
    PPU_STATUS  = 0x2002,
    PPU_OAM_ADDR = 0x2003,
    PPU_OAM_DATA = 0x2004,
    PPU_SCROLL  = 0x2005,
    PPU_ADDR    = 0x2006,
    PPU_DATA    = 0x2007,
};

const uint16 PPU_MIRROR_START  = 0x2008;
const uint16 PPU_MIRROR_END    = 0x3FFF;
const uint16 PPU_MIRROR_SIZE   = 0x1FF8;

enum {
    PULSE1_VOL      = 0x4000,
    PULSE1_SWEEP    = 0x4001,
    PULSE1_TLOW     = 0x4002,
    PULSE1_THI_LENC = 0x4003,
    PULSE2_VOL      = 0x4004,
    PULSE2_SWEEP    = 0x4005,
    PULSE2_TLOW     = 0x4006,
    PULSE3_THI_LENC = 0x4007,

    TRI_LINCONT     = 0x4008,
    TRI_TLOW        = 0x400A,
    TRI_THI_LENC    = 0x400B,

    NOISE_VOL       = 0x400C,
    NOISE_PERIOD    = 0x400E,
    NOISE_LENC      = 0x400F,

    DMC_FREQ        = 0x4010,
    DMC_LOADC       = 0x4011,
    DMC_START       = 0x4012,
    DMC_LEN         = 0x4013,

    APU_CHN_STAT    = 0x4015,
    APU_FC          = 0x4017,
};

const uint16 PPU_OAM_DMA               = 0x4014;
const uint16 JOYREG1              = 0x4016;
const uint16 JOYREG2              = 0x4017;

const uint16 APU_DISABLED_START   = 0x4018;
const uint16 APU_DISABLED_END     = 0x401F;

const uint16 CART_SPACE_START     = 0x4020;

const uint16 SRAM_START           = 0x6000;
const uint16 SRAM_END             = 0x7FFF;
const uint16 SRAM_SIZE            = 0x1FFF;

const uint16 PRGROM_START         = 0x8000;
const uint16 PRGROM_END           = 0xFFFF;
const uint16 PRGROM_SIZE          = 0x7FFF;

const uint16 NMIVEC               = 0xFFFA;
const uint16 RESETVEC             = 0xFFFC;
const uint16 IRQBRKVEC            = 0xFFFE;

} // namespace CPUMap

namespace PPUMap {

const int MEMSIZE = 0x4000;

enum {
    PATTERN_TABLEFT_START  = 0x0000,
    PATTERN_TABLEFT_END    = 0x0FFF,
    PATTERN_TABRIGHT_START = 0x1000,
    PATTERN_TABRIGHT_END   = 0x1FFF,
    PATTERN_TAB_SIZE       = 0x1000,
};

enum {
    NAME_TAB0_START  = 0x2000,
    NAME_TAB0_END    = 0x23FF,
    NAME_TAB1_START  = 0x2400,
    NAME_TAB1_END    = 0x27FF,
    NAME_TAB2_START  = 0x2800,
    NAME_TAB2_END    = 0x2BFF,
    NAME_TAB3_START  = 0x2C00,
    NAME_TAB3_END    = 0x2FFF,
    NAME_TAB_SIZE    = 0x0400,
    NAME_TAB_MIRROR_START = 0x3000,
    NAME_TAB_MIRROR_END   = 0x3EFF,
    NAME_TAB_MIRROR_SIZE  = 0x0F00,
};

enum {
    PALRAM_START           = 0x3F00,
    PALRAM_END             = 0x3F1F,
    PALRAM_SIZE            = 0x0020,
    PALRAM_MIRROR_START    = 0x3F20,
    PALRAM_MIRROR_END      = 0x3FFF,
    PALRAM_MIRROR_SIZE     = 0x0E00,
};

const uint16 OAM_SIZE = 0x100;

} // namespace PPUMap

} // namespace Core

#endif
