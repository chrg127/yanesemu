#pragma once

#include <span>
#include <memory>
#include <emu/util/uint.hpp>

namespace core {

class Emulator;

struct Mapper {
protected:
    std::span<u8> prgrom, chrrom;
    Emulator *emulator;

public:
    explicit Mapper(std::span<u8> prg, std::span<u8> chr, Emulator *m)
        : prgrom(prg), chrrom(chr), emulator(m)
    { }

    virtual ~Mapper() = default;
    virtual u8 read_wram(u16 addr) = 0;
    virtual u8 read_rom(u16 addr) = 0;
    virtual u8 read_chr(u16 addr) = 0;
    virtual void write_wram(u16 addr, u8 data) = 0;
    virtual void write_rom(u16 addr, u8 data) = 0;
    virtual void write_chr(u16 addr, u8 data) = 0;

    static std::unique_ptr<Mapper> create(unsigned number, std::span<u8> prg, std::span<u8> chr, Emulator *e);
};


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
    int counter  = 0;
    u5 shift     = 0;
    u2 prg_mode  = 3;
    u2 chr_mode  = 0;
    u5 prg_bank  = 0;
    u5 chr_bank0 = 0;
    u5 chr_bank1 = 0;

public:
    using Mapper::Mapper;
    u8 read_wram(u16 addr)             { return 0; }
    u8 read_rom(u16 addr);
    u8 read_chr(u16 addr)              { return chrrom[addr]; }
    void write_wram(u16 addr, u8 data) { }
    void write_rom(u16 addr, u8 data);
    void write_chr(u16 addr, u8 data)  { }
};

} // namespace core
