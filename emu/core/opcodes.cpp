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
    op.low = fetch();
    (this->*f)(op.low);
    last_cycle();
}

void CPU::addrmode_zero_read(InstrFuncRead f)
{
    // cycles: 3
    op.low = fetch();
    (this->*f)(readmem(op.low));
    last_cycle();
}

void CPU::addrmode_zerox_read(InstrFuncRead f)
{
    // cycles: 4
    op.low = fetch();
    (this->*f)(readmem(op.low + xreg));
    // increment due to indexed addressing
    cycle();
    last_cycle();
}

void CPU::addrmode_zeroy_read(InstrFuncRead f)
{
    // cycles: 4
    op.low = fetch();
    (this->*f)(readmem(op.low + yreg));
    cycle();
    last_cycle();
}

void CPU::addrmode_abs_read(InstrFuncRead f)
{
    // cycles: 4
    op.low = fetch();
    op.high = fetch();
    (this->*f)(readmem(op.reg));
    last_cycle();
}

void CPU::addrmode_absx_read(InstrFuncRead f)
{
    // cycles: 4+1
    Reg16 res;

    op.low = fetch();
    // cycle 3 is second operand fetch + adding X to the full reg
    op.high = fetch();
    res = readmem(op.reg+xreg);
    (this->*f)(res.reg);
    if (op.high != res.high)
        cycle();
    last_cycle();
}

void CPU::addrmode_absy_read(InstrFuncRead f)
{
    // cycles: 4+1
    Reg16 res;

    op.low = fetch();
    op.high = fetch();
    res = readmem(op.reg+yreg);
    (this->*f)(res.reg);
    if (op.high != res.high)
        cycle();
    last_cycle();
}

void CPU::addrmode_indx_read(InstrFuncRead f)
{
    // cycles: 6
    Reg16 res;

    op.low = fetch();
    cycle();
    res.low = readmem(op.low+xreg);
    res.high = readmem(op.low+xreg+1);
    (this->*f)(readmem(res.reg));
    last_cycle();
}

void CPU::addrmode_indy_read(InstrFuncRead f)
{
    // cycles: 5+1
    Reg16 res;

    op.low = fetch();
    res.low = readmem(op.low);
    res.high = readmem(op.low+1);
    res.reg += yreg;
    (this->*f)(readmem(res.reg));
    if (op.high != res.high)
        cycle();
    last_cycle();
}



void CPU::addrmode_accum_modify(InstrFuncMod f)
{
    // cycles: 2
    cycle();
    accum = (this->*f)(accum);
    last_cycle();
}

void CPU::addrmode_zero_modify(InstrFuncMod f)
{
    //cycles: 5
    Reg16 res;

    op.low = fetch();
    res = (this->*f)(readmem(op.low));
    // the cpu uses a cycle to write back an unmodified value
    cycle();
    writemem(op.low, res.reg);
    last_cycle();
}

void CPU::addrmode_zerox_modify(InstrFuncMod f)
{
    // cycles: 6
    Reg16 res;

    op.low = fetch();
    cycle();
    res = (this->*f)(readmem(op.low + xreg));
    cycle();
    writemem(op.low + xreg, res.reg);
    last_cycle();
}

void CPU::addrmode_abs_modify(InstrFuncMod f)
{
    // cycles: 6
    Reg16 res;

    op.low = fetch();
    op.high = fetch();
    res = (this->*f)(readmem(op.reg));
    cycle();
    writemem(op.reg, res.reg);
    last_cycle();
}

void CPU::addrmode_absx_modify(InstrFuncMod f)
{
    // cycles: 7
    Reg16 res;

    op.low = fetch();
    op.high = fetch();
    res = (this->*f)(readmem(op.reg + xreg));
    // reread from effective address
    cycle();
    // write the value back to effective address
    cycle();
    writemem(op.reg + xreg, res.reg);
    last_cycle();
}



void CPU::addrmode_zero_write(uint8 val)
{
    // cycles: 3
    op.low = fetch();
    writemem(op.low, val);
    last_cycle();
}

void CPU::addrmode_zerox_write(uint8 val)
{
    // cycles: 4
    op.low = fetch();
    cycle();
    writemem(op.low + xreg, val);
    last_cycle();
}

void CPU::addrmode_zeroy_write(uint8 val)
{
    // cycles: 4
    op.low = fetch();
    cycle();
    writemem(op.low + yreg, val);
    last_cycle();
}

void CPU::addrmode_abs_write(uint8 val)
{
    // cycles: 4
    op.low = fetch();
    op.high = fetch();
    writemem(op.reg, val);
    last_cycle();
}

