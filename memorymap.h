#ifndef MEMORYMAP_H_INCLUDED
#define MEMORYMAP_H_INCLUDED

/* General defines for memory map sections. */

#define MEMSIZE 0xFFFF

#define RAM_START   0
#define RAM_END     0x07FF
#define RAM_SIZE    0x0800

#define STACK_START 0x0100
#define STACK_END   0x01FF
#define STACK_SIZE  0xFF

#define RAM_MIRROR1_START   0x0800
#define RAM_MIRROR1_END     0x0FFF
#define RAM_MIRROR2_START   0x1000
#define RAM_MIRROR2_END     0x17FF
#define RAM_MIRROR3_START   0x1800
#define RAM_MIRROR3_END     0x17FF

enum PPUREG : int {
    CTRL    = 0x2000,
    MASK    = 0x2001,
    STATUS  = 0x2002,
    OAMADDR = 0x2003,
    OAMDATA = 0x2004,
    SCROLL  = 0x2005,
    ADDR    = 0x2006,
    DATA    = 0x2007,
};

#define PPU_MIRROR_START 0x2008
#define PPU_MIRROR_END   0x3FFF
#define PPU_MIRROR_SIZE  0x1FF8

enum APUREG : int {
    PULSE1_VOL      = 0x4000,
    PULSE1_SWEEP    = 0x4001,
    PULSE1_TLOW     = 0x4002,
    PULSE1_THI_LENC = 0x4003,
    PULSE2_VOL      = 0x4004,
    PULSE2_SWEEP    = 0x4005,
    PULSE2_TLOW     = 0x4006,
    PULSE3_THI_LENC = 0x4007,

    TRI_LINCONT = 0x4008,
    TRI_TLOW    = 0x400A,
    TRI_THI_LENC = 0x400B,

    NOISE_VOL   = 0x400C,
    NOISE_PERIOD= 0x400E,
    NOISE_LENC  = 0x400F,

    DMC_FREQ    = 0x4010,
    DMC_LOADC   = 0x4011,
    DMC_START   = 0x4012,
    DMC_LEN     = 0x4013,

    APU_CHN_STAT = 0x4015,
    APU_FC      = 0x4017,
};

#define OAMDMA  0x4014
#define JOYREG1 0x4016
#define JOYREG2 0x4017

#define APU_DISABLED_START  0x4018
#define APU_DISABLED_END    0x401F

#define CARTRIDGE_SPACE_START   0x4020

#define SRAM_START  0x6000
#define SRAM_END    0x7FFF
#define SRAM_SIZE   0x1FFF

#define PRGROM_START    0x8000
#define PRGROM_END      0xFFFF
#define PRGROM_SIZE     0x7FFF

#define NMIVEC      0xFFFA
#define RESETVEC    0xFFFC
#define IRQBRKVEC   0xFFFE

#endif

