=== Directory description ===

    cpu.*               The CPU component. Includes code for instructions and
                        interrupts.
    bus.*               Contains a wrapper for NES memory. Note that this is
                        the CPU memory bus, the PPU has a different bus.
    disassemble.cpp     Disassembling routines. #include'd in cpu.cpp.
    opcodes.cpp         All instruction routines, of course. #include'd in cpu.cpp.
    ppu.hpp             The PPU component.
    memorymap.hpp       CPU and PPU memory map constants.
    types.hpp           General supporting types for every component to use.
