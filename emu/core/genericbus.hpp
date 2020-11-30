#ifndef BUS_HPP_INCLUDED
#define BUS_HPP_INCLUDED

#include <functional>
#include "types.hpp"

namespace Core {

/* This bus offers high flexibity and speed in addressing memory areas.
 * It works by using the map() function to bind a memory area to a
 * function which has to convert addresses suitable for addressing. */
class Bus {
    static const int MAX_ADDRESSERS = 8;
    using Addresser = std::function<uint16(uint16)>;

    std::size_t     size = 0;
    uint8           *mem = nullptr;
    unsigned int    *lookup = nullptr;
    Addresser       ad_tab[MAX_ADDRESSERS];
    bool            counter[MAX_ADDRESSERS];

public:
    Bus() = default;
    Bus(const std::size_t s);
    Bus(const Bus &) = delete;
    Bus(Bus &&b)
    { operator=(std::move(b)); }
    ~Bus();

    Bus & operator=(const Bus &) = delete;
    Bus & operator=(Bus &&b);

    void   reset(const std::size_t s);
    uint8  read(const uint16 addr) const;
    void   write(const uint16 addr, const uint8 data);
    void   map(uint16 start, uint16 end, Addresser addresser);
    uint8 *memory() const
    { return mem; }
    // uint16 test(uint16 addr)
    // {
    //     return ad_tab[lookup[addr]](addr);
    // }
};

} // namespace Core

#endif
