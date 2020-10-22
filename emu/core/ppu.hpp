#ifndef PPU_HPP_INCLUDED
#define PPU_HPP_INCLUDED

#include <emu/core/types.hpp>
#include <emu/core/memorymap.hpp>
#include <emu/io/file.hpp>
#include "oam.hpp"
#include "background.hpp"

namespace Core {

// forward decls
class PPUBus;

class PPU {
    PPUBus *bus = nullptr;
    OAM oam;
    Background background;

    // ppu's internal data bus. gets filled on reading and writing to regs.
    uint8_t io_latch;

    uint8_t ctrl;
    uint8_t mask;
    uint8_t status;
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

    bool nmi_enabled;
    bool ext_bus_dir;
    uint8_t vram_increment;

    struct {
        bool greyscale, red, green, blue;
    } effects;

    int cycle;

    void scanline_render();
    void scanline_empty();
    inline int getrow(int cycle)
    {
        return cycle/340;
    }
    inline int getcol(int cycle)
    {
        return cycle%340;
    }

public:
    PPU(PPUBus *b) : bus(b)
    { }
    PPU(const PPU &) = delete;
    PPU(PPU &&) = delete;
    PPU &operator=(const PPU &) = delete;
    PPU &operator=(PPU &&) = delete;
    ~PPU()
    { }

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
