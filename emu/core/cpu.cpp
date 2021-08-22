#include "cpu.hpp"

#include <emu/util/debug.hpp>

namespace core {

#define INSIDE_CPU_CPP
#include <emu/core/instructions.cpp>
#undef INSIDE_CPU_CPP

void CPU::power(bool reset)
{
    if (!reset) {
        r.acc = 0;
        r.x = 0;
        r.y = 0;
        r.sp = 0;
        r.pc = 0;
        r.flags.reset();
        for (uint16 i = 0; i < 0x800; i++)
            bus->write(i, 0);
        for (uint16 i = 0x4000; i < 0x4013; i++)
            bus->write(i, 0);
        bus->write(0x4017, 0);
    }
    r.flags.intdis = 1;
    r.cycles = 0;
    dma.flag = false;
    dma.page = 0;
    bus->write(0x4015, 0);
    // an interrupt is performed during 6502 start up. this is why SP = $FD.
    signal.resetpending = true;
    interrupt();
}

void CPU::run()
{
    if (signal.execnmi) {
        signal.execnmi = 0;
        cycle();
        interrupt();
        return;
    }
    if (signal.execirq) {
        signal.execirq = 0;
        cycle();
        interrupt();
        return;
    }
    if (dma.flag) {
        oamdma_loop(dma.page);
        return;
    }
    execute(fetch());
}

void CPU::fire_irq()
{
    signal.irqpending = true;
}

void CPU::fire_nmi()
{
    signal.nmipending = true;
}

uint8 CPU::fetch()
{
    if (fetch_callback)
        fetch_callback(r.pc.v, 'x');
    cycle();
    return bus->read(r.pc.v++);
}

// This is here mostly so we can differentiate between actual instructions
// and operand fetches. It's useful for the debugger.
uint8 CPU::fetchop()
{
    cycle();
    return bus->read(r.pc.v++);
}

void CPU::execute(uint8 instr)
{
#define INSTR_IMPLD(id, func) \
    case id: instr_##func(); return;
#define INSTR_AMODE(id, name, mode, type) \
    case id: addrmode_##mode##_##type(&CPU::instr_##name); return;
#define INSTR_WRITE(id, mode, val) \
    case id: addrmode_##mode##_write(val); return;
#define INSTR_OTHER(id, func, ...) \
    case id: instr_##func(__VA_ARGS__); return;

    switch(instr) {
        INSTR_IMPLD(0x00, brk)
        INSTR_AMODE(0x01, ora, indx, read)
        INSTR_AMODE(0x05, ora, zero, read)
        INSTR_AMODE(0x06, asl, zero, modify)
        INSTR_IMPLD(0x08, php)
        INSTR_AMODE(0x09, ora, imm, read)
        INSTR_AMODE(0x0A, asl, accum, modify)
        INSTR_AMODE(0x0D, ora, abs, read)
        INSTR_AMODE(0x0E, asl, abs, modify)
        INSTR_OTHER(0x10, branch, r.flags.neg == 0)     // bpl
        INSTR_AMODE(0x11, ora, indy, read)
        INSTR_AMODE(0x15, ora, zerox, read)
        INSTR_AMODE(0x16, asl, zerox, modify)
        INSTR_OTHER(0x18, flag, r.flags.carry, false)   // clc
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
        INSTR_OTHER(0x30, branch, r.flags.neg == 1)     // bmi
        INSTR_AMODE(0x31, and, indy, read)
        INSTR_AMODE(0x35, and, zerox, read)
        INSTR_AMODE(0x36, rol, zerox, modify)
        INSTR_OTHER(0x38, flag, r.flags.carry, true)    //sec
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
        INSTR_OTHER(0x50, branch, r.flags.ov == 0)      // bvc
        INSTR_AMODE(0x51, eor, indy, read)
        INSTR_AMODE(0x55, eor, zerox, read)
        INSTR_AMODE(0x56, lsr, zerox, modify)
        INSTR_OTHER(0x58, flag, r.flags.intdis, false)  //cli
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
        INSTR_OTHER(0x70, branch, r.flags.ov == 1)      // bvs
        INSTR_AMODE(0x71, adc, indy, read)
        INSTR_AMODE(0x75, adc, zerox, read)
        INSTR_AMODE(0x76, ror, zerox, modify)
        INSTR_OTHER(0x78, flag, r.flags.intdis, true)   //sei
        INSTR_AMODE(0x79, adc, absy, read)
        INSTR_AMODE(0x7D, adc, absx, read)
        INSTR_AMODE(0x7E, ror, absx, modify)
        INSTR_WRITE(0x81, indx, r.acc)                      // sta
        INSTR_WRITE(0x84, zero, r.y)                       // sty
        INSTR_WRITE(0x85, zero, r.acc)                      // sta
        INSTR_WRITE(0x86, zero, r.x)                       // stx
        INSTR_IMPLD(0x88, dey)
        INSTR_OTHER(0x8A, transfer, r.x, r.acc)            // txa
        INSTR_WRITE(0x8C, abs, r.y)                        // sty
        INSTR_WRITE(0x8D, abs, r.acc)                       // sta
        INSTR_WRITE(0x8E, abs, r.x)                        // stx
        INSTR_OTHER(0x90, branch, r.flags.carry == 0)    // bcc
        INSTR_WRITE(0x91, indy, r.acc)                      // sta
        INSTR_WRITE(0x94, zerox, r.y)                      // sty
        INSTR_WRITE(0x95, zerox, r.acc)                     // sta
        INSTR_WRITE(0x96, zeroy, r.x)                      // stx
        INSTR_OTHER(0x98, transfer, r.y, r.acc)            // tya
        INSTR_WRITE(0x99, absy, r.acc)                      // sta
        INSTR_OTHER(0x9A, transfer, r.x, r.sp)               // txs
        INSTR_WRITE(0x9D, absx, r.acc)                      // sta
        INSTR_AMODE(0xA0, ldy, imm, read)
        INSTR_AMODE(0xA1, lda, indx, read)
        INSTR_AMODE(0xA2, ldx, imm, read)
        INSTR_AMODE(0xA4, ldy, zero, read)
        INSTR_AMODE(0xA5, lda, zero, read)
        INSTR_AMODE(0xA6, ldx, zero, read)
        INSTR_OTHER(0xA8, transfer, r.acc, r.y)            // tay
        INSTR_AMODE(0xA9, lda, imm, read)
        INSTR_OTHER(0xAA, transfer, r.acc, r.x)            // tax
        INSTR_AMODE(0xAC, ldy, abs, read)
        INSTR_AMODE(0xAD, lda, abs, read)
        INSTR_AMODE(0xAE, ldx, abs, read)
        INSTR_OTHER(0xB0, branch, r.flags.carry == 1)    // bcs
        INSTR_AMODE(0xB1, lda, indy, read)
        INSTR_AMODE(0xB4, ldy, zerox, read)
        INSTR_AMODE(0xB5, lda, zerox, read)
        INSTR_AMODE(0xB6, ldx, zeroy, read)
        INSTR_OTHER(0xB8, flag, r.flags.ov, false)       // clv
        INSTR_AMODE(0xB9, lda, absy, read)
        INSTR_OTHER(0xBA, transfer, r.sp, r.x)               // tsx
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
        INSTR_OTHER(0xD0, branch, r.flags.zero == 0)    // bne
        INSTR_AMODE(0xD1, cmp, indy, read)
        INSTR_AMODE(0xD5, cmp, zerox, read)
        INSTR_AMODE(0xD6, dec, zerox, modify)
        INSTR_OTHER(0xD8, flag, r.flags.decimal, false) //cld
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
        INSTR_OTHER(0xF0, branch, r.flags.zero == 1)    // beq
        INSTR_AMODE(0xF1, sbc, indy, read)
        INSTR_AMODE(0xF5, sbc, zerox, read)
        INSTR_AMODE(0xF6, inc, zerox, modify)
        INSTR_OTHER(0xF8, flag, r.flags.decimal, true)   // sed
        INSTR_AMODE(0xF9, sbc, absy, read)
        INSTR_AMODE(0xFD, sbc, absx, read)
        INSTR_AMODE(0xFE, inc, absx, modify)
        default:
            if (error_callback)
                error_callback(instr, r.pc.v);
    }
#undef INSTR_IMPLD
#undef INSTR_AMODE
#undef INSTR_WRITE
#undef INSTR_OTHER
}

void CPU::interrupt()
{
    // one cycle for reading next instruction byte and throw away
    cycle();
    push(r.pc.h);
    push(r.pc.l);
    push((uint8) r.flags);
    // reset this here just in case
    r.flags.breakf = 0;
    r.flags.intdis = 1;
    // interrupt hijacking
    // reset is put at the top so that it will always run. i'm not sure if
    // this is the actual behavior - nesdev says nothing about it.
    uint16 vec;
    if (signal.resetpending) {
        signal.resetpending = false;
        vec = RESET_VEC;
    } else if (signal.nmipending) {
        signal.nmipending = false;
        vec = NMI_VEC;
    } else if (signal.irqpending) {
        signal.irqpending = false;
        vec = IRQ_BRK_VEC;
    } else
        vec = IRQ_BRK_VEC;
    r.pc.l = readmem(vec);
    r.pc.h = readmem(vec+1);
}

void CPU::push(uint8 val)
{
    writemem(r.sp + STACK_BASE, val);
    r.sp--;
}

uint8 CPU::pull()
{
    r.sp++;
    return readmem(r.sp + STACK_BASE);
}

void CPU::cycle()
{
    r.cycles++;
}

// NOTE: doesn't increment cycles!
void CPU::last_cycle()
{
    nmipoll();
    irqpoll();
}

void CPU::irqpoll()
{
    if (!signal.execirq && !r.flags.intdis && signal.irqpending)
        signal.execirq = true;
}

void CPU::nmipoll()
{
    if (!signal.execnmi && signal.nmipending)
        signal.execnmi = true;
}

uint8 CPU::readmem(uint16 addr)
{
    if (fetch_callback)
        fetch_callback(addr, 'r');
    cycle();
    return bus->read(addr);
}

void CPU::writemem(uint16 addr, uint8 data)
{
    if (fetch_callback)
        fetch_callback(addr, 'w');
    cycle();
    bus->write(addr, data);
}

/* These two functions read the registers located between 0x4000 - 0x4020. */
uint8 CPU::readreg(uint16 addr)
{
    switch (addr) {
    case 0x4000: case 0x4001: case 0x4002: case 0x4003: case 0x4004: case 0x4005: case 0x4006: case 0x4007:
    case 0x4008: case 0x400A: case 0x400B: case 0x400C: case 0x400E: case 0x400F: case 0x4010: case 0x4011:
    case 0x4012: case 0x4013: case 0x4014: case 0x4015: case 0x4016: case 0x4017:
        return 0;
    default:
        return 0;
    }
}

void CPU::writereg(uint16 addr, uint8 data)
{
    switch (addr) {

    // SQ1_VOL
    case 0x4000:
        break;

    // SQ1_SWEEP
    case 0x4001:
        break;

    // SQ1_LO
    case 0x4002:
        break;

    // SQ1_HI
    case 0x4003:
        break;

    // SQ2_VOL
    case 0x4004:
        break;

    // SQ2_SWEEP
    case 0x4005:
        break;

    // SQ2_LO
    case 0x4006:
        break;

    // SQ2_HI
    case 0x4007:
        break;

    // TRI_LINEAR
    case 0x4008:
        break;

    // TRI_LO
    case 0x400A:
        break;

    // TRI_HI
    case 0x400B:
        break;

    // NOISE_VOL
    case 0x400C:
        break;

    // NOISE_LO
    case 0x400E:
        break;

    // NOISE_HI
    case 0x400F:
        break;

    // DMC_FREQ
    case 0x4010:
        break;

    // DMC_RAW
    case 0x4011:
        break;

    // DMC_START
    case 0x4012:
        break;

    // DMC_LEN
    case 0x4013:
        break;

    // OAMDMA
    case 0x4014:
        dma.flag = true;
        dma.page = data;
        break;

    // SND_CHN
    case 0x4015:
        break;

    // JOY1
    case 0x4016:
        break;

    // JOY2
    case 0x4017:
        break;

    default:
#ifdef DEBUG
        if (addr < 0x4000 || addr > 0x4020)
            panic("wrong address passed to {}", __PRETTY_FUNCTION__);
#endif
        break;
    }
}

void CPU::oamdma_loop(uint8 page)
{
    uint16 start = uint16(page) << 8;
    uint16 end = uint16(page) << 8 | 0xFF;
    cycle();
    if (r.cycles % 2 == 1)
        cycle();
    while (start <= end)
        writemem(0x2004, readmem(start++));
    dma.flag = false;
}

} // namespace core
