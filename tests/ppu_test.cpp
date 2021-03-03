#include <fmt/core.h>
#include <emu/core/ppu.hpp>
#include <emu/core/bus.hpp>

int main()
{
    Core::PPU ppu;
    Core::Bus cpu_bus = Core::Bus(0x10000);
    Core::Bus ppu_bus = Core::Bus(0x4000);
    ppu.power();
    ppu.attach_bus(&ppu_bus, &cpu_bus, Core::PPU::Mirroring::VERT);
    ppu.set_nmi_callback([]() { fmt::print("got nmi\n"); });
    ppu.writereg(0x2006, 0x21);
    ppu.writereg(0x2006, 0xEF);
    fmt::print("{}\n", ppu.get_info());
}
