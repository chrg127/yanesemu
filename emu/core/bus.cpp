#include "bus.hpp"

#include <cassert>
#include <algorithm>
#include <emu/util/stringops.hpp>
#include <emu/util/debug.hpp>

namespace Core {

Bus & Bus::operator=(Bus &&b)
{
    lookup = std::move(b.lookup);
    for (int i = 0; i < TABSIZ; i++) {
        reader_tab[i] = b.reader_tab[i];
        writer_tab[i] = b.writer_tab[i];
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
    reader_tab[id] = reader;
    writer_tab[id] = writer;
    std::fill(lookup.begin() + start, lookup.begin() + end, id);
}

void Bus::remap(uint16 start, uint32 end, Reader reader, Writer writer)
{
    unsigned id = lookup[start];
    if (start != 0 && lookup[start-1] == id) {
        warning("remap: {} is not the real start of the area\n", start);
        return;
    }
    if (lookup[end-1] != id) {
        warning("remap: {} isn't the real end of the area\n", end);
        return;
    }
    reader_tab[id] = reader;
    writer_tab[id] = writer;
}

} // namespace Core

