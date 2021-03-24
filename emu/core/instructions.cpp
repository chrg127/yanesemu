#ifndef INSIDE_CPU_CPP
#error "Only emu/core/cpu.cpp may #include this file."
#else

/* NOTE: All instructions have an impled cycle from fetching the instruction
 * itself. (The call is done in CPU::main())
 * for most instruction, the polling happens during the final cycle of the
 * instruction, before the opcode fetch of the next instruction. if polling
 * detects an interrupt, the interrupt sequence is executed as the next
 * "instruction". */

// NOTE: addressing mode functions.
void CPU::addrmode_imm_read(InstrFuncRead f)
{
    // cycles: 2
    opargs.low = fetchop();
    (this->*f)(opargs.low);
    last_cycle();
}

void CPU::addrmode_zero_read(InstrFuncRead f)
{
    // cycles: 3
    opargs.low = fetchop();
    (this->*f)(readmem(opargs.low));
    last_cycle();
}

void CPU::addrmode_zerox_read(InstrFuncRead f)
{
    // cycles: 4
    opargs.low = fetchop();
    (this->*f)(readmem(opargs.low + r.x));
    // increment due to indexed addressing
    cycle();
    last_cycle();
}

void CPU::addrmode_zeroy_read(InstrFuncRead f)
{
    // cycles: 4
    opargs.low = fetchop();
    (this->*f)(readmem(opargs.low + r.y));
    cycle();
    last_cycle();
}

void CPU::addrmode_abs_read(InstrFuncRead f)
{
    // cycles: 4
    opargs.low = fetchop();
    opargs.high = fetchop();
    (this->*f)(readmem(opargs.full));
    last_cycle();
}

void CPU::addrmode_absx_read(InstrFuncRead f)
{
    // cycles: 4+1
    Reg16 res;

    opargs.low = fetchop();
    // cycle 3 is second operand fetch + adding X to the full reg
    opargs.high = fetchop();
    res = readmem(opargs.full+r.x);
    (this->*f)(res.full);
    if (opargs.high != res.high)
        cycle();
    last_cycle();
}

void CPU::addrmode_absy_read(InstrFuncRead f)
{
    // cycles: 4+1
    Reg16 res;

    opargs.low = fetchop();
    opargs.high = fetchop();
    res = readmem(opargs.full+r.y);
    (this->*f)(res.full);
    if (opargs.high != res.high)
        cycle();
    last_cycle();
}

void CPU::addrmode_indx_read(InstrFuncRead f)
{
    // cycles: 6
    Reg16 res;

    opargs.low = fetchop();
    cycle();
    res.low = readmem(opargs.low+r.x);
    res.high = readmem(opargs.low+r.x+1);
    (this->*f)(readmem(res.full));
    last_cycle();
}

void CPU::addrmode_indy_read(InstrFuncRead f)
{
    // cycles: 5+1
    Reg16 res;

    opargs.low = fetchop();
    res.low = readmem(opargs.low);
    res.high = readmem(opargs.low+1);
    res.full += r.y;
    (this->*f)(readmem(res.full));
    if (opargs.high != res.high)
        cycle();
    last_cycle();
}



void CPU::addrmode_accum_modify(InstrFuncMod f)
{
    // cycles: 2
    cycle();
    r.acc = (this->*f)(r.acc);
    last_cycle();
}

void CPU::addrmode_zero_modify(InstrFuncMod f)
{
    //cycles: 5
    Reg16 res;

    opargs.low = fetchop();
    res = (this->*f)(readmem(opargs.low));
    // the cpu uses a cycle to write back an unmodified value
    cycle();
    writemem(opargs.low, res.full);
    last_cycle();
}

void CPU::addrmode_zerox_modify(InstrFuncMod f)
{
    // cycles: 6
    Reg16 res;

    opargs.low = fetchop();
    cycle();
    res = (this->*f)(readmem(opargs.low + r.x));
    cycle();
    writemem(opargs.low + r.x, res.full);
    last_cycle();
}

void CPU::addrmode_abs_modify(InstrFuncMod f)
{
    // cycles: 6
    Reg16 res;

    opargs.low = fetchop();
    opargs.high = fetchop();
    res = (this->*f)(readmem(opargs.full));
    cycle();
    writemem(opargs.full, res.full);
    last_cycle();
}

void CPU::addrmode_absx_modify(InstrFuncMod f)
{
    // cycles: 7
    Reg16 res;

    opargs.low = fetchop();
    opargs.high = fetchop();
    res = (this->*f)(readmem(opargs.full + r.x));
    // reread from effective address
    cycle();
    // write the value back to effective address
    cycle();
    writemem(opargs.full + r.x, res.full);
    last_cycle();
}



void CPU::addrmode_zero_write(uint8 val)
{
    // cycles: 3
    opargs.low = fetchop();
    writemem(opargs.low, val);
    last_cycle();
}

void CPU::addrmode_zerox_write(uint8 val)
{
    // cycles: 4
    opargs.low = fetchop();
    cycle();
    writemem(opargs.low + r.x, val);
    last_cycle();
}

