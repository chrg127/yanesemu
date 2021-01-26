#include "genericbus.hpp"

#include <cassert>
#include <algorithm>
// #include <cstring> // memcpy
#include <emu/util/stringops.hpp>
#include <emu/util/debug.hpp>

namespace Core {

Bus::~Bus()
{
    if (lookup)
        delete[] lookup;
}

Bus & Bus::operator=(Bus &&b)
{
    size   = b.size;
    lookup = b.lookup;
    for (int i = 0; i < TABSIZ; i++) {
        rtab[i] = b.rtab[i];
        wtab[i] = b.wtab[i];
    }
    std::memcpy(assigned, b.assigned, TABSIZ);
    b.lookup = nullptr;
    return *this;
}

void Bus::reset(const std::size_t s)
{
    if (lookup)
        delete[] lookup;
    size = s;
    lookup = new unsigned[size]();
    std::memset(assigned, 0, TABSIZ);
}

uint8 Bus::read(const uint16 addr) const
{
    return rtab[lookup[addr]](addr);
}

void Bus::write(const uint16 addr, const uint8 data)
{
    wtab[lookup[addr]](addr, data);
}

void Bus::map(uint16 start, uint32 end, Reader reader, Writer writer)
{
    int id = 0;

    /* search for a new id */
    while (assigned[id]) {
        if (++id > TABSIZ) {
            error("mapping exausted");
            return;
        }
    }
    assigned[id] = true;
    rtab[id] = reader;
    wtab[id] = writer;
    std::fill(lookup+start, lookup+end, id);
    // NOTE to self: memset acts on bytes, fill acts on "objects"
    // what this means is that, in an array of T where sizeof(T) > 1,
    // filling the array with memset(arr, 1, size) will write something
    // like 01010101 for each element of the array.
    // fill does not do such a thing, and is preferable everywhere
    // but byte array.
    // uint8 *offset = lookup+start;
    // size_t offsize = (lookup+end) - (lookup+start);
    // std::memset(offset, id, offsize);
}

} // namespace Core

