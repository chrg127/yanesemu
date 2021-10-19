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
        uint8 acc = 0;
        uint8 x   = 0;
        uint8 y   = 0;
        uint8 sp  = 0;
        struct {
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

            void operator=(uint8 data)
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
        uint8 page;
    } dma;

    unsigned long cpu_cycles = 0;
    Bus<CPUBUS_SIZE> *bus = nullptr;
    util::Word opargs = 0;
    std::function<void(uint8, uint16)> error_callback;

public:
    explicit CPU(Bus<CPUBUS_SIZE> *rambus) : bus(rambus) { }

    void power(bool reset = false);
    void run();
    uint8 readreg(uint16 addr);
    void writereg(uint16 addr, uint8 data);
    void fire_irq();
    void fire_nmi();

    unsigned long cycles() const { return cpu_cycles; }
    void on_error(auto &&f) { error_callback = f; }

    friend class debugger::CPUDebugger;

private:
    uint8 fetch();
    uint8 fetchop();
    void execute(uint8 instr);
    void interrupt();
    void push(uint8 val);
    uint8 pull();
    void irqpoll();
    void nmipoll();
    void cycle();
    void last_cycle();
    uint8 readmem(uint16 addr);
    void writemem(uint16 addr, uint8 data);
    void oamdma_loop(uint8 page);

    // instructions.cpp
    using InstrFuncRead = void (CPU::*)(uint8);
    using InstrFuncMod = uint8 (CPU::*)(uint8);

    void addrmode_imm_read(InstrFuncRead f);
    void addrmode_zero_read(InstrFuncRead f);
    void addrmode_zero_ind_read(InstrFuncRead f, uint8 reg);
    void addrmode_abs_read(InstrFuncRead f);
    void addrmode_abs_ind_read(InstrFuncRead f, uint8 reg);
    void addrmode_indx_read(InstrFuncRead f);
    void addrmode_indy_read(InstrFuncRead f);

    void addrmode_accum_modify(InstrFuncMod f);
    void addrmode_zero_modify(InstrFuncMod f);
    void addrmode_zerox_modify(InstrFuncMod f);
    void addrmode_abs_modify(InstrFuncMod f);
    void addrmode_absx_modify(InstrFuncMod f);

    // used by sta, stx and sty
    void addrmode_zero_write(uint8 val);
    void addrmode_zero_ind_write(uint8 val, uint8 reg);
    void addrmode_abs_write(uint8 val);
    void addrmode_abs_ind_write(uint8 val, uint8 reg);
    void addrmode_indx_write(uint8 val);
    void addrmode_indy_write(uint8 val);

    /* instruction functions missing (as they are not needed):
     * - sta, stx, sty (use addrmode_write functions directly)
     * - beq, bne, bmi, bpl, bvc, bvs, bcc, bcs (use instr_branch)
     * - tax, txa, tay, tya, txs, tsx (use instr_transfer)
     * - sec, clc, sei, cli, clv, cld (use instr_flag) */
    void instr_load(uint8 val, uint8 &reg);
    void instr_compare(uint8 val, uint8 reg);
    void instr_inc_reg(uint8 &reg);
    void instr_dec_reg(uint8 &reg);

    void instr_branch(bool take);
    void instr_flag(bool &flag, bool v);
    void instr_transfer(uint8 from, uint8 &to);
    void instr_lda(uint8 val);
    void instr_ldx(uint8 val);
    void instr_ldy(uint8 val);
    void instr_cmp(uint8 val);
    void instr_cpx(uint8 val);
    void instr_cpy(uint8 val);
    void instr_adc(uint8 val);
    void instr_sbc(uint8 val);
    void instr_ora(uint8 val);
    void instr_and(uint8 val);
    void instr_eor(uint8 val);
    void instr_bit(uint8 val);
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

} // namespace core
