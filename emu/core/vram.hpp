struct VRAM {
    uint8_t memory[PPUMap::MEMSIZE];
    std::function <uint8_t (uint16_t)> get_nt_addr;
    uint16_t vaddr;
    Reg16 tmp;
    uint8_t finex;
    uint8_t increment;
    uint8_t readbuf;

    VRAM() { };

    void power(const ROM &chrrom, int mirroring);
    void reset();
    uint16_t address(uint16_t addr);
    uint8_t read();
    uint8_t read(uint16_t addr);
    uint8_t readdata();
    void write(uint8_t data);
    void write(uint16_t addr, uint8_t data);
    void writedata(uint8_t data);
    void incv();

    friend class PPU;
};
