struct Bus {
    uint8_t memory[CPUMap::MEMSIZE];
    bool write_enable = false;

    void initmem(uint8_t *prgrom, std::size_t romsize);
    uint8_t read(uint16_t addr);
    void write(uint16_t addr, uint8_t val);
    void reset()
    { }
    uint8_t *getmemory()
    { return memory; }
};
