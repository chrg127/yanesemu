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
        r.x   = 0;
        r.y   = 0;
        r.sp  = 0;
        r.pc  = 0;
        r.flags = 0;
        r.flags.unused = 1;
    }
    r.flags.intdis = 1;
    cpu_cycles = 0;
    dma.flag = false;
    dma.page = 0;

    if (!reset) {
        for (uint16 i = 0; i < 0x800; i++)
            bus->write(i, 0);
    }

    // reset interrupt
    status.reset_pending = true;
    interrupt();
}

void CPU::run()
{
    if (status.exec_nmi) {
        status.exec_nmi = 0;
        cycle();
        interrupt();
        return;
    }
    if (status.exec_irq) {
        status.exec_irq = 0;
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
    status.irq_pending = true;
}

void CPU::fire_nmi()
{
    status.nmi_pending = true;
}

uint8 CPU::fetch()
{
    cycle();
    return bus->read(r.pc.v++);
}

void CPU::execute(uint8 instr)
{
#define INSTR_IMPLIED(id, func) \
    case id: instr_##func(); return;
#define INSTR_ADDRMODE(id, name, mode, type, ...) \
    case id: addrmode_##mode##_##type(&CPU::instr_##name __VA_OPT__(,) __VA_ARGS__); return;
#define INSTR_WRITE(id, name, mode, val, ...) \
    case id: addrmode_##mode##_write(val __VA_OPT__(,) __VA_ARGS__); return;
#define INSTR_OTHER(id, name, func, ...) \
    case id: instr_##func(__VA_ARGS__); return;

    switch(instr) {
        INSTR_IMPLIED (0x00, brk)
        INSTR_ADDRMODE(0x01, ora, indx, read)
        INSTR_ADDRMODE(0x05, ora, zero, read)
        INSTR_ADDRMODE(0x06, asl, zero, modify)
        INSTR_IMPLIED (0x08, php)
        INSTR_ADDRMODE(0x09, ora, imm, read)
        INSTR_ADDRMODE(0x0A, asl, accum, modify)
        INSTR_ADDRMODE(0x0D, ora, abs, read)
        INSTR_ADDRMODE(0x0E, asl, abs, modify)
        INSTR_OTHER   (0x10, bpl, branch, r.flags.neg == 0)
        INSTR_ADDRMODE(0x11, ora, indy, read)
        INSTR_ADDRMODE(0x15, ora, zero_ind, read, r.x)
        INSTR_ADDRMODE(0x16, asl, zerox, modify)
        INSTR_OTHER   (0x18, clc, flag, r.flags.carry, false)
        INSTR_ADDRMODE(0x19, ora, abs_ind, read, r.y)
        INSTR_ADDRMODE(0x1D, ora, abs_ind, read, r.x)
        INSTR_ADDRMODE(0x1E, asl, absx, modify)
        INSTR_IMPLIED (0x20, jsr)
        INSTR_ADDRMODE(0x21, and, indx, read)
        INSTR_ADDRMODE(0x24, bit, zero, read)
        INSTR_ADDRMODE(0x25, and, zero, read)
        INSTR_ADDRMODE(0x26, rol, zero, modify)
        INSTR_IMPLIED (0x28, plp)
        INSTR_ADDRMODE(0x29, and, imm, read)
        INSTR_ADDRMODE(0x2A, rol, accum, modify)
        INSTR_ADDRMODE(0x2C, bit, abs, read)
        INSTR_ADDRMODE(0x2D, and, abs, read)
        INSTR_ADDRMODE(0x2E, rol, abs, modify)
        INSTR_OTHER   (0x30, bmi, branch, r.flags.neg == 1)
        INSTR_ADDRMODE(0x31, and, indy, read)
        INSTR_ADDRMODE(0x35, and, zero_ind, read, r.x)
        INSTR_ADDRMODE(0x36, rol, zerox, modify)
        INSTR_OTHER   (0x38, sec, flag, r.flags.carry, true)
        INSTR_ADDRMODE(0x39, and, abs_ind, read, r.y)
        INSTR_ADDRMODE(0x3D, and, abs_ind, read, r.x)
        INSTR_ADDRMODE(0x3E, rol, absx, modify)
        INSTR_IMPLIED (0x40, rti)
        INSTR_ADDRMODE(0x41, eor, indx, read)
        INSTR_ADDRMODE(0x45, eor, zero, read)
        INSTR_ADDRMODE(0x46, lsr, zero, modify)
        INSTR_IMPLIED (0x48, pha)
        INSTR_ADDRMODE(0x49, eor, imm, read)
        INSTR_ADDRMODE(0x4A, lsr, accum, modify)
        INSTR_IMPLIED (0x4C, jmp)
        INSTR_ADDRMODE(0x4D, eor, abs, read)
        INSTR_ADDRMODE(0x4E, lsr, abs, modify)
        INSTR_OTHER   (0x50, bvc, branch, r.flags.ov == 0)
        INSTR_ADDRMODE(0x51, eor, indy, read)
        INSTR_ADDRMODE(0x55, eor, zero_ind, read, r.x)
        INSTR_ADDRMODE(0x56, lsr, zerox, modify)
        INSTR_OTHER   (0x58, cli, flag, r.flags.intdis, false)
        INSTR_ADDRMODE(0x59, eor, abs_ind, read, r.y)
        INSTR_ADDRMODE(0x5D, eor, abs_ind, read, r.x)
        INSTR_ADDRMODE(0x5E, lsr, absx, modify)
        INSTR_IMPLIED (0x60, rts)
        INSTR_ADDRMODE(0x61, adc, indx, read)
        INSTR_ADDRMODE(0x65, adc, zero, read)
        INSTR_ADDRMODE(0x66, ror, zero, modify)
        INSTR_IMPLIED (0x68, pla)
        INSTR_ADDRMODE(0x69, adc, imm, read)
        INSTR_ADDRMODE(0x6A, ror, accum, modify)
        INSTR_IMPLIED (0x6C, jmp_ind)
        INSTR_ADDRMODE(0x6D, adc, abs, read)
        INSTR_ADDRMODE(0x6E, ror, abs, modify)
        INSTR_OTHER   (0x70, bvs, branch, r.flags.ov == 1)
        INSTR_ADDRMODE(0x71, adc, indy, read)
        INSTR_ADDRMODE(0x75, adc, zero_ind, read, r.x)
        INSTR_ADDRMODE(0x76, ror, zerox, modify)
        INSTR_OTHER   (0x78, sei, flag, r.flags.intdis, true)
        INSTR_ADDRMODE(0x79, adc, abs_ind, read, r.y)
        INSTR_ADDRMODE(0x7D, adc, abs_ind, read, r.x)
        INSTR_ADDRMODE(0x7E, ror, absx, modify)
        INSTR_WRITE   (0x81, sta, indx, r.acc)
        INSTR_WRITE   (0x84, sty, zero, r.y)
        INSTR_WRITE   (0x85, sta, zero, r.acc)
        INSTR_WRITE   (0x86, stx, zero, r.x)
        INSTR_IMPLIED (0x88, dey)
        INSTR_OTHER   (0x8A, txa, transfer, r.x, r.acc)
        INSTR_WRITE   (0x8C, sty, abs, r.y)
        INSTR_WRITE   (0x8D, sta, abs, r.acc)
        INSTR_WRITE   (0x8E, stx, abs, r.x)
        INSTR_OTHER   (0x90, bcc, branch, r.flags.carry == 0)
        INSTR_WRITE   (0x91, sta, indy, r.acc)
        INSTR_WRITE   (0x94, sty, zero_ind, r.y, r.x)
        INSTR_WRITE   (0x95, sta, zero_ind, r.acc, r.x)
        INSTR_WRITE   (0x96, stx, zero_ind, r.x, r.y)
        INSTR_OTHER   (0x98, tya, transfer, r.y, r.acc)
        INSTR_WRITE   (0x99, sta, abs_ind, r.acc, r.y)
        INSTR_OTHER   (0x9A, txs, transfer, r.x, r.sp)
        INSTR_WRITE   (0x9D, sta, abs_ind, r.acc, r.x)
        INSTR_ADDRMODE(0xA0, ldy, imm, read)
        INSTR_ADDRMODE(0xA1, lda, indx, read)
        INSTR_ADDRMODE(0xA2, ldx, imm, read)
        INSTR_ADDRMODE(0xA4, ldy, zero, read)
        INSTR_ADDRMODE(0xA5, lda, zero, read)
        INSTR_ADDRMODE(0xA6, ldx, zero, read)
        INSTR_OTHER   (0xA8, tay, transfer, r.acc, r.y)
        INSTR_ADDRMODE(0xA9, lda, imm, read)
        INSTR_OTHER   (0xAA, tax, transfer, r.acc, r.x)
        INSTR_ADDRMODE(0xAC, ldy, abs, read)
        INSTR_ADDRMODE(0xAD, lda, abs, read)
        INSTR_ADDRMODE(0xAE, ldx, abs, read)
        INSTR_OTHER   (0xB0, bcs, branch, r.flags.carry == 1)
        INSTR_ADDRMODE(0xB1, lda, indy, read)
        INSTR_ADDRMODE(0xB4, ldy, zero_ind, read, r.x)
        INSTR_ADDRMODE(0xB5, lda, zero_ind, read, r.x)
        INSTR_ADDRMODE(0xB6, ldx, zero_ind, read, r.y)
        INSTR_OTHER   (0xB8, clv, flag, r.flags.ov, false)
        INSTR_ADDRMODE(0xB9, lda, abs_ind, read, r.y)
        INSTR_OTHER   (0xBA, tsx, transfer, r.sp, r.x)
        INSTR_ADDRMODE(0xBC, ldy, abs_ind, read, r.x)
        INSTR_ADDRMODE(0xBD, lda, abs_ind, read, r.x)
        INSTR_ADDRMODE(0xBE, ldx, abs_ind, read, r.y)
        INSTR_ADDRMODE(0xC0, cpy, imm, read)
        INSTR_ADDRMODE(0xC1, cmp, indx, read)
        INSTR_ADDRMODE(0xC4, cpy, zero, read)
        INSTR_ADDRMODE(0xC5, cmp, zero, read)
        INSTR_ADDRMODE(0xC6, dec, zero, modify)
        INSTR_IMPLIED (0xC8, iny)
        INSTR_ADDRMODE(0xC9, cmp, imm, read)
        INSTR_IMPLIED (0xCA, dex)
        INSTR_ADDRMODE(0xCC, cpy, abs, read)
        INSTR_ADDRMODE(0xCD, cmp, abs, read)
        INSTR_ADDRMODE(0xCE, dec, abs, modify)
        INSTR_OTHER   (0xD0, bne, branch, r.flags.zero == 0)
        INSTR_ADDRMODE(0xD1, cmp, indy, read)
        INSTR_ADDRMODE(0xD5, cmp, zero_ind, read, r.x)
        INSTR_ADDRMODE(0xD6, dec, zerox, modify)
        INSTR_OTHER   (0xD8, cld, flag, r.flags.decimal, false)
        INSTR_ADDRMODE(0xD9, cmp, abs_ind, read, r.y)
        INSTR_ADDRMODE(0xDD, cmp, abs_ind, read, r.x)
        INSTR_ADDRMODE(0xDE, dec, absx, modify)
        INSTR_ADDRMODE(0xE0, cpx, imm, read)
        INSTR_ADDRMODE(0xE1, sbc, indx, read)
        INSTR_ADDRMODE(0xE4, cpx, zero, read)
        INSTR_ADDRMODE(0xE5, sbc, zero, read)
        INSTR_ADDRMODE(0xE6, inc, zero, modify)
        INSTR_IMPLIED (0xE8, inx)
        INSTR_ADDRMODE(0xE9, sbc, imm, read)
        INSTR_IMPLIED (0xEA, nop)
        INSTR_ADDRMODE(0xEC, cpx, abs, read)
        INSTR_ADDRMODE(0xED, sbc, abs, read)
        INSTR_ADDRMODE(0xEE, inc, abs, modify)
        INSTR_OTHER   (0xF0, beq, branch, r.flags.zero == 1)
        INSTR_ADDRMODE(0xF1, sbc, indy, read)
        INSTR_ADDRMODE(0xF5, sbc, zero_ind, read, r.x)
        INSTR_ADDRMODE(0xF6, inc, zerox, modify)
        INSTR_OTHER   (0xF8, sed, flag, r.flags.decimal, true)
        INSTR_ADDRMODE(0xF9, sbc, abs_ind, read, r.y)
        INSTR_ADDRMODE(0xFD, sbc, abs_ind, read, r.x)
        INSTR_ADDRMODE(0xFE, inc, absx, modify)
        default:
            if (error_callback)
                error_callback(instr, r.pc.v);
    }
#undef INSTR_IMPLIED
#undef INSTR_ADDRMODE
#undef INSTR_WRITE
#undef INSTR_OTHER
}

void CPU::interrupt()
{
    // one cycle for reading next instruction byte and throw away
    cycle();
    push(r.pc.h);
    push(r.pc.l);
    push(uint8(r.flags));
    // reset this here just in case
    r.flags.breakf = 0;
    r.flags.intdis = 1;
    // interrupt hijacking
    // reset is put at the top so that it will always run. i'm not sure if
    // this is the actual behavior - nesdev says nothing about it.
    uint16 vec;
    if (status.reset_pending) {
        status.reset_pending = false;
        vec = RESET_VEC;
    } else if (status.nmi_pending) {
        status.nmi_pending = false;
        vec = NMI_VEC;
    } else if (status.irq_pending) {
        status.irq_pending = false;
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
    cpu_cycles++;
}

// NOTE: doesn't increment cycles!
void CPU::last_cycle()
{
    nmipoll();
    irqpoll();
}

void CPU::irqpoll()
{
    if (!status.exec_irq && !r.flags.intdis && status.irq_pending)
        status.exec_irq = true;
}

void CPU::nmipoll()
{
    if (!status.exec_nmi && status.nmi_pending)
        status.exec_nmi = true;
}

uint8 CPU::readmem(uint16 addr)
{
    cycle();
    return bus->read(addr);
}

void CPU::writemem(uint16 addr, uint8 data)
{
    cycle();
    bus->write(addr, data);
}

/* These two functions read the registers located between 0x4000 - 0x4020. */
uint8 CPU::readreg(uint16 addr)
{
    switch (addr) {
    case 0x4000: case 0x4001: case 0x4002: case 0x4003: case 0x4004: case 0x4005: case 0x4006: case 0x4007:
    case 0x4008: case 0x400A: case 0x400B: case 0x400C: case 0x400E: case 0x400F: case 0x4010: case 0x4011:
    case 0x4012: case 0x4013: case 0x4014: case 0x4015:
        return 0;
    case 0x4016:
        return port1->device->read();
    case 0x4017:
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
        port1->device->latch(data & 1);
        break;

    // JOY2
    case 0x4017:
        break;

    default:
#ifdef DEBUG
        if (addr < 0x4000 || addr > 0x4020)
            panic("at {}\n", __func__);
#endif
        break;
    }
}

void CPU::oamdma_loop(uint8 page)
{
    uint16 start = uint16(page) << 8;
    uint16 end = uint16(page) << 8 | 0xFF;
    cycle();
    if (cpu_cycles % 2 == 1)
        cycle();
    while (start <= end)
        writemem(0x2004, readmem(start++));
    dma.flag = false;
}

} // namespace core
