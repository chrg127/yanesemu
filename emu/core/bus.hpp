#ifndef BUS_HPP_INCLUDED
#define BUS_HPP_INCLUDED

#include <functional>
#include <emu/util/heaparray.hpp>
#include "types.hpp"

namespace Core {

/* This bus offers high flexibity and speed in addressing memory areas.
 * It works by using the map() function to bind a memory area to a
 * function which has to convert addresses suitable for addressing. */
class Bus {
    static const int TABSIZ = 8;
    using Reader = std::function<uint8(uint16)>;
    using Writer = std::function<void(uint16, uint8)>;

    // uint32 is used to permit a size of 0x10000
    // you can't, however, address more than 0xFFFF
    HeapArray<unsigned> lookup;
    Reader rtab[TABSIZ];
    Writer wtab[TABSIZ];
    bool assigned[TABSIZ];

public:
    Bus() = default;
    Bus(const uint32 size) { reset(size); }

    Bus(const Bus &)             = delete;
    Bus & operator=(const Bus &) = delete;
    Bus(Bus &&b) { operator=(std::move(b)); }
    Bus & operator=(Bus &&b);

    void map(uint16 start, uint32 end, Reader reader, Writer writer);
    void reset(const std::size_t newsize);

    uint8 read(const uint16 addr) const             { return rtab[lookup[addr]](addr); }
    void write(const uint16 addr, const uint8 data) { wtab[lookup[addr]](addr, data); }
};

} // namespace Core

#endif
