#ifndef MEMORYMAP_H_INCLUDED
#define MEMORYMAP_H_INCLUDED

namespace Core {

/* General defines for memory map sections. */
const int MEMSIZE              = 0xFFFF;

const int RAM_START            = 0;
const int RAM_END              = 0x07FF;
const int RAM_SIZE             = 0x0800;

const int STACK_START          = 0x0100;
const int STACK_END            = 0x01FF;
const int STACK_SIZE           = 0xFF;

const int RAM_MIRROR1_START    = 0x0800;
const int RAM_MIRROR1_END      = 0x0FFF;
const int RAM_MIRROR2_START    = 0x1000;
const int RAM_MIRROR2_END      = 0x17FF;
const int RAM_MIRROR3_START    = 0x1800;
const int RAM_MIRROR3_END      = 0x17FF;

enum PPUREG : int {
    CTRL                            = 0x2000,
    MASK                            = 0x2001,
    STATUS                          = 0x2002,
    OAMADDR                         = 0x2003,
    OAMDATA                         = 0x2004,
    SCROLL                          = 0x2005,
    ADDR                            = 0x2006,
    DATA                            = 0x2007,
};

const int PPU_MIRROR_START     = 0x2008;
const int PPU_MIRROR_END       = 0x3FFF;
const int PPU_MIRROR_SIZE      = 0x1FF8;

enum APUREG : int {
    PULSE1_VOL                      = 0x4000,
    PULSE1_SWEEP                    = 0x4001,
    PULSE1_TLOW                     = 0x4002,
    PULSE1_THI_LENC                 = 0x4003,
    PULSE2_VOL                      = 0x4004,
    PULSE2_SWEEP                    = 0x4005,
    PULSE2_TLOW                     = 0x4006,
    PULSE3_THI_LENC                 = 0x4007,

    TRI_LINCONT                     = 0x4008,
    TRI_TLOW                        = 0x400A,
    TRI_THI_LENC                    = 0x400B,

    NOISE_VOL                       = 0x400C,
    NOISE_PERIOD                    = 0x400E,
    NOISE_LENC                      = 0x400F,

    DMC_FREQ                        = 0x4010,
    DMC_LOADC                       = 0x4011,
    DMC_START                       = 0x4012,
    DMC_LEN                         = 0x4013,

    APU_CHN_STAT                    = 0x4015,
    APU_FC                          = 0x4017,
};

const int OAMDMA               = 0x4014;
const int JOYREG1              = 0x4016;
const int JOYREG2              = 0x4017;

const int APU_DISABLED_START   = 0x4018;
const int APU_DISABLED_END     = 0x401F;

const int CART_SPACE_START     = 0x4020;

const int SRAM_START           = 0x6000;
const int SRAM_END             = 0x7FFF;
const int SRAM_SIZE            = 0x1FFF;

const int PRGROM_START         = 0x8000;
const int PRGROM_END           = 0xFFFF;
const int PRGROM_SIZE          = 0x7FFF;

const int NMIVEC               = 0xFFFA;
const int RESETVEC             = 0xFFFC;
const int IRQBRKVEC            = 0xFFFE;

#endif

} // namespace Core

