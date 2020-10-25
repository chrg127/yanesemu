struct Background {
    bool patterntab_addr;
    uint8_t nt_base_addr;
    bool show;
    bool show_leftmost;

    // low - for the low bg byte
    // high - for the high bg byte
    // both hold data for two tiles and are shifted every cycle
    Reg16 shift_low, shift_high;
    // these two hold info for one tile, not two
    uint8_t shift_attr1, shift_attr2;

    struct {
        uint8_t nt, at, lowbg, hibg;
    } internal_latch;

    void power();
    void cycle();
    void fetch_nt();
    void fetch_at();
    void fetch_lowbg();
    void fetch_highbg();
};
