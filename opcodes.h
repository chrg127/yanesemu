/*
 * DO NOT #include this file normally. It must be #include'd only inside the CPU class in cpu.h.
 */

typedef uint8_t (CPU::*OpcodeFuncVal)(uint8_t);
typedef void (CPU::*OpcodeFuncVoid)();
typedef void (CPU::*OpcodeFuncJmp)(uint16_t);

// NOTE: addressing mode functions.
inline void addrmode_impl(OpcodeFuncVoid f)
{
    (this->*f)();
}

inline void addrmode_imm(OpcodeFuncVal f)
{
    (this->*f)(fetch_op());
}

inline void addrmode_accum(OpcodeFuncVal f)
{
    accum = (this->*f)(accum);
}

inline void addrmode_zero(OpcodeFuncVal f)
{
    write_mem( (this->*f)(read_mem( fetch_op() )) );
}

inline void addrmode_zerox(OpcodeFuncVal f)
{
    uint8_t op = fetch_op() + xreg;
    write_mem( (this->*f)(read_mem(op)) );
}

inline void addrmode_zeroy(OpcodeFuncVal f)
{
    uint8_t op = fetch_op() + yreg;
    write_mem( (this->*f)(read_mem(op)) );
}

inline void addrmode_abs(OpcodeFuncVal f)
{
    uint8_t low = fetch_op();
    uint16_t addr = buildval16(low, fetch_op());
    write_mem( (this->*f)(read_mem(addr)) );
}

inline void addrmode_absjmp(OpcodeFuncJmp f)
{
    uint8_t low = fetch_op();
    uint16_t addr = buildval16(low, fetch_op());
    (this->*f)(addr);
}

inline void addrmode_absx(OpcodeFuncVal f)
{
    uint8_t low = fetch_op();
    uint16_t addr = buildval16(low, fetch_op()) + xreg;
    write_mem( (this->*f)(read_mem(addr)) );
}

inline void addrmode_absy(OpcodeFuncVal f)
{
    uint8_t low = fetch_op();
    uint16_t addr = buildval16(low, fetch_op()) + yreg;
    write_mem( (this->*f)(read_mem(addr)) );
}

inline void addrmode_indjmp(OpcodeFuncJmp f)
{
    uint8_t low = fetch_op();
    uint16_t addr = buildval16(low, fetch_op());
    (this->*f)(read_mem(addr));
}

inline void addrmode_indx(OpcodeFuncVal f)
{
    uint8_t op = fetch_op() + xreg;
    uint8_t low = read_mem(op);
    uint16_t addr = buildval16(low, read_mem(op+1));
    write_mem( (this->*f)(read_mem(addr)) );
}

inline void addrmode_indy(OpcodeFuncVal f)
{
    uint8_t op = fetch_op();
    uint8_t low = read_mem(op);
    uint16_t addr = buildval16(low, read_mem(op+1))+yreg;
    write_mem( (this->*f)(read_mem(addr)) );
}


// NOTE: all instruction functions.
void instr_brk()
{
    push(pc);
    push(procstatus.reg);
    uint8_t low = read_mem(IRQBRKVEC);
    pc = buildval16(low, read_mem(IRQBRKVEC+1));
    procstatus.breakc = 1;
}

#define load_func(regist, regname) \
uint8_t instr_ld##regist(uint8_t val) \
{ \
    regname = val; \
    procstatus.zero = (regname == 0); \
    procstatus.neg = (regname & 0x80); \
    return val; \
}

load_func(a, accum)
load_func(x, xreg)
load_func(y, yreg)

#define store_func(regist, regname) \
uint8_t instr_st##regist(uint8_t val) \
{ \
    val = regname; \
    procstatus.zero = (regname == 0); \
    procstatus.neg = (regname & 0x80); \
    return val; \
}

store_func(a, accum)
store_func(x, xreg)
store_func(y, yreg)

#define inc_func(name, regname) \
void instr_##name() { regname++; }

inc_func(inx, xreg)
inc_func(iny, yreg)

uint8_t instr_inc(uint8_t val)
{
    return ++val;
}

#define dec_func(name, regname) \
void instr_##name() \
{ \
    regname--; \
}
dec_func(dex, xreg)
dec_func(dey, yreg)

uint8_t instr_dec(uint8_t val)
{
    return --val;
}

#define transfer_func(regist1, reg1name, regist2, reg2name) \
    void instr_t##regist1##regist2() { reg2name = reg1name; }
transfer_func(x, xreg, a, accum)
transfer_func(y, yreg, a, accum)
transfer_func(a, accum, x, xreg)
transfer_func(a, accum, y, yreg)
transfer_func(s, sp, x, xreg)
transfer_func(x, xreg, s, sp)

