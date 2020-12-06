#include <emu/core/cartridge.hpp>

namespace Core {

enum {
    ERRID_SUCCESS = 0,
    ERRID_INVFORMAT,
    ERRID_NES20,
    ERRID_INVNAME,
};

bool Cartridge::parseheader()
{
    auto parse_ines = [=]() {
        if (header[5] == 0)
            has.chrram = true;
        prgram_size = header[8];
        // the specification says this bit exists,
        // but no emus make use of it
        //region = header[9] & 1;
        region = (header[10] & 3);
        has.prgram = header[10] & 0x10;
        has.bus_conflicts = header[10] & 0x20;
    };
    auto parse_nes2_0 = [=]() {
        // int shift;

        // prgrom_size |= (header[9] & 0xF) << 8;
        // chrrom_size |= (header[9] & 0xF0) << 8;
        // mapper      |= (header[8] & 0xF) << 8;
        // submapper   = header[8] & 0xF0;

        // shift = header[10] & 0xF;
        // if (shift == 0)
        //     has.prgram = false;
        // else {
        //     has.prgram = true;
        //     prgram_size = 64 << shift;
        // }

        // shift = header[10] & 0xF0;
        // eeprom_size = (shift == 0) ? 0 : 64 << shift;

        // shift = header[11] & 0xF;
        // if (shift == 0)
        //     has.chrram = false;
        // else {
        //     has.chrram = true;
        //     chrram_size = 64 << shift;
        // }
        // shift = header[11] & 0xF0;
        // chrnvram_size = (shift == 0) ? 0 : 64 << shift;

        // cpu_ppu_timing =  header[12] & 3;

        // vs_ppu_type = vs_hw_type = 0;
        // if (console_type == CONSOLE_TYPE_VSSYSTEM) {
        //     vs_ppu_type = header[13] & 0xF;
        //     vs_hw_type = header[13] & 0xF0;
        // } else if (console_type == 3)
        //     console_type = header[13] & 0xF;

        // misc_roms_num = header[14] & 3;

        //     def_expansion_dev = header[15] & 0x3F;
    };
    fformat = Format::INVALID;
    if (header[0] == 'N' && header[1] == 'E' && header[2] == 'S'
            && header[3] == 0x1A)
        fformat = Format::INES;
    if (fformat == Format::INES && (header[7] & 0xC) == 0x8) {
        errid = ERRID_NES20;
        return false;
    }
    if (fformat == Format::INVALID) {
        errid = ERRID_INVFORMAT;
        return false;
    }

    nt_mirroring        = header[6] & 1;
    has.battery         = header[6] & 2;
    has.trainer         = header[6] & 4;
    has.fourscreenmode  = header[6] & 8;

    console_type = header[7] & 3;

    mapper = (header[6] & 0xF0) >> 4;
    mapper |= (header[7] & 0xF0);

    // if (fformat == Format::INES)
    parse_ines();
    // else if (fformat == Format::NES20)
    //     parse_nes20();
    return true;
}

bool Cartridge::open(std::string_view s)
{
    if (!romfile.open(s, Util::File::Mode::READ)) {
        errid = ERRID_INVNAME;
        return false;
    }
    romfile.readb(header, HEADER_LEN);
    if (!parseheader())
        return false;
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
    return true;
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

void Cartridge::printinfo(Util::File &log) const
{
    if (!log.isopen())
        return;
    log.printf("%s: ", romfile.getfilename().c_str());
    if (fformat == Format::INES)
        log.printf("iNES");
    else
        log.printf("NES 2.0");
    log.printf(", mapper %d, %dx16k PRG ROM, %dx8k CHR ROM",
            mapper, prgrom.getsize(), chrrom.getsize());
    if (has.prgram)
        log.printf(", %d PRG RAM", prgram_size);
    if (has.chrram)
        log.printf(", %d CHR RAM", chrram_size);
    if (eeprom_size != 0)
        log.printf(", %d EEPROM", eeprom_size);
    if (chrnvram_size != 0)
        log.printf(", %d CHR NVRAM", chrnvram_size);

    log.printf(nt_mirroring == NT_HORZ ? ", H-Mirror" : ", V-Mirror");
    if (has.battery)
        log.printf(", SRAM enabled");
    if (has.trainer)
        log.printf(", Trainer enabled");
    if (has.fourscreenmode)
        log.printf(", Four screen mode enabled");
    log.putc('\n');
}

std::string_view Cartridge::geterr() const
{
    static std::string_view rom_errmsg[] = {
        "no errors",
        "invalid NES format",
        "NES 2.0 not yet supported",
        "can't open rom file",
    };
    return rom_errmsg[errid];
}

} // namespace Core

