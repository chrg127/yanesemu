#include "memory.hpp"

#include <cassert>
#include <array>
#include <emu/util/bits.hpp>
#include <emu/util/uint.hpp>

namespace core {

void Memory::map(Bus<CPUBUS_SIZE> &rambus, Bus<PPUBUS_SIZE> &vrambus, Mirroring mirroring)
{
    change_mirroring(mirroring, vrambus);
}


void Memory::power(bool reset, char fillval)
{
}

} // namespace core
