/* to be included only by cpu.cpp */

/* for most instruction, the polling happens during the final cycle of the
 * instruction, before the opcode fetch of the next instruction. if polling
 * detects an interrupt, the interrupt sequence is executed as the next
 * "instruction". */
/* if both IRQ and NMI are pending, the NMI will be handled and the IRQ
 * forgotten.
 */
/* polling quirks:
 * if IRQ is pending and RTI is executed and clears the intdis flag, the 
 * CPU will invoke the handler immediately after RTI finished executing.
 * CLI, SEI, and PLP change the intdis flag after polling for interrupts.
 * this means they can delay interrupts. */
/* fuck branch polling */

// NOTE: addressing mode functions.
void CPU::addrmode_imm_read(OpcodeFuncRead f)
{
    operandnew.low = fetch_op();
    cycle(1);
    (this->*f)(operandnew.low);
    last_cycle();
}

void CPU::addrmode_zero_read(OpcodeFuncRead f)
{
    operandnew.low = fetch_op();
    cycle(1);
    (this->*f)(bus.read(operandnew.low));
    cycle(1);
    last_cycle();
}

void CPU::addrmode_zerox_read(OpcodeFuncRead f)
{
    operandnew.low = fetch_op();
    cycle(1);
    (this->*f)(bus.read(operandnew.low + xreg));
    cycle(2);
    last_cycle();
}

void CPU::addrmode_zeroy_read(OpcodeFuncRead f)
{
    operandnew.low = fetch_op();
    cycle(1);
    (this->*f)(bus.read(operandnew.low + yreg));
    cycle(2);
    last_cycle();
}

void CPU::addrmode_abs_read(OpcodeFuncRead f)
{
    operandnew.low = fetch_op();
    cycle(1);
    operandnew.high = fetch_op();
    cycle(1);
    (this->*f)(bus.read(operandnew.reg));
    cycle(1);
    last_cycle();
}

void CPU::addrmode_absx_read(OpcodeFuncRead f)
{
    operandnew.low = fetch_op();
    cycle(1);
    operandnew.high = fetch_op();
    cycle(1);
    uint16_t res = bus.read(operandnew.reg+xreg);
    (this->*f)(res);
    cycle(2);
    if (operandnew.high != (res >> 8))
        cycle(1);
}

void CPU::addrmode_absy_read(OpcodeFuncRead f)
{
    operandnew.low = fetch_op();
    cycle(1);
    operandnew.high = fetch_op();
    cycle(1);
    uint16_t res = bus.read(operandnew.reg+yreg);
    (this->*f)(res);
    cycle(2);
    if (operandnew.high != (res >> 8))
        cycle(1);
}

void CPU::addrmode_indx_read(OpcodeFuncRead f)
{
    operandnew.low = fetch_op();
    cycle(1);
    uint16_t res = buildval16(bus.read(operandnew.low+xreg), bus.read(operandnew.low+xreg+1));
    cycle(2);
    (this->*f)(bus.read(res));
    cycle(2);
    last_cycle();
}

void CPU::addrmode_indy_read(OpcodeFuncRead f)
{
    operandnew.low = fetch_op();
    cycle(1);
    uint16_t res = buildval16(bus.read(operandnew.low), bus.read(operandnew.low+1)) + yreg;
    cycle(2);
    (this->*f)(bus.read(res));
    cycle(2);
    if (operandnew.high != (res >> 8))
        cycle(1);
}



void CPU::addrmode_accum_modify(OpcodeFuncMod f)
{
    accum = (this->*f)(accum);
    cycle(1);
    last_cycle();
}

void CPU::addrmode_zero_modify(OpcodeFuncMod f)
{
    operandnew.low = fetch_op();
    cycle(1);
    uint8_t res = (this->*f)(bus.read(operandnew.low));
    cycle(2);
    bus.write(operandnew.low, res);
    cycle(1);
    last_cycle();
}

void CPU::addrmode_zerox_modify(OpcodeFuncMod f)
{
    operandnew.low = fetch_op();
    cycle(1);
    uint8_t res = (this->*f)(bus.read(operandnew.low + xreg));
    cycle(1);
    bus.write(operandnew.low + xreg, res);
    cycle(1);
    cycle(2);
    last_cycle();
}

void CPU::addrmode_abs_modify(OpcodeFuncMod f)
{
    operandnew.low = fetch_op();
    cycle(1);
    operandnew.high = fetch_op();
    cycle(1);
    uint8_t res = (this->*f)(bus.read(operandnew.reg));
    cycle(2);
    bus.write(operandnew.reg, res);
    cycle(1);
    last_cycle();
}

void CPU::addrmode_absx_modify(OpcodeFuncMod f)
{
    operandnew.low = fetch_op();
    cycle(1);
    operandnew.high = fetch_op();
    cycle(1);
    uint8_t res = (this->*f)(bus.read(operandnew.reg + xreg));
    cycle(2);
    bus.write(operandnew.reg + xreg, res);
    cycle(1);
    cycle(1);
    last_cycle();
}



