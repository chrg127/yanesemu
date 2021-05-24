#include <fmt/core.h>
#include <emu/core/cartridge.hpp>
#include <emu/core/cpu.hpp>
#include <emu/core/ppu.hpp>
#include <emu/core/bus.hpp>
#include <emu/util/bits.hpp>
#include <emu/util/unsigned.hpp>
#include <emu/util/file.hpp>

using Util::File;

int main()
{
    Core::Bus cpu_bus = Core::Bus(0x10000);
    Core::Bus ppu_bus = Core::Bus(0x4000);
    Core::CPU cpu{&cpu_bus};
    Core::PPU ppu{&cpu_bus, &ppu_bus};
    File out = File::assoc(stdout);
    auto romfile = File::open("testrom/test.nes", Util::Access::READ);
    auto cart = Core::parse_cartridge(*romfile);

    ppu.set_mirroring(cart->mirroring);
    cpu_bus.map(Core::CARTRIDGE_START, 0x8000,
            [&](uint16 addr) { return 0; },
            [&](uint16 addr, uint8 data) { /***********/ });
    cpu_bus.map(0x8000, Core::CPUBUS_SIZE,
            [&](uint16 addr)
            {
                uint16 offset = addr - 0x8000;
                uint16 start = cart->prgrom.size() - 0x8000;
                uint16 eff_addr = start + offset;
                return cart->prgrom[eff_addr];
            },
            [&](uint16 addr, uint8 data) { /***********/ });
    ppu_bus.map(Core::PT_START, Core::NT_START,
            [&](uint16 addr) { return cart->chrrom[addr]; },
            [&](uint16 addr, uint8 data) { /***********/ });
    cpu.power();
    ppu.power();
    ppu.on_nmi([]() { fmt::print("got nmi\n"); });

    cpu_bus.write(0x2000, 0);
    cpu_bus.write(0x2001, 0);
    cpu_bus.read(0x2002);
    cpu_bus.write(0x2006, 0x3F);
    cpu_bus.write(0x2006, 0);
    // palette
    for (int i = 8; i != 0; i--) {
        cpu_bus.write(0x2007, 0x0F);
        cpu_bus.write(0x2007, 0);
        cpu_bus.write(0x2007, 0x10);
        cpu_bus.write(0x2007, 0x30);
    }
    cpu_bus.write(0x2006, 0x20);
    cpu_bus.write(0x2006, 0);

    cpu_bus.read(0x2002);
    cpu_bus.write(0x2006, 0x21);
    cpu_bus.write(0x2006, 0xEC);
    cpu_bus.write(0x2007, 0x07);
    cpu_bus.write(0x2007, 0x08);
    cpu_bus.write(0x2007, 0x09);
    cpu_bus.write(0x2007, 0x09);
    cpu_bus.write(0x2007, 0x0A);
    // fmt::print("{}\n", ppu.get_info());

    cpu_bus.read(0x2002);
    cpu_bus.write(0x2006, 0x01);
    cpu_bus.write(0x2006, 0xEC);
    cpu_bus.write(0x2001, 0b00001010);
    // dump(f, ppu_bus);
/*
    ppu.inc_v_vertpos();
    for (int i = 0; i < 8; i++) {
        ppu.fetch_nt(1);
        ppu.fetch_attr(1);
        ppu.fetch_lowbg(1);
        ppu.fetch_highbg(1);
        // fmt::print("{:X} {:X} {:X} {:X}\n", ppu.tile.nt, ppu.tile.attr, ppu.tile.low, ppu.tile.high);
        // fmt::print("{}\n", ppu.get_info());
        ppu.shift_fill();
        for (int j = 0; j < 8; j++)
            ppu.shift_run();
        for (int j = 0; j < 8; j++) {
            ppu.shift_run();
            uint8 color = ppu.bg_output();
            fmt::print("{}", color == 0xF ? '.' : '*');
            // fmt::print("color: {:X}\n", color);
        }
        fmt::print("\n");
        ppu.inc_v_vertpos();
    }
*/
    ppu_bus.read(0x2BE0);

    // 2000 -> 0
    // 2400 -> 0
    // 2800 -> 400
    // 2c00 -> 400
}

