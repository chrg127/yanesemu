#ifndef BUS_HPP_INCLUDED
#define BUS_HPP_INCLUDED

#include <functional>
#include "types.hpp"

namespace Core {

/* This bus offers high flexibity and speed in addressing memory areas.
 * It works by using the map() function to bind a memory area to a
 * function which has to convert addresses suitable for addressing. */
class Bus {
    static const int TABSIZ = 8;
    using Reader = std::function<uint8(uint16)>;
    using Writer = std::function<void(uint16, uint8)>;

    std::size_t size = 0;
    unsigned   *lookup = nullptr;
    Reader      rtab[TABSIZ];
    Writer      wtab[TABSIZ];
    bool        assigned[TABSIZ];

public:
    Bus() = default;
    Bus(const std::size_t s)
    { reset(s); }
    Bus(const Bus &) = delete;
    Bus(Bus &&b)
    { operator=(std::move(b)); }
    ~Bus();
    Bus & operator=(const Bus &) = delete;
    Bus & operator=(Bus &&b);

    void   reset(const std::size_t s);
    uint8  read(const uint16 addr) const;
    void   write(const uint16 addr, const uint8 data);
    void   map(uint16 start, uint16 end, Reader reader, Writer writer);
};

} // namespace Core

#endif
