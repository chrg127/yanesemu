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
    auto read = [&](u8 magic, u8 bank0, u8 bank1) {
        size_t offset = addr & util::bitmask(magic);
        bool choice = getbit(addr, magic) == 0;
        size_t start = (choice ? bank0 : bank1) << magic;
        return prgrom[start + offset];
    };

    switch (prg_mode) {
    case 0: case 1: return read(15, prg_bank, prg_bank);
    case 2:         return read(14,        0, prg_bank);
    case 3:         return read(14, prg_bank,        1);
    default:        panic("MMC1::read_rom()");
    }
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
            case 0: case 1: emulator->change_mirroring(Mirroring::OneScreen); break;
            case 2:         emulator->change_mirroring(Mirroring::Vertical); break;
            case 3:         emulator->change_mirroring(Mirroring::Horizontal); break;
            }
            prg_mode = getbits(shift, 2, 2);
            chr_mode = getbits(shift, 4, 1);
            break;
        }
        case 1: chr_bank0 = shift; break;
        case 2: chr_bank1 = shift; break;
        case 3: prg_bank  = shift; break;
        }
        shift = 0;
        counter = 0;
    }
}

} // namespace core
