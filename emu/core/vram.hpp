struct VRAM {
    uint8_t memory[PPUMap::MEMSIZE];
    std::function <uint8_t (uint16_t)> get_nt_addr;
    uint16_t vaddr;
    Reg16 tmp;
    uint8_t finex;
    uint8_t increment;
    uint8_t buf;

    VRAM(int mirroring);
    void power(const ROM &chrrom);
    void reset();
    uint16_t address();
    uint8_t read();
    uint8_t read(uint16_t addr);
    void write(uint8_t data);
    void write(uint16_t addr, uint8_t data);
    void incv();

    friend class PPU;
};
