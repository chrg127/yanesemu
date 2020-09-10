#ifndef NESROM_H_INCLUDED
#define NESROM_H_INCLUDED

#include <cstdio>
#include <cstdint>
#include <string>

const int HEADER_LEN = 16;
const int TRAINER_LEN = 512;

namespace nesrom {

enum NametabMirror : int {
    NAMETAB_HORZ = 0,
    NAMETAB_VERT,
};

enum Region : int {
    REGION_NTSC = 0,
    REGION_PAL,
};

enum ConsoleType : int {
    CONSOLE_TYPE_NES = 0,
    CONSOLE_TYPE_VSSYSTEM,
    CONSOLE_TYPE_PLAYCHOICE,
    CONSOLE_TYPE_FAMICLONE_DECMODE,
    CONSOLE_TYPE_VRTECH_VT01_MONOCHR,
    CONSOLE_TYPE_VRTECH_VT01_REDCYAN,
    CONSOLE_TYPE_VRTECH_VT02,
    CONSOLE_TYPE_VRTECH_VT03,
    CONSOLE_TYPE_VRTECH_VT09,
    CONSOLE_TYPE_VRTECH_VT32,
    CONSOLE_TYPE_VRTECH_VT369,
    CONSOLE_TYPE_UMC,
};

enum CpuTiming : int {
    CPUTIMING_RP2C02 = 0,
    CPUTIMING_RP2C07,
    CPUTIMING_UMC6527P,
    MULTIPLE,
};

enum VsPPU : int {
    VSPPU_RP2C03B = 0,
    VSPPU_RP2C03G,
    VSPPU_RP2C040001,
    VSPPU_RP2C040002,
    VSPPU_RP2C040003,
    VSPPU_RP2C040004,
    VSPPU_RC2C03B,
    VSPPU_RC2C03C,
    VSPPU_RC2C0501,
    VSPPU_RC2C0502,
    VSPPU_RC2C0503,
    VSPPU_RC2C0504,
    VSPPU_RC2C0505,
};

enum VsHardware : int {
    VSHW_UNISYS_NORMAL = 0,
    VSHW_UNISYS_RBI,
    VSHW_UNISYS_TKO,
    VSHW_UNISYS_XEVIOUS,
    VSHW_UNISYS_ICECLIMBER,
    VSHW_DUALSYS_NORMAL,
    VSHW_DUALSYS_RAID,
};

class ROM {
    FILE *romfile = nullptr;
    std::string filename;
    int debugmsg = 0;

    enum class Format {
        INVALID,
        INES,
        NES20,
    } fformat = Format::INVALID;

    uint8_t header[HEADER_LEN];
    uint8_t trainer[TRAINER_LEN];

    uint16_t mapper   = 0;
    uint8_t submapper = 0;

    uint8_t *prgrom = nullptr;
    uint32_t prgrom_size    = 0;
    uint8_t *chrrom = nullptr;
    uint32_t chrrom_size    = 0;

    bool has_prgram         = false;
    uint32_t prgram_size    = 0;

    bool has_chrram         = false;
    uint32_t chrram_size    = 0;

    bool has_battery        = false;
    bool has_trainer        = false;
    bool has_fourscreenmode = false;

    uint32_t eeprom_size    = 0;
    uint32_t chrnvram_size  = 0;

    int nametab_mirroring   = NAMETAB_HORZ;
    int region              = REGION_NTSC;
    int console_type        = CONSOLE_TYPE_NES;
    int cpu_ppu_timing      = CPUTIMING_RP2C02;
    int vs_ppu_type         = VSPPU_RP2C03B;
    int vs_hw_type          = VSHW_UNISYS_NORMAL;

    //weird
    bool has_bus_conflicts      = false;
    uint8_t misc_roms_num       = 0;
    uint8_t def_expansion_dev   = 0;

    bool parseheader();
    void parse_ines();
    void parse_nes20();
    inline void read(size_t n, uint8_t *buf)
    {
        std::fread(buf, 1, n, romfile);
    }

public:

    ~ROM()
    {
        close();
    }

    bool open(const std::string &name);
    void close();
    void printinfo(FILE *logfile);
    std::string &geterr();

    Format file_format() const
    { return fformat; }
    uint16_t mappertype() const
    { return mapper; }
    bool hasprgram() const
    { return has_prgram; }
    bool haschrram() const
    { return has_chrram; }
    uint8_t *get_prgrom() const
    { return prgrom; }
    size_t get_prgrom_size() const
    { return prgrom_size*16384; }
};

} //namespace nesrom

#endif
