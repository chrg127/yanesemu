=== Directory description ===

    cpu.*               The CPU component. Includes code for instructions and
                        interrupts.
    bus.cpp             Contains a wrapper for NES memory.
    disassemble.cpp     Disassembling routines. #include'd in cpu.cpp.
    opcodes.cpp         All instruction routines, of course. #include'd in cpu.cpp.
    ppu.hpp             The PPU component.
    memorymap.hpp       CPU and PPU memory map constants.
    types.hpp           General supporting types for every component to use.
    
    ppu.*               PPU component. #include's background, vram, oam.
    background.*        PPU background stuff.
    oam.*               PPU OAM stuff.
    vram.*              PPU's VRAM.
    
    rom.hpp             A ROM type, see that file for more info.
    cartridge.*         A cartridge type that is used to parse ROM files and
                        keep info about them.
