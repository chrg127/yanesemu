#ifndef INSIDE_CPU_CPP
#error "Only emu/core/cpu.cpp may #include this file."
#else

using util::Word;

#define call(f, ...) (this->*f)(__VA_ARGS__)

/*
 * All instructions have an impled cycle from fetching the instruction
 * itself (The call is done in CPU::main()).
 *
 * For most instruction, the polling happens during the final cycle of the
 * instruction, before the opcode fetch of the next instruction. If polling
 * detects an interrupt, the interrupt sequence is executed as the next
 * "instruction".
 */

void CPU::addrmode_imm_read(InstrFuncRead f)
{
    u8 op = fetch();
    call(f, op);
    last_cycle();
}

void CPU::addrmode_zero_read(InstrFuncRead f)
{
    u8 op = fetch();
    call(f, readmem(op));
    last_cycle();
}

void CPU::addrmode_zero_ind_read(InstrFuncRead f, u8 reg)
{
    u8 op = fetch();
    call(f, readmem(op + reg));
    // increment due to indexed addressing
    cycle();
    last_cycle();
}

void CPU::addrmode_abs_read(InstrFuncRead f)
{
    Word op;
    op.l = fetch();
    op.h = fetch();
    call(f, readmem(op.v));
    last_cycle();
}

void CPU::addrmode_abs_ind_read(InstrFuncRead f, u8 reg)
{
    Word op;
    op.l = fetch();
    op.h = fetch();
    u8 tmp = op.h;
    op.v += reg;
    if (op.h != tmp)
        cycle();
    call(f, readmem(op.v));
    last_cycle();
}

void CPU::addrmode_indx_read(InstrFuncRead f)
{
    Word res;
    u8 op = fetch();
    cycle();
    res.l = readmem(op + r.x    );
    res.h = readmem(op + r.x + 1);
    call(f, readmem(res.v));
    last_cycle();
}

void CPU::addrmode_indy_read(InstrFuncRead f)
{
    Word res;
    u8 op = fetch();
    res.l = readmem(op    );
    res.h = readmem(op + 1);
    u8 tmp = res.h;
    res.v += r.y;
    if (res.h != tmp)
        cycle();
    call(f, readmem(res.v));
    last_cycle();
}



void CPU::addrmode_accum_modify(InstrFuncMod f)
{
    cycle();
    r.acc = call(f, r.acc);
    last_cycle();
}

void CPU::addrmode_zero_modify(InstrFuncMod f)
{
    u8 op = fetch();
    u8 res = call(f, readmem(op));
    // the cpu uses a cycle to write back an unmodified value
    cycle();
    writemem(op, res);
    last_cycle();
}

void CPU::addrmode_zerox_modify(InstrFuncMod f)
{
    u8 op = fetch();
    cycle();
    u8 res = call(f, readmem(op + r.x));
    cycle();
    writemem(op + r.x, res);
    last_cycle();
}

void CPU::addrmode_abs_modify(InstrFuncMod f)
{
    Word op;
    op.l = fetch();
    op.h = fetch();
    u8 res = call(f, readmem(op.v));
    cycle();
    writemem(op.v, res);
    last_cycle();
}

void CPU::addrmode_absx_modify(InstrFuncMod f)
{
    Word op;
    op.l = fetch();
    op.h = fetch();
    u8 res = call(f, readmem(op.v + r.x));
    // reread from effective address
    cycle();
    // write the value back to effective address
    cycle();
    writemem(op.v + r.x, res);
    last_cycle();
}

#undef call



void CPU::addrmode_zero_write(u8 val)
{
    u8 op = fetch();
    writemem(op, val);
    last_cycle();
}

void CPU::addrmode_zero_ind_write(u8 val, u8 reg)
{
    u8 op = fetch();
    cycle();
    writemem(op + reg, val);
    last_cycle();
}

void CPU::addrmode_abs_write(u8 val)
{
    Word op;
    op.l = fetch();
    op.h = fetch();
    writemem(op.v, val);
    last_cycle();
}

