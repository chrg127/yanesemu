#include "nesrom.h"
#include <cstdio>

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

    mirroring           = header[6] & 1;
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

        // all the stuff only specified in NES 2.0: set to zero.
        cpu_ppu_timing = 0;
        vs_hw_type = vs_ppu_type = 0;
        ext_console_type = 0;
        misc_roms_num = default_expansion_dev = 0;

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
        } else if (console_type == EXTTYPE)
            ext_console_type = header[13] & 0xF;

        misc_roms_num = header[14] & 3;

        default_expansion_dev = header[15] & 0x3F;
    }
}

int RomFile::open(char *name)
{
    file = std::fopen(name, "rb");
    if (!file)
        return 1;
    parseheader();
    return 0;
}

void RomFile::close()
{
    if (file)
        fclose(file);
}

void RomFile::printinfo()
{
    if (fformat == INES)
        std::printf("format = iNES\n");
    else if (fformat == NES20)
        std::printf("format = NES2.0\n");
    else
        std::printf("format = invalid\n");

}

