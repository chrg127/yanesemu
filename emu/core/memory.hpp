#pragma once

#include <emu/core/const.hpp>
#include <emu/core/bus.hpp>

namespace Core {

class Memory {
    std::array<uint8, Core::RAM_SIZE> rammem;
    std::array<uint8, Core::VRAM_SIZE> vrammem;
    std::array<uint8, Core::PAL_SIZE> palmem;
    std::array<uint8, Core::OAM_SIZE> oammem;
public:
    void bus_map(Bus<CPUBUS_SIZE> &rambus, Bus<PPUBUS_SIZE> &vrambus, Mirroring mirroring);
    void power(bool reset, char fillval = 0);
};

} // namespace Core
