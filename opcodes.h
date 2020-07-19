inline void instr_brk_impl()
{

}



inline void instr_ora_imm()
{
    accum |= fetch_op();
}

inline void instr_ora_zero()
{
    accum |= read_mem(fetch_op());
}

inline void instr_ora_zerox()
{
    accum |= read_mem(fetch_op() + xreg);
}

inline void instr_ora_abs()
{
    uint8_t op1 = fetch_op();
    accum |= read_mem(build_addr(op1, fetch_op));
}

inline void instr_ora_absx()
{
    uint8_t op1 = fetch_op();
    accum |= read_mem(build_addr(op1, fetch_op)+xreg);
}

inline void instr_ora_absy()
{
    uint8_t op1 = fetch_op();
    accum |= read_mem(build_addr(op1, fetch_op)+yreg);
}

inline void instr_ora_indx()
{
    uint8_t op = fetch_op() + xreg;
    uint8_t low = read_mem(op);
    accum |= read_mem1(build_addr(low, read_mem(op+1));
}

inline void instr_ora_indy()
{
    uint8_t addr = read_mem(fetch_op());
    accum |= read_mem(addr + yreg);
}



inline void instr_asl_accum()
{

}

inline void instr_asl_zero()
{

}

inline void instr_asl_zerox()
{

}

inline void instr_asl_abs()
{

}

inline void instr_asl_absx()
{

}



inline void instr_php_impl()
{

}

inline void instr_bpl_impl()
{

}

inline void instr_clc_impl()
{

}

inline void instr_jsr_abs()
{

}

inline void instr_and_indx()
{

}

inline void instr_bit_zero()
{

}

inline void instr_and_zero()
{

}

inline void instr_rol_zero()
{

}

inline void instr_plp_impl()
{

}

inline void instr_and_imm()
{

}

inline void instr_rol_accum()
{

}

inline void instr_bit_abs()
{

}

inline void instr_and_abs()
{

}

inline void instr_rol_abs()
{

}

inline void instr_bmi_impl()
{

}

inline void instr_and_indy()
{

}

inline void instr_and_zerox()
{

}

inline void instr_rol_zerox()
{

}

inline void instr_sec_impl()
{

}

inline void instr_and_absy()
{

}

inline void instr_and_absx()
{

}

inline void instr_rol_absx()
{

}

inline void instr_rti_impl()
{

}

inline void instr_eor_indx()
{

}

inline void instr_eor_zero()
{

}

inline void instr_lsr_zero()
{

}

inline void instr_pha_impl()
{

}

inline void instr_eor_imm()
{

}

inline void instr_lsr_accum()
{

}

inline void instr_jmp_abs()
{

}

inline void instr_eor_abs()
{

}

inline void instr_lsr_abs()
{

}

inline void instr_bvc_impl()
{

}

inline void instr_eor_indy()
{

}

inline void instr_eor_zerox()
{

}

inline void instr_lsr_zerox()
{

}

inline void instr_cli_impl()
{

}

inline void instr_eor_absy()
{

}

inline void instr_eor_absx()
{

}

inline void instr_lsr_absx()
{

}

inline void instr_rts_impl()
{

}

inline void instr_adc_indx()
{

}

inline void instr_adc_zero()
{

}

inline void instr_ror_zero()
{

}

inline void instr_pla_impl()
{

}

inline void instr_adc_imm()
{

}

inline void instr_ror_accum()
{

}

inline void instr_jmp_ind()
{

}

inline void instr_adc_abs()
{

}

inline void instr_ror_abs()
{

}

inline void instr_bvs_impl()
{

}

inline void instr_adc_indy()
{

}

inline void instr_adc_zerox()
{

}

inline void instr_ror_zerox()
{

}

inline void instr_sei_impl()
{

}

inline void instr_adc_absy()
{

}

inline void instr_adc_absx()
{

}

inline void instr_ror_absx()
{

}

inline void instr_sta_indx()
{

}

inline void instr_sty_zero()
{

}

inline void instr_sta_zero()
{

}

inline void instr_stx_zero()
{

}

inline void instr_dey_impl()
{

}

inline void instr_txa_impl()
{

}

inline void instr_sty_abs()
{

}

inline void instr_sta_abs()
{

}

inline void instr_stx_abs()
{

}

inline void instr_bcc_impl()
{

}

inline void instr_sta_indy()
{

}

inline void instr_sty_zerox()
{

}

inline void instr_sta_zerox()
{

}

inline void instr_stx_zeroy()
{

}

inline void instr_tya_impl()
{

}

inline void instr_sta_absy()
{

}

inline void instr_txs_impl()
{

}

inline void instr_sta_absx()
{

}

inline void instr_ldy_imm()
{

}

inline void instr_lda_indx()
{

}

inline void instr_ldx_imm()
{

}

inline void instr_ldy_zero()
{

}

inline void instr_lda_zero()
{

}

inline void instr_ldx_zero()
{

}

inline void instr_tay_impl()
{

}

inline void instr_lda_imm()
{

}

inline void instr_tax_impl()
{

}

inline void instr_ldy_abs()
{

}

inline void instr_lda_abs()
{

}

inline void instr_ldx_abs()
{

}

inline void instr_bcs_impl()
{

}

inline void instr_lda_indy()
{

}

inline void instr_ldy_zerox()
{

}

inline void instr_lda_zerox()
{

}

inline void instr_ldx_zeroy()
{

}

inline void instr_clv_impl()
{

}

inline void instr_lda_absy()
{

}

inline void instr_tsx_impl()
{

}

inline void instr_ldy_absx()
{

}

inline void instr_lda_absx()
{

}

inline void instr_ldx_absy()
{

}

inline void instr_cpy_imm()
{

}

inline void instr_cmp_indx()
{

}

inline void instr_cpy_zero()
{

}

inline void instr_cmp_zero()
{

}

inline void instr_dec_zero()
{

}

inline void instr_iny_impl()
{

}

inline void instr_cmp_imm()
{

}

inline void instr_dex_impl()
{

}

inline void instr_cpy_abs()
{

}

inline void instr_cmp_abs()
{

}

inline void instr_dec_abs()
{

}

inline void instr_bne_impl()
{

}

inline void instr_cmp_indy()
{

}

inline void instr_cmp_zerox()
{

}

inline void instr_dec_zerox()
{

}

inline void instr_cmp_absy()
{

}

inline void instr_cmp_absx()
{

}

inline void instr_dec_absx()
{

}

inline void instr_cpx_imm()
{

}

inline void instr_sbc_indx()
{

}

inline void instr_cpx_zero()
{

}

inline void instr_sbc_zero()
{

}

inline void instr_inc_zero()
{

}

inline void instr_inx_impl()
{

}

inline void instr_sbc_imm()
{

}

inline void instr_nop_impl()
{

}

inline void instr_cpx_abs()
{

}

inline void instr_sbc_abs()
{

}

inline void instr_inc_abs()
{

}

inline void instr_beq_impl()
{

}

inline void instr_sbc_indy()
{

}

inline void instr_sbc_zerox()
{

}

inline void instr_inc_zerox()
{

}

inline void instr_sbc_absy()
{

}

inline void instr_sbc_absx()
{

}

inline void instr_inc_absx()
{

}

