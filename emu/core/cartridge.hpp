#ifndef CORE_CARTRIDGE_HPP_INCLUDED
#define CORE_CARTRIDGE_HPP_INCLUDED

#include <string_view>
#include <emu/core/const.hpp>
#include <emu/util/unsigned.hpp>
#include <emu/util/heaparray.hpp>

namespace Util { class File; }

namespace Core {

class Bus;

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
