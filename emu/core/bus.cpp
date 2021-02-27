#include "bus.hpp"

#include <cassert>
#include <algorithm>
// #include <cstring> // memcpy
#include <emu/util/stringops.hpp>
#include <emu/util/debug.hpp>

namespace Core {

Bus & Bus::operator=(Bus &&b)
{
    lookup = std::move(b.lookup);
    for (int i = 0; i < TABSIZ; i++) {
        rtab[i] = b.rtab[i];
        wtab[i] = b.wtab[i];
    }
    std::memcpy(assigned, b.assigned, TABSIZ);
    return *this;
}

void Bus::reset(const std::size_t newsize)
{
    lookup.reset(newsize);
    std::memset(assigned, 0, TABSIZ);
}

void Bus::map(uint16 start, uint32 end, Reader reader, Writer writer)
{
    int id = 0;

    // search for a new id
    while (assigned[id]) {
        if (++id > TABSIZ) {
            error("mapping exausted\n");
            return;
        }
    }
    assigned[id] = true;
    rtab[id] = reader;
    wtab[id] = writer;
    std::fill(lookup.begin() + start, lookup.begin() + end, id);
}

} // namespace Core

