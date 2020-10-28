#ifndef CARTRIDGE_HPP_INCLUDED
#define CARTRIDGE_HPP_INCLUDED

#include <cstdint>
#include <string>
#include <string_view>
#include <emu/utils/file.hpp>
#include <emu/core/rom.hpp>

namespace Core {

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
    VSHW_UNISYS_TKO, VSHW_UNISYS_XEVIOUS,
    VSHW_UNISYS_ICECLIMBER,
    VSHW_DUALSYS_NORMAL,
    VSHW_DUALSYS_RAID,
};

class Cartridge {
    IO::File romfile;
    int errid = 0;
    enum class Format {
        INVALID,
        INES,
        NES20,
    } fformat = Format::INVALID;

    ROM prgrom;
    ROM chrrom;
    static const int HEADER_LEN = 16;
    static const int TRAINER_LEN = 512;
    uint8_t header[HEADER_LEN];
    uint8_t trainer[TRAINER_LEN];
    uint16_t mapper   = 0;
    uint8_t submapper = 0;

    uint32_t prgram_size    = 0;
    uint32_t chrram_size    = 0;
    uint32_t eeprom_size    = 0;
    uint32_t chrnvram_size  = 0;
    int nametab_mirroring   = NAMETAB_HORZ;
    int region              = REGION_NTSC;
    int console_type        = CONSOLE_TYPE_NES;
    int cpu_ppu_timing      = CPUTIMING_RP2C02;
    int vs_ppu_type         = VSPPU_RP2C03B;
    int vs_hw_type          = VSHW_UNISYS_NORMAL;
    uint8_t misc_roms_num       = 0;
    uint8_t def_expansion_dev   = 0;

    struct {
        bool prgram         = false;
        bool chrram         = false;
        bool battery        = false;
        bool trainer        = false;
        bool fourscreenmode = false;
        bool bus_conflicts  = false;
    } has;

    bool parseheader();
    void parse_ines();
    void parse_nes20();

public:
    ~Cartridge()
    { }

    bool open(std::string_view s);
    void printinfo(IO::File &f);
    std::string_view geterr();

    Format file_format() const
    { return fformat; }
    ROM &get_prgrom()
    { return prgrom; }
    ROM &get_chrrom()
    { return chrrom; }
    uint16_t mappertype() const
    { return mapper; }
    bool hasprgram() const
    { return has.prgram; }
    bool haschrram() const
    { return has.chrram; }
};

} // namespace Core

#endif
