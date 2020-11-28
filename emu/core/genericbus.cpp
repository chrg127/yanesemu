#include "genericbus.hpp"

#include <cstring> // memcpy
#include <emu/util/stringops.hpp>
#include <emu/util/debug.hpp>

namespace Core {

Bus::Bus(const std::size_t s)
{
    reset(s);
}

Bus::~Bus()
{
    if (mem)
        delete[] mem;
    if (lookup)
        delete lookup;
    mem  = nullptr;
    size = 0;
}

Bus & Bus::operator=(Bus &&b)
{
    size   = b.size;
    mem    = b.mem;
    lookup = b.lookup;
    std::memcpy(ad_tab, b.ad_tab, MAX_ADDRESSERS);
    std::memcpy(ad_tab, b.ad_tab, MAX_ADDRESSERS);
    b.mem    = nullptr;
    b.lookup = nullptr;
    return *this;
}

void Bus::reset(const std::size_t s)
{
    if (mem)
        delete[] mem;
    if (lookup)
        delete lookup;
    mem     = new uint8[s];
    lookup  = new unsigned int[s];
    size    = s;
    ad_tab[0] = [](uint16 addr) { return addr; };
    for (auto &x : counter)
        x = false;
}

uint8 Bus::read(const uint16 addr) const
{
    return mem[ad_tab[lookup[addr]](addr)];
}

void Bus::write(const uint16 addr, const uint8 data)
{
    mem[ad_tab[lookup[addr]](addr)] = data;
}

/* Maps area between start-end to addresser. Mapping already mapped areas
 * is allowed; if such errors happen, the new function will simply replace
 * the old one. Only MAX_ADDRESSERS are allowed and the first one is used
 * by a default addresser that simply returns its input. */
void Bus::map(uint16 start, uint16 end, Addresser addresser)
{
    int id = 0;

    if (!mem) {
        error("bus memory not initialized");
        return;
    }
    /* search for a new id */
    while (counter[id]) {
        if (++id > MAX_ADDRESSERS) {
            error("mapping exausted");
            return;
        }
    }
    counter[id] = true;
    ad_tab[id] = addresser;
    for (uint16 i = start; i < end; i++)
        lookup[i] = id;
}

} // namespace Core

