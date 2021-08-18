#pragma once

#include <concepts>
#include <functional>
#include <array>
#include <emu/util/uint.hpp>
#include <emu/util/debug.hpp>

namespace Core {

template <std::size_t BusSize>
class Bus {
    static const int TABSIZE = 16;
    using Reader = std::function<uint8(uint16)>;
    using Writer = std::function<void(uint16, uint8)>;

    std::array<unsigned, BusSize> lookup;
    Reader readtab[TABSIZE];
    Writer writetab[TABSIZE];
    bool assigned[TABSIZE];

public:
    Bus() = default;
    Bus(const Bus<BusSize> &) = delete;
    Bus<BusSize> operator=(const Bus<BusSize> &) = delete;

    void map(uint32 start, uint32 end, auto &&reader, auto &&writer)
    {
        int id = 0;

        // search for a new id
        while (assigned[id]) {
            if (++id > TABSIZE) {
                error("mapping exhausted\n");
                return;
            }
        }
        assigned[id] = true;
        readtab[id] = reader;
        writetab[id] = writer;
        std::fill(lookup.begin() + start, lookup.begin() + end, id);
    }

    void reset()
    {
        std::fill(std::begin(assigned), std::end(assigned), false);
        std::fill(lookup.begin(), lookup.end(), 0);
    }

    uint8 read(uint16 addr) const { return readtab[lookup[addr]](addr); }
    void write(uint16 addr, uint8 data) { writetab[lookup[addr]](addr, data); }
};

} // namespace Core
