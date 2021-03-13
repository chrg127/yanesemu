#include <emu/core/cartridge.hpp>

#include <cassert>
#include <fmt/core.h>
#include <emu/core/bus.hpp>
#include <emu/util/file.hpp>
#include <emu/util/debug.hpp>

namespace Core {

enum class Format {
    INVALID, INES, NES20,
};

bool Cartridge::parse(Util::File &romfile)
{
    if (!romfile)
        return false;
    name = romfile.filename();

    romfile.bread(header, HEADER_LEN);

    Format file_format = Format::INVALID;
    if (header[0] == 'N' && header[1] == 'E' && header[2] == 'S' && header[3] == 0x1A) {
        if ((header[7] & 0xC) == 0x8)
            return false; // NES 2.0 is not supported yet
        file_format = Format::INES;
        format = "iNES";
    }
    if (file_format == Format::INVALID)
        return false;

    auto parse_common = [this]() {
        nt_mirroring        = (header[6] & 1) == 0 ? Mirroring::HORZ : Mirroring::VERT;
        has.battery         = header[6] & 2;
        has.trainer         = header[6] & 4;
        // has.fourscreenmode  = header[6] & 8;
        // console_type        = header[7] & 3;
        mapper              = ((header[6] & 0xF0) >> 4) | (header[7] & 0xF0);
    };
    auto parse_ines = [this]() {
        if (header[5] == 0)
            has.chrram = true;
        prgram_size = header[8];
        // region = header[9] & 1;
        // region = (header[10] & 3);
        has.prgram = header[10] & 0x10;
        // has.bus_conflicts = header[10] & 0x20;
    };
    auto parse_nes20 = [this]() {
        panic("NES 2.0 isn't supported, this shouldn't be called\n");
    };
    parse_common();
    file_format == Format::INES ? parse_ines() : parse_nes20();

    if (has.trainer)
        romfile.bread(trainer, TRAINER_LEN);
    if (!has.prgram) {
        prgrom.reset(header[4]*16384);
        romfile.bread(prgrom.data(), prgrom.size());
    }
    if (!has.chrram) {
        chrrom.reset(header[5]*8192);
        romfile.bread(chrrom.data(), chrrom.size());
    }
    return true;
}

uint8 Cartridge::read_prgrom(uint16 addr)
{
    /* mapper defined function to convert addresses goes here */
    uint16 offset = addr - 0x8000;
    uint16 start = prgrom.size() - 0x8000;
    uint16 eff_addr = start + offset;
    return prgrom[eff_addr];
}

uint8 Cartridge::read_chrrom(uint16 addr)
{
    return chrrom[addr];
}

std::string Cartridge::getinfo() const
{
    return fmt::format(
        "{}: {}, mapper {}, {}x16k PRG ROM, {}x8k CHR ROM, {} PRG RAM, "
        "{} CHR RAM, {}-Mirror{}{}",
        name, format, mapper,
        header[4], header[5],
        // chrrom.size(),
        has.prgram ? prgram_size : 0,
        has.chrram ? chrram_size : 0,
        nt_mirroring == Mirroring::HORZ ? 'H' : 'V',
        has.battery ? ", contains SRAM" : "",
        has.trainer ? ", contains Trainer" : ""
    );
}

void Cartridge::attach_bus(Bus *rambus, Bus *vrambus)
{
    rambus->map(CARTRIDGE_START, 0x8000,
            [this] (uint16 addr) { return 0; },
            [this] (uint16 addr, uint8 data) { /***********/ });
    rambus->map(0x8000, CPUBUS_SIZE,
            [this] (uint16 addr) { return read_prgrom(addr); },
            [this] (uint16 addr, uint8 data) { /***********/ });
    vrambus->map(PT_START, NT_START,
            [this] (uint16 addr) { return read_chrrom(addr); },
            [this] (uint16 addr, uint8 data) { /***********/ });
}

} // namespace Core

