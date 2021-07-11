#pragma once

#include <emu/core/const.hpp>
#include <emu/core/bus.hpp>

namespace Core {
namespace Memory {

void bus_map(Bus<CPUBUS_SIZE> &rambus, Bus<PPUBUS_SIZE> &vrambus, Mirroring mirroring);
void power(bool reset, char fillval = 0);

} // namespace Memory
} // namespace Core
