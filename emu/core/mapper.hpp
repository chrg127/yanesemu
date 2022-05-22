#pragma once

#include <span>
#include <memory>
#include <emu/util/common.hpp>
#include <emu/util/uint.hpp>

namespace core {

class System;

struct Mapper {
protected:
    System *system;
    std::span<u8> prgrom, chrrom;

public:
    Mapper(System *s, std::span<u8> prg, std::span<u8> chr)
        : system(s), prgrom(prg), chrrom(chr)
    { }

    virtual ~Mapper() = default;
    virtual u8 read_wram(u16 addr) = 0;
    virtual u8 read_rom(u16 addr) = 0;
    virtual u8 read_chr(u16 addr) = 0;
    virtual void write_wram(u16 addr, u8 data) = 0;
    virtual void write_rom(u16 addr, u8 data) = 0;
    virtual void write_chr(u16 addr, u8 data) = 0;

    static std::unique_ptr<Mapper> create(unsigned number, System *s);
};

struct NROM : public Mapper {
    using Mapper::Mapper;
    u8 read_wram(u16 addr)             { return 0; }
    u8 read_rom(u16 addr)              { return prgrom[addr & (prgrom.size() - 1)]; }
    u8 read_chr(u16 addr)              { return chrrom[addr         ]; }
    void write_wram(u16 addr, u8 data) { }
    void write_rom(u16 addr, u8 data)  { }
    void write_chr(u16 addr, u8 data)  { }
};

class MMC1 : public Mapper {
    int counter  = 0;
    u5 shift     = 0;

    struct {
        u2 mode  = 3;
        // bank[0] always contains the value of the first bank
        // bank[3] always contains the value of the last bank
        // bank[1] and [2] act as if they're the same; they're mapper settable
        u5 bank[4] = { 0, 0, 0, 0 };
    } prg;

    struct {
        u1 mode = 0;
        u5 bank[2] = { 0, 0 };
    } chr;

    u8 read(std::span<u8> rom, u16 addr, u5 *bank, u1 mode, u8 magic_start);

public:
    MMC1(System *s, std::span<u8> prgrom, std::span<u8> chr) : Mapper(s, prgrom, chr)
    {
        prg.bank[3] = prgrom.size() / (0x4000) - 1;
    }

    u8 read_wram(u16 addr) { return 0; }
    u8 read_rom(u16 addr);
    u8 read_chr(u16 addr);
    void write_wram(u16 addr, u8 data) { }
    void write_rom(u16 addr, u8 data);
    void write_chr(u16 addr, u8 data)  { }
};

} // namespace core
