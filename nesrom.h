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

enum NametabMirroring : int {
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
    FAMICLONE_DECMODE,
    VRTECH_VT01_MONOCHR,
    VRTECH_VT01_REDCYAN,
    VRTECH_VT02,
    VRTECH_VT03,
    VRTECH_VT09,
    VRTECH_VT32,
    VRTECH_VT369,
    UMC,
};

enum CPUTiming : int {
    RP2C02 = 0,
    RP2C07 = 1,
    UMC6527P = 3,
    MULTIPLE = 2,
};

enum VsPPU : int {
    RP2C03B = 0,
    RP2C03G,
    RP2C040001,
    RP2C040002,
    RP2C040003,
    RP2C040004,
    RC2C03B,
    RC2C03C,
    RC2C0501,
    RC2C0502,
    RC2C0503,
    RC2C0504,
    RC2C0505,
};

enum VsHardware : int {
    UNISYS_NORMAL = 0,
    UNISYS_RBI,
    UNISYS_TKO,
    UNISYS_XEVIOUS,
    UNISYS_ICECLIMBER,
    DUALSYS_NORMAL,
    DUALSYS_RAID,
};

class RomFile {
    FILE *file;
    char *fname;

    uint8_t header[HEADER_LEN];
    uint8_t trainer[TRAINER_LEN];
    uint8_t *prgrom;
    uint8_t *chrrom;

    NesFmt fformat;
    uint16_t mapper;
    uint8_t submapper;

    bool has_prgram;
    bool has_chrram;
    uint32_t prgrom_size;
    uint32_t chrrom_size;
    uint32_t prgram_size;
    uint32_t chrram_size;
    uint32_t eeprom_size;
    uint32_t chrnvram_size;

    uint8_t nametab_mirroring;
    bool has_battery;
    bool has_trainer;
    bool has_fourscreenmode;
    uint8_t region;
    uint8_t console_type;

    uint8_t cpu_ppu_timing;

    uint8_t vs_ppu_type;
    uint8_t vs_hw_type;

    //weird
    bool has_bus_conflicts;
    uint8_t misc_roms_num;
    uint8_t def_expansion_dev;

    void parseheader();

public:

    RomFile();

    ~RomFile()
    {
        close();
    }


    NesFmt file_format() { return fformat; }
    uint16_t mappertype() { return mapper; }
    bool hasprgram() { return has_prgram; }
    bool haschrram() { return has_chrram; }
    uint8_t *get_prgrom() { return prgrom; }

    int open(char * const name);
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
