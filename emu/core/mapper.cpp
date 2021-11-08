#include "mapper.hpp"

#include <emu/core/emulator.hpp>
#include <emu/core/const.hpp>
#include <emu/util/bits.hpp>
#include <emu/util/debug.hpp>

namespace core {

std::unique_ptr<Mapper> Mapper::create(unsigned number, System *s)
{
    switch (number) {
    case 0: return std::make_unique<NROM>(s, s->prgrom, s->chrrom);
    case 1: return std::make_unique<MMC1>(s, s->prgrom, s->chrrom);
    default: return nullptr;
    }
}

u8 MMC1::read(std::span<u8> rom, u16 addr, u5 *bank, u1 mode, u8 magic_start)
{
    u8 magic        = magic_start - mode;
    size_t offset   = addr & util::bitmask(magic);
    u1 index        = util::getbit(addr, magic);
    size_t start    = bank[index] << magic;
    return rom[start + offset];
}

u8 MMC1::read_rom(u16 addr)
{
    // 0, 1 -> 1; 2 -> 0; 3 -> 2
    int displacement = (prg.mode & 2) == 0 ? 1 : (prg.mode & 1) << 1;
    return read(prgrom, addr, &prg.bank[displacement], prg.mode >> 1, 15);
}

u8 MMC1::read_chr(u16 addr)
{
    return read(chrrom, addr, chr.bank, chr.mode, 13);
}

void MMC1::write_rom(u16 addr, u8 data)
{
    unsigned bit = util::getbit(data, 0);
    bool reset   = util::getbit(data, 7);

    if (reset) {
        shift = counter = 0;
        return;
    }

    shift = util::setbit(shift >> 1, 4, bit);
    if (++counter == 5) {
        unsigned regno = util::getbits(addr, 13, 2);
        switch (regno) {
        case 0: {
            prg.mode = util::getbits(shift, 2, 2);
            chr.mode = util::getbits(shift, 4, 1);
            u2 mirroring = util::getbits(shift, 0, 2);
            switch (mirroring) {
            case 0: case 1: system->change_mirroring(Mirroring::OneScreen);  break;
            case 2:         system->change_mirroring(Mirroring::Vertical);   break;
            case 3:         system->change_mirroring(Mirroring::Horizontal); break;
            }
            break;
        }
        case 1: chr.bank[0] = shift; break;
        case 2: chr.bank[1] = shift; break;
        case 3: prg.bank[1] = prg.bank[2] = shift; break;
        }
        shift = 0;
        counter = 0;
    }
}

} // namespace core
