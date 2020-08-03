/* to be included only by cpu.cpp */

// NOTE: addressing mode functions.
void CPU::addrmode_imm_read(OpcodeFuncRead f)
{
    operand = fetch_op();
    (this->*f)(operand);
    cycle(2);
}

void CPU::addrmode_zero_read(OpcodeFuncRead f)
{
    operand = fetch_op();
    (this->*f)(bus.read(operand));
    cycle(3);
}

void CPU::addrmode_zerox_read(OpcodeFuncRead f)
{
    operand = fetch_op();
    (this->*f)(bus.read(operand + xreg));
    cycle(4);
}

void CPU::addrmode_zeroy_read(OpcodeFuncRead f)
{
    operand = fetch_op();
    (this->*f)(bus.read(operand + yreg));
    cycle(4);
}

void CPU::addrmode_abs_read(OpcodeFuncRead f)
{
    operand = fetch_op();
    operand2 = fetch_op();
    uint16_t addr = buildval16(operand, operand2);
    (this->*f)(bus.read(addr));
    cycle(4);
}

void CPU::addrmode_absx_read(OpcodeFuncRead f)
{
    operand = fetch_op();
    operand2 = fetch_op();
    uint16_t addr = buildval16(operand, operand2);
    uint16_t res = addr + xreg;
    (this->*f)(bus.read(addr));
    cycle(4);
    if ((addr >> 8) != (res >> 8))
        cycle(1);
}

void CPU::addrmode_absy_read(OpcodeFuncRead f)
{
    operand = fetch_op();
    operand2 = fetch_op();
    uint16_t addr = buildval16(operand, operand2);
    uint16_t res = addr + yreg;
    (this->*f)(bus.read(addr));
    cycle(4);
    if ((addr >> 8) != (res >> 8))
        cycle(1);
}

void CPU::addrmode_indx_read(OpcodeFuncRead f)
{
    operand = fetch_op();
    uint8_t low = bus.read(operand + xreg);
    uint16_t addr = buildval16(low, bus.read(operand+1));
    (this->*f)(bus.read(addr));
    cycle(6);
}

void CPU::addrmode_indy_read(OpcodeFuncRead f)
{
    operand = fetch_op();
    uint8_t low = bus.read(operand);
    uint16_t addr = buildval16(low, bus.read(operand+1));
    uint16_t res = addr + yreg;
    (this->*f)(bus.read(addr));
    cycle(5);
    if ((addr >> 8) != (res >> 8))
        cycle(1);
}



void CPU::addrmode_accum_modify(OpcodeFuncMod f)
{
    accum = (this->*f)(accum);
    cycle(2);
}

void CPU::addrmode_zero_modify(OpcodeFuncMod f)
{
    operand = fetch_op();
    uint8_t res = (this->*f)(bus.read(operand));
    bus.write(operand, res);
    cycle(5);
}

void CPU::addrmode_zerox_modify(OpcodeFuncMod f)
{
    operand = fetch_op();
    uint8_t res = (this->*f)(bus.read(operand + xreg));
    bus.write(operand + xreg, res);
    cycle(6);
}

void CPU::addrmode_abs_modify(OpcodeFuncMod f)
{
    operand = fetch_op();
    operand2 = fetch_op();
    uint16_t addr = buildval16(operand, operand2);
    uint8_t res = (this->*f)(bus.read(addr));
    bus.write(addr, res);
    cycle(6);
}

void CPU::addrmode_absx_modify(OpcodeFuncMod f)
{
    operand = fetch_op();
    operand2 = fetch_op();
    uint16_t addr = buildval16(operand, operand2) + xreg;
    uint8_t res = (this->*f)(bus.read(addr));
    bus.write(addr, res);
    cycle(7);
}



void CPU::addrmode_zero_write(uint8_t val)
{
    operand = fetch_op();
    bus.write(operand, val);
    cycle(2);
}

void CPU::addrmode_zerox_write(uint8_t val)
{
    operand = fetch_op() + xreg;
    bus.write(operand + xreg, val);
    cycle(2);
}

void CPU::addrmode_zeroy_write(uint8_t val)
{
    operand = fetch_op();
    bus.write(operand + yreg, val);
    cycle(2);
}

