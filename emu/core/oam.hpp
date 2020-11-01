struct OAM {
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

    void power();
    void reset();
    uint8_t read(uint16_t addr);
    void write(uint16_t addr, uint8_t data);
};
