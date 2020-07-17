#include "nesrom.h"
#include <cstdio>

RomFile::RomFile()
{
    file = NULL;
    fformat = NesFmt::INVALID;

    // set all of these to 0, just to be secure.
    mapper = submapper = 0;
    prgrom_size = chrrom_size = prgram_size = chrram_size = eeprom_size = chrnvram_size = 0;
    mapper = submapper = 0;
    nametab_mirroring = HORZ;
    has_battery = has_trainer = has_fourscreenmode = false;
    region = NTSC;
    console_type = NES;
    cpu_ppu_timing = 0;
    vs_ppu_type = vs_hw_type = 0;
    has_bus_conflicts = false;
    misc_roms_num = 0;
    def_expansion_dev = 0;
}

void RomFile::parseheader()
{
    int shift;

    read(HEADER_LEN, header);
    fformat = NesFmt::INVALID;
    if (header[0] == 'N' && header[1] == 'E' && header[2] == 'S' && header[3] == 0x1A)
        fformat = NesFmt::INES;
    if (fformat == NesFmt::INES && (header[7] & 0xC) == 0x8)
        fformat = NesFmt::NES20;

    prgrom_size = header[4];
    chrrom_size = header[5];

    nametab_mirroring           = header[6] & 1;
    has_battery         = header[6] & 2;
    has_trainer         = header[6] & 4;
    has_fourscreenmode  = header[6] & 8;

    console_type = header[7] & 3;

    mapper = (header[6] & 0xF0) >> 4;
    mapper |= (header[7] & 0xF0);

    if (fformat == NesFmt::INES) {
        if (chrrom_size == 0)
            has_chrram = true;

        prgram_size = header[8];
        // the specification says this bit exists, but no emus make use of it
        //region = header[9] & 1;
        region = (header[10] & 3);
        has_prgram = header[10] & 0x10;
        has_bus_conflicts = header[10] & 0x20;

    } else if (fformat == NesFmt::NES20) {
        prgrom_size |= (header[9] & 0xF) << 8;
        chrrom_size |= (header[9] & 0xF0) << 8;
        mapper |= (header[8] & 0xF) << 8;
        submapper = header[8] & 0xF0;

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
        if (console_type == VSSYSTEM) {
            vs_ppu_type = header[13] & 0xF;
            vs_hw_type = header[13] & 0xF0;
        } else if (console_type == 3)
            console_type = header[13] & 0xF;

        misc_roms_num = header[14] & 3;

        def_expansion_dev = header[15] & 0x3F;
    }
}

int RomFile::open(char * const name)
{
    fname = name;
    file = std::fopen(name, "rb");
    if (!file)
        return 1;
    parseheader();
    // get trainer
    if (has_trainer)
        read(TRAINER_LEN, trainer);
    // allocate program ROM and character ROM
    if (!has_prgram) {
        prgrom = new uint8_t[prgrom_size*16384];
        read(prgrom_size*16384, prgrom);
    } else {
        prgrom = nullptr;
    }
    if (!has_chrram) {
        chrrom = new uint8_t[chrrom_size*8192];
        read(chrrom_size*8192, chrrom);
    } else {
        chrrom = nullptr;
    }

    return 0;
}

void RomFile::close()
{
    if (file)
        fclose(file);
    if (prgrom)
        delete[] prgrom;
    if (chrrom)
        delete[] chrrom;
}

void RomFile::printinfo()
{
    std::printf("%s: ", fname);
    if (fformat == INES)
        std::printf("iNES");
    else
        std::printf("NES 2.0");
    std::printf(", %dx16k PRG ROM, %dx8k CHR ROM", prgrom_size, chrrom_size);
    if (has_prgram)
        std::printf(", %d PRG RAM", prgram_size);
    if (has_chrram)
        std::printf(", %d CHR RAM", chrram_size);
    if (eeprom_size != 0)
        std::printf(", %d EEPROM", eeprom_size);
    if (chrnvram_size != 0)
        std::printf(", %d CHR NVRAM", chrnvram_size);

    if (nametab_mirroring == HORZ)
        std::printf(", H-Mirroring");
    else
        std::printf(", Vertical mirroring");
    if (has_battery)
        std::printf(", SRAM enabled");
    if (has_trainer)
        std::printf(", Trainer enabled");
    if (has_fourscreenmode)
        std::printf(", Four screen mode enabled");

    std::puts("");
}

