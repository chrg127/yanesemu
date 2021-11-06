#pragma once

#include <emu/core/const.hpp>
#include <emu/core/bus.hpp>

namespace core {

class Memory {
public:
    void map(Bus<CPUBUS_SIZE> &rambus, Bus<PPUBUS_SIZE> &vrambus, Mirroring mirroring);
    void change_mirroring(Mirroring mirroring, Bus<PPUBUS_SIZE> &bus);
    void power(bool reset, char fillval = 0);
};

} // namespace core