void CPU::addrmode_abs_ind_write(u8 val, u8 reg)
{
    Word op;
    op.l = fetch();
    op.h = fetch();
    cycle();
    writemem(op.v + r.x, val);
    last_cycle();
}

void CPU::addrmode_indx_write(u8 val)
{
    u8 op = fetch();
    // read from address, add x to it
    cycle();
    Word res;
    res.l = readmem(op + r.x    );
    res.h = readmem(op + r.x + 1);
    writemem(res.v, val);
    last_cycle();
}

void CPU::addrmode_indy_write(u8 val)
{
    u8 op = fetch();
    Word res;
    res.l = readmem(op    );
    res.h = readmem(op + 1);
    res.v += r.y;
    cycle();
    writemem(res.v, val);
    last_cycle();
}




void CPU::instr_branch(bool take)
{
    last_cycle();
    u8 op = fetch();
    if (!take)
        return;
    cycle();
    Word tmp = r.pc;
    r.pc.v += (int8_t) op;
    last_cycle();
    if (tmp.h != r.pc.h)
        cycle();
}

void CPU::instr_flag(bool &flag, bool v)
{
    last_cycle();
    cycle();
    flag = v;
}

void CPU::instr_transfer(u8 from, u8 &to)
{
    last_cycle();
    cycle();
    to = from;
    r.flags.zero = (to == 0);
    r.flags.neg  = (to & 0x80);
}



void CPU::instr_load(u8 val, u8 &reg)
{
    reg = val;
    r.flags.zero = reg == 0;
    r.flags.neg  = reg & 0x80;
}

void CPU::instr_lda(u8 val) { instr_load(val, r.acc); }
void CPU::instr_ldx(u8 val) { instr_load(val, r.x); }
void CPU::instr_ldy(u8 val) { instr_load(val, r.y); }

void CPU::instr_compare(u8 val, u8 reg)
{
    int res = reg - val;
    r.flags.zero     = res == 0;
    r.flags.neg      = res & 0x80;
    r.flags.carry    = res >= 0;
}

void CPU::instr_cmp(u8 val) { instr_compare(val, r.acc); }
void CPU::instr_cpx(u8 val) { instr_compare(val, r.x); }
void CPU::instr_cpy(u8 val) { instr_compare(val, r.y); }

void CPU::instr_adc(u8 val)
{
    int sum = r.acc + val + r.flags.carry;
    r.flags.zero     = (u8) sum == 0;
    r.flags.neg      = sum & 0x80;
    r.flags.carry    = sum > 0xFF;
    r.flags.ov       = (r.acc^sum) & ~(r.acc^val) & 0x80;
    r.acc = sum;
}

void CPU::instr_sbc(u8 val)
{
    u8 tmp = ~val;
    int sum = r.acc + tmp + r.flags.carry;
    r.flags.zero     = (u8) sum == 0;
    r.flags.neg      = sum & 0x80;
    r.flags.carry    = sum > 0xFF;
    r.flags.ov       = (r.acc^sum) & ~(r.acc^val) & 0x80;
    r.acc = sum;
}



void CPU::instr_ora(u8 val)
{
    r.acc |= val;
    r.flags.neg  = r.acc & 0x80;
    r.flags.zero = r.acc == 0;
}

void CPU::instr_and(u8 val)
{
    r.acc &= val;
    r.flags.neg  = r.acc & 0x80;
    r.flags.zero = r.acc == 0;
}

void CPU::instr_eor(u8 val)
{
    r.acc ^= val;
    r.flags.neg  = r.acc & 0x80;
    r.flags.zero = r.acc == 0;
}

void CPU::instr_bit(u8 val)
{
    r.flags.neg  = (r.acc & val) == 0;
    r.flags.zero = val == 0;
    r.flags.ov   = val & 0x40;
}

u8 CPU::instr_inc(u8 val)
{
    val++;
    r.flags.zero = val == 0;
    r.flags.neg  = val & 0x80;
    return val;
}

u8 CPU::instr_dec(u8 val)
{
    val--;
    r.flags.zero = val == 0;
    r.flags.neg  = val & 0x80;
    return val;
}

