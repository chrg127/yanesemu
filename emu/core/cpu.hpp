#pragma once

#include <functional>
#include <emu/core/const.hpp>
#include <emu/core/bus.hpp>
#include <emu/util/bits.hpp>
#include <emu/util/uint.hpp>

namespace Debugger {
    class Debugger;
    class CPUDebugger;
}

namespace Core {

/* The CPU has 2 main states:
 * - When it is constructed, all members are initialized to 0, except
 *   for the following:
 *   - bus, which is not mapped;
 *   - fetch_callback, which is a function containing nothing
 * - On power(), every member is initialized as if a power signal was sent to
 *   the CPU.
 * To initialize the CPU, one therefore must first create a bus before creating
 * the CPU, then power() must be called.
 */
class CPU {
    struct ProcStatus {
        bool carry   = 0;
        bool zero    = 0;
        bool intdis  = 0;
        bool decimal = 0;
        bool breakf  = 0;
        bool unused  = 1;
        bool ov      = 0;
        bool neg     = 0;

        explicit operator uint8() const
        {
            return carry  << 0  | zero   << 1  | intdis << 2 | decimal << 3 |
                   breakf << 4  | unused << 5  | ov     << 6 | neg     << 7;
        }

        ProcStatus & operator=(uint8 data)
        {
            carry   = data & 0x01;
            zero    = data & 0x02;
            intdis  = data & 0x04;
            decimal = data & 0x08;
            breakf  = data & 0x10;
            unused  = data & 0x20;
            ov      = data & 0x40;
            neg     = data & 0x80;
            return *this;
        }

        void reset()
        {
            carry = zero = intdis = decimal = breakf = ov = neg = 0;
            unused = 1;
        }
    };

    struct {
        util::Word pc  = 0;
        uint8 acc = 0;
        uint8 x   = 0;
        uint8 y   = 0;
        uint8 sp  = 0;
        ProcStatus flags;
        unsigned long cycles = 0;
    } r;

    struct {
        bool nmipending = false;
        bool irqpending = false;
        bool resetpending = false;
        bool execnmi    = false;
        bool execirq    = false;
    } signal;

    struct {
        bool flag;
        uint8 page;
    } dma;

    Bus<CPUBUS_SIZE> *bus = nullptr;
    util::Word opargs = 0;
    std::function<void(uint16, char)> fetch_callback;
    std::function<void(uint8, uint16)> error_callback;

public:
    explicit CPU(Bus<CPUBUS_SIZE> *rambus)
        : bus(rambus)
    { }

    void power(bool reset = false);
    void run();
    uint8 readreg(uint16 addr);
    void writereg(uint16 addr, uint8 data);
    void fire_irq();
    void fire_nmi();

    unsigned long cycles() const { return r.cycles; }
    void on_fetch(auto &&f) { fetch_callback = f; }
    void on_error(auto &&f) { error_callback = f; }

    friend class Debugger::Debugger;
    friend class Debugger::CPUDebugger;

private:
    uint8 fetch();
    uint8 fetchop();
    void  execute(uint8 instr);
    void  interrupt();
    void  push(uint8 val);
    uint8 pull();
    void  irqpoll();
    void  nmipoll();
    void  cycle();
    void  last_cycle();
    uint8 readmem(uint16 addr);
    void  writemem(uint16 addr, uint8 data);
    void oamdma_loop(uint8 page);

    // instructions.cpp
    using InstrFuncRead = void (CPU::*)(const uint8);
    using InstrFuncMod = uint8 (CPU::*)(uint8);

    void addrmode_imm_read(InstrFuncRead f);
    void addrmode_zero_read(InstrFuncRead f);
    void addrmode_zerox_read(InstrFuncRead f);
    void addrmode_zeroy_read(InstrFuncRead f);
    void addrmode_abs_read(InstrFuncRead f);
    void addrmode_absx_read(InstrFuncRead f);
    void addrmode_absy_read(InstrFuncRead f);
    void addrmode_indx_read(InstrFuncRead f);
    void addrmode_indy_read(InstrFuncRead f);
    void addrmode_accum_modify(InstrFuncMod f);
    void addrmode_zero_modify(InstrFuncMod f);
    void addrmode_zerox_modify(InstrFuncMod f);
    void addrmode_abs_modify(InstrFuncMod f);
    void addrmode_absx_modify(InstrFuncMod f);

    // these are only used by STA, STX, and STY
    void addrmode_zero_write(uint8 val);
    void addrmode_zerox_write(uint8 val);
    void addrmode_zeroy_write(uint8 val);
    void addrmode_abs_write(uint8 val);
    void addrmode_absx_write(uint8 val);
    void addrmode_absy_write(uint8 val);
    void addrmode_indx_write(uint8 val);
    void addrmode_indy_write(uint8 val);

    /* instruction functions missing (as they are not needed):
     * - STA, STX, STY (use addrmode_write functions directly)
     * - BEQ, BNE, BMI, BPL, BVC, BVS, BCC, BCS (use instr_branch)
     * - TAX, TXA, TAY, TYA, TXS, TSX (use instr_transfer)
     * . SEC, CLC, SEI, CLI, CLV, CLD (use instr_flag) */
    void instr_branch(bool take);
    void instr_flag(bool &flag, bool v);
    void instr_transfer(uint8 from, uint8 &to);
    void instr_lda(const uint8 val);
    void instr_ldx(const uint8 val);
    void instr_ldy(const uint8 val);
    void instr_cmp(const uint8 val);
    void instr_cpx(const uint8 val);
    void instr_cpy(const uint8 val);
    void instr_adc(const uint8 val);
    void instr_sbc(const uint8 val);
    void instr_ora(const uint8 val);
    void instr_and(const uint8 val);
    void instr_eor(const uint8 val);
    void instr_bit(const uint8 val);
    uint8 instr_inc(uint8 val);
    uint8 instr_dec(uint8 val);
    uint8 instr_asl(uint8 val);
    uint8 instr_lsr(uint8 val);
    uint8 instr_rol(uint8 val);
    uint8 instr_ror(uint8 val);

    // these instruction are called directly
    void instr_inx();
    void instr_iny();
    void instr_dex();
    void instr_dey();
    void instr_php();
    void instr_pha();
    void instr_plp();
    void instr_pla();
    void instr_jsr();
    void instr_jmp();
    void instr_jmp_ind();
    void instr_rts();
    void instr_brk();
    void instr_rti();
    void instr_nop();
};

} // namespace Core
