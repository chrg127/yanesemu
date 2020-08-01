/* to be included only by cpu.cpp */

// NOTE: addressing mode functions.
void CPU::addrmode_imm_read(OpcodeFuncRead f)
{
    uint8_t op = fetch_op();
    (this->*f)(op);
    DBGPRINTF(" #$%02X", op);
}

void CPU::addrmode_zero_read(OpcodeFuncRead f)
{
    uint8_t op = fetch_op();
    (this->*f)(bus.read(op));
    DBGPRINTF(" $%02X", op);
}

void CPU::addrmode_zerox_read(OpcodeFuncRead f)
{
    uint8_t op = fetch_op() + xreg;
    (this->*f)(bus.read(op));
    DBGPRINTF(" $%02X,x", op);
}

void CPU::addrmode_zeroy_read(OpcodeFuncRead f)
{
    uint8_t op = fetch_op() + yreg;
    (this->*f)(bus.read(op));
    DBGPRINTF(" $%02X,y", op);
}

void CPU::addrmode_abs_read(OpcodeFuncRead f)
{
    uint8_t low = fetch_op();
    uint16_t addr = buildval16(low, fetch_op());
    (this->*f)(bus.read(addr));
    DBGPRINTF(" $%04X", addr);
}

void CPU::addrmode_absx_read(OpcodeFuncRead f)
{
    uint8_t low = fetch_op();
    uint16_t addr = buildval16(low, fetch_op()) + xreg;
    (this->*f)(bus.read(addr));
    DBGPRINTF(" $%04X,x", addr);
}

void CPU::addrmode_absy_read(OpcodeFuncRead f)
{
    uint8_t low = fetch_op();
    uint16_t addr = buildval16(low, fetch_op()) + yreg;
    (this->*f)(bus.read(addr));
    DBGPRINTF(" $%04X,y", addr);
}

void CPU::addrmode_indx_read(OpcodeFuncRead f)
{
    uint8_t op = fetch_op() + xreg;
    uint8_t low = bus.read(op);
    uint16_t addr = buildval16(low, bus.read(op+1));
    (this->*f)(bus.read(addr));
    DBGPRINTF(" ($%02X,x)", op);
}

void CPU::addrmode_indy_read(OpcodeFuncRead f)
{
    uint8_t op = fetch_op();
    uint8_t low = bus.read(op);
    uint16_t addr = buildval16(low, bus.read(op+1))+yreg;
    (this->*f)(bus.read(addr));
    DBGPRINTF(" ($%02X),y", op);
}



void CPU::addrmode_accum_modify(OpcodeFuncMod f)
{
    accum = (this->*f)(accum);
    DBGPRINT(" A");
}

void CPU::addrmode_zero_modify(OpcodeFuncMod f)
{
    uint8_t op = fetch_op();
    uint8_t res = (this->*f)(bus.read(op));
    bus.write(op, res);
    DBGPRINTF(" $%02X", op);
}

void CPU::addrmode_zerox_modify(OpcodeFuncMod f)
{
    uint8_t op = fetch_op() + xreg;
    uint8_t res = (this->*f)(bus.read(op));
    bus.write(op, res);
    DBGPRINTF(" $%02X,x", op);
}

void CPU::addrmode_zeroy_modify(OpcodeFuncMod f)
{
    uint8_t op = fetch_op() + yreg;
    uint8_t res = (this->*f)(bus.read(op));
    bus.write(op, res);
    DBGPRINTF(" $%02X,y", op);
}

void CPU::addrmode_abs_modify(OpcodeFuncMod f)
{
    uint8_t low = fetch_op();
    uint16_t addr = buildval16(low, fetch_op());
    uint8_t res = (this->*f)(bus.read(addr));
    bus.write(addr, res);
    DBGPRINTF(" $%04X", addr);
}

void CPU::addrmode_absx_modify(OpcodeFuncMod f)
{
    uint8_t low = fetch_op();
    uint16_t addr = buildval16(low, fetch_op()) + xreg;
    uint8_t res = (this->*f)(bus.read(addr));
    bus.write(addr, res);
    DBGPRINTF(" $%04X,x", addr);
}

void CPU::addrmode_absy_modify(OpcodeFuncMod f)
{
    uint8_t low = fetch_op();
    uint16_t addr = buildval16(low, fetch_op()) + yreg;
    uint8_t res = (this->*f)(bus.read(addr));
    bus.write(addr, res);
    DBGPRINTF(" $%04X,y", addr);
}

void CPU::addrmode_indx_modify(OpcodeFuncMod f)
{
    uint8_t op = fetch_op() + xreg;
    uint8_t low = bus.read(op);
    uint16_t addr = buildval16(low, bus.read(op+1));
    uint8_t res = (this->*f)(bus.read(addr));
    bus.write(addr, res);
    DBGPRINTF(" ($%02X,x)", op);
}

