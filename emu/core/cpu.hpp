#ifndef CORE_CPU_HPP_INCLUDED
#define CORE_CPU_HPP_INCLUDED

#include <functional>
#include <string>
#include <emu/core/const.hpp>
#include <emu/core/bus.hpp>
#include <emu/core/opcodeinfo.hpp>
#include <emu/util/unsigned.hpp>

namespace Core {

class CPU {
    using InstrFuncRead = void (CPU::*)(const uint8);
    using InstrFuncMod = uint8 (CPU::*)(uint8);

    uint8 rammem[RAM_SIZE];
    Bus *bus = nullptr;
    unsigned long cycles = 0;
    std::function<void (uint16, char)> fetch_callback;

    // used in opcodes.cpp
    Reg16 opargs = 0;

    // registers
    Reg16 pc    = 0;
    uint8 accum = 0;
    uint8 xreg  = 0;
    uint8 yreg  = 0;
    uint8 sp    = 0;
    ProcStatus procstatus;

    // interrupt signals
    bool nmipending = false;
    bool irqpending = false;
    bool resetpending = false;
    bool execnmi    = false;
    bool execirq    = false;

public:
    void run();
    void power();
    void reset();
    void attach_bus(Bus *rambus);
    void fire_irq();
    void fire_nmi();
    // debugging
    Opcode nextopcode() const;
    uint16 nextaddr(const Opcode &op) const;
    Opcode disassemble() const;
    std::string status() const;

    unsigned long get_cycles() { return cycles; }
    void register_fetch_callback(auto &&callback) { fetch_callback = callback; }

private:
    uint8 fetch();
    uint8 fetcharg();
    void execute(uint8 opcode);
    void interrupt();
    void push(uint8 val);
    uint8 pull();
    void irqpoll();
    void nmipoll();
    void cycle();
    void last_cycle();

    // opcodes.cpp
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
    void addrmode_zeroy_modify(InstrFuncMod f);
    void addrmode_abs_modify(InstrFuncMod f);
    void addrmode_absx_modify(InstrFuncMod f);
    void addrmode_absy_modify(InstrFuncMod f);
    void addrmode_indx_modify(InstrFuncMod f);
    void addrmode_indy_modify(InstrFuncMod f);
    // these are only used by STA, STX, and STY
    void addrmode_zero_write(uint8 val);
    void addrmode_zerox_write(uint8 val);
    void addrmode_zeroy_write(uint8 val);
    void addrmode_abs_write(uint8 val);
    void addrmode_absx_write(uint8 val);
    void addrmode_absy_write(uint8 val);
    void addrmode_indx_write(uint8 val);
    void addrmode_indy_write(uint8 val);
    /*
     * opcode functions missing (as they are not needed):
     * - STA, STX, STY (use addrmode_write functions directly)
     * - BEQ, BNE, BMI, BPL, BVC, BVS, BCC, BCS (use instr_branch)
     * - TAX, TXA, TAY, TYA, TXS, TSX (use instr_transfer)
     * . SEC, CLC, SEI, CLI, CLV, CLD (use instr_flag)
     */
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

    uint8 readmem(uint16 addr);
    void writemem(uint16 addr, uint8 data);

    uint8 read_apu_reg(uint16 addr) { return 0; }
    void write_apu_reg(uint16 addr, uint8 data) { }

    friend class Debugger;
};

} // namespace Core

#endif

