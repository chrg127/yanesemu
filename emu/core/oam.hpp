#ifndef OAM_HPP_INCLUDED
#define OAM_HPP_INCLUDED

namespace Core {

struct OAM {
    uint8_t *oam = nullptr;
    
    bool sprsize;
    bool patterntab_addr;
    bool show;
    bool show_leftmost;
    uint8_t addr;
    uint8_t data;
    uint8_t dma;

    uint8_t shifts[8];
    uint8_t latches[8];
    uint8_t counters[8];

    OAM() : oam(new uint8_t[PPUMap::OAM_SIZE])
    { }
    OAM(const OAM &) = delete;
    OAM(OAM &&) = delete;
    OAM & operator= (const OAM &) = delete;
    OAM & operator= (OAM &&) = delete;
    
    void power();
    uint8_t read(uint16_t addr);
    void write(uint16_t addr, uint8_t data);
};

}
#endif
