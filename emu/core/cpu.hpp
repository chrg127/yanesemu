#pragma once

#include <functional>
#include <emu/core/const.hpp>
#include <emu/core/bus.hpp>
#include <emu/util/bits.hpp>
#include <emu/util/uint.hpp>

namespace debugger { class CPUDebugger; }

namespace core {

class CPU {

    struct {
        util::Word pc = 0;
        u8 acc = 0;
        u8 x   = 0;
        u8 y   = 0;
        u8 sp  = 0;
        struct {
            bool carry   = 0;
            bool zero    = 0;
            bool intdis  = 0;
            bool decimal = 0;
            bool breakf  = 0;
            bool unused  = 1;
            bool ov      = 0;
            bool neg     = 0;

            explicit operator u8() const
            {
                return carry  << 0  | zero   << 1  | intdis << 2 | decimal << 3 |
                       breakf << 4  | unused << 5  | ov     << 6 | neg     << 7;
            }

            void operator=(u8 data)
            {
                carry   = data & (1 << 0); zero    = data & (1 << 1); intdis  = data & (1 << 2);
                decimal = data & (1 << 3); breakf  = data & (1 << 4); unused  = data & (1 << 5);
                ov      = data & (1 << 6); neg     = data & (1 << 7);
            }
        } flags;
    } r;

    struct {
        bool nmi_pending   = false;
        bool irq_pending   = false;
        bool reset_pending = false;
        bool exec_nmi      = false;
        bool exec_irq      = false;
    } status;

    struct {
        bool flag;
        u8 page;
    } dma;

    unsigned long cpu_cycles = 0;
    Bus<CPUBUS_SIZE> *bus = nullptr;
    util::Word opargs = 0;
    std::function<void(u8, u16)> error_callback;

public:
    explicit CPU(Bus<CPUBUS_SIZE> *rambus) : bus(rambus) { }

    void power(bool reset = false);
    void run();
    u8 readreg(u16 addr);
    void writereg(u16 addr, u8 data);
    void fire_irq();
    void fire_nmi();

    unsigned long cycles() const { return cpu_cycles; }
    void on_error(auto &&f) { error_callback = f; }

    friend class debugger::CPUDebugger;

private:
    u8 fetch();
    u8 fetchop();
    void execute(u8 instr);
    void interrupt();
    void push(u8 val);
    u8 pull();
    void irqpoll();
    void nmipoll();
    void cycle();
    void last_cycle();
    u8 readmem(u16 addr);
    void writemem(u16 addr, u8 data);
    void oamdma_loop(u8 page);

    // instructions.cpp
    using InstrFuncRead = void (CPU::*)(u8);
    using InstrFuncMod = u8 (CPU::*)(u8);

    void addrmode_imm_read(InstrFuncRead f);
    void addrmode_zero_read(InstrFuncRead f);
    void addrmode_zero_ind_read(InstrFuncRead f, u8 reg);
    void addrmode_abs_read(InstrFuncRead f);
    void addrmode_abs_ind_read(InstrFuncRead f, u8 reg);
    void addrmode_indx_read(InstrFuncRead f);
    void addrmode_indy_read(InstrFuncRead f);

    void addrmode_accum_modify(InstrFuncMod f);
    void addrmode_zero_modify(InstrFuncMod f);
    void addrmode_zerox_modify(InstrFuncMod f);
    void addrmode_abs_modify(InstrFuncMod f);
    void addrmode_absx_modify(InstrFuncMod f);

    // used by sta, stx and sty
    void addrmode_zero_write(u8 val);
    void addrmode_zero_ind_write(u8 val, u8 reg);
    void addrmode_abs_write(u8 val);
    void addrmode_abs_ind_write(u8 val, u8 reg);
    void addrmode_indx_write(u8 val);
    void addrmode_indy_write(u8 val);

    /* instruction functions missing (as they are not needed):
     * - sta, stx, sty (use addrmode_write functions directly)
     * - beq, bne, bmi, bpl, bvc, bvs, bcc, bcs (use instr_branch)
     * - tax, txa, tay, tya, txs, tsx (use instr_transfer)
     * - sec, clc, sei, cli, clv, cld (use instr_flag) */
    void instr_load(u8 val, u8 &reg);
    void instr_compare(u8 val, u8 reg);
    void instr_inc_reg(u8 &reg);
    void instr_dec_reg(u8 &reg);

    void instr_branch(bool take);
    void instr_flag(bool &flag, bool v);
    void instr_transfer(u8 from, u8 &to);
    void instr_lda(u8 val);
    void instr_ldx(u8 val);
    void instr_ldy(u8 val);
    void instr_cmp(u8 val);
    void instr_cpx(u8 val);
    void instr_cpy(u8 val);
    void instr_adc(u8 val);
    void instr_sbc(u8 val);
    void instr_ora(u8 val);
    void instr_and(u8 val);
    void instr_eor(u8 val);
    void instr_bit(u8 val);
    u8 instr_inc(u8 val);
    u8 instr_dec(u8 val);
    u8 instr_asl(u8 val);
    u8 instr_lsr(u8 val);
    u8 instr_rol(u8 val);
    u8 instr_ror(u8 val);

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

} // namespace core
