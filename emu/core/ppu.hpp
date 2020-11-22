#ifndef CORE_PPU_HPP_INCLUDED
#define CORE_PPU_HPP_INCLUDED

#include <functional>
#include <emu/core/types.hpp>
#include <emu/core/memorymap.hpp>
#include <emu/utils/file.hpp>

namespace Core {

class PPU {
    unsigned long cycles = 0;
    unsigned long lines  = 0;

    uint8_t output[256*224];

    struct VRAM {
        PPU &outer;

        uint8_t memory[0x4000];
        std::function<uint8_t(uint16_t)> ntaddr;
        uint16_t vaddr;
        Reg16 tmp;
        uint8_t fine_x;
        uint8_t increment;
        uint8_t readbuf;

        VRAM(PPU &p) : outer(p)
        { }
        void power(const ROM &chrrom, int mirroring);
        void reset();
        void inc_horzpos();
        void inc_vertpos();
        void copy_horzpos();
        void copy_vertpos();
        uint16_t address(uint16_t addr);
        uint8_t read();
        uint8_t read(uint16_t addr);
        uint8_t readdata();
        void write(uint8_t data);
        void write(uint16_t addr, uint8_t data);
        void writedata(uint8_t data);
    } vram;

    struct Background {
        PPU &outer;

        bool patterntab_addr;
        uint8_t nt_base_addr;
        bool show;
        bool show_leftmost;
        struct {
            // low - for the low bg byte
            // high - for the high bg byte
            // both hold data for two tiles and are shifted every cycle
            uint16_t low, high;
            // these two hold info for one tile, not two
            // 1 - high, 2 - low
            uint8_t attr1, attr2;
            bool attrhigh_latch, attrlow_latch;
        } shift;
        struct {
            uint8_t nt, attr, lowbg, hibg;
        } latch;

        Background(PPU &p) : outer(p)
        { }
        void power();
        void reset();
        void fill_shifts();
        void shift_run();
    } bg;

    struct OAM {
        PPU &outer;

        uint8_t oam[PPUMap::OAM_SIZE];

        bool sprsize;
        bool patterntab_addr;
        bool show;
        bool show_leftmost;
        uint8_t addr;
        uint8_t data;
        uint8_t shifts[8];
        uint8_t latches[8];
        uint8_t counters[8];

        OAM(PPU &p) : outer(p)
        { }
        void power();
        void reset();
        uint8_t read(uint16_t addr);
        void write(uint16_t addr, uint8_t data);
    } oam;

    // ppu's internal data bus. gets filled on reading and writing to regs.
    uint8_t io_latch;
    struct {
        bool toggle; // used by PPUSCROLL and PPUADDR
    } latch;
    bool nmi_enabled;
    bool ext_bus_dir;
    struct {
        bool grey, red, green, blue;
    } effects;
    bool vblank;
    bool spr0hit;
    bool spr_ov;
    bool odd_frame;
    // uint24 *paltab; // pointer to an array of colors, loaded by load_palette()

    void begin_frame();
    void cycle_fetchnt(bool cycle);
    void cycle_fetchattr(bool cycle);
    void cycle_fetchlowbg(bool cycle);
    void cycle_fetchhighbg(bool cycle);
    void cycle_incvhorz();
    void cycle_incvvert();
    void cycle_copyhorz();
    void cycle_copyvert();
    void cycle_shift();
    void cycle_fillshifts();
    void cycle_outputpixel();
    void vblank_begin();
    void vblank_end();

    void fetch_nt(bool dofetch);
    void fetch_attr(bool dofetch);
    void fetch_lowbg(bool dofetch);
    void fetch_highbg(bool dofetch);
    void output_pixel();
    uint8_t output_bgpixel();
    uint8_t output_sppixel();

    // void load_palette();
    uint8_t getcolor(bool select, uint8_t pal, uint8_t palind);

    friend class Background;
    friend class OAM;
    friend class PPUBus;

public:
    PPU() : vram(*this), bg(*this), oam(*this)
    { }

    void power(const ROM &chrrom, int mirroring);
    void reset();
    void main();
    uint8_t readreg(const uint16_t which);
    void writereg(const uint16_t which, const uint8_t data);

    // for ppumain.cpp
    template <unsigned int Cycle>
    void ccycle();
    template <unsigned int Line>
    void lcycle(unsigned int cycle);
    template <unsigned Cycle>
    void background_cycle();
    void idlec();

    inline const uint8_t *getmemory() const
    { return vram.memory; }
    inline uint32_t getmemsize() const
    { return PPUMap::MEMSIZE; }
};

} // namespace Core

#endif
