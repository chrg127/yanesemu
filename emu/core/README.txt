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

=== CPU ===

The NES CPU is a standard 6502 CPU that lacks the decimal mode feature. You may
want a copy of the 6502 opcodes nearby in case you plan to understand anything
here.
The CPU is implemented as a stateful machine, with its state determined by the
registers and memory. main() advances the state for 1 single CPU instruction. A
single instruction does not mean a single cycle -- instructions may take two or
more cycles.

main() does its job by first analyzing if any interrupts took place. Interrupts
are a fundamental concept of every CPU and the reader should know at least what
their primary job is. There are 3 interrupts in the NES CPU: RESET, NMI and IRQ.
    - RESET happens when the CPU starts.
    - An NMI is fired by the PPU every time it goes into v-blank.
    - IRQs are not used anywhere in the NES.
For the sake of accuracy, all three kinds of interrupts are emulated.
After checking for interrupts, the CPU goes on to fetch an opcode and execute
it. Note that an "opcode" here is always an 8-bit value -- there are no opcodes
with a value over 255.

fetch() simply reads main memory using the PC register.

execute() takes an opcode and interprets it using a switch. The way opcode
functions work is that there are two types:
    - Addressing Mode functions, which take an instruction function, set stuff up
      according to the mode and execute the instruction function.
    - Instruction functions, which simply emulate an opcodes without caring
      about addressing modes.
Some instruction functions are called directly, such as BRK.

interrupt() should be viewed as an instruction function that is special. It
simply emulates interrupt behavior, but interrupts itself could be viewed as
instructions. This function implements interrupts according to the 6502
behavior, except for a bunch of if's, which are used to emulate "interrupt
hijacing". Interrupt hijacking happens when...

push() is a shorthand function for writing to the CPU stack. Likewise with
pull().

cycle() simply increases the cycles. In its current state, "cycles" is useless.
It may become useful later, with a new emulator version, when the PPU is finally
joined together with the CPU.

lastcycle() polls interrupts. What does this mean? While the CPU never really
polls other devices on the same bus line, it must however be listening for
interrupts. Therefore, there must be sometimes where it polls for the NMI and
IRQ lines, so that it can later handle them. The CPU usually polls interrupts at
the end of an instruction, but there is weird behavior with some ones
(specifically, all the branch instructions and few others).

irqpoll() and nmipoll() have just been described. To emulate interrupt behavior, a
device is supposed to use fire_nmi() or fire_irq(), which update nmi/irqpending.
Later, nmi/irqpoll() check if an nmi/irq is pending and whether the CPU can
execute them right now (there are specific cases where it can't execute them).
If so, they set execnmi/irq so that later the CPU will have to handle them.

power() powers the CPU. A ROM must be sent, so that the CPU can initialize
memory and get started. reset() resets the CPU so that it will start over.
printinfo() is not worth going over.
