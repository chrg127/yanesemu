#include <emu/core/cartridge.hpp>

#include <fmt/core.h>
#include <emu/core/bus.hpp>
#include <emu/util/file.hpp>

namespace Core {

void Cartridge::parse(Util::File &romfile) //std::string_view rompath)
{
    if (!romfile)
        throw std::runtime_error("rom file should be already open");
    name = romfile.getfilename();

    romfile.readb(header, HEADER_LEN);
    auto parse_common = [this]() {
        file_format = Format::INVALID;
        if (header[0] == 'N' && header[1] == 'E' && header[2] == 'S' && header[3] == 0x1A) {
            if ((header[7] & 0xC) == 0x8)
                throw std::runtime_error("NES 2.0 format is not yet supported");
            file_format = Format::INES;
        }
        if (file_format == Format::INVALID)
            throw std::runtime_error("invalid ROM header");
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
        // TODO
    };
    parse_common();
    file_format == Format::INES ? parse_ines() : parse_nes20();

    if (has.trainer)
        romfile.readb(trainer, TRAINER_LEN);
    if (!has.prgram) {
        prgrom.reset(header[4]*16384);
        romfile.readb(prgrom.data(), prgrom.size());
    }
    if (!has.chrram) {
        chrrom.reset(header[5]*8192);
        romfile.readb(chrrom.data(), chrrom.size());
    }
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
        name,
        file_format == Format::INES ? "iNES" : "NES 2.0",
        mapper,
        prgrom.size(),
        chrrom.size(),
        has.prgram ? prgram_size : 0,
        has.chrram ? chrram_size : 0,
        // eeprom_size,
        // chrnvram_size,
        nt_mirroring == Mirroring::HORZ ? 'H' : 'V',
        has.battery ? ", contains SRAM" : "",
        has.trainer ? ", contains Trainer, " : ""
        // has.fourscreenmode ? ", has Four screen mode" : ""
    );
}

void Cartridge::attach_bus(Bus *cpubus, Bus *ppubus)
{
    cpubus->map(CARTRIDGE_START, 0x8000,
            [=] (uint16 addr) { return 0; },
            [=] (uint16 addr, uint8 data) { /***********/ });
    cpubus->map(0x8000, CPUBUS_SIZE,
            [=] (uint16 addr) { return read_prgrom(addr); },
            [=] (uint16 addr, uint8 data) { /***********/ });
    ppubus->map(PT_START, NT_START,
            [=] (uint16 addr) { return read_chrrom(addr); },
            [=] (uint16 addr, uint8 data) { /***********/ });
}

} // namespace Core

