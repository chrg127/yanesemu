#include <emu/io/nesrom.hpp>

#include <emu/io/file.hpp>
#define DEBUG
#include <emu/utils/debug.hpp>

namespace IO {

enum ErrID : int {
    ERRID_SUCCESS = 0, ERRID_INVFORMAT, ERRID_NES20, ERRID_INVNAME,
};

/* NOTE: private functions */
bool ROM::parseheader()
{
    readb(header, HEADER_LEN);
    fformat = Format::INVALID;
    if (header[0] == 'N' && header[1] == 'E' && header[2] == 'S' && header[3] == 0x1A)
        fformat = Format::INES;
    if (fformat == Format::INES && (header[7] & 0xC) == 0x8) {
        //fformat = Format::NES20;
        errid = ERRID_NES20;
        return false;
    }
    if (fformat == Format::INVALID) {
        errid = ERRID_INVFORMAT;
        return false;
    }

    prgrom_size = header[4];
    chrrom_size = header[5];

    nametab_mirroring   = header[6] & 1;
    has_battery         = header[6] & 2;
    has_trainer         = header[6] & 4;
    has_fourscreenmode  = header[6] & 8;

    console_type = header[7] & 3;

    mapper = (header[6] & 0xF0) >> 4;
    mapper |= (header[7] & 0xF0);

    if (fformat == Format::INES)
        parse_ines();
    else if (fformat == Format::NES20)
        parse_nes20();
    return true;
}

void ROM::parse_ines()
{
    if (chrrom_size == 0)
        has_chrram = true;
    prgram_size = header[8];
    // the specification says this bit exists, but no emus make use of it
    //region = header[9] & 1;
    region = (header[10] & 3);
    has_prgram = header[10] & 0x10;
    has_bus_conflicts = header[10] & 0x20;
}

void ROM::parse_nes20()
{
    int shift;

    prgrom_size |= (header[9] & 0xF) << 8;
    chrrom_size |= (header[9] & 0xF0) << 8;
    mapper      |= (header[8] & 0xF) << 8;
    submapper   = header[8] & 0xF0;

    shift = header[10] & 0xF;
    if (shift == 0)
        has_prgram = false;
    else {
        has_prgram = true;
        prgram_size = 64 << shift;
    }

    shift = header[10] & 0xF0;
    eeprom_size = (shift == 0) ? 0 : 64 << shift;

    shift = header[11] & 0xF;
    if (shift == 0)
        has_chrram = false;
    else {
        has_chrram = true;
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
}

bool ROM::open(const std::string &s)
{
    if (!File::open(s, Mode::READ)) {
        errid = ERRID_INVNAME;
        return false;
    }
    if (!parseheader())
        return false;
    // get trainer
    if (has_trainer)
        readb(trainer, TRAINER_LEN);
    // allocate program ROM and character ROM
    if (!has_prgram) {
        prgrom = new uint8_t[prgrom_size*16384];
        readb(prgrom, prgrom_size*16384);
    }
    if (!has_chrram) {
        chrrom = new uint8_t[chrrom_size*8192];
        readb(chrrom, chrrom_size*8192);
    }

    return true;
}

void ROM::printinfo(File &lf)
{
    if (!lf.isopen())
        return;
    lf.printf("%s: ", filename.c_str());
    if (fformat == Format::INES)
        lf.printf("iNES");
    else
        lf.printf("NES 2.0");
    lf.printf(", mapper %d, %dx16k PRG ROM, %dx8k CHR ROM",
            mapper, prgrom_size, chrrom_size);
    if (has_prgram)
        lf.printf(", %d PRG RAM", prgram_size);
    if (has_chrram)
        lf.printf(", %d CHR RAM", chrram_size);
    if (eeprom_size != 0)
        lf.printf(", %d EEPROM", eeprom_size);
    if (chrnvram_size != 0)
        lf.printf(", %d CHR NVRAM", chrnvram_size);

    if (nametab_mirroring == NAMETAB_HORZ)
        lf.printf(", H-Mirror");
    else
        lf.printf(", V-Mirror");
    if (has_battery)
        lf.printf(", SRAM enabled");
    if (has_trainer)
        lf.printf(", Trainer enabled");
    if (has_fourscreenmode)
        lf.printf(", Four screen mode enabled");
    lf.putc('\n');
}

std::string_view ROM::geterr()
{
    static std::string_view rom_errmsg[] = {
        "no errors", "invalid NES format", "NES 2.0 not yet supported", "can't open rom file",
    };
    return rom_errmsg[errid];
}

} // namespace nesrom