void CPU::addrmode_absx_write(uint8 val)
{
    // cycles: 5
    op.low = fetch();
    op.high = fetch();
    cycle();
    writemem(op.reg + xreg, val);
    last_cycle();
}

void CPU::addrmode_absy_write(uint8 val)
{
    // cycles: 5
    op.low = fetch();
    op.high = fetch();
    cycle();
    writemem(op.reg + yreg, val);
    last_cycle();
}

void CPU::addrmode_indx_write(uint8 val)
{
    // cycles: 6
    Reg16 res;

    op.low = fetch();
    // read from addres, add X to it
    cycle();
    res.low = readmem(op.low+xreg);
    res.high = readmem(op.low+xreg+1);
    writemem(res.reg, val);
    last_cycle();
}

void CPU::addrmode_indy_write(uint8 val)
{
    // cycles: 6
    Reg16 res;

    op.low = fetch();
    res.low = readmem(op.low);
    res.high = readmem(op.low+1);
    res.reg += yreg;
    cycle();
    writemem(res.reg, val);
    last_cycle();
}




void CPU::instr_branch(bool take)
{
    // cycles: 2+1+1
    Reg16 tmp;

    last_cycle();
    op.low = fetch();
    tmp = pc;
    if (!take)
        return;
    cycle();
    pc.reg += (int8_t) op.low;
    last_cycle();
    if (tmp.high != pc.high)
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
    procstatus.zero = (to == 0);
    procstatus.neg  = (to & 0x80);
}



// NOTE: all instruction functions.
void CPU::instr_lda(const uint8 val)
{
    ;
    accum = val;
    procstatus.zero = accum == 0;
    procstatus.neg  = accum & 0x80;
}

void CPU::instr_ldx(const uint8 val)
{
    xreg = val;
    procstatus.zero = xreg == 0;
    procstatus.neg  = xreg & 0x80;
}

void CPU::instr_ldy(const uint8 val)
{
    yreg = val;
    procstatus.zero = yreg == 0;
    procstatus.neg  = yreg & 0x80;
}

void CPU::instr_cmp(const uint8 val)
{
    int res = accum-val;
    procstatus.zero     = res == 0;
    procstatus.neg      = res & 0x80;
    procstatus.carry    = res >= 0;
}

void CPU::instr_cpx(const uint8 val)
{
    int res = xreg-val;
    procstatus.zero     = res == 0;
    procstatus.neg      = res & 0x80;
    procstatus.carry    = res >= 0;
}

void CPU::instr_cpy(const uint8 val)
{
    int res = yreg-val;
    procstatus.zero     = res == 0;
    procstatus.neg      = res & 0x80;
    procstatus.carry    = res >= 0;
}

void CPU::instr_adc(const uint8 val)
{
    int sum = accum + val + procstatus.carry;
    procstatus.zero     = (uint8) sum == 0;
    procstatus.neg      = sum & 0x80;
    procstatus.carry    = sum > 0xFF;
    procstatus.ov       = (accum^sum) & ~(accum^val) & 0x80;
    accum = sum;
}

void CPU::instr_sbc(const uint8 val)
{
    uint8 tmp = ~val;
    int sum = accum + tmp + procstatus.carry;
    procstatus.zero     = (uint8) sum == 0;
    procstatus.neg      = sum & 0x80;
    procstatus.carry    = sum > 0xFF;
    procstatus.ov       = (accum^sum) & ~(accum^val) & 0x80;
    accum = sum;
}

void CPU::instr_ora(const uint8 val)
{
    accum |= val;
    procstatus.neg  = accum & 0x80;
    procstatus.zero = accum == 0;
}

void CPU::instr_and(const uint8 val)
{
    accum &= val;
    procstatus.neg  = accum & 0x80;
    procstatus.zero = accum == 0;
}

void CPU::instr_eor(const uint8 val)
{
    accum ^= val;
    procstatus.neg  = accum & 0x80;
    procstatus.zero = accum == 0;
}

void CPU::instr_bit(const uint8 val)
{
    procstatus.neg  = (accum & val) == 0;
    procstatus.zero = val == 0;
    procstatus.ov   = val & 0x40;
}



uint8 CPU::instr_inc(uint8 val)
{
    val++;
    procstatus.zero = val == 0;
    procstatus.neg  = val & 0x80;
    return val;
}

uint8 CPU::instr_dec(uint8 val)
{
    val--;
    procstatus.zero = val == 0;
    procstatus.neg  = val & 0x80;
    return val;
}

