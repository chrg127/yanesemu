#ifndef CORE_BUS_HPP_INCLUDED
#define CORE_BUS_HPP_INCLUDED

#include <functional>
#include <emu/util/unsigned.hpp>
#include <emu/util/heaparray.hpp>

namespace Core {

class Bus {
    static const int TABSIZ = 16;
    using Reader = std::function<uint8(uint16)>;
    using Writer = std::function<void(uint16, uint8)>;

    Util::HeapArray<unsigned> lookup;
    Reader readtab[TABSIZ];
    Writer writetab[TABSIZ];
    bool assigned[TABSIZ];

public:
    Bus() = default;
    explicit Bus(const uint32 size) { reset(size); }

    Bus(const Bus &) = delete;
    Bus(Bus &&b) { operator=(std::move(b)); }
    Bus & operator=(const Bus &) = delete;
    Bus & operator=(Bus &&b);

    void map(uint16 start, uint32 end, Reader reader, Writer writer);
    void remap(uint16 start, uint32 end, Reader reader, Writer writer);
    void reset(const std::size_t newsize);

    uint8 read(const uint16 addr) const             { return readtab[lookup[addr]](addr); }
    void write(const uint16 addr, const uint8 data) { writetab[lookup[addr]](addr, data); }
    std::size_t size() const                        { return lookup.size(); }
};

} // namespace Core

#endif