#define cmp_func(name, regname) \
uint8_t instr_##name(uint8_t val) \
{ \
    uint8_t res = regname-val; \
    procstatus.zero     = (res == 0) ? 1 : 0; \
    procstatus.ov       = ((res & 0x80) != (regname & 1)) ? 1 : 0; \
    procstatus.carry    = procstatus.ov; \
    procstatus.neg      = res & 0x80; \
    return val; \
}
cmp_func(cmp, accum)
cmp_func(cpx, xreg)
cmp_func(cpy, yreg)


uint8_t instr_adc(uint8_t val)
{
    uint8_t sign = (accum & 0x80) >> 8;
    accum += val + procstatus.carry;
    procstatus.zero = (accum == 0);
    procstatus.ov   = ((accum & 0x80) != sign);
    procstatus.carry = procstatus.ov;
    procstatus.neg  = accum & 0x80;
    return val;
}

uint8_t instr_sbc(uint8_t val)
{
    uint8_t sign = (accum & 0x80) >> 8;
    accum -= val - (1-procstatus.carry);
    procstatus.zero = (accum == 0);
    procstatus.ov   = ((accum & 0x80) != sign);
    procstatus.carry = procstatus.ov;
    procstatus.neg  = accum & 0x80;
    return val;
}

#define clearflag_func(name, flag) \
void instr_##name() { flag = 0; }
#define setflag_func(name, flag) \
void instr_##name() { flag = 1; }

clearflag_func(clc, procstatus.carry)
clearflag_func(clv, procstatus.ov)
clearflag_func(cli, procstatus.intdis)
setflag_func(sec, procstatus.carry)
setflag_func(sei, procstatus.intdis)

uint8_t instr_ora(uint8_t val)
{
    accum |= val;
    procstatus.neg  = accum & 0x80;
    procstatus.zero = (accum == 0);
    return val;
}

uint8_t instr_and(uint8_t val)
{
    accum &= val;
    procstatus.neg  = accum & 0x80;
    procstatus.zero = (accum == 0);
    return val;
}

uint8_t instr_eor(uint8_t val)
{
    accum ^= val;
    procstatus.neg  = accum & 0x80;
    procstatus.zero = (accum == 0);
    return val;
}

uint8_t instr_bit(uint8_t val)
{
    uint8_t res = val & accum;
    procstatus.neg  = res & 0x80;
    procstatus.zero = (res == 0);
    procstatus.ov   = res & 0x40;
    return val;
}

uint8_t instr_asl(uint8_t val)
{
    procstatus.carry = val & 0x80;
    val <<= 1;
    procstatus.neg  = val & 0x80;
    procstatus.zero = (val == 0);
    return val;
}

uint8_t instr_lsr(uint8_t val)
{
    procstatus.carry = val & 1;
    val >>= 1;
    procstatus.neg  = val & 0x80;
    procstatus.zero = (val == 0);
    return val;
}

uint8_t instr_rol(uint8_t val)
{
    uint8_t bit0 = procstatus.carry;
    procstatus.carry = val & 0x80;
    val <<= 1;
    val |= bit0;
    procstatus.neg  = val & 0x80;
    procstatus.zero = (val == 0);
    return val;
}

uint8_t instr_ror(uint8_t val)
{
    uint8_t bit7 = procstatus.carry << 8;
    procstatus.carry = val & 1;
    val >>= 1;
    val |= bit7 << 8;
    procstatus.neg  = val & 0x80;
    procstatus.zero = (val == 0);
    return val;
}

void instr_php()
{
    push(procstatus.reg);
}

void instr_plp()
{
    procstatus.reg = pull();
}

void instr_pha()
{
    push(accum);
}

void instr_pla()
{
    accum = pull();
}


#define branch_fset_func(name, flag) \
void instr_##name() { pc += fetch_op()*flag; }
#define branch_fcl_func(name, flag) \
void instr_##name() { pc += fetch_op()*(flag+1); }

branch_fset_func(bmi, procstatus.neg)
branch_fset_func(bvs, procstatus.ov)
branch_fset_func(bcs, procstatus.carry)
branch_fset_func(beq, procstatus.zero)
branch_fcl_func(bpl, procstatus.neg)
branch_fcl_func(bvc, procstatus.ov)
branch_fcl_func(bcc, procstatus.carry)
branch_fcl_func(bne, procstatus.zero)


void instr_jmp(uint16_t addr)
{
    pc = addr;
}

void instr_jsr(uint16_t addr)
{
    push(pc >> 8);
    push(pc & 0xFF);
    pc = addr;
}

void instr_rts()
{
    pc = pull()-1;
}

void instr_rti()
{
    procstatus.reg = pull();
    pc = pull();
}

void instr_nop() { }

#undef branch_fset_func
#undef branch_fcl_func
#undef inc_func
#undef cmp_func
#undef clearflag_func
#undef setflag_func
#undef store_func
#undef dec_func
#undef transfer_func
#undef load_func
