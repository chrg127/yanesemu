/* to be included in cpu.h */

typedef void (CPU::*OpcodeFuncRead)(const uint8_t);
typedef uint8_t (CPU::*OpcodeFuncMod)(uint8_t);


/* to ensure correctness (and due to the fact that the cycles taken by each
 * opcode is dependent by what the opcode does), split opcodes into four
 * categories: read, write, modify and other. (this is similar to what
 * bsnes does) */

/* read */
void addrmode_imm_read(OpcodeFuncRead f);
void addrmode_zero_read(OpcodeFuncRead f);
void addrmode_zerox_read(OpcodeFuncRead f);
void addrmode_zeroy_read(OpcodeFuncRead f);
void addrmode_abs_read(OpcodeFuncRead f);
void addrmode_absx_read(OpcodeFuncRead f);
void addrmode_absy_read(OpcodeFuncRead f);
void addrmode_indx_read(OpcodeFuncRead f);
void addrmode_indy_read(OpcodeFuncRead f);
/* modify */
void addrmode_accum_modify(OpcodeFuncMod f);
void addrmode_zero_modify(OpcodeFuncMod f);
void addrmode_zerox_modify(OpcodeFuncMod f);
void addrmode_zeroy_modify(OpcodeFuncMod f);
void addrmode_abs_modify(OpcodeFuncMod f);
void addrmode_absx_modify(OpcodeFuncMod f);
void addrmode_absy_modify(OpcodeFuncMod f);
void addrmode_indx_modify(OpcodeFuncMod f);
void addrmode_indy_modify(OpcodeFuncMod f);
/* write (used just by STA, STX and STY, which is also why there are no
 * instr_func for those */
void addrmode_zero_write(uint8_t val);
void addrmode_zerox_write(uint8_t val);
void addrmode_zeroy_write(uint8_t val);
void addrmode_abs_write(uint8_t val);
void addrmode_absx_write(uint8_t val);
void addrmode_absy_write(uint8_t val);
void addrmode_indx_write(uint8_t val);
void addrmode_indy_write(uint8_t val);

/* other */
void instr_branch(bool take);
void instr_push(uint8_t val);
void instr_flag(bool &flag, bool v);
void instr_transfer(uint8_t from, uint8_t &to);

/* read instructions
 * these just need to read their operand. */
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

/* modify instructions
 * these do need to modify their operand. they usually act on an address
 * and sometimes with the accumulator */
uint8_t instr_inc(uint8_t val);
uint8_t instr_dec(uint8_t val);
uint8_t instr_asl(uint8_t val);
uint8_t instr_lsr(uint8_t val);
uint8_t instr_rol(uint8_t val);
uint8_t instr_ror(uint8_t val);

/* other instructions
 * outliers. they are called directly instead of pssing from
 * an addrmode_function */
void instr_inx();
void instr_iny();
void instr_dex();
void instr_dey();
void instr_plp();
void instr_pla();
void instr_jsr();
void instr_jmp();
void instr_jmp_ind();
void instr_rts();
void instr_brk();
void instr_rti();
void instr_nop();

/* opcode functions missing (as they are not needed:
 * - STA, STX, STY
 * - PHP, PHA
 * - BEQ, BNE, BMI, BPL, BVC, BVS, BCC, BCS
 * - TAX, TXA, TAY, TYA, TXS, TSX
 * . SEC, CLC, SEI, CLI, CLV, CLD
 * these all have a dedicated addrmode_function.
 */

