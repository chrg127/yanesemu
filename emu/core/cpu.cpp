#include <emu/core/cpu.hpp>

#include <cstdio>
#include <cctype>
#include <cstring>
#include <emu/core/cartridge.hpp>
#include <emu/core/ppu.hpp>
#include <emu/util/file.hpp>
#include <emu/util/stringops.hpp>
#define DEBUG
#include <emu/util/debug.hpp>

namespace Core {

#define INSIDE_CPU_CPP
#include <emu/core/opcodes.cpp>
#include <emu/core/disassemble.cpp>
#undef INSIDE_CPU_CPP

/* Fetch next opcode from memory */
uint8 CPU::fetch()
{
    return bus.read(pc.reg++);
}

/* Executes a single instruction. */
void CPU::execute(uint8 opcode)
{
#define INSTR_IMPLD(id, func) \
    case id: instr_##func(); return;
#define INSTR_AMODE(id, name, mode, type) \
    case id: addrmode_##mode##_##type(&CPU::instr_##name); return;
#define INSTR_WRITE(id, mode, val) \
    case id: addrmode_##mode##_write(val); return;
#define INSTR_OTHER(id, func, ...) \
    case id: instr_##func(__VA_ARGS__); return;

    switch(opcode) {
        INSTR_IMPLD(0x00, brk)
        INSTR_AMODE(0x01, ora, indx, read)
        INSTR_AMODE(0x05, ora, zero, read)
        INSTR_AMODE(0x06, asl, zero, modify)
        INSTR_IMPLD(0x08, php)
        INSTR_AMODE(0x09, ora, imm, read)
        INSTR_AMODE(0x0A, asl, accum, modify)
        INSTR_AMODE(0x0D, ora, abs, read)
        INSTR_AMODE(0x0E, asl, abs, modify)
        INSTR_OTHER(0x10, branch, procstatus.neg == 0)     // bpl
        INSTR_AMODE(0x11, ora, indy, read)
        INSTR_AMODE(0x15, ora, zerox, read)
        INSTR_AMODE(0x16, asl, zerox, modify)
        INSTR_OTHER(0x18, flag, procstatus.carry, false)   // clc
        INSTR_AMODE(0x19, ora, absy, read)
        INSTR_AMODE(0x1D, ora, absx, read)
        INSTR_AMODE(0x1E, asl, absx, modify)
        INSTR_IMPLD(0x20, jsr)
        INSTR_AMODE(0x21, and, indx, read)
        INSTR_AMODE(0x24, bit, zero, read)
        INSTR_AMODE(0x25, and, zero, read)
        INSTR_AMODE(0x26, rol, zero, modify)
        INSTR_IMPLD(0x28, plp)
        INSTR_AMODE(0x29, and, imm, read)
        INSTR_AMODE(0x2A, rol, accum, modify)
        INSTR_AMODE(0x2C, bit, abs, read)
        INSTR_AMODE(0x2D, and, abs, read)
        INSTR_AMODE(0x2E, rol, abs, modify)
        INSTR_OTHER(0x30, branch, procstatus.neg == 1)     // bmi
        INSTR_AMODE(0x31, and, indy, read)
        INSTR_AMODE(0x35, and, zerox, read)
        INSTR_AMODE(0x36, rol, zerox, modify)
        INSTR_OTHER(0x38, flag, procstatus.carry, true)    //sec
        INSTR_AMODE(0x39, and, absy, read)
        INSTR_AMODE(0x3D, and, absx, read)
        INSTR_AMODE(0x3E, rol, absx, modify)
        INSTR_IMPLD(0x40, rti)
        INSTR_AMODE(0x41, eor, indx, read)
        INSTR_AMODE(0x45, eor, zero, read)
        INSTR_AMODE(0x46, lsr, zero, modify)
        INSTR_IMPLD(0x48, pha)
        INSTR_AMODE(0x49, eor, imm, read)
        INSTR_AMODE(0x4A, lsr, accum, modify)
        INSTR_IMPLD(0x4C, jmp)
        INSTR_AMODE(0x4D, eor, abs, read)
        INSTR_AMODE(0x4E, lsr, abs, modify)
        INSTR_OTHER(0x50, branch, procstatus.ov == 0)      // bvc
        INSTR_AMODE(0x51, eor, indy, read)
        INSTR_AMODE(0x55, eor, zerox, read)
        INSTR_AMODE(0x56, lsr, zerox, modify)
        INSTR_OTHER(0x58, flag, procstatus.intdis, false)  //cli
        INSTR_AMODE(0x59, eor, absy, read)
        INSTR_AMODE(0x5D, eor, absx, read)
        INSTR_AMODE(0x5E, lsr, absx, modify)
        INSTR_IMPLD(0x60, rts)
        INSTR_AMODE(0x61, adc, indx, read)
        INSTR_AMODE(0x65, adc, zero, read)
        INSTR_AMODE(0x66, ror, zero, modify)
        INSTR_IMPLD(0x68, pla)
        INSTR_AMODE(0x69, adc, imm, read)
        INSTR_AMODE(0x6A, ror, accum, modify)
        INSTR_IMPLD(0x6C, jmp_ind)
        INSTR_AMODE(0x6D, adc, abs, read)
        INSTR_AMODE(0x6E, ror, abs, modify)
        INSTR_OTHER(0x70, branch, procstatus.ov == 1)      // bvs
        INSTR_AMODE(0x71, adc, indy, read)
        INSTR_AMODE(0x75, adc, zerox, read)
        INSTR_AMODE(0x76, ror, zerox, modify)
        INSTR_OTHER(0x78, flag, procstatus.intdis, true)   //sei
        INSTR_AMODE(0x79, adc, absy, read)
        INSTR_AMODE(0x7D, adc, absx, read)
        INSTR_AMODE(0x7E, ror, absx, modify)
        INSTR_WRITE(0x81, indx, accum)                      // sta
        INSTR_WRITE(0x84, zero, yreg)                       // sty
        INSTR_WRITE(0x85, zero, accum)                      // sta
        INSTR_WRITE(0x86, zero, xreg)                       // stx
        INSTR_IMPLD(0x88, dey)
        INSTR_OTHER(0x8A, transfer, xreg, accum)            // txa
        INSTR_WRITE(0x8C, abs, yreg)                        // sty
        INSTR_WRITE(0x8D, abs, accum)                       // sta
        INSTR_WRITE(0x8E, abs, xreg)                        // stx
        INSTR_OTHER(0x90, branch, procstatus.carry == 0)    // bcc
        INSTR_WRITE(0x91, indy, accum)                      // sta
        INSTR_WRITE(0x94, zerox, yreg)                      // sty
        INSTR_WRITE(0x95, zerox, accum)                     // sta
        INSTR_WRITE(0x96, zeroy, xreg)                      // stx
        INSTR_OTHER(0x98, transfer, yreg, accum)            // tya
        INSTR_WRITE(0x99, absy, accum)                      // sta
        INSTR_OTHER(0x9A, transfer, xreg, sp)               // txs
        INSTR_WRITE(0x9D, absx, accum)                      // sta
        INSTR_AMODE(0xA0, ldy, imm, read)
        INSTR_AMODE(0xA1, lda, indx, read)
        INSTR_AMODE(0xA2, ldx, imm, read)
        INSTR_AMODE(0xA4, ldy, zero, read)
        INSTR_AMODE(0xA5, lda, zero, read)
        INSTR_AMODE(0xA6, ldx, zero, read)
        INSTR_OTHER(0xA8, transfer, accum, yreg)            // tay
        INSTR_AMODE(0xA9, lda, imm, read)
        INSTR_OTHER(0xAA, transfer, accum, xreg)            // tax
        INSTR_AMODE(0xAC, ldy, abs, read)
        INSTR_AMODE(0xAD, lda, abs, read)
        INSTR_AMODE(0xAE, ldx, abs, read)
        INSTR_OTHER(0xB0, branch, procstatus.carry == 1)    // bcs
        INSTR_AMODE(0xB1, lda, indy, read)
        INSTR_AMODE(0xB4, ldy, zerox, read)
        INSTR_AMODE(0xB5, lda, zerox, read)
        INSTR_AMODE(0xB6, ldx, zeroy, read)
        INSTR_OTHER(0xB8, flag, procstatus.ov, false)       // clv
        INSTR_AMODE(0xB9, lda, absy, read)
        INSTR_OTHER(0xBA, transfer, sp, xreg)               // tsx
        INSTR_AMODE(0xBC, ldy, absx, read)
        INSTR_AMODE(0xBD, lda, absx, read)
        INSTR_AMODE(0xBE, ldx, absy, read)
        INSTR_AMODE(0xC0, cpy, imm, read)
        INSTR_AMODE(0xC1, cmp, indx, read)
        INSTR_AMODE(0xC4, cpy, zero, read)
        INSTR_AMODE(0xC5, cmp, zero, read)
        INSTR_AMODE(0xC6, dec, zero, modify)
        INSTR_IMPLD(0xC8, iny)
        INSTR_AMODE(0xC9, cmp, imm, read)
        INSTR_IMPLD(0xCA, dex)
        INSTR_AMODE(0xCC, cpy, abs, read)
        INSTR_AMODE(0xCD, cmp, abs, read)
        INSTR_AMODE(0xCE, dec, abs, modify)
        INSTR_OTHER(0xD0, branch, procstatus.zero == 0)    // bne
        INSTR_AMODE(0xD1, cmp, indy, read)
        INSTR_AMODE(0xD5, cmp, zerox, read)
        INSTR_AMODE(0xD6, dec, zerox, modify)
        INSTR_OTHER(0xD8, flag, procstatus.decimal, false) //cld
        INSTR_AMODE(0xD9, cmp, absy, read)
        INSTR_AMODE(0xDD, cmp, absx, read)
        INSTR_AMODE(0xDE, dec, absx, modify)
        INSTR_AMODE(0xE0, cpx, imm, read)
        INSTR_AMODE(0xE1, sbc, indx, read)
        INSTR_AMODE(0xE4, cpx, zero, read)
        INSTR_AMODE(0xE5, sbc, zero, read)
        INSTR_AMODE(0xE6, inc, zero, modify)
        INSTR_IMPLD(0xE8, inx)
        INSTR_AMODE(0xE9, sbc, imm, read)
        INSTR_IMPLD(0xEA, nop)
        INSTR_AMODE(0xEC, cpx, abs, read)
        INSTR_AMODE(0xED, sbc, abs, read)
        INSTR_AMODE(0xEE, inc, abs, modify)
        INSTR_OTHER(0xF0, branch, procstatus.zero == 1)    // beq
        INSTR_AMODE(0xF1, sbc, indy, read)
        INSTR_AMODE(0xF5, sbc, zerox, read)
        INSTR_AMODE(0xF6, inc, zerox, modify)
        INSTR_OTHER(0xF8, flag, procstatus.decimal, true)   // sed
        INSTR_AMODE(0xF9, sbc, absy, read)
        INSTR_AMODE(0xFD, sbc, absx, read)
        INSTR_AMODE(0xFE, inc, absx, modify)
        default:
            DBGPRINTF("error: unknown opcode: %02X\n", opcode);
            return;
    }

#undef INSTR_IMPLD
#undef INSTR_AMODE
#undef INSTR_WRITE
#undef INSTR_OTHER
}

/* Emulates the interrupt behavior common to all four interrupt signals
 * (reset, irq, nmi and brk) */
void CPU::interrupt(bool reset)
{
    uint16 vec;

    // one cycle for reading next instruction byte and throw away
    cycle();
    push(pc.high);
    push(pc.low);
    push(procstatus.reg());
    // reset this here just in case
    procstatus.breakf = 0;
    procstatus.intdis = 1;
    // Interrupt hijacking
    // there's a special handling for the reset interrupt. more research should
    // be done for a better solution.
    if (reset) 
        vec = CPUMap::RESETVEC;
    else if (nmipending) {
        nmipending = false;
        vec = CPUMap::NMIVEC;
    } else if (irqpending) {
        irqpending = false;
        vec = CPUMap::IRQBRKVEC;
    } else
        vec = CPUMap::IRQBRKVEC;
    pc.low = bus.read(vec);
    pc.high = bus.read(vec+1);
}

/* Pushes a value to the hardware stack */
void CPU::push(uint8 val)
{
    bus.write(0x0100+sp, val);
    sp--;
}

/* Pulls and returns a value from the hardware stack */
uint8 CPU::pull()
{
    ++sp;
    return bus.read(0x0100+sp);
}

/* Adds n cycles to the cycle counter */
void CPU::cycle()
{
    cycles++;
}

/* Executes last cyle polling, doesn't increment cycles */
void CPU::last_cycle()
{
    nmipoll();
    irqpoll();
}

/* Poll for the IRQ and NMI respectively. A poll is made on the penultimate
 * cycle of an instruction. */

void CPU::irqpoll()
{
    if (!execirq && !procstatus.intdis && irqpending)
        execirq = true;
}

void CPU::nmipoll()
{
    if (!execnmi && nmipending)
        execnmi = true;
}



/* Executes one whole fetch-decode-execute cycle, giving priority
 * to interrupt signals first. */
void CPU::main()
{
    if (execnmi) {
        cycle();
        interrupt();
        execnmi = false;
        return;
    }
    if (execirq) {
        cycle();
        interrupt();
        execirq = false;
        return;
    }
    curropcode = fetch();
    execute(curropcode);
}

/* Emulates the start/reset function of the 6502. */
void CPU::power()
{
    bus.map(0, 0x2000,
        [=](uint16 addr)             { cycle(); return mem[addr]; },
        [=](uint16 addr, uint8 data) { cycle(); mem[addr] = data; } );
    bus.map(0x2000, 0x2008,
        [=](uint16 addr)             { cycle(); return ppu->readreg(addr); },
        [=](uint16 addr, uint8 data) { cycle(); ppu->writereg(addr, data); });
    bus.map(0x2008, 0x8000,
        [=](uint16 addr)             { cycle(); return mem[addr]; },
        [=](uint16 addr, uint8 data) { cycle(); mem[addr] = data; } );
    bus.map(0x8000, 0xFFFF+1,
        [=](uint16 addr)             { cycle(); return cart->read_prgrom(addr); },
        [=](uint16 addr, uint8 data) { cycle(); return 0; });
    // prgrom.copy_to(memory+CPUMap::PRGROM_START,
    //  prgrom.getsize() - (CPUMap::PRGROM_SIZE+1), CPUMap::PRGROM_SIZE);
    // bus.write_enable = true;
    sp = 0;
    pc = 0;
    interrupt(true);
}

/* Sends an IRQ signal. Used by other devices. */
void CPU::fire_irq()
{
    irqpending = true;
}

/* Sends an NMI signal */
void CPU::fire_nmi()
{
    nmipending = true;
}

/* Simply resets registers and memory. */
void CPU::reset()
{
    pc = sp = accum = xreg = yreg = 0;
    procstatus.reset();
}

/* Prints info about the instruction which has just been executed and
 * the status of the registers. */
void CPU::printinfo(Util::File &lf) const
{
    if (!lf.isopen())
        return;

    lf.printf("PC: %04X A: %02X X: %02X Y: %02X S: %02X ",
                   pc.reg, accum, xreg, yreg, sp);

#define WRITEFLAG(f, c) lf.printf("%c", (f == 1) ? std::toupper(c) : std::tolower(c) )
    WRITEFLAG(procstatus.neg,     'n');
    WRITEFLAG(procstatus.ov,      'v');
    WRITEFLAG(procstatus.unused,  'u');
    WRITEFLAG(procstatus.breakf,  'b');
    WRITEFLAG(procstatus.decimal, 'd');
    WRITEFLAG(procstatus.intdis,  'i');
    WRITEFLAG(procstatus.zero,    'z');
    WRITEFLAG(procstatus.carry,   'c');
#undef WRITEFLAG

    lf.printf(" cycles: %d", cycles);
    lf.putc('\n');
}

} // namespace Core

#undef INSIDE_CPU_CPP
