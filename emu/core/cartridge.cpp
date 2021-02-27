#include <emu/core/cartridge.hpp>

#include <fmt/core.h>
#include <emu/core/memmap.hpp>

namespace Core {

enum {
    ERR_SUCCESS = 0,
    ERR_INVALID_FORMAT,
    ERR_NES20,
    ERR_INVALID_NAME,
};

/*
bool Cartridge::parseheader()
{
    auto parse_nes2_0 = [this]() {
        int shift;
        prgrom_size |= (header[9] & 0xF) << 8;
        chrrom_size |= (header[9] & 0xF0) << 8;
        mapper      |= (header[8] & 0xF) << 8;
        submapper   = header[8] & 0xF0;
        shift = header[10] & 0xF;
        if (shift == 0)
            has.prgram = false;
        else {
            has.prgram = true;
            prgram_size = 64 << shift;
        }
        shift = header[10] & 0xF0;
        eeprom_size = (shift == 0) ? 0 : 64 << shift;
        shift = header[11] & 0xF;
        if (shift == 0)
            has.chrram = false;
        else {
            has.chrram = true;
            chrram_size = 64 << shift;
        }
        shift = header[11] & 0xF0;
        chrnvram_size = (shift == 0) ? 0 : 64 << shift;
        cpu_ppu_timing =  header[12] & 3;
        vs_ppu_type = vs_hw_type = 0;
        if (console_type == CONSOLE_TYPE_VSSYSTEM) {
            vs_ppu_type = header[13] & 0xF;
            vs_hw_type = header[13] & 0xF0;
        } else if (console_type == 3)
            console_type = header[13] & 0xF;
        misc_roms_num = header[14] & 3;
            def_expansion_dev = header[15] & 0x3F;
    };
    return true;
}
*/

void Cartridge::open(std::string_view rompath)
{
    // open file
    if (!romfile.open(rompath, Util::File::Mode::READ)) {
        throw std::runtime_error("can't open ROM file");
        // errid = ERR_INVALID_NAME;
        // return false;
    }

    // parse header
    romfile.readb(header, HEADER_LEN);
    auto parse_common = [this]() {
        file_format = Format::INVALID;
        if (header[0] == 'N' && header[1] == 'E' && header[2] == 'S' && header[3] == 0x1A) {
            if ((header[7] & 0xC) == 0x8)
                throw std::runtime_error("NES 2.0 format is not yet supported");
            file_format = Format::INES;
        }
        if (file_format == Format::INVALID)
            throw std::runtime_error("invalid format: the file is not a real ROM");
        nt_mirroring        = header[6] & 1;
        has.battery         = header[6] & 2;
        has.trainer         = header[6] & 4;
        has.fourscreenmode  = header[6] & 8;
        console_type        = header[7] & 3;
        mapper              = ((header[6] & 0xF0) >> 4) | (header[7] & 0xF0);
    };
    auto parse_ines = [this]() {
        if (header[5] == 0)
            has.chrram = true;
        prgram_size = header[8];
        // the specification says this bit exists, but no emus make use of it
        //region = header[9] & 1;
        region = (header[10] & 3);
        has.prgram = header[10] & 0x10;
        has.bus_conflicts = header[10] & 0x20;
    };
    auto parse_nes2_0 = [this]() {
        // TODO
    };
    parse_common();
    file_format == Format::INES ? parse_ines() : parse_nes2_0();

    // copy trainer, prgrom, chrrom
    if (has.trainer)
        romfile.readb(trainer, TRAINER_LEN);
    if (!has.prgram) {
        prgrom.alloc(header[4]*16384);
        romfile.readb(prgrom.getmem(), prgrom.getsize());
        prgrom.lock();
    }
    if (!has.chrram) {
        chrrom.alloc(header[5]*8192);
        romfile.readb(chrrom.getmem(), chrrom.getsize());
        chrrom.lock();
    }
    // return true;
}

uint8 Cartridge::read_prgrom(uint16 addr)
{
    /* mapper defined function to convert addresses goes here */
    uint16 offset = addr - 0x8000;
    uint16 start = prgrom.getsize() - 0x8000;
    uint16 eff_addr = start + offset;
    return prgrom.read(eff_addr);
}

uint8 Cartridge::read_chrrom(uint16 addr)
{
    return chrrom.read(addr);
}

std::string Cartridge::getinfo() const
{
    return fmt::format(
        "{}: {}, mapper {}, {}x16k PRG ROM, {}x8k CHR ROM, {} PRG RAM, "
        "{} CHR RAM, {} EEPROM, {} CHR NVRAM, {}-Mirror{}{}{}",
        romfile.getfilename(),
        file_format == Format::INES ? "iNES" : "NES 2.0",
        mapper,
        prgrom.getsize(),
        chrrom.getsize(),
        has.prgram ? prgram_size : 0,
        has.chrram ? chrram_size : 0,
        eeprom_size,
        chrnvram_size,
        nt_mirroring == NT_HORZ ? 'H' : 'V',
        has.battery ? ", contains SRAM" : "",
        has.trainer ? ", contains Trainer, " : "",
        has.fourscreenmode ? ", has Four screen mode" : ""
    );
}

void Cartridge::attach_bus(Bus *cpu, Bus *ppu)
{
    cpubus = cpu;
    ppubus = ppu;
    cpubus->map(CARTRIDGE_START, 0x8000,
            [=] (uint16 addr) { return 0; },
            [=] (uint16 addr, uint8 data) { });
    cpubus->map(0x8000, CPUBUS_SIZE,
            [=] (uint16 addr) { return read_prgrom(addr); },
            [=] (uint16 addr, uint8 data) { /***********/ });
    ppubus->map(PT_START, NT_START,
            [=] (uint16 addr) { return read_chrrom(addr); },
            [=] (uint16 addr, uint8 data) { /***********/ });
}

} // namespace Core