void CPU::addrmode_abs_write(uint8_t val)
{
    operand = fetch_op();
    operand2 = fetch_op();
    uint16_t addr = buildval16(operand, operand2);
    bus.write(addr, val);
    cycle(3);
}

void CPU::addrmode_absx_write(uint8_t val)
{
    operand = fetch_op();
    operand2 = fetch_op();
    uint16_t addr = buildval16(operand, operand2) + xreg;
    bus.write(addr, val);
    cycle(3);
}

void CPU::addrmode_absy_write(uint8_t val)
{
    operand = fetch_op();
    operand2 = fetch_op();
    uint16_t addr = buildval16(operand, operand2) + yreg;
    bus.write(addr, val);
    cycle(3);
}

void CPU::addrmode_indx_write(uint8_t val)
{
    operand = fetch_op() + xreg;
    uint8_t low = bus.read(operand);
    uint16_t addr = buildval16(low, bus.read(operand+1));
    bus.write(addr, val);
    cycle(2);
}

void CPU::addrmode_indy_write(uint8_t val)
{
    operand = fetch_op();
    uint8_t low = bus.read(operand);
    uint16_t addr = buildval16(low, bus.read(operand+1))+yreg;
    bus.write(addr, val);
    cycle(2);
}




void CPU::instr_branch(bool take)
{
    operand = fetch_op();
    uint8_t op = operand;
    uint8_t oldpc = pc;
    cycle(2);
    if (!take) {
        cycle(1);
        op = 0;
    }
    pc += (int8_t) op;
    if ((oldpc >> 8) != (pc >> 8))
        cycle(2);
}

void CPU::instr_flag(bool &flag, bool v)
{
    flag = v;
    cycle(2);
}

void CPU::instr_transfer(uint8_t from, uint8_t &to)
{
    to = from;
    procstatus.zero = (to == 0);
    procstatus.neg  = (to & 0x80);
    cycle(2);
}



// NOTE: all instruction functions.
void CPU::instr_lda(const uint8_t val)
{
    accum = val;
    procstatus.zero = (accum == 0);
    procstatus.neg  = (accum & 0x80);
}

void CPU::instr_ldx(const uint8_t val)
{
    xreg = val;
    procstatus.zero = (xreg == 0);
    procstatus.neg  = (xreg & 0x80);
}

void CPU::instr_ldy(const uint8_t val)
{
    yreg = val;
    procstatus.zero = (yreg == 0);
    procstatus.neg  = (yreg & 0x80);
}

void CPU::instr_cmp(const uint8_t val)
{
    uint8_t res = accum-val;
    procstatus.zero     = (res == 0) ? 1 : 0;
    procstatus.ov       = ((res & 0x80) != (accum & 1)) ? 1 : 0;
    procstatus.carry    = procstatus.ov;
    procstatus.neg      = res & 0x80;
}

void CPU::instr_cpx(const uint8_t val)
{
    uint8_t res = xreg-val;
    procstatus.zero     = (res == 0) ? 1 : 0;
    procstatus.ov       = ((res & 0x80) != (xreg & 1)) ? 1 : 0;
    procstatus.carry    = procstatus.ov;
    procstatus.neg      = res & 0x80;
}

void CPU::instr_cpy(const uint8_t val)
{
    uint8_t res = yreg-val;
    procstatus.zero     = (res == 0) ? 1 : 0;
    procstatus.ov       = ((res & 0x80) != (yreg & 1)) ? 1 : 0;
    procstatus.carry    = procstatus.ov;
    procstatus.neg      = res & 0x80;
}

void CPU::instr_adc(const uint8_t val)
{
    uint8_t sign = (accum & 0x80) >> 8;
    accum += val + procstatus.carry;
    procstatus.zero = (accum == 0);
    procstatus.ov   = ((accum & 0x80) != sign);
    procstatus.carry = procstatus.ov;
    procstatus.neg  = accum & 0x80;
}

void CPU::instr_sbc(const uint8_t val)
{
    uint8_t sign = (accum & 0x80) >> 8;
    accum -= val - (1-procstatus.carry);
    procstatus.zero = (accum == 0);
    procstatus.ov   = ((accum & 0x80) != sign);
    procstatus.carry = procstatus.ov;
    procstatus.neg  = accum & 0x80;
}

void CPU::instr_ora(const uint8_t val)
{
    accum |= val;
    procstatus.neg  = accum & 0x80;
    procstatus.zero = (accum == 0);
}

