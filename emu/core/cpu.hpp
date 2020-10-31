#ifndef CORE_CPU_HPP_INCLUDED
#define CORE_CPU_HPP_INCLUDED

#include <cstddef>
#include <string>
#include <emu/core/types.hpp>
#include <emu/core/memorymap.hpp>
#include <emu/core/ppu.hpp>

namespace Utils { class File; }

namespace Core {

class CPU {

    struct Bus {
        uint8_t memory[CPUMap::MEMSIZE];
        bool write_enable = false;
        PPU *ppu;

        void init(const ROM &prgrom);
        uint8_t read(uint16_t addr);
        void write(uint16_t addr, uint8_t val);
        void reset()
        { }
        const uint8_t *getmemory() const
        { return memory; }
    } bus;

    uint8_t curropcode;
    Reg16 op;       // operand
    Reg16 result;   // for results in addrmode_* functions
    uint16_t pc_branch; // save this for disassembling branch instructions

    Reg16 pc;
    uint8_t accum   = 0;
    uint8_t xreg    = 0;
    uint8_t yreg    = 0;
    uint8_t sp      = 0;
    struct {
        bool carry      = 0;
        bool zero       = 0;
        bool intdis     = 0;
        bool decimal    = 0;
        bool breakf     = 0;
        bool unused     = 1;
        bool ov         = 0;
        bool neg        = 0;

        uint8_t reg()
        {
            return carry  << 0  | zero   << 1  | intdis << 2 | decimal << 3 |
                   breakf << 4  | unused << 5  | ov     << 6 | neg     << 7;
        }

        inline void operator=(const uint8_t data)
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

    uint8_t fetch();
    void execute(uint8_t opcode);
    void interrupt(bool reset = false);
    void push(uint8_t val);
    uint8_t pull();
    void cycle();
    void last_cycle();
    void irqpoll();
    void nmipoll();

    inline uint8_t readmem(uint16_t addr)
    {
        cycle();
        return bus.read(addr);
    }

    inline void writemem(uint16_t addr, uint8_t val)
    {
        cycle();
        bus.write(addr, val);
    }

    using InstrFuncRead = void (CPU::*)(const uint8_t);
    using InstrFuncMod = uint8_t (CPU::*)(uint8_t);

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
    void addrmode_zero_write(uint8_t val);
    void addrmode_zerox_write(uint8_t val);
    void addrmode_zeroy_write(uint8_t val);
    void addrmode_abs_write(uint8_t val);
    void addrmode_absx_write(uint8_t val);
    void addrmode_absy_write(uint8_t val);
    void addrmode_indx_write(uint8_t val);
    void addrmode_indy_write(uint8_t val);

    /* 
     * opcode functions missing (as they are not needed):
     * - STA, STX, STY (use addrmode_write functions directly)
     * - BEQ, BNE, BMI, BPL, BVC, BVS, BCC, BCS (use instr_branch)
     * - TAX, TXA, TAY, TYA, TXS, TSX (use instr_transfer)
     * . SEC, CLC, SEI, CLI, CLV, CLD (use instr_flag)
     */

    void instr_branch(bool take);
    void instr_flag(bool &flag, bool v);
    void instr_transfer(uint8_t from, uint8_t &to);

    void instr_lda(const uint8_t val);
    void instr_ldx(const uint8_t val);
    void instr_ldy(const uint8_t val);
    void instr_cmp(const uint8_t val);
    void instr_cpx(const uint8_t val);
    void instr_cpy(const uint8_t val);
    void instr_adc(const uint8_t val);
    void instr_sbc(const uint8_t val);
    void instr_ora(const uint8_t val);
    void instr_and(const uint8_t val);
    void instr_eor(const uint8_t val);
    void instr_bit(const uint8_t val);

    uint8_t instr_inc(uint8_t val);
    uint8_t instr_dec(uint8_t val);
    uint8_t instr_asl(uint8_t val);
    uint8_t instr_lsr(uint8_t val);
    uint8_t instr_rol(uint8_t val);
    uint8_t instr_ror(uint8_t val);

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
    CPU()
    {
        bus.write_enable = false;
        procstatus.reset();
    }

    void main();
    void power(const ROM &prgrom);
    void fire_irq();
    void fire_nmi();
    void reset();
    std::string disassemble() const;
    void printinfo(Utils::File &f) const;
    inline const uint8_t *getmemory() const
    { return bus.getmemory(); }
    inline uint32_t getsize() const
    { return CPUMap::MEMSIZE; }
    inline uint8_t peek_opcode() const
    { return curropcode; }

    inline void attach_ppu(PPU *ppu)
    { bus.ppu = ppu; }
};

} // namespace Core

#undef INSIDE_CPU_HPP

#endif

