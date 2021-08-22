#pragma once

#include <emu/core/const.hpp>
#include <emu/core/bus.hpp>

namespace core {

class Memory {
    std::array<uint8, core::RAM_SIZE> rammem;
    std::array<uint8, core::VRAM_SIZE> vrammem;
    std::array<uint8, core::PAL_SIZE> palmem;
public:
    void map(Bus<CPUBUS_SIZE> &rambus, Bus<PPUBUS_SIZE> &vrambus, Mirroring mirroring);
    void power(bool reset, char fillval = 0);
};

} // namespace core