void CPU::addrmode_zero_write(uint8_t val)
{
    operandnew.low = fetch_op();
    cycle(1);
    bus.write(operandnew.low, val);
    cycle(1);
    last_cycle();
}

void CPU::addrmode_zerox_write(uint8_t val)
{
    operandnew.low = fetch_op();
    cycle(1);
    bus.write(operandnew.low + xreg, val);
    cycle(2);
    last_cycle();
}

void CPU::addrmode_zeroy_write(uint8_t val)
{
    operandnew.low = fetch_op();
    cycle(1);
    bus.write(operandnew.low + yreg, val);
    cycle(2);
    last_cycle();
}

void CPU::addrmode_abs_write(uint8_t val)
{
    operandnew.low = fetch_op();
    cycle(1);
    operandnew.high = fetch_op();
    cycle(1);
    bus.write(operandnew.reg, val);
    cycle(1);
    last_cycle();
}

void CPU::addrmode_absx_write(uint8_t val)
{
    operandnew.low = fetch_op();
    cycle(1);
    operandnew.high = fetch_op();
    cycle(1);
    bus.write(operandnew.reg + xreg, val);
    cycle(2);
    last_cycle();
}

void CPU::addrmode_absy_write(uint8_t val)
{
    operandnew.low = fetch_op();
    cycle(1);
    operandnew.high = fetch_op();
    cycle(1);
    bus.write(operandnew.reg + yreg, val);
    cycle(2);
    last_cycle();
}

void CPU::addrmode_indx_write(uint8_t val)
{
    operandnew.low = fetch_op();
    cycle(1);
    uint16_t addr = buildval16(bus.read(operandnew.low+xreg), bus.read(operandnew.low+1+xreg));
    cycle(3);
    bus.write(addr, val);
    cycle(1);
    last_cycle();
}

void CPU::addrmode_indy_write(uint8_t val)
{
    operandnew.low = fetch_op();
    cycle(1);
    uint16_t addr = buildval16(bus.read(operandnew.low), bus.read(operandnew.low+1))+yreg;
    cycle(3);
    bus.write(addr, val);
    cycle(1);
    last_cycle();
}




void CPU::instr_branch(bool take)
{
    reg16 tmp;

    operandnew.low = fetch_op();
    tmp = pc;
    cycle(1);
    last_cycle();
    if (!take)
        return;
    pc.reg += (int8_t) operandnew.low;
    last_cycle();
    if (tmp.high != pc.high)
        cycle(1);
}

void CPU::instr_flag(bool &flag, bool v)
{
    cycle(1);
    last_cycle();
    flag = v;
}

void CPU::instr_transfer(uint8_t from, uint8_t &to)
{
    cycle(1);
    last_cycle();
    to = from;
    procstatus.zero = (to == 0);
    procstatus.neg  = (to & 0x80);
}



// NOTE: all instruction functions.
void CPU::instr_lda(const uint8_t val)
{
    accum = val;
    procstatus.zero = accum == 0;
    procstatus.neg  = accum & 0x80;
}

void CPU::instr_ldx(const uint8_t val)
{
    xreg = val;
    procstatus.zero = xreg == 0;
    procstatus.neg  = xreg & 0x80;
}

void CPU::instr_ldy(const uint8_t val)
{
    yreg = val;
    procstatus.zero = yreg == 0;
    procstatus.neg  = yreg & 0x80;
}

void CPU::instr_cmp(const uint8_t val)
{
    int res = accum-val;
    procstatus.zero     = res == 0;
    procstatus.neg      = res & 0x80;
    procstatus.carry    = res >= 0;
}

void CPU::instr_cpx(const uint8_t val)
{
    int res = xreg-val;
    procstatus.zero     = res == 0;
    procstatus.neg      = res & 0x80;
    procstatus.carry    = res >= 0;
}

void CPU::instr_cpy(const uint8_t val)
{
    int res = yreg-val;
    procstatus.zero     = res == 0;
    procstatus.neg      = res & 0x80;
    procstatus.carry    = res >= 0;
}

void CPU::instr_adc(const uint8_t val)
{
    int res = accum + val + procstatus.carry;
    procstatus.zero     = (uint8_t) res == 0;
    procstatus.neg      = res & 0x80;
    procstatus.carry    = res > 0xFF;
    procstatus.ov       = (accum^res) & ~(accum^val) & 0x80;
    accum = res;
}

void CPU::instr_sbc(const uint8_t val)
{
    uint8_t tmp = ~val;
    int res = accum + tmp + procstatus.carry;
    procstatus.zero     = (uint8_t) res == 0;
    procstatus.neg      = res & 0x80;
    procstatus.carry    = res > 0xFF;
    procstatus.ov       = (accum^res) & ~(accum^val) & 0x80;
    accum = res;
}

void CPU::instr_ora(const uint8_t val)
{
    accum |= val;
    procstatus.neg  = accum & 0x80;
    procstatus.zero = accum == 0;
}

void CPU::instr_and(const uint8_t val)
{
    accum &= val;
    procstatus.neg  = accum & 0x80;
    procstatus.zero = accum == 0;
}

