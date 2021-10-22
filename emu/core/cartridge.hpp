#pragma once

#include <optional>
#include <span>
#include <emu/core/const.hpp>
#include <emu/util/array.hpp>
#include <emu/util/mappedfile.hpp>
#include <emu/util/uint.hpp>

namespace Util { class File; }

namespace core {

namespace Cartridge {
    const int HEADER_SIZE = 16;
    const int TRAINER_SIZE = 512;

    enum class Format {
        iNES,
        NES_2_0,
    };

    enum class Console {
        NES, VsSystem, Playchoice, Extended
    };

    struct Data {
        std::string filename;
        Format format;
        Mirroring mirroring;
        u32 mapper;
        u32 chrram_size;
        Console console_type;
        std::span<u8> prgrom;
        std::span<u8> chrrom;
        std::span<u8> header;
        std::span<u8> trainer;

        struct {
            bool battery;
            bool trainer;
            bool chrram;
        } has;

        struct NES20Data {
            bool has_prgram;
            u32 prgram_size;
            u32 submapper;
            u32 prg_nvram_size;
            u32 chr_nvram_size;
            u2 timing_mode;
            u4 vs_ppu;
            u4 vs_hardware;
            u4 extended_console_type;
            u2 misc_roms;
            u6 default_expansion_device;
        };
        std::optional<NES20Data> nes20_data;

        std::string to_string() const;
    };
} // namespace Cartridge

std::optional<Cartridge::Data> parse_cartridge(io::MappedFile &romfile);

} // namespace core