void CPU::addrmode_indy_modify(OpcodeFuncMod f)
{
    uint8_t op = fetch_op();
    uint8_t low = bus.read(op);
    uint16_t addr = buildval16(low, bus.read(op+1))+yreg;
    uint8_t res = (this->*f)(bus.read(addr));
    bus.write(addr, res);
    DBGPRINTF(" ($%02X),y", op);
}



void CPU::addrmode_zero_write(uint8_t val)
{
    uint8_t op = fetch_op();
    bus.write(op, val);
    DBGPRINTF(" $%02X", op);
}

void CPU::addrmode_zerox_write(uint8_t val)
{
    uint8_t op = fetch_op() + xreg;
    bus.write(op, val);
    DBGPRINTF(" $%02X,x", op);
}

void CPU::addrmode_zeroy_write(uint8_t val)
{
    uint8_t op = fetch_op() + yreg;
    bus.write(op, val);
    DBGPRINTF(" $%02X,y", op);
}

void CPU::addrmode_abs_write(uint8_t val)
{
    uint8_t low = fetch_op();
    uint16_t addr = buildval16(low, fetch_op());
    bus.write(addr, val);
    DBGPRINTF(" $%04X", addr);
}

void CPU::addrmode_absx_write(uint8_t val)
{
    uint8_t low = fetch_op();
    uint16_t addr = buildval16(low, fetch_op()) + xreg;
    bus.write(addr, val);
    DBGPRINTF(" $%04X,x", addr);
}

void CPU::addrmode_absy_write(uint8_t val)
{
    uint8_t low = fetch_op();
    uint16_t addr = buildval16(low, fetch_op()) + yreg;
    bus.write(addr, val);
    DBGPRINTF(" $%04X,y", addr);
}

void CPU::addrmode_indx_write(uint8_t val)
{
    uint8_t op = fetch_op() + xreg;
    uint8_t low = bus.read(op);
    uint16_t addr = buildval16(low, bus.read(op+1));
    bus.write(addr, val);
    DBGPRINTF(" ($%02X,x)", op);
}

void CPU::addrmode_indy_write(uint8_t val)
{
    uint8_t op = fetch_op();
    uint8_t low = bus.read(op);
    uint16_t addr = buildval16(low, bus.read(op+1))+yreg;
    bus.write(addr, val);
    DBGPRINTF(" ($%02X),y", op);
}




void CPU::instr_branch(bool take)
{
    uint8_t op = fetch_op();
    DBGPRINTF(" #$%02X", op);
    if (!take) {
        DBGPRINT(" [Branch not taken]");
        return;
    }
    pc += (int8_t) op;
    DBGPRINT(" [Branch taken]");
}

void CPU::instr_push(uint8_t val)
{
    push(val);
    cycle(3);
}

void CPU::instr_flag(bool &flag, bool v)
{
    flag = v;
}

void CPU::instr_transfer(uint8_t from, uint8_t &to)
{
    to = from;
    procstatus.zero = (to == 0);
    procstatus.neg  = (to & 0x80);
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


void CPU::instr_inx()
{
    xreg++;
    procstatus.zero = (xreg == 0);
    procstatus.neg  = (xreg & 0x80);
}

void CPU::instr_iny()
{
    yreg++;
    procstatus.zero = (xreg == 0);
    procstatus.neg  = (xreg & 0x80);
}

void CPU::instr_dex()
{
    xreg--;
    procstatus.zero = (xreg == 0);
    procstatus.neg  = (xreg & 0x80);
}

void CPU::instr_dey()
{
    yreg--;
    procstatus.zero = (xreg == 0);
    procstatus.neg  = (xreg & 0x80);
}

void CPU::instr_plp()
{
    procstatus = pull();
}

void CPU::instr_pla()
{
    accum = pull();
}

void CPU::instr_rts()
{
    uint8_t low = pull();
    uint16_t addr = buildval16(low, pull());
    pc = addr;
}

void CPU::instr_brk()
{
    push(pc);
    push(procstatus.reg());
    uint8_t low = bus.read(IRQBRKVEC);
    pc = buildval16(low, bus.read(IRQBRKVEC+1));
    procstatus.breakc = 1;
}

void CPU::instr_rti()
{
    procstatus = pull();
    pc = pull();
    // other stuff
}

void CPU::instr_nop()
{
    cycle(2);
}

void CPU::instr_jsr()
{
    uint8_t low = fetch_op();
    uint16_t addr = buildval16(low, fetch_op());
    push(pc >> 8);
    push(pc & 0xFF);
    pc = addr;
    DBGPRINTF(" $%04X", addr);
}

void CPU::instr_jmp()
{
    uint8_t low = fetch_op();
    uint16_t addr = buildval16(low, fetch_op());
    pc = addr;
    DBGPRINTF(" $%04X", addr);
}

void CPU::instr_jmp_ind()
{
    uint8_t low = fetch_op();
    uint16_t addr = buildval16(low, fetch_op());
    pc = bus.read(addr);
    DBGPRINTF(" ($%04X)", addr);
}