u8 CPU::instr_asl(u8 val)
{
    r.flags.carry = val & 0x80;
    val <<= 1;
    r.flags.zero = val == 0;
    r.flags.neg  = val & 0x80;
    return val;
}

u8 CPU::instr_lsr(u8 val)
{
    r.flags.carry = val & 1;
    val >>= 1;
    r.flags.zero = val == 0;
    r.flags.neg  = val & 0x80;
    return val;
}

u8 CPU::instr_rol(u8 val)
{
    bool carry = r.flags.carry;
    r.flags.carry = val & 0x80;
    val = val << 1 | carry;
    r.flags.zero = val == 0;
    r.flags.neg  = val & 0x80;
    return val;
}

u8 CPU::instr_ror(u8 val)
{
    bool c = r.flags.carry;
    r.flags.carry = val & 1;
    val = val >> 1 | c << 7;
    r.flags.zero = val == 0;
    r.flags.neg  = val & 0x80;
    return val;
}


void CPU::instr_inc_reg(u8 &reg)
{
    cycle();
    reg++;
    r.flags.zero = (reg == 0);
    r.flags.neg  = (reg & 0x80);
    last_cycle();
}

void CPU::instr_dec_reg(u8 &reg)
{
    cycle();
    reg--;
    r.flags.zero = (reg == 0);
    r.flags.neg  = (reg & 0x80);
    last_cycle();
}

void CPU::instr_inx() { instr_inc_reg(r.x); }
void CPU::instr_iny() { instr_inc_reg(r.y); }
void CPU::instr_dex() { instr_dec_reg(r.x); }
void CPU::instr_dey() { instr_dec_reg(r.y); }

void CPU::instr_php()
{
    // one cycle for reading next instruction and throwing away
    cycle();
    r.flags.breakf = 1;
    push((u8) r.flags);
    r.flags.breakf = 0;
    last_cycle();
}

void CPU::instr_pha()
{
    // one cycle for reading next instruction and throwing away
    cycle();
    push(r.acc);
    last_cycle();
}

void CPU::instr_plp()
{
    // one cycle for reading next instruction, one for incrementing S
    cycle();
    cycle();
    // plp polls for interrupts before pulling
    last_cycle();
    r.flags = pull();
    r.flags.breakf = 0;
}

void CPU::instr_pla()
{
    // one cycle for reading next instruction, one for incrementing S
    cycle();
    cycle();
    r.acc = pull();
    r.flags.zero = r.acc == 0;
    r.flags.neg  = r.acc & 0x80;
    last_cycle();
}

void CPU::instr_jsr()
{
    u8 low = fetch();
    // internal operation
    cycle();
    push(r.pc.h);
    push(r.pc.l);
    r.pc.h = fetch();
    r.pc.l = low;
}

void CPU::instr_jmp()
{
    u8 op = fetch();
    r.pc.h = fetch();
    r.pc.l = op;
    last_cycle();
}

void CPU::instr_jmp_ind()
{
    Word op;
    op.l = fetch();
    op.h = fetch();
    // hardware bug
    if (op.l == 0xFF) {
        r.pc.l = readmem(op.v);
        // reset the l byte, e.g. $02FF -> $0200
        r.pc.h = readmem(op.v & 0xFF00);
    } else {
        r.pc.l = readmem(op.v);
        r.pc.h = readmem(op.v+1);
    }
    last_cycle();
}

void CPU::instr_rts()
{
    // one for useless read of the next instruction
    cycle();
    // one for incrementing S
    cycle();
    r.pc.l = pull();
    r.pc.h = pull();
    r.pc.v++;
    // cycle for incrementing pc
    cycle();
    last_cycle();
}

void CPU::instr_rti()
{
    // one for useless read of the next instruction
    cycle();
    // one for incrementing S
    cycle();
    r.flags = pull();
    // reset this just to be sure
    r.flags.breakf = 0;
    r.pc.l = pull();
    r.pc.h = pull();
    last_cycle();
}

void CPU::instr_brk()
{
    r.flags.breakf = 1;
    // cycles are counted in the interrupt function
    interrupt();
    // the break flag will be reset in the interrupt
}

void CPU::instr_nop()
{
    cycle();
    last_cycle();
}

#endif
