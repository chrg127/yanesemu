#ifndef CORE_CPU_HPP_INCLUDED
#define CORE_CPU_HPP_INCLUDED

#include <string>
#include <emu/core/types.hpp>
#include <emu/core/memorymap.hpp>
#include <emu/core/genericbus.hpp>

namespace Util { class File; }

namespace Core {

class Cartridge;
class PPU;

class CPU {
    Cartridge *cart;
    PPU *ppu;

    Bus bus = Bus(0xFFFF+1);
    uint8 mem[0xFFFF+1];

    uint8 curropcode;
    Reg16 op;       // operand

    Reg16 pc;
    uint8 accum   = 0;
    uint8 xreg    = 0;
    uint8 yreg    = 0;
    uint8 sp      = 0;
    struct {
        bool carry      = 0;
        bool zero       = 0;
        bool intdis     = 0;
        bool decimal    = 0;
        bool breakf     = 0;
        bool unused     = 1;
        bool ov         = 0;
        bool neg        = 0;

        uint8 reg()
        {
            return carry  << 0  | zero   << 1  | intdis << 2 | decimal << 3 |
                   breakf << 4  | unused << 5  | ov     << 6 | neg     << 7;
        }

        inline void operator=(const uint8 data)
        {
            carry   = data & 0x01;
            zero    = data & 0x02;
            intdis  = data & 0x04;
            decimal = data & 0x08;
            breakf  = data & 0x10;
            unused  = data & 0x20;
            ov      = data & 0x40;
            neg     = data & 0x80;
        }

        void reset()
        {
            carry = zero = intdis = decimal = breakf = ov = neg = 0;
            unused = 1;
        }
    } procstatus;

    // interrupt signals
    int cycles      = 0;
    bool nmipending = false;
    bool execnmi    = false;
    bool irqpending = false;
    bool execirq    = false;

    uint8 fetch();
    void execute(uint8 opcode);
    void interrupt(bool reset = false);
    void push(uint8 val);
    uint8 pull();
    void cycle();
    void last_cycle();
    void irqpoll();
    void nmipoll();

    // inline uint8 readmem(uint16 addr)
    // {
    //     cycle();
    //     return bus.read(addr);
    // }

    // inline void writemem(uint16 addr, uint8 val)
    // {
    //     cycle();
    //     bus.write(addr, val);
    // }

    using InstrFuncRead = void (CPU::*)(const uint8);
    using InstrFuncMod = uint8 (CPU::*)(uint8);

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

public:
    void main();
    void power();
    void fire_irq();
    void fire_nmi();
    void reset();
    std::string disassemble() const;
    void printinfo(Util::File &f) const;

    inline int get_cycles()
    { return cycles; }

    inline const uint8 *getmemory() const
    { return mem; }

    inline uint8 peek_opcode() const
    { return curropcode; }

    inline void attach_ppu(PPU *p)
    { ppu = p; }

    inline void load_cartridge(Cartridge *c)
    { cart = c; }
};

} // namespace Core

#endif

