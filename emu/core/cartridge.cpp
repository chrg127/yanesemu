#include "cartridge.hpp"

#include <fmt/core.h>
#include <emu/util/file.hpp>
#include <emu/util/bits.hpp>

using Util::getbits;
using Util::getbit;
using Util::bitmask;

namespace Core {

std::string Cartridge::Data::to_string() const
{
    return fmt::format(
        "{}: {}, mapper {}, {}x16k PRG ROM, {}x8k CHR ROM, "
        "{} CHR RAM, {}-Mirror{}{}",
        filename,
        format == Format::INES ? "iNES" : "NES 2.0",
        mapper,
        header[4],
        header[5],
        has.chrram ? chrram_size : 0,
        mirroring == Mirroring::HORZ ? 'H' : mirroring == Mirroring::VERT ? 'V' : 'O',
        has.battery ? ", contains SRAM" : "",
        has.trainer ? ", contains Trainer" : ""
    );
}

std::optional<Cartridge::Data> parse_cartridge(Util::File &romfile)
{
    static const uint8 constants[] = { 'N', 'E', 'S', 0x1A };
    static_assert(sizeof(constants) == 4);

    Cartridge::Data cart;
    romfile.bread(cart.header, Cartridge::HEADER_SIZE);
    if (std::memcmp(cart.header, constants, sizeof(constants)) != 0)
        return std::nullopt;
    cart.filename = romfile.filename();
    cart.format = (getbits(cart.header[7], 2, 2) == 2)
                ? Cartridge::Format::NES20
                : Cartridge::Format::INES;
    uint32 prgrom_size = cart.header[4];
    uint32 chrrom_size = cart.header[5];
    cart.mirroring = getbit(cart.header[6], 3) ? Mirroring::FOUR_SCREEN
                   : getbit(cart.header[6], 0) ? Mirroring::VERT
                   :                             Mirroring::HORZ;
    cart.mapper = (cart.header[7] & 0b11110000) | getbits(cart.header[6], 4, 4);
    cart.has.battery = getbit(cart.header[6], 1);
    cart.has.trainer = getbit(cart.header[6], 2);
    cart.has.chrram = cart.header[5] == 0;

    const auto detect_console = [](uint2 bits)
    {
        switch (bits) {
        case 0: return Cartridge::Console::NES;
        case 1: return Cartridge::Console::VSSYSTEM;
        case 2: return Cartridge::Console::PLAYCHOICE;
        default: return Cartridge::Console::EXTENDED;
        }
    };
    cart.console_type = detect_console(getbits(cart.header[7], 0, 2));

    // detection of PRG RAM and CHR RAM
    // these are weird. here's all known interaction:
    // - there is always a PRG ROM. there may be a PRG RAM.
    // - there is either a CHR ROM or a CHR RAM. CHR RAM exists only if
    //   CHR ROM size is specified to be 0.
    // - in some uncommon cases there may be both CHR ROM and CHR RAM.

    if (cart.format == Cartridge::Format::INES) {
        cart.chrram_size = cart.has.chrram ? Util::to_kib(8) : 0;
        cart.nes20_data = std::nullopt;
    } else {
        Cartridge::Data::NES20Data data;
        cart.mapper         = (cart.header[8] & bitmask(4)) << 8 | cart.mapper;
        data.submapper      = getbits(cart.header[8], 4, 4);
        prgrom_size         = getbits(cart.header[9], 0, 4) << 8 | prgrom_size;
        chrrom_size         = getbits(cart.header[9], 4, 4) << 8 | chrrom_size;
        data.prgram_size    = 64 << getbits(cart.header[10], 0, 4);
        data.prg_nvram_size = 64 << getbits(cart.header[10], 4, 4);
        cart.chrram_size    = 64 << getbits(cart.header[11], 0, 4);
        data.chr_nvram_size = 64 << getbits(cart.header[11], 4, 4);
        data.timing_mode    = getbits(cart.header[12], 0, 2);
        if (cart.console_type == Cartridge::Console::VSSYSTEM) {
            data.vs_ppu = getbits(cart.header[13], 0, 4);
            data.vs_hardware = getbits(cart.header[13], 4, 4);
        } else if (cart.console_type == Cartridge::Console::VSSYSTEM) {
            data.extended_console_type = getbits(cart.header[13], 0, 4);
        }
        data.misc_roms = getbits(cart.header[14], 0, 2);
        data.default_expansion_device = getbits(cart.header[14], 0, 6);

        cart.nes20_data = data;
    }

    if (cart.has.trainer)
        romfile.bread(cart.trainer, Cartridge::TRAINER_SIZE);
    cart.prgrom = romfile.read_bytes(prgrom_size * Util::to_kib(16));
    if (!cart.has.chrram)
        cart.chrrom = romfile.read_bytes(chrrom_size * Util::to_kib(8));
    return cart;
}

} // namespace Core
