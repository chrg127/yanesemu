#include "mapper.hpp"

#include <emu/core/emulator.hpp>
#include <emu/core/const.hpp>
#include <emu/util/bits.hpp>
#include <emu/util/debug.hpp>

using util::getbit;
using util::getbits;
using util::setbit;

namespace core {

std::unique_ptr<Mapper> Mapper::create(unsigned number, System *s)
{
    switch (number) {
    case 0: return std::make_unique<NROM>(s, s->prgrom, s->chrrom);
    case 1: return std::make_unique<MMC1>(s, s->prgrom, s->chrrom);
    default: return nullptr;
    }
}

u8 MMC1::read_rom(u16 addr)
{
    u8 magic      = 15 - prg.mode;
    size_t offset = addr & util::bitmask(magic);
    u1 bank       = getbit(addr, magic);
    size_t start  = *prg.ptrs[bank] << magic;
    return prgrom[start + offset];
}

u8 MMC1::read_chr(u16 addr)
{
    u8 magic      = 13 - chr.mode; // mode = 0 -> 8k (1 << 13), mode = 1 -> 4k (1 << 12)
    size_t offset = addr & util::bitmask(magic);
    u1 i          = getbit(addr, magic);
    int start     = chr.bank[i] << magic;
    return chrrom[start + offset];
}

void MMC1::write_rom(u16 addr, u8 data)
{
    unsigned bit = getbit(data, 0);
    bool reset   = getbit(data, 7);

    if (reset) {
        shift = counter = 0;
        return;
    }

    shift = setbit(shift >> 1, 4, bit);
    if (++counter == 5) {
        unsigned regno = getbits(addr, 13, 2);
        switch (regno) {
        case 0: {
            u2 mirroring = getbits(shift, 0, 2);
            switch (mirroring) {
            case 0: case 1: system->change_mirroring(Mirroring::OneScreen);  break;
            case 2:         system->change_mirroring(Mirroring::Vertical);   break;
            case 3:         system->change_mirroring(Mirroring::Horizontal); break;
            }

            switch (getbits(shift, 2, 2)) {
            case 0: case 1: prg.mode = 0; prg.ptrs[0] = &prg.bank;  prg.ptrs[1] = &prg.bank; break;
            case 2:         prg.mode = 1; prg.ptrs[0] = &prg.first; prg.ptrs[1] = &prg.bank; break;
            case 3:         prg.mode = 1; prg.ptrs[0] = &prg.bank;  prg.ptrs[1] = &prg.last; break;
            }

            chr.mode = getbits(shift, 4, 1);
            break;
        }
        case 1: chr.bank[0] = shift; break;
        case 2: chr.bank[1] = shift; break;
        case 3: prg.bank    = shift; break;
        }
        shift = 0;
        counter = 0;
    }
}

} // namespace core
