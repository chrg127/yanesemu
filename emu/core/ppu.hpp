#include <cstddef>
#include <emu/core/memorymap.hpp>

namespace Core {

class PPU {
    uint8_t gamepak_rom_ram[];
    uint8_t console_ram[];
    uint8_t palette[];
    uint8_t oam[];

    uint8_t reg_ctrl;

};

}
