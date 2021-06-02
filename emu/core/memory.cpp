#include "memory.hpp"

#include <cassert>
#include <array>
#include <emu/util/bits.hpp>
#include <emu/util/unsigned.hpp>
#include <emu/core/const.hpp>

namespace Core {

std::array<uint8, Core::RAM_SIZE> rammem;
std::array<uint8, Core::VRAM_SIZE> vrammem;
std::array<uint8, Core::PAL_SIZE> palmem;
std::array<uint8, Core::OAM_SIZE> oammem;

static std::function<uint16(uint16)> get_decode(Mirroring mirroring)
{
    switch (mirroring) {
    case Mirroring::HORZ:
        return [](uint16 addr)
        {
            auto tmp = addr & 0xFFF;
            auto bits = Util::getbits(tmp, 10, 2) >> 1;
            return Util::setbits(tmp, 10, 2, bits);
        };
    case Mirroring::VERT:
        return [](uint16 addr) { return addr & 0x7FF; };
    default:
        assert(false);
    }
}

void memory_bus_map(Bus<CPUBUS_SIZE> &rambus, Bus<PPUBUS_SIZE> &vrambus, Mirroring mirroring)
{
    rambus.map(RAM_START, PPUREG_START,
             [](uint16 addr)             { return rammem[addr & 0x7FF]; },
             [](uint16 addr, uint8 data) { rammem[addr & 0x7FF] = data; });
    vrambus.map(PAL_START, 0x4000,
            [](uint16 addr)
            {
                assert((addr & 0x1F) < PAL_SIZE);
                return palmem[addr & 0x1F];
            },
            [](uint16 addr, uint8 data) { assert((addr & 0x1F) < PAL_SIZE); palmem[addr & 0x1F] = data; });
    vrambus.map(NT_START, PAL_START,
            [](uint16 addr)             { return 0; },
            [](uint16 addr, uint8 data) { /*******/ });

    const auto decode = get_decode(mirroring);
    vrambus.map(NT_START, PAL_START,
            [decode](uint16 addr)
            {
                addr = decode(addr);
                assert(addr < VRAM_SIZE);
                return vrammem[addr];
            },
            [decode](uint16 addr, uint8 data)
            {
                addr = decode(addr);
                assert(addr < VRAM_SIZE);
                vrammem[addr] = data;
            });
}

void memory_power(bool reset, char fillval)
{
    std::fill(rammem.begin(), rammem.end(), fillval);
    if (!reset) {
        std::fill(vrammem.begin(), vrammem.end(), fillval);
        std::fill(palmem.begin(),  palmem.end(), fillval);
    }
    std::fill(oammem.begin(), oammem.end(), fillval);
}

} // namespace Core