uint8 CPU::instr_asl(uint8 val)
{
    procstatus.carry = val & 0x80;
    val <<= 1;
    procstatus.zero = val == 0;
    procstatus.neg  = val & 0x80;
    return val;
}

uint8 CPU::instr_lsr(uint8 val)
{
    procstatus.carry = val & 1;
    val >>= 1;
    procstatus.zero = val == 0;
    procstatus.neg  = val & 0x80;
    return val;
}

uint8 CPU::instr_rol(uint8 val)
{
    bool c = procstatus.carry;
    procstatus.carry = val & 0x80;
    val = val << 1 | c;
    procstatus.zero = val == 0;
    procstatus.neg  = val & 0x80;
    return val;
}

uint8 CPU::instr_ror(uint8 val)
{
    bool c = procstatus.carry;
    procstatus.carry = val & 1;
    val = val >> 1 | c << 7;
    procstatus.zero = val == 0;
    procstatus.neg  = val & 0x80;
    return val;
}


void CPU::instr_inx()
{
    cycle();
    xreg++;
    procstatus.zero = (xreg == 0);
    procstatus.neg  = (xreg & 0x80);
    last_cycle();
}

void CPU::instr_iny()
{
    cycle();
    yreg++;
    procstatus.zero = (yreg == 0);
    procstatus.neg  = (yreg & 0x80);
    last_cycle();
}

void CPU::instr_dex()
{
    cycle();
    xreg--;
    procstatus.zero = (xreg == 0);
    procstatus.neg  = (xreg & 0x80);
    last_cycle();
}

void CPU::instr_dey()
{
    cycle();
    yreg--;
    procstatus.zero = (yreg == 0);
    procstatus.neg  = (yreg & 0x80);
    last_cycle();
}

void CPU::instr_php()
{
    // cycles: 3
    // one cycle for reading next instruction and throwing away
    cycle();
    procstatus.breakf = 1;
    push(procstatus.reg());
    procstatus.breakf = 0;
    last_cycle();
}

void CPU::instr_pha()
{
    // cycles: 3
    // one cycle for reading next instruction and throwing away
    cycle();
    push(accum);
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
    procstatus = pull();
    procstatus.breakf = 0;
}

void CPU::instr_pla()
{
    // cycles: 4
    // one cycle for reading next instruction, one for incrementing S
    cycle();
    cycle();
    accum = pull();
    procstatus.zero = accum == 0;
    procstatus.neg  = accum & 0x80;
    last_cycle();
}

void CPU::instr_jsr()
{
    // cycles: 6
    op.low = fetch();
    // also, http://nesdev.com/6502_cpu.txt says that first the low byte is
    // copied, then the high byte is fetched. doing the ops in this order is
    // wrong and a bug, fetch the high byte first before doing whatever with pc.
    op.high = fetch();
    pc.reg--;
    // internal operation, 1 cycle
    cycle();
    push(pc.high);
    push(pc.low);
    // the original hardware technically fetches the next operand right into the pc's high byte.
    // I save it in op.high first to enable disassembling.
    pc.low = op.low;
    pc.high = op.high;
    last_cycle();
}

void CPU::instr_jmp()
{
    // cycles: 3
    op.low = fetch();
    op.high = fetch();
    pc = op.reg;
    last_cycle();
}

// We could have another addressing mode function for this... but I decided I'd rather
// have 1 less function and call this one directly as it's used by one instruction
void CPU::instr_jmp_ind()
{
    // cycles: 5
    op.low = fetch();
    op.high = fetch();
    // hardware bug
    if (op.low == 0xFF) {
        pc.low = readmem(op.reg);
        // reset the low byte, e.g. $02FF -> $0200
        pc.high = readmem(op.reg & 0xFF00);
    } else {
        pc.low = readmem(op.reg);
        pc.high = readmem(op.reg+1);
    }
    last_cycle();
}

void CPU::instr_rts()
{
    // cycles: 6
    // one for read of the next instruction, one for incrementing S
    cycle();
    cycle();
    pc.low = pull();
    pc.high = pull();
    pc.reg++;
    // cycle for incrementing pc
    cycle();
    last_cycle();
}

void CPU::instr_brk()
{
    procstatus.breakf = 1;
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
    procstatus = pull();
    // reset this just to be sure
    procstatus.breakf = 0;
    pc.low = pull();
    pc.high = pull();
    last_cycle();
}

void CPU::instr_nop()
{
    cycle();
    last_cycle();
}

#endif
