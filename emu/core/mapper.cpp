#include "mapper.hpp"

#include <emu/util/bits.hpp>
#include <emu/util/debug.hpp>

using util::getbit;
using util::getbits;
using util::setbit;

std::unique_ptr<Mapper> create_mapper(unsigned number, std::span<u8> prg, std::span<u8> chr)
{
    switch (number) {
    case 0: return std::make_unique<NROM>(prg, chr);
    case 1: return std::make_unique<MMC1>(prg, chr);
    default: return nullptr;
    }
}

void MMC1::write_shift(u16 addr, u8 data)
{
    unsigned bit = getbit(data, 1);
    bool reset   = getbit(data, 7);

    if (reset) {
        shift = counter = 0;
        return;
    }

    shift = setbit(shift >> 1, 4, bit);
    if (++counter == 5) {
        unsigned regno = getbits(addr, 13, 2);
        regs[regno] = shift;
        shift = 0;
        counter = 0;
    }
}


u8 MMC1::read_rom(u16 addr)
{
    // return prgrom[addr - 0x8000];
    auto read_bank = [&](u8 bank1, u8 bank2, size_t bank_size) -> u8 {
        // first 14 bits = offset into bank
        // bit 15 = choose bank1 or bank2
        u8 offset = addr & util::bitmask(14);
        bool choice = getbit(addr, 15) == 0;
        size_t start = (choice ? bank1 : bank2) * bank_size;
        return prgrom[start + offset];
    };

    u8 control = regs[0];
    u8 bankno = getbits(regs[3], 0, 4);

    switch (getbits(control, 2, 2)) {
    case 0: case 1: return read_bank(bankno, bankno, 0x4000);
    case 2:         return read_bank(0,      bankno, 0x4000);
    case 3:         return read_bank(bankno, 0xF,    0x4000);
    default:        panic("MMC1::read_rom()");
    }
}
