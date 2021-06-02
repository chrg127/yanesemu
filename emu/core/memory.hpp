#ifndef MEMORY_HPP_INCLUDED
#define MEMORY_HPP_INCLUDED

#include <emu/core/const.hpp>
#include <emu/core/bus.hpp>

namespace Core {

void memory_bus_map(Bus<CPUBUS_SIZE> &rambus, Bus<PPUBUS_SIZE> &vrambus, Mirroring mirroring);
void memory_power(bool reset, char fillval = 0);

} // namespace Core

#endif
