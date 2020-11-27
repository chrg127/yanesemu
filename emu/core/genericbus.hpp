#ifndef BUS_HPP_INCLUDED
#define BUS_HPP_INCLUDED

#include "types.hpp"


template <unsigned Memsize>
class Bus {
    uint8 mem[Memsize];

public:
    struct Area {
        uint16 start;
        unsigned size;
    };

    Bus(array<Area> areas);

    uint8 read(uint16 addr);
    void  write(uint16 addr, uint8 data);
};

template <unsigned Memsize>
Bus(array<Area> areas)
{

}

template <unsigned Memsize>
uint8 Bus<Memsize>::read(uint16 addr)
{
    return mem[addr];
}

template <unsigned Memsize>
void Bus<Memsize>::write(uint16 addr, uint8 data)
{
    mem[addr] = data;
}

#endif
