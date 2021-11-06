#include "mapper.hpp"

#include <emu/core/emulator.hpp>
#include <emu/core/const.hpp>
#include <emu/util/bits.hpp>
#include <emu/util/debug.hpp>

using util::getbit;
using util::getbits;
using util::setbit;

namespace core {

std::unique_ptr<Mapper> Mapper::create(unsigned number, std::span<u8> prg, std::span<u8> chr, Emulator *e)
{
    switch (number) {
    case 0: return std::make_unique<NROM>(prg, chr, e);
    case 1: return std::make_unique<MMC1>(prg, chr, e);
    default: return nullptr;
    }
}

u8 MMC1::read_rom(u16 addr)
{
    size_t offset = addr & util::bitmask(prg.magic);
    u1 bank = getbit(addr, prg.magic);
    size_t start = *prg.ptrs[bank] << prg.magic;
    return prgrom[start + offset];
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
            // u2 mirroring = getbits(shift, 0, 2);
            // switch (mirroring) {
            // case 0: case 1: emulator->change_mirroring(Mirroring::OneScreen); break;
            // case 2:         emulator->change_mirroring(Mirroring::Vertical); break;
            // case 3:         emulator->change_mirroring(Mirroring::Horizontal); break;
            // }

            switch (getbits(shift, 2, 2)) {
            case 0: case 1: prg.magic = 15; prg.ptrs[0] = &prg.bank;  prg.ptrs[1] = &prg.bank; break;
            case 2:         prg.magic = 14; prg.ptrs[0] = &prg.first; prg.ptrs[1] = &prg.bank; break;
            case 3:         prg.magic = 14; prg.ptrs[0] = &prg.bank;  prg.ptrs[1] = &prg.last; break;
            }

            chr.mode = getbits(shift, 4, 1);
            break;
        }
        case 1: chr.banks[0] = shift; break;
        case 2: chr.banks[1] = shift; break;
        case 3: prg.bank  = shift; break;
        }
        shift = 0;
        counter = 0;
    }
}

} // namespace core
