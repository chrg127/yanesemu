#pragma once

#include <optional>
#include <span>
#include <emu/core/const.hpp>
#include <emu/util/array.hpp>
#include <emu/util/mappedfile.hpp>
#include <emu/util/unsigned.hpp>

namespace Util { class File; }

namespace Core {

namespace Cartridge {
    const int HEADER_SIZE = 16;
    const int TRAINER_SIZE = 512;

    enum class Format {
        INES,
        NES20,
    };

    enum class Console {
        NES, VSSYSTEM, PLAYCHOICE, EXTENDED
    };

    struct Data {
        std::string filename;
        Format format;
        Mirroring mirroring;
        uint32 mapper;
        uint32 chrram_size;
        Console console_type;
        std::span<uint8> prgrom;
        std::span<uint8> chrrom;
        std::span<uint8> header;
        std::span<uint8> trainer;

        struct {
            bool battery;
            bool trainer;
            bool chrram;
        } has;

        struct NES20Data {
            bool has_prgram;
            uint32 prgram_size;
            uint32 submapper;
            uint32 prg_nvram_size;
            uint32 chr_nvram_size;
            uint2 timing_mode;
            uint4 vs_ppu;
            uint4 vs_hardware;
            uint4 extended_console_type;
            uint2 misc_roms;
            uint6 default_expansion_device;
        };
        std::optional<NES20Data> nes20_data;

        std::string to_string() const;
    };
} // namespace Cartridge

std::optional<Cartridge::Data> parse_cartridge(Util::MappedFile &romfile);

} // namespace Core
