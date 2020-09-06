#ifndef NESROM_H_INCLUDED
#define NESROM_H_INCLUDED

#include <cstdio>
#include <cstdint>

const int HEADER_LEN = 16;
const int TRAINER_LEN = 512;

namespace nesrom {


enum CpuTiming : int {
    RP2C02 = 0,
    RP2C07 = 1,
    UMC6527P = 3,
    MULTIPLE = 2,
};

enum NTMirror : int {
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

class ROM {
    FILE *file;
    char *fname;

    enum class Format {
        INVALID,
        INES,
        NES20,
    } fformat;

    uint8_t header[HEADER_LEN];
    uint8_t trainer[TRAINER_LEN];
    uint8_t *prgrom;
    uint8_t *chrrom;

    uint16_t mapper;
    uint8_t submapper;

    bool has_prgram;
    bool has_chrram;
    bool has_battery;
    bool has_trainer;
    bool has_fourscreenmode;

    uint32_t prgrom_size;
    uint32_t chrrom_size;
    uint32_t prgram_size;
    uint32_t chrram_size;
    uint32_t eeprom_size;
    uint32_t chrnvram_size;

    int nametab_mirroring;
    int region;
    int console_type;
    int cpu_ppu_timing;
    int vs_ppu_type;
    int vs_hw_type;

    //weird
    bool has_bus_conflicts;
    uint8_t misc_roms_num;
    uint8_t def_expansion_dev;
    
    // for debugging purposes
    int dbgmsg;

    int parseheader();
    void parse_ines();
    void parse_nes20();
    inline void read(size_t n, uint8_t *buf)
    {
        std::fread(buf, 1, n, file);
    }

public:

    ROM();
    ~ROM()
    {
        close();
    }

    int open(char * const name);
    void close();
    void printinfo();
    const char *get_errormsg();

    Format file_format()
    { return fformat; }
    uint16_t mappertype()
    { return mapper; }
    bool hasprgram()
    { return has_prgram; }
    bool haschrram()
    { return has_chrram; }
    uint8_t *get_prgrom()
    { return prgrom; }
    size_t get_prgrom_size()
    { return prgrom_size*16384; }
};

} //namespace nesrom

#endif
