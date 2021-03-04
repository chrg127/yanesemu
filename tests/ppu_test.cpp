#include <fmt/core.h>
#include <emu/core/cartridge.hpp>
#include <emu/core/cpu.hpp>
#include <emu/core/ppu.hpp>
#include <emu/core/bus.hpp>
#include <emu/util/bits.hpp>
#include <emu/util/unsigned.hpp>
#include <emu/util/file.hpp>

namespace Core {
void dump(Util::File &dumpfile, Bus &bus)
{
    if (!dumpfile)
        return;
    auto dump_mem = [&](Bus &bus) {
        for (std::size_t i = 0; i < bus.size(); ) {
            dumpfile.print("{:04X}: ", i);
            for (std::size_t j = 0; j < 16; j++) {
                dumpfile.print("{:02X} ", bus.read(i));
                i++;
            }
            dumpfile.putc('\n');
        }
        dumpfile.putc('\n');
    };
    dump_mem(bus);
}
}

using Util::File;

int main()
{
    Core::CPU cpu;
    Core::PPU ppu;
    Core::Bus cpu_bus = Core::Bus(0x10000);
    Core::Bus ppu_bus = Core::Bus(0x4000);
    Core::Cartridge cart;
    File f;

    f.assoc(stdout, File::Mode::WRITE);
    cpu.attach_bus(&cpu_bus);
    ppu.attach_bus(&ppu_bus, &cpu_bus, Core::PPU::Mirroring::VERT);
    cart.open("testrom/test.nes");
    cart.attach_bus(&cpu_bus, &ppu_bus);
    cpu.power();
    ppu.power();
    ppu.set_nmi_callback([]() { fmt::print("got nmi\n"); });

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
    fmt::print("{}\n", ppu.get_info());

    cpu_bus.read(0x2002);
    cpu_bus.write(0x2006, 0x01);
    cpu_bus.write(0x2006, 0xEC);
    cpu_bus.write(0x2001, 0b00001010);

    dump(f, ppu_bus);
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
            fmt::print("color: {:X}\n", color);
        }
        fmt::print("\n");
        ppu.inc_v_vertpos();
    }

}