void CPU::addrmode_zeroy_write(uint8 val)
{
    // cycles: 4
    opargs.low = fetchop();
    cycle();
    writemem(opargs.low + r.y, val);
    last_cycle();
}

void CPU::addrmode_abs_write(uint8 val)
{
    // cycles: 4
    opargs.low = fetchop();
    opargs.high = fetchop();
    writemem(opargs.full, val);
    last_cycle();
}

void CPU::addrmode_absx_write(uint8 val)
{
    // cycles: 5
    opargs.low = fetchop();
    opargs.high = fetchop();
    cycle();
    writemem(opargs.full + r.x, val);
    last_cycle();
}

void CPU::addrmode_absy_write(uint8 val)
{
    // cycles: 5
    opargs.low = fetchop();
    opargs.high = fetchop();
    cycle();
    writemem(opargs.full + r.y, val);
    last_cycle();
}

void CPU::addrmode_indx_write(uint8 val)
{
    // cycles: 6
    Reg16 res;

    opargs.low = fetchop();
    // read from addres, add X to it
    cycle();
    res.low = readmem(opargs.low+r.x);
    res.high = readmem(opargs.low+r.x+1);
    writemem(res.full, val);
    last_cycle();
}

void CPU::addrmode_indy_write(uint8 val)
{
    // cycles: 6
    Reg16 res;

    opargs.low = fetchop();
    res.low = readmem(opargs.low);
    res.high = readmem(opargs.low+1);
    res.full += r.y;
    cycle();
    writemem(res.full, val);
    last_cycle();
}




void CPU::instr_branch(bool take)
{
    // cycles: 2+1+1
    Reg16 tmp;

    last_cycle();
    opargs.low = fetchop();
    tmp = r.pc;
    if (!take)
        return;
    cycle();
    r.pc.full += (int8_t) opargs.low;
    last_cycle();
    if (tmp.high != r.pc.high)
        cycle();
}

void CPU::instr_flag(bool &flag, bool v)
{
    // cycles: 2
    last_cycle();
    cycle();
    flag = v;
}

void CPU::instr_transfer(uint8 from, uint8 &to)
{
    // cycles: 2
    last_cycle();
    cycle();
    to = from;
    r.flags.zero = (to == 0);
    r.flags.neg  = (to & 0x80);
}



// NOTE: all instruction functions.
void CPU::instr_lda(const uint8 val)
{
    r.acc = val;
    r.flags.zero = r.acc == 0;
    r.flags.neg  = r.acc & 0x80;
}

void CPU::instr_ldx(const uint8 val)
{
    r.x = val;
    r.flags.zero = r.x == 0;
    r.flags.neg  = r.x & 0x80;
}

void CPU::instr_ldy(const uint8 val)
{
    r.y = val;
    r.flags.zero = r.y == 0;
    r.flags.neg  = r.y & 0x80;
}

void CPU::instr_cmp(const uint8 val)
{
    int res = r.acc-val;
    r.flags.zero     = res == 0;
    r.flags.neg      = res & 0x80;
    r.flags.carry    = res >= 0;
}

void CPU::instr_cpx(const uint8 val)
{
    int res = r.x-val;
    r.flags.zero     = res == 0;
    r.flags.neg      = res & 0x80;
    r.flags.carry    = res >= 0;
}

void CPU::instr_cpy(const uint8 val)
{
    int res = r.y-val;
    r.flags.zero     = res == 0;
    r.flags.neg      = res & 0x80;
    r.flags.carry    = res >= 0;
}

void CPU::instr_adc(const uint8 val)
{
    int sum = r.acc + val + r.flags.carry;
    r.flags.zero     = (uint8) sum == 0;
    r.flags.neg      = sum & 0x80;
    r.flags.carry    = sum > 0xFF;
    r.flags.ov       = (r.acc^sum) & ~(r.acc^val) & 0x80;
    r.acc = sum;
}

void CPU::instr_sbc(const uint8 val)
{
    uint8 tmp = ~val;
    int sum = r.acc + tmp + r.flags.carry;
    r.flags.zero     = (uint8) sum == 0;
    r.flags.neg      = sum & 0x80;
    r.flags.carry    = sum > 0xFF;
    r.flags.ov       = (r.acc^sum) & ~(r.acc^val) & 0x80;
    r.acc = sum;
}

void CPU::instr_ora(const uint8 val)
{
    r.acc |= val;
    r.flags.neg  = r.acc & 0x80;
    r.flags.zero = r.acc == 0;
}

void CPU::instr_and(const uint8 val)
{
    r.acc &= val;
    r.flags.neg  = r.acc & 0x80;
    r.flags.zero = r.acc == 0;
}

void CPU::instr_eor(const uint8 val)
{
    r.acc ^= val;
    r.flags.neg  = r.acc & 0x80;
    r.flags.zero = r.acc == 0;
}

void CPU::instr_bit(const uint8 val)
{
    r.flags.neg  = (r.acc & val) == 0;
    r.flags.zero = val == 0;
    r.flags.ov   = val & 0x40;
}



