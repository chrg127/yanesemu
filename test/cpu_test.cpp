#include <emu/core/cpu.hpp>
#include <emu/core/bus.hpp>
#include <emu/core/controller.hpp>
#include <catch2/catch.hpp>

using namespace core;

struct CPUTest {
    Bus<CPUBUS_SIZE> bus;
    ControllerPort port;
    CPU cpu{&bus, &port};
    std::array<u8, CPUBUS_SIZE> mem;

    CPUTest()
    {
        bus.map(0, 0xffff, [&](u16 addr) { return mem[addr]; }, [&](u16 addr, u8 data) { mem[addr] = data; });
    }

    int get_cycles(u8 id, u8 low, u8 high)
    {
        cpu.r.acc  = 0; cpu.r.x  = 0; cpu.r.y = 0;
        cpu.r.pc.v = 0; cpu.r.sp = 0;
        int curr = cpu.cycles();
        cpu.run_instr(id, low, high);
        return cpu.cycles() - curr;
    }

    void test_cycles();
};

METHOD_AS_TEST_CASE(CPUTest::test_cycles, "cycles");
void CPUTest::test_cycles()
{
    REQUIRE(get_cycles(0x00, 0, 0) == 7); // brk
    REQUIRE(get_cycles(0x01, 0, 0) == 6); // ora, indx
    REQUIRE(get_cycles(0x05, 0, 0) == 3); // ora, zero
    REQUIRE(get_cycles(0x06, 0, 0) == 5); // asl, zero
    REQUIRE(get_cycles(0x08, 0, 0) == 3); // php
    REQUIRE(get_cycles(0x09, 0, 0) == 2); // ora, imm
    REQUIRE(get_cycles(0x0A, 0, 0) == 2); // asl, accum
    REQUIRE(get_cycles(0x0D, 0, 0) == 4); // ora, abs
    REQUIRE(get_cycles(0x0E, 0, 0) == 6); // asl, abs
    REQUIRE(get_cycles(0x11, 0, 0) == 5); // ora, indy
    REQUIRE(get_cycles(0x15, 0, 0) == 4); // ora, zerox
    REQUIRE(get_cycles(0x16, 0, 0) == 6); // asl, zerox
    REQUIRE(get_cycles(0x18, 0, 0) == 2); // clc
    REQUIRE(get_cycles(0x19, 0, 0) == 4); // ora, absy
    REQUIRE(get_cycles(0x1D, 0, 0) == 4); // ora, absx
    REQUIRE(get_cycles(0x1E, 0, 0) == 7); // asl, absx
    REQUIRE(get_cycles(0x20, 0, 0) == 6); // jsr
    REQUIRE(get_cycles(0x21, 0, 0) == 6); // and, indx
    REQUIRE(get_cycles(0x24, 0, 0) == 3); // bit, zero
    REQUIRE(get_cycles(0x25, 0, 0) == 3); // and, zero
    REQUIRE(get_cycles(0x26, 0, 0) == 5); // rol, zero
    REQUIRE(get_cycles(0x28, 0, 0) == 4); // plp
    REQUIRE(get_cycles(0x29, 0, 0) == 2); // and, imm
    REQUIRE(get_cycles(0x2A, 0, 0) == 2); // rol, accum
    REQUIRE(get_cycles(0x2C, 0, 0) == 4); // bit, abs
    REQUIRE(get_cycles(0x2D, 0, 0) == 4); // and, abs
    REQUIRE(get_cycles(0x2E, 0, 0) == 6); // rol, abs
    REQUIRE(get_cycles(0x31, 0, 0) == 5); // and, indy
    REQUIRE(get_cycles(0x35, 0, 0) == 4); // and, zerox
    REQUIRE(get_cycles(0x36, 0, 0) == 6); // rol, zerox
    REQUIRE(get_cycles(0x38, 0, 0) == 2); // sec
    REQUIRE(get_cycles(0x39, 0, 0) == 4); // and, absy
    REQUIRE(get_cycles(0x3D, 0, 0) == 4); // and, absx
    REQUIRE(get_cycles(0x3E, 0, 0) == 7); // rol, absx
    REQUIRE(get_cycles(0x40, 0, 0) == 6); // rti
    REQUIRE(get_cycles(0x41, 0, 0) == 6); // eor, indx
    REQUIRE(get_cycles(0x45, 0, 0) == 3); // eor, zero
    REQUIRE(get_cycles(0x46, 0, 0) == 5); // lsr, zero
    REQUIRE(get_cycles(0x48, 0, 0) == 3); // pha
    REQUIRE(get_cycles(0x49, 0, 0) == 2); // eor, imm
    REQUIRE(get_cycles(0x4A, 0, 0) == 2); // lsr, accum
    REQUIRE(get_cycles(0x4C, 0, 0) == 3); // jmp
    REQUIRE(get_cycles(0x4D, 0, 0) == 4); // eor, abs
    REQUIRE(get_cycles(0x4E, 0, 0) == 6); // lsr, abs
    REQUIRE(get_cycles(0x51, 0, 0) == 5); // eor, indy
    REQUIRE(get_cycles(0x55, 0, 0) == 4); // eor, zerox
    REQUIRE(get_cycles(0x56, 0, 0) == 6); // lsr, zerox
    REQUIRE(get_cycles(0x58, 0, 0) == 2); // cli
    REQUIRE(get_cycles(0x59, 0, 0) == 4); // eor, absy
    REQUIRE(get_cycles(0x5D, 0, 0) == 4); // eor, absx
    REQUIRE(get_cycles(0x5E, 0, 0) == 7); // lsr, absx
    REQUIRE(get_cycles(0x60, 0, 0) == 6); // rts
    REQUIRE(get_cycles(0x61, 0, 0) == 6); // adc, indx
    REQUIRE(get_cycles(0x65, 0, 0) == 3); // adc, zero
    REQUIRE(get_cycles(0x66, 0, 0) == 5); // ror, zero
    REQUIRE(get_cycles(0x68, 0, 0) == 4); // pla
    REQUIRE(get_cycles(0x69, 0, 0) == 2); // adc, imm
    REQUIRE(get_cycles(0x6A, 0, 0) == 2); // ror, accum
    REQUIRE(get_cycles(0x6C, 0, 0) == 5); // jmp_ind
    REQUIRE(get_cycles(0x6D, 0, 0) == 4); // adc, abs
    REQUIRE(get_cycles(0x6E, 0, 0) == 6); // ror, abs
    REQUIRE(get_cycles(0x71, 0, 0) == 5); // adc, indy
    REQUIRE(get_cycles(0x75, 0, 0) == 4); // adc, zerox
    REQUIRE(get_cycles(0x76, 0, 0) == 6); // ror, zerox
    REQUIRE(get_cycles(0x78, 0, 0) == 2); // sei, flag
    REQUIRE(get_cycles(0x79, 0, 0) == 4); // adc, absy
    REQUIRE(get_cycles(0x7D, 0, 0) == 4); // adc, absx
    REQUIRE(get_cycles(0x7E, 0, 0) == 7); // ror, absx
    REQUIRE(get_cycles(0x81, 0, 0) == 6); // sta, indx
    REQUIRE(get_cycles(0x84, 0, 0) == 3); // sty, zero
    REQUIRE(get_cycles(0x85, 0, 0) == 3); // sta, zero
    REQUIRE(get_cycles(0x86, 0, 0) == 3); // stx, zero
    REQUIRE(get_cycles(0x88, 0, 0) == 2); // dey
    REQUIRE(get_cycles(0x8A, 0, 0) == 2); // txa
    REQUIRE(get_cycles(0x8C, 0, 0) == 4); // sty, abs
    REQUIRE(get_cycles(0x8D, 0, 0) == 4); // sta, abs
    REQUIRE(get_cycles(0x8E, 0, 0) == 4); // stx, abs
    REQUIRE(get_cycles(0x91, 0, 0) == 6); // sta, indy
    REQUIRE(get_cycles(0x94, 0, 0) == 4); // sty, zeroy
    REQUIRE(get_cycles(0x95, 0, 0) == 4); // sta, zerox
    REQUIRE(get_cycles(0x96, 0, 0) == 4); // stx, zeroy
    REQUIRE(get_cycles(0x98, 0, 0) == 2); // tya
    REQUIRE(get_cycles(0x99, 0, 0) == 5); // sta, absy
    REQUIRE(get_cycles(0x9A, 0, 0) == 2); // txs
    REQUIRE(get_cycles(0x9D, 0, 0) == 5); // sta, absx
    REQUIRE(get_cycles(0xA0, 0, 0) == 2); // ldy, imm
    REQUIRE(get_cycles(0xA1, 0, 0) == 6); // lda, indx
    REQUIRE(get_cycles(0xA2, 0, 0) == 2); // ldx, imm
    REQUIRE(get_cycles(0xA4, 0, 0) == 3); // ldy, zero
    REQUIRE(get_cycles(0xA5, 0, 0) == 3); // lda, zero
    REQUIRE(get_cycles(0xA6, 0, 0) == 3); // ldx, zero
    REQUIRE(get_cycles(0xA8, 0, 0) == 2); // tay
    REQUIRE(get_cycles(0xA9, 0, 0) == 2); // lda, imm
    REQUIRE(get_cycles(0xAA, 0, 0) == 2); // tax
    REQUIRE(get_cycles(0xAC, 0, 0) == 4); // ldy, abs
    REQUIRE(get_cycles(0xAD, 0, 0) == 4); // lda, abs
    REQUIRE(get_cycles(0xAE, 0, 0) == 4); // ldx, abs
    REQUIRE(get_cycles(0xB1, 0, 0) == 5); // lda, indy
    REQUIRE(get_cycles(0xB4, 0, 0) == 4); // ldy, zerox
    REQUIRE(get_cycles(0xB5, 0, 0) == 4); // lda, zerox
    REQUIRE(get_cycles(0xB6, 0, 0) == 4); // ldx, zeroy
    REQUIRE(get_cycles(0xB8, 0, 0) == 2); // clv
    REQUIRE(get_cycles(0xB9, 0, 0) == 4); // lda, absy
    REQUIRE(get_cycles(0xBA, 0, 0) == 2); // tsx
    REQUIRE(get_cycles(0xBC, 0, 0) == 4); // ldy, absx
    REQUIRE(get_cycles(0xBD, 0, 0) == 4); // lda, absx
    REQUIRE(get_cycles(0xBE, 0, 0) == 4); // ldx, absy
    REQUIRE(get_cycles(0xC0, 0, 0) == 2); // cpy, imm
    REQUIRE(get_cycles(0xC1, 0, 0) == 6); // cmp, indx
    REQUIRE(get_cycles(0xC4, 0, 0) == 3); // cpy, zero
    REQUIRE(get_cycles(0xC5, 0, 0) == 3); // cmp, zero
    REQUIRE(get_cycles(0xC6, 0, 0) == 5); // dec, zero
    REQUIRE(get_cycles(0xC8, 0, 0) == 2); // iny
    REQUIRE(get_cycles(0xC9, 0, 0) == 2); // cmp, imm
    REQUIRE(get_cycles(0xCA, 0, 0) == 2); // dex
    REQUIRE(get_cycles(0xCC, 0, 0) == 4); // cpy, abs
    REQUIRE(get_cycles(0xCD, 0, 0) == 4); // cmp, abs
    REQUIRE(get_cycles(0xCE, 0, 0) == 6); // dec, abs
    REQUIRE(get_cycles(0xD1, 0, 0) == 5); // cmp, indy
    REQUIRE(get_cycles(0xD5, 0, 0) == 4); // cmp, zerox
    REQUIRE(get_cycles(0xD6, 0, 0) == 6); // dec, zerox
    REQUIRE(get_cycles(0xD8, 0, 0) == 2); // cld
    REQUIRE(get_cycles(0xD9, 0, 0) == 4); // cmp, absy
    REQUIRE(get_cycles(0xDD, 0, 0) == 4); // cmp, absx
    REQUIRE(get_cycles(0xDE, 0, 0) == 7); // dec, absx
    REQUIRE(get_cycles(0xE0, 0, 0) == 2); // cpx, imm
    REQUIRE(get_cycles(0xE1, 0, 0) == 6); // sbc, indx
    REQUIRE(get_cycles(0xE4, 0, 0) == 3); // cpx, zero
    REQUIRE(get_cycles(0xE5, 0, 0) == 3); // sbc, zero
    REQUIRE(get_cycles(0xE6, 0, 0) == 5); // inc, zero
    REQUIRE(get_cycles(0xE8, 0, 0) == 2); // inx
    REQUIRE(get_cycles(0xE9, 0, 0) == 2); // sbc, imm
    REQUIRE(get_cycles(0xEA, 0, 0) == 2); // nop
    REQUIRE(get_cycles(0xEC, 0, 0) == 4); // cpx, abs
    REQUIRE(get_cycles(0xED, 0, 0) == 4); // sbc, abs
    REQUIRE(get_cycles(0xEE, 0, 0) == 6); // inc, abs
    REQUIRE(get_cycles(0xF1, 0, 0) == 5); // sbc, indy
    REQUIRE(get_cycles(0xF5, 0, 0) == 4); // sbc, zerox
    REQUIRE(get_cycles(0xF6, 0, 0) == 6); // inc, zerox
    REQUIRE(get_cycles(0xF8, 0, 0) == 2); // sed
    REQUIRE(get_cycles(0xF9, 0, 0) == 4); // sbc, absy
    REQUIRE(get_cycles(0xFD, 0, 0) == 4); // sbc, absx
    REQUIRE(get_cycles(0xFE, 0, 0) == 7); // inc, absx

    cpu.r.flags = 0;
    cpu.r.flags.neg   = 1; REQUIRE(get_cycles(0x10, 0, 0) == 2); // bpl
    cpu.r.flags.neg   = 0; REQUIRE(get_cycles(0x10, 0, 0) == 3); // bpl
    cpu.r.flags.neg   = 0; REQUIRE(get_cycles(0x30, 0, 0) == 2); // bmi
    cpu.r.flags.neg   = 1; REQUIRE(get_cycles(0x30, 0, 0) == 3); // bmi
    cpu.r.flags.ov    = 1; REQUIRE(get_cycles(0x50, 0, 0) == 2); // bvc
    cpu.r.flags.ov    = 0; REQUIRE(get_cycles(0x50, 0, 0) == 3); // bvc
    cpu.r.flags.ov    = 0; REQUIRE(get_cycles(0x70, 0, 0) == 2); // bvs
    cpu.r.flags.ov    = 1; REQUIRE(get_cycles(0x70, 0, 0) == 3); // bvs
    cpu.r.flags.carry = 1; REQUIRE(get_cycles(0x90, 0, 0) == 2); // bcc
    cpu.r.flags.carry = 0; REQUIRE(get_cycles(0x90, 0, 0) == 3); // bcc
    cpu.r.flags.carry = 0; REQUIRE(get_cycles(0xB0, 0, 0) == 2); // bcs
    cpu.r.flags.carry = 1; REQUIRE(get_cycles(0xB0, 0, 0) == 3); // bcs
    cpu.r.flags.zero  = 1; REQUIRE(get_cycles(0xD0, 0, 0) == 2); // bne
    cpu.r.flags.zero  = 0; REQUIRE(get_cycles(0xD0, 0, 0) == 3); // bne
    cpu.r.flags.zero  = 0; REQUIRE(get_cycles(0xF0, 0, 0) == 2); // beq
    cpu.r.flags.zero  = 1; REQUIRE(get_cycles(0xF0, 0, 0) == 3); // beq
}
