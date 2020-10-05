#ifndef PPU_HPP_INCLUDED
#define PPU_HPP_INCLUDED

#include <emu/core/types.hpp>
#include <emu/core/memorymap.hpp>

namespace Core {

// forward decls
class PPUBus;

class PPU {
    static const int SCANLINE_COUNT = 240;
    static const int SCANLINE_WIDTH = 256;

    PPUBus *bus = nullptr;
    uint8_t *oam = nullptr;
    uint8_t *vram = nullptr;    // 2 KiB

    bool did_reset = false;

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

        inline void clear()
        { scroll = addr = latch = 0; }
    } addr_latch;
    uint8_t ppu_data;
    uint8_t oam_dma;

    // nmi regs
    bool nmi_occurred, nmi_output;
    /*
     * Start of vertical blanking: Set NMI_occurred in PPU to true.
     * End of vertical blanking, sometime in pre-render scanline:
     * Set NMI_occurred to false. 
     * Read PPUSTATUS: Return old status of NMI_occurred in bit 7, then
     * set NMI_occurred to false.
     * Write to PPUCTRL: Set NMI_output to bit 7.
     */
    
    // rendering regs
    // background
    struct {
        uint8_t vram_addr;
        uint8_t tmp_vram_addr;
        uint8_t fine_x_scroll;
        bool write_toggle;
        uint16_t shift16_1, shift16_2;
        uint8_t shift8_1, shift8_2;
    } bg_regs;

    struct {
        uint8_t shift_regs[8];
        uint8_t latches[8];
        uint8_t counters[8];
    } sprite_regs;

    void render();
    void scanline();
    void sprite_evaluation();

public:
    PPU() : oam(new uint8_t[PPUMap::OAM_SIZE])
    { }

    ~PPU()
    {
        delete[] oam;
    }

    void power(uint8_t *chrrom);
    void reset();
    void main();
    uint8_t readreg(const uint16_t which);
    void writereg(const uint16_t which, const uint8_t data);
};

enum {
    PPU_CTRL_NAMETABLE_ADDR      = 0x03, // 0: $2000, 1: $2400, 2: $2800, 3: $2C00
    PPU_CTRL_VRAM_INCREMENT      = 0x04, // 0: add 1, going across; 1: add 32, going down
    PPU_CTRL_SPR_ADDR            = 0x08, // 0: $0000, 1: $1000
    PPU_CTRL_BG_ADDR             = 0x10, // 0: $0000, 1: $1000
    PPU_CTRL_SPRSIZE             = 0x20, // 0: 8x8, 1: 16x16
    PPU_CTRL_MASTER_SLAVE_SELECT = 0x40, // 0: read backdrop from ext pins, 1: out color on ext pins
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
    LBITS       = 0x1F,
    SPR_OV      = 0x20,
    SPR_ZERO    = 0x40,
    VBLANK      = 0x80,
};

} // namespace Core

#endif
