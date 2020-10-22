#ifndef BACKGROUND_HPP_INCLUDED
#define BACKGROUND_HPP_INCLUDED

namespace Core {

struct Background {
    uint8_t *vram_mem = nullptr; // 2 KiB

    bool patterntab_addr;
    uint8_t nt_base_addr;
    bool show;
    bool show_leftmost;

    uint16_t addr, tmp;
    uint8_t fine_x_scroll:
    // low - for the low bg byte
    // high - for the high bg byte
    // both hold data for two tiles and are shifted every cycle
    Reg16 shift_low, shift_high;
    // these two hold info for one tile, not two
    uint8_t shift_attr1, shift_attr2;

    struct {
        uint8_t nt, at, lowbg, hibg;
    } internal_latch;

    Background() : vram_mem(new uint8_t[2048])
    { }
    Background(const Background &) = delete;
    Background(Background &&) = delete;
    Background & operator= (const Background &) = delete;
    Background & operator= (Background &&) = delete;

    uint8_t read(uint16_t addr);
    void write(uint16_t addr, uint8_t data);
    void cycle();
    void fetch_nt();
    void fetch_at();
    void fetch_lowbg();
    void fetch_highbg();
};

}

#endif
