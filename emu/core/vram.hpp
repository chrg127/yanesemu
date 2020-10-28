struct VRAM {
    uint8_t memory[PPUMap::MEMSIZE];
    std::function <uint8_t (uint16_t)> get_nt_addr;
    uint16_t addr;
    Reg16 tmp;
    uint8_t finex;
    uint8_t increment;
    uint8_t buf;

    VRAM(int mirroring);

    void initmem(const ROM &chrrom);
    uint8_t &getref(const uint16_t addr);
    uint8_t read();
    void write(uint8_t data);
    const uint8_t *getmemory() const
    { return memory; }
    uint32_t getmemsize() const
    { return PPUMap::MEMSIZE; }

    friend class PPU;
};