void CPU::instr_and(const uint8_t val)
{
    accum &= val;
    procstatus.neg  = accum & 0x80;
    procstatus.zero = (accum == 0);
}

void CPU::instr_eor(const uint8_t val)
{
    accum ^= val;
    procstatus.neg  = accum & 0x80;
    procstatus.zero = (accum == 0);
}

void CPU::instr_bit(const uint8_t val)
{
    uint8_t res = accum & val;
    procstatus.neg  = res & 0x80;
    procstatus.zero = (res == 0);
    procstatus.ov   = res & 0x40;
}



uint8_t CPU::instr_inc(uint8_t val)
{
    val++;
    procstatus.zero = (val == 0);
    procstatus.neg  = (val & 0x80);
    return val;
}

uint8_t CPU::instr_dec(uint8_t val)
{
    val--;
    procstatus.zero = (val == 0);
    procstatus.neg  = (val & 0x80);
    return val;
}

uint8_t CPU::instr_asl(uint8_t val)
{
    procstatus.carry = val & 0x80;
    val <<= 1;
    procstatus.neg  = val & 0x80;
    procstatus.zero = (val == 0);
    return val;
}

uint8_t CPU::instr_lsr(uint8_t val)
{
    procstatus.carry = val & 1;
    val >>= 1;
    procstatus.neg  = val & 0x80;
    procstatus.zero = (val == 0);
    return val;
}

uint8_t CPU::instr_rol(uint8_t val)
{
    uint8_t bit0 = procstatus.carry;
    procstatus.carry = val & 0x80;
    val <<= 1;
    val |= bit0;
    procstatus.neg  = val & 0x80;
    procstatus.zero = (val == 0);
    return val;
}

uint8_t CPU::instr_ror(uint8_t val)
{
    uint8_t bit7 = procstatus.carry << 8;
    procstatus.carry = val & 1;
    val >>= 1;
    val |= bit7 << 8;
    procstatus.neg  = val & 0x80;
    procstatus.zero = (val == 0);
    return val;
}



#define func_increase(reg, regname) \
void CPU::instr_in##reg() \
{ \
    regname++; \
    procstatus.zero = (xreg == 0); \
    procstatus.neg  = (xreg & 0x80); \
    cycle(2); \
}
func_increase(x, xreg)
func_increase(y, yreg)

#define func_decrease(reg, regname) \
void CPU::instr_de##reg() \
{ \
    regname--; \
    procstatus.zero = (xreg == 0); \
    procstatus.neg  = (xreg & 0x80); \
    cycle(2); \
}
func_decrease(x, xreg)
func_decrease(y, yreg)

void CPU::instr_php()
{
    procstatus.breakf = 1;
    push(procstatus.reg());
    procstatus.breakf = 0;
    cycle(3);
}

void CPU::instr_pha()
{
    push(accum);
    cycle(3);
}

void CPU::instr_plp()
{
    procstatus = pull();
    cycle(4);
}

void CPU::instr_pla()
{
    accum = pull();
    procstatus.zero = (accum == 0);
    procstatus.neg  = (accum & 0x80);
    cycle(4);
}

void CPU::instr_jsr()
{
    operand = fetch_op();
    operand2 = fetch_op();
    push(pc >> 8);
    push(pc & 0xFF);
    pc = buildval16(operand, operand2);
    cycle(6);
}

void CPU::instr_jmp()
{
    operand = fetch_op();
    operand2 = fetch_op();
    pc = buildval16(operand, operand2);
    cycle(3);
}

void CPU::instr_jmp_ind()
{
    operand = fetch_op();
    operand2 = fetch_op();
    pc = bus.read(buildval16(operand, operand2));
    cycle(5);
}

void CPU::instr_rts()
{
    uint8_t low = pull();
    pc = buildval16(low, pull());
    cycle(6);
}

void CPU::instr_brk()
{
    procstatus.breakf = 1;
    interrupt(IRQBRKVEC);
    // the break flag only exists in a copy, so reset it here
    procstatus.breakf = 0;
    cycle(7);
}

void CPU::instr_rti()
{
    procstatus = pull();
    uint8_t low = pull();
    pc = buildval16(low, pull());
    cycle(6);
}

void CPU::instr_nop()
{
    cycle(2);
}

