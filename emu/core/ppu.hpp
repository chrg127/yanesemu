#ifndef PPU_HPP_INCLUDED
#define PPU_HPP_INCLUDED

#include <emu/core/types.hpp>
#include <emu/core/memorymap.hpp>

namespace Core {

// forward decls
class PPUBus;

class PPU {
    PPUBus *bus = nullptr;
    uint8_t *oam = nullptr;

    bool did_reset = false;
    
    // ppu's internal data bus. gets filled on reading and writing to regs.
    uint8_t ppu_io_latch;

    uint8_t ctrl;
    uint8_t mask;
    uint8_t status;
    uint8_t oam_addr;
    uint8_t oam_data;
    struct {
        uint8_t x = 0, y = 0;
        bool latch = false;

        void operator=(uint8_t data)
        {
            // ((!latch) ? x : y) = data;
            if (!latch)
                x = data;
            else
                y = data;
            latch ^= 1;
        }

        inline void clear()
        { x = y = latch = 0; }
    } scroll;
    struct {
        uint8_t hi = 0, lo = 0;
        bool latch = false;

        void operator=(uint8_t data)
        {
            if (!latch)
                hi = data;
            else
                lo = data;
            latch ^= 1;
        }

        inline void clear()
        { hi = lo = latch = 0; }
    } address;
    uint8_t ppu_data;
    uint8_t oam_dma;

    uint8_t scroll_latch, address_latch;

public:
    PPU()
    {
        oam = new uint8_t[PPUMap::OAM_SIZE];
    }

    ~PPU()
    {
        delete[] oam;
    }
    
    void power(uint8_t *chrrom);
    void reset();
    uint8_t readreg(const uint16_t which);
    void writereg(const uint16_t which, const uint8_t data);
};

enum PPU_CTRL : int {
    NAMETABLE_ADDR      = 0x03, // 0: $2000, 1: $2400, 2: $2800, 3: $2C00
    VRAM_INCREMENT      = 0x04, // 0: add 1, going across; 1: add 32, going down
    SPR_ADDR            = 0x08, // 0: $0000, 1: $1000
    BG_ADDR             = 0x10, // 0: $0000, 1: $1000
    SPRSIZE             = 0x20, // 0: 8x8, 1: 16x16
    MASTER_SLAVE_SELECT = 0x40, // 0: read backdrop from ext pins, 1: out color on ext pins
    NMI_ENABLE          = 0x80, // 0: off, 1: on
};

enum PPU_MASK : int {
    GREYSCALE       = 0x01,
    BG_LEFT_ENABLE  = 0x02,
    SP_LEFT_ENABLE  = 0x04,
    BG_ENABLE       = 0x08,
    SP_ENABLE       = 0x10,
    EMPHASIS_RED    = 0x20,
    EMPHASIS_GREEN  = 0x40,
    EMPHASIS_BLUE   = 0x80,
};

enum PPU_STATUS : int {
    LBITS       = 0x1F,
    SPR_OV      = 0x20,
    SPR_ZERO    = 0x40,
    VBLANK      = 0x80,
};

} // namespace Core

#endif