uint8 CPU::instr_inc(uint8 val)
{
    val++;
    r.flags.zero = val == 0;
    r.flags.neg  = val & 0x80;
    return val;
}

uint8 CPU::instr_dec(uint8 val)
{
    val--;
    r.flags.zero = val == 0;
    r.flags.neg  = val & 0x80;
    return val;
}

uint8 CPU::instr_asl(uint8 val)
{
    r.flags.carry = val & 0x80;
    val <<= 1;
    r.flags.zero = val == 0;
    r.flags.neg  = val & 0x80;
    return val;
}

uint8 CPU::instr_lsr(uint8 val)
{
    r.flags.carry = val & 1;
    val >>= 1;
    r.flags.zero = val == 0;
    r.flags.neg  = val & 0x80;
    return val;
}

uint8 CPU::instr_rol(uint8 val)
{
    bool c = r.flags.carry;
    r.flags.carry = val & 0x80;
    val = val << 1 | c;
    r.flags.zero = val == 0;
    r.flags.neg  = val & 0x80;
    return val;
}

uint8 CPU::instr_ror(uint8 val)
{
    bool c = r.flags.carry;
    r.flags.carry = val & 1;
    val = val >> 1 | c << 7;
    r.flags.zero = val == 0;
    r.flags.neg  = val & 0x80;
    return val;
}


void CPU::instr_inx()
{
    cycle();
    r.x++;
    r.flags.zero = (r.x == 0);
    r.flags.neg  = (r.x & 0x80);
    last_cycle();
}

void CPU::instr_iny()
{
    cycle();
    r.y++;
    r.flags.zero = (r.y == 0);
    r.flags.neg  = (r.y & 0x80);
    last_cycle();
}

void CPU::instr_dex()
{
    cycle();
    r.x--;
    r.flags.zero = (r.x == 0);
    r.flags.neg  = (r.x & 0x80);
    last_cycle();
}

void CPU::instr_dey()
{
    cycle();
    r.y--;
    r.flags.zero = (r.y == 0);
    r.flags.neg  = (r.y & 0x80);
    last_cycle();
}

void CPU::instr_php()
{
    // cycles: 3
    // one cycle for reading next instruction and throwing away
    cycle();
    r.flags.breakf = 1;
    push(r.flags);
    r.flags.breakf = 0;
    last_cycle();
}

void CPU::instr_pha()
{
    // cycles: 3
    // one cycle for reading next instruction and throwing away
    cycle();
    push(r.acc);
    last_cycle();
}

void CPU::instr_plp()
{
    // cycles: 4
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
    // cycles: 4
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
    // cycles: 6
    opargs.low = fetchop();
    // also, http://nesdev.com/6502_cpu.txt says that first the low byte is
    // copargsied, then the high byte is fetched. doing the opargss in this order is
    // wrong and a bug, fetch the high byte first before doing whatever with pc.
    opargs.high = fetchop();
    r.pc.full--;
    // internal opargseration, 1 cycle
    cycle();
    push(r.pc.high);
    push(r.pc.low);
    // the original hardware technically fetches the next opargserand right into the pc's high byte.
    // I save it in opargs.high first to enable disassembling.
    r.pc.low = opargs.low;
    r.pc.high = opargs.high;
    // r.pc = opargs.full; would be better!
    last_cycle();
}

void CPU::instr_jmp()
{
    // cycles: 3
    opargs.low = fetchop();
    opargs.high = fetchop();
    r.pc = opargs.full;
    last_cycle();
}

// We could have another addressing mode function for this... but I decided I'd rather
// have 1 less function and call this one directly as it's used by one instruction
void CPU::instr_jmp_ind()
{
    // cycles: 5
    opargs.low = fetchop();
    opargs.high = fetchop();
    // hardware bug
    if (opargs.low == 0xFF) {
        r.pc.low = readmem(opargs.full);
        // reset the low byte, e.g. $02FF -> $0200
        r.pc.high = readmem(opargs.full & 0xFF00);
    } else {
        r.pc.low = readmem(opargs.full);
        r.pc.high = readmem(opargs.full+1);
    }
    last_cycle();
}

void CPU::instr_rts()
{
    // cycles: 6
    // one for read of the next instruction, one for incrementing S
    cycle();
    cycle();
    r.pc.low = pull();
    r.pc.high = pull();
    r.pc.full++;
    // cycle for incrementing pc
    cycle();
    last_cycle();
}

void CPU::instr_brk()
{
    r.flags.breakf = 1;
    // cycles are counted in the interrupt function
    interrupt();
    // the break flag will be reset in the interrupt
}

void CPU::instr_rti()
{
    // cycles: 6
    // one for read of the next instruction, one for incrementing S
    cycle();
    cycle();
    r.flags = pull();
    // reset this just to be sure
    r.flags.breakf = 0;
    r.pc.low = pull();
    r.pc.high = pull();
    last_cycle();
}

void CPU::instr_nop()
{
    cycle();
    last_cycle();
}

#endif
