#include "memory.hpp"

#include <cassert>
#include <array>
#include <emu/util/bits.hpp>
#include <emu/util/unsigned.hpp>

namespace Core {

using DecodeFn = uint16 (*)(uint16);
static DecodeFn get_decode(Mirroring mirroring)
{
    switch (mirroring) {
    case Mirroring::HORZ:
        return [](uint16 addr) -> uint16
        {
            auto tmp = addr & 0xFFF;
            auto bits = Util::getbits(tmp, 10, 2) >> 1;
            return Util::setbits(tmp, 10, 2, bits);
        };
    case Mirroring::VERT:
        return [](uint16 addr) -> uint16 { return addr & 0x7FF; };
    default: exit(1);
        // panic("invalid value passed to get_decode\n");
    }
}

void Memory::bus_map(Bus<CPUBUS_SIZE> &rambus, Bus<PPUBUS_SIZE> &vrambus, Mirroring mirroring)
{
    rambus.map(RAM_START, PPUREG_START,
        [this](uint16 addr)             { return rammem[addr & 0x7FF]; },
        [this](uint16 addr, uint8 data) { rammem[addr & 0x7FF] = data; });

    vrambus.map(PAL_START, 0x4000,
        [this](uint16 addr)
        {
            assert((addr & 0x1F) < PAL_SIZE);
            return palmem[addr & 0x1F];
        },
        [this](uint16 addr, uint8 data)
        {
            assert((addr & 0x1F) < PAL_SIZE);
            palmem[addr & 0x1F] = data;
        });

    vrambus.map(NT_START, PAL_START,
        [](uint16 addr)             { return 0; },
        [](uint16 addr, uint8 data) { /*******/ });

    const auto decode = get_decode(mirroring);
    vrambus.map(NT_START, PAL_START,
        [this, decode](uint16 addr)
        {
            addr = decode(addr);
            assert(addr < VRAM_SIZE);
            return vrammem[addr];
        },
        [this, decode](uint16 addr, uint8 data)
        {
            addr = decode(addr);
            assert(addr < VRAM_SIZE);
            vrammem[addr] = data;
        });
}

void Memory::power(bool reset, char fillval)
{
    std::fill(rammem.begin(), rammem.end(), fillval);
    if (!reset) {
        std::fill(vrammem.begin(), vrammem.end(), fillval);
        std::fill(palmem.begin(),  palmem.end(), fillval);
    }
    std::fill(oammem.begin(), oammem.end(), fillval);
}

} // namespace Core
