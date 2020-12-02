#include "genericbus.hpp"

#include <cstring> // memcpy
#include <emu/util/stringops.hpp>
#include <emu/util/debug.hpp>

namespace Core {

Bus::~Bus()
{
    if (lookup)
        delete lookup;
    mem  = nullptr;
    size = 0;
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

void Bus::map(uint16 start, uint16 end, Reader reader, Writer writer)
{
    int id = 0;

    /* search for a new id */
    while (assigned[id]) {
        if (++id > TABSIZ) {
            error("mapping exausted");
            return;
        }
    }
    rtab[id] = reader;
    wtab[id] = writer;
    for (uint16 i = start; i != end; i++)
        lookup[i] = id;
}

} // namespace Core

