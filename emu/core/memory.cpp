#include "memory.hpp"

#include <cassert>
#include <array>
#include <emu/util/bits.hpp>
#include <emu/util/uint.hpp>

namespace core {

using DecodeFn = u16 (*)(u16);
static DecodeFn get_decode(Mirroring mirroring)
{
    switch (mirroring) {
    case Mirroring::Horizontal:
        return [](u16 addr) -> u16 {
            auto tmp = addr & 0xFFF;
            auto bits = util::getbits(tmp, 10, 2) >> 1;
            return util::setbits(tmp, 10, 2, bits);
        };
    case Mirroring::Vertical:
        return [](u16 addr) -> u16 { return addr & 0x7FF; };
    default:
        panic("invalid value passed to get_decode\n");
    }
}

void Memory::map(Bus<CPUBUS_SIZE> &rambus, Bus<PPUBUS_SIZE> &vrambus, Mirroring mirroring)
{
    rambus.map(RAM_START, PPUREG_START, [this](u16 addr)          { return rammem[addr & 0x7FF]; },
                                        [this](u16 addr, u8 data) { rammem[addr & 0x7FF] = data; });
    vrambus.map(PAL_START, 0x4000, [this](u16 addr) { return palmem[addr & 0x1F]; },
                                   [this](u16 addr, u8 data) { palmem[addr & 0x1F] = data; });
    change_mirroring(mirroring, vrambus);
}

void Memory::change_mirroring(Mirroring mirroring, Bus<PPUBUS_SIZE> &vrambus)
{
    const auto decode = get_decode(mirroring);
    vrambus.map(NT_START, PAL_START, [this, decode](u16 addr)          { return vrammem[decode(addr)]; },
                                     [this, decode](u16 addr, u8 data) { vrammem[decode(addr)] = data; });
}

void Memory::power(bool reset, char fillval)
{
    std::fill(rammem.begin(), rammem.end(), fillval);
    if (!reset) {
        std::fill(vrammem.begin(), vrammem.end(), fillval);
        std::fill(palmem.begin(),  palmem.end(), fillval);
    }
}

} // namespace core
