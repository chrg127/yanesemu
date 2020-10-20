#ifndef PPU_HPP_INCLUDED
#define PPU_HPP_INCLUDED

#include <emu/core/types.hpp>
#include <emu/core/memorymap.hpp>
#include <emu/io/file.hpp>

namespace Core {

// forward decls
class PPUBus;

class PPU {
    PPUBus *bus = nullptr;
    uint8_t *oam = nullptr;
    uint8_t *vram_mem = nullptr; // 2 KiB

    // ppu's internal data bus. gets filled on reading and writing to regs.
    uint8_t io_latch;

    // registers
    uint8_t ctrl;
    uint8_t mask;
    uint8_t status;
    uint8_t oam_addr;
    uint8_t oam_data;
    struct {
        uint8_t latch;
        uint16_t scroll;
        uint16_t addr;
        bool toggle = 0;    // needed, unfortunately, for a few operations

        inline void clear()
        { scroll = addr = latch = toggle = 0; }
    } addr_latch;
    uint8_t ppu_data;
    uint8_t oam_dma;

    struct {
        uint16_t addr; // 15 bits
        uint16_t tmp;  // 15 bits
        uint8_t fine_x_scroll; // 3 bits
        // low - for the low bg byte
        // high - for the high bg byte
        // both hold data for two tiles and are shifted every cycle
        Reg16 shift_low, shift_high;
        // these two hold info for one tile, not two
        uint8_t shift_attr1, shift_attr2;
    } vram;

    struct {
        uint8_t nt, at, lowbg, hibg;
    } internal_latch;

    struct {
        uint8_t shifts[8];
        uint8_t latches[8];
        uint8_t counters[8];
    } oam_regs;

    int lineno = 0, linec = 0;

    void scanline_render();
    void scanline_empty();
    void fetch_nt();
    void fetch_at();
    void fetch_lowbg();
    void fetch_highbg();

public:
    PPU(PPUBus *b) : bus(b), oam(new uint8_t[PPUMap::OAM_SIZE])
    { }
    PPU(const PPU &) = delete;
    PPU(PPU &&) = delete;
    PPU &operator=(const PPU &) = delete;
    PPU &operator=(PPU &&) = delete;
    ~PPU()
    {
        delete[] oam;
    }

    void power(uint8_t *chrrom);
    void reset();
    void main();
    uint8_t readreg(const uint16_t which);
    void writereg(const uint16_t which, const uint8_t data);
    void printinfo(IO::File &log);
};

enum {
    PPU_CTRL_NAMETABLE_ADDR      = 0x03, // 0: $2000, 1: $2400, 2: $2800, 3: $2C00
    PPU_CTRL_VRAM_INCREMENT      = 0x04, // 0: add 1, going across; 1: add 32, going down
    PPU_CTRL_SPR_ADDR            = 0x08, // 0: $0000, 1: $1000
    PPU_CTRL_BG_ADDR             = 0x10, // 0: $0000, 1: $1000
    PPU_CTRL_SPRSIZE             = 0x20, // 0: 8x8, 1: 16x16
    PPU_EXT_BUS_DIR              = 0x40, // 0: read bg color from ext-pins; 1: discouraged use
    PPU_CTRL_NMI_ENABLE          = 0x80, // 0: off, 1: on
};

enum {
    PPU_MASK_GREYSCALE       = 0x01,
    PPU_MASK_BG_LEFT_ENABLE  = 0x02,
    PPU_MASK_SP_LEFT_ENABLE  = 0x04,
    PPU_MASK_BG_ENABLE       = 0x08,
    PPU_MASK_SP_ENABLE       = 0x10,
    PPU_MASK_EMPHASIS_RED    = 0x20,
    PPU_MASK_EMPHASIS_GREEN  = 0x40,
    PPU_MASK_EMPHASIS_BLUE   = 0x80,
};

enum PPU_STATUS : int {
    PPU_STATUS_LBITS       = 0x1F,
    PPU_STATUS_SPR_OV      = 0x20,
    PPU_STATUS_SPR_ZERO    = 0x40,
    PPU_STATUS_VBLANK      = 0x80,
};

} // namespace Core

#endif