void CPU::instr_eor(const uint8_t val)
{
    accum ^= val;
    procstatus.neg  = accum & 0x80;
    procstatus.zero = accum == 0;
}

void CPU::instr_bit(const uint8_t val)
{
    procstatus.neg  = (accum & val) == 0;
    procstatus.zero = val == 0;
    procstatus.ov   = val & 0x40;
}



uint8_t CPU::instr_inc(uint8_t val)
{
    val++;
    procstatus.zero = val == 0;
    procstatus.neg  = val & 0x80;
    return val;
}

uint8_t CPU::instr_dec(uint8_t val)
{
    val--;
    procstatus.zero = val == 0;
    procstatus.neg  = val & 0x80;
    return val;
}

uint8_t CPU::instr_asl(uint8_t val)
{
    procstatus.carry = val & 0x80;
    val <<= 1;
    procstatus.zero = val == 0;
    procstatus.neg  = val & 0x80;
    return val;
}

uint8_t CPU::instr_lsr(uint8_t val)
{
    procstatus.carry = val & 1;
    val >>= 1;
    procstatus.zero = val == 0;
    procstatus.neg  = val & 0x80;
    return val;
}

uint8_t CPU::instr_rol(uint8_t val)
{
    bool c = procstatus.carry;
    procstatus.carry = val & 0x80;
    val = val << 1 | c;
    procstatus.zero = val == 0;
    procstatus.neg  = val & 0x80;
    return val;
}

uint8_t CPU::instr_ror(uint8_t val)
{
    bool c = procstatus.carry;
    procstatus.carry = val & 1;
    val = val >> 1 | c << 7;
    procstatus.zero = val == 0;
    procstatus.neg  = val & 0x80;
    return val;
}



#define func_increase(reg, regname) \
void CPU::instr_in##reg() \
{ \
    cycle(1); \
    last_cycle(); \
    regname++; \
    procstatus.zero = xreg == 0; \
    procstatus.neg  = xreg & 0x80; \
}
func_increase(x, xreg)
func_increase(y, yreg)

#define func_decrease(reg, regname) \
void CPU::instr_de##reg() \
{ \
    cycle(1); \
    last_cycle(); \
    regname--; \
    procstatus.zero = xreg == 0; \
    procstatus.neg  = xreg & 0x80; \
}
func_decrease(x, xreg)
func_decrease(y, yreg)

#undef func_increase
#undef func_decrease

void CPU::instr_php()
{
    procstatus.breakf = 1;
    push(procstatus.reg());
    procstatus.breakf = 0;
    cycle(2);
    last_cycle();
}

void CPU::instr_pha()
{
    push(accum);
    cycle(2);
    last_cycle();
}

void CPU::instr_plp()
{
    cycle(3);
    last_cycle();
    procstatus = pull();
    procstatus.breakf = 0;
}

void CPU::instr_pla()
{
    accum = pull();
    procstatus.zero = accum == 0;
    procstatus.neg  = accum & 0x80;
    cycle(3);
    last_cycle();
}

void CPU::instr_jsr()
{
    operandnew.low = fetch_op();
    cycle(1);
    operandnew.high = fetch_op();
    cycle(1);
    pc.reg--;
    push(pc.high);
    cycle(1);
    push(pc.low);
    cycle(1);
    pc.reg = operandnew.reg;
    cycle(1);
    last_cycle();
}

void CPU::instr_jmp()
{
    operandnew.low = fetch_op();
    cycle(1);
    operandnew.high = fetch_op();
    cycle(1);
    pc.reg = operandnew.reg;
    last_cycle();
}

/* We could have another addressing mode function for this... but I decided
 * I'd rather have 1 less function and call this one directly as it's used
 * by one instruction */
void CPU::instr_jmp_ind()
{
    operandnew.low = fetch_op();
    cycle(1);
    operandnew.high = fetch_op();
    cycle(1);
    uint16_t addr = operandnew.reg;
    // Hardware bug.
    if (operandnew.low == 0xFF) {
        pc.reg = buildval16(bus.read(addr), bus.read(addr & 0xFF00));
    } else
        pc.reg = buildval16(bus.read(addr), bus.read(addr+1));
    cycle(3);
    last_cycle();
}

void CPU::instr_rts()
{
    uint8_t low = pull();
    cycle(1);
    pc.reg = buildval16(low, pull());
    cycle(1);
    pc.reg++;
    cycle(3);
    last_cycle();
}

void CPU::instr_brk()
{
    procstatus.breakf = 1;
    // cycles are counted in the interrupt function
    interrupt(IRQBRKVEC);
    // the break flag only exists in the copy, so reset it here
    procstatus.breakf = 0;
}

void CPU::instr_rti()
{
    procstatus = pull();
    procstatus.breakf = 0;
    cycle(1);
    uint8_t low = pull();
    cycle(1);
    pc.reg = buildval16(low, pull());
    cycle(1);
    cycle(2);
    last_cycle();
}

void CPU::instr_nop()
{
    cycle(1);
    last_cycle();
}

