#ifndef NESROM_H_INCLUDED
#define NESROM_H_INCLUDED

#include <cstdio>
#include <cstdint>

#define HEADER_LEN 16
#define TRAINER_LEN 512

enum NesFmt {
    INVALID,
    INES,
    NES20,
};

enum Mirroring : int {
    HORZ = 0,
    VERT = 1,
};

enum Region : int {
    NTSC = 0,
    PAL  = 2,
};

enum ConsoleType : int {
    NES = 0,
    VSSYSTEM,
    PLAYCHOICE,
    EXTTYPE,
};

class RomFile {
    FILE *file;

    uint8_t header[HEADER_LEN];
    uint8_t trainer[TRAINER_LEN];

    NesFmt fformat;
    uint16_t mapper;
    uint8_t submapper;

    uint16_t prgrom_size;
    uint16_t chrrom_size;
    bool has_prgram;
    uint32_t prgram_size;
    bool has_chrram;
    uint32_t chrram_size;
    uint32_t eeprom_size;
    uint32_t chrnvram_size;

    int mirroring;
    bool has_battery;
    bool has_trainer;
    bool has_fourscreenmode;
    uint8_t region;
    uint8_t console_type;

    uint8_t cpu_ppu_timing;

    uint8_t vs_ppu_type;
    uint8_t vs_hw_type;

    uint8_t ext_console_type;

    //weird
    bool has_bus_conflicts;
    uint8_t misc_roms_num;
    uint8_t default_expansion_dev;

    void parseheader();

public:

    RomFile()
    {
        file = NULL;
        mapper = submapper = 0;
        prgrom_size = chrrom_size = 0;
    }

    ~RomFile()
    {
        close();
    }

    int open(char *name);
    void close();

    inline void read(size_t n, uint8_t *buf)
    {
        std::fread(buf, 1, n, file);
    }

    inline bool eof()
    {
        return std::feof(file);
    }

    void printinfo();
};

#endif
