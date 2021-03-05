#ifndef CORE_CARTRIDGE_HPP_INCLUDED
#define CORE_CARTRIDGE_HPP_INCLUDED

#include <string_view>
#include <emu/core/const.hpp>
#include <emu/util/unsigned.hpp>
#include <emu/util/heaparray.hpp>

namespace Util { class File; }

namespace Core {

class Bus;

/*
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
*/

class Cartridge {
    static const int HEADER_LEN = 16;
    static const int TRAINER_LEN = 512;

    enum class Format {
        INVALID,
        INES,
        NES20,
    } file_format = Format::INVALID;

    std::string name;
    Util::HeapArray<uint8> prgrom;
    Util::HeapArray<uint8> chrrom;
    uint8 header[HEADER_LEN];
    uint8 trainer[TRAINER_LEN];
    uint16 mapper = 0;
    uint8 submapper = 0;
    uint32 prgram_size = 0;
    uint32 chrram_size = 0;
    Mirroring nt_mirroring = Mirroring::VERT;

    struct {
        bool prgram  = false;
        bool chrram  = false;
        bool battery = false;
        bool trainer = false;
    } has;

    /*
    uint32 eeprom_size   = 0;
    uint32 chrnvram_size = 0;
    int region           = REGION_NTSC;
    int console_type     = CONSOLE_TYPE_NES;
    int cpu_ppu_timing   = CPUTIMING_RP2C02;
    int vs_ppu_type      = VSPPU_RP2C03B;
    int vs_hw_type       = VSHW_UNISYS_NORMAL;
    uint8 misc_roms_num       = 0;
    uint8 def_expansion_dev   = 0;
    */
    //bool fourscreenmode = false;
    //bool bus_conflicts  = false;

public:
    void parse(Util::File &romfile);
    uint8 read_prgrom(uint16 addr);
    uint8 read_chrrom(uint16 addr);
    void attach_bus(Bus *cpubus, Bus *ppubus);
    std::string getinfo() const;

    uint16 mappertype() const   { return mapper; }
    bool hasprgram() const      { return has.prgram; }
    bool haschrram() const      { return has.chrram; }
    Mirroring mirroring() const { return nt_mirroring; }
};

} // namespace Core

#endif
