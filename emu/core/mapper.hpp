#pragma once

#include <span>
#include <memory>
#include <emu/util/uint.hpp>

struct Mapper {
protected:
    std::span<u8> prgrom, chrrom;

public:
    explicit Mapper(std::span<u8> prg, std::span<u8> chr)
        : prgrom(prg), chrrom(chr)
    { }

    virtual ~Mapper() = default;
    virtual u8 read_wram(u16 addr) = 0;
    virtual u8 read_rom(u16 addr) = 0;
    virtual u8 read_chr(u16 addr) = 0;
    virtual void write_wram(u16 addr, u8 data) = 0;
    virtual void write_rom(u16 addr, u8 data) = 0;
    virtual void write_chr(u16 addr, u8 data) = 0;
};

std::unique_ptr<Mapper> create_mapper(unsigned number, std::span<u8> prg, std::span<u8> chr);

struct NROM : public Mapper {
    using Mapper::Mapper;
    u8 read_wram(u16 addr)             { return 0; }
    u8 read_rom(u16 addr)              { return prgrom[addr - 0x8000]; }
    u8 read_chr(u16 addr)              { return chrrom[addr         ]; }
    void write_wram(u16 addr, u8 data) { }
    void write_rom(u16 addr, u8 data)  { }
    void write_chr(u16 addr, u8 data)  { }
};

struct MMC1 : public Mapper {
    int counter = 0;
    u5 shift    = 0;
    // 0 = control; 1 = chr bank 0; 2 = chr bank 1; 3 = prg bank
    u5 regs[4];

public:
    void write_shift(u16 addr, u8 data);

    using Mapper::Mapper;
    u8 read_wram(u16 addr)             { return 0; }
    u8 read_rom(u16 addr);
    u8 read_chr(u16 addr)              { return chrrom[addr]; }
    void write_wram(u16 addr, u8 data) { }
    void write_rom(u16 addr, u8 data)  { write_shift(addr, data); }
    void write_chr(u16 addr, u8 data)  { }
};
