MAPPER_NUM = 0
MIRRORING  = 1 ; vertical mirroring
USE_SRAM   = 0

;; header
; PRG size: 16 KiB, CHR size: 8 KiB
.segment "HEADER"
.byte 'N', 'E', 'S', $1A
.byte $02
.byte $01
.byte MIRRORING | (USE_SRAM << 1) | ((MAPPER_NUM & $f) << 4)
.byte (MAPPER_NUM & $f0)
.byte $0, $0, $0, $0, $0, $0, $0, $0

.segment "TILES"
.incbin "background.chr"
.incbin "sprite.chr"

.segment "VECTORS"
.word nmi
.word reset
.word irq

.segment "ZEROPAGE"
tmp1:           .res 1
tmp2:           .res 1
tmp3:           .res 1
tmp4:           .res 1
tmp5:           .res 1
tmp6:           .res 1
tmp7:           .res 1
tmp8:           .res 1
scroll_y:       .res 1 ; x scroll position
scroll_x:       .res 1 ; y scroll position
nmi_flag:       .res 1
buttons:        .res 1 ; bit vector storing pressed buttons. format: ABSSUDLR

.segment "RAM"
spritebuf:      .res 256

.segment "RODATA"
paltab:
.byte $0F,$15,$26,$37 ; bg0 purple/pink
.byte $0F,$09,$19,$29 ; bg1 green
.byte $0F,$01,$11,$21 ; bg2 blue
.byte $0F,$00,$10,$30 ; bg3 greyscale
.byte $0F,$18,$28,$38 ; sp0 yellow
.byte $0F,$14,$24,$34 ; sp1 purple
.byte $0F,$1B,$2B,$3B ; sp2 teal
.byte $0F,$12,$22,$32 ; sp3 marine
message_string: .byte "press a button"
button_string:  .byte "abssudlr" ;.byte $61, $62, $73, $73, $75, $64, $6C, $72

.segment "CODE"
reset:
    sei             ; disable all interrupts
    cld
    ldx #%01000000  ; set apu frame sequencer interrupt
    stx $4017
    ldx #$FF        ; reset stack position
    txs
    inx
    stx $2000       ; turn off screen (ctrl = 0, mask = 0)
    stx $2001
    lda $2002       ; reset $2005 and $2006 latch
    jsr waitvblank  ; wait 2 frames
    jsr waitvblank
    txa

resetram:
    sta $00,x
    sta $0100,x
    sta $0200,x
    sta $0300,x
    sta $0400,x
    sta $0500,x
    sta $0600,x
    sta $0700,x
    inx
    bne resetram

    lda #0          ; disable all apu channels
    sta $4015

    jsr waitvblank
    jsr clear_spbuf
    jsr clearnt
    jsr loadpal

    lda #$00
    sta scroll_y
    sta scroll_x
    jsr waitvblank
    jsr game_init
    lda #$20
    sta $2006
    lda #$00
    sta $2006
    sta $2005
    lda #%10100000 ; enable nmi, set sprite height as 16
    sta $2000
    lda #%00011110 ; enable rendering of background and sprites
    sta $2001

mainloop:
    jsr waitnmi
    jsr joypad_poll
    jsr gamecode
    jmp mainloop

waitnmi:
    lda nmi_flag
waitnmi_wait:
    cmp nmi_flag
    beq waitnmi_wait
    lda #0
    sta nmi_flag
    rts

wait_sprite_zero_hit:
    lda $2002
    and #$40
    beq wait_sprite_zero_hit
    rts

waitvblank:
    lda $2002
    bpl waitvblank
    rts

clear_spbuf:
    ldx #$00
    lda #$ff ; put all sprites below screen
clear_spbuf_loop:
    sta spritebuf, x
    inx
    bne clear_spbuf_loop
    rts

clearnt:
    lda $2002
    lda #$20
    sta $2006
    lda #$00
    sta $2006
    lda #$00
    ldy #$08
    ldx #$00
clearnt_loop:
    sta $2007
    dex
    bne clearnt_loop
    dey
    bne clearnt_loop
    rts

loadpal:
    lda $2002
    lda #$3F
    sta $2006
    lda #$00
    sta $2006
    ldx #0
loadpal_loop:
    lda paltab,x
    sta $2007
    inx
    cpx #32
    bne loadpal_loop
    rts

; run before rendering is enabled
game_init:
    lda #<message_string
    sta 0
    lda #>message_string
    sta 1
    lda #$21
    sta 2
    lda #$48
    sta 3
    lda #14
    sta 4
    jsr write_string
    lda #$23
    sta $2006
    lda #$c0
    sta $2006
    lda #$ff
    ldx #0
attr_loop:
    sta $2007
    inx
    cpx #$40
    bne attr_loop
    rts

joypad_poll:
    lda #1
    sta $4016
    sta buttons
    lsr a
    sta $4016
joypad_poll_loop:
    lda $4016
    lsr a
    rol buttons
    bcc joypad_poll_loop
    rts

gamecode:
    jsr write_buttons
    ; lda #0
    ; sta $2005
    ; sta $2005
    rts

write_buttons:
    lda #$80
    sta tmp1
    ldy #0
write_buttons_loop:
    lda buttons
    and tmp1
    beq write_buttons_jump
    lda button_string,y
    jmp write_buttons_continue
write_buttons_jump:
    lda #0
write_buttons_continue:
    sta $0400,y
    iny
    lsr tmp1
    bne write_buttons_loop
    iny
    lda #0
    sta $0400,y
    rts

; 00 = pointer to string, 02: position, 04: string size (< 256)
; only usable during vblank
write_string:
    lda $2002
    lda $02
    sta $2006
    lda $03
    sta $2006
    ldy #$00
write_string_loop:
    lda ($00),y
    sta $2007
    iny
    cpy $04
    bne write_string_loop
    rts

; X = index into spritebuf, 01 = pointer to sprite
create_sprite:
    ldy #0
create_sprite_loop:
    lda ($00),y
    sta spritebuf,x
    inx
    iny
    cpy #4
    bne create_sprite_loop
    rts

nmi:
    pha
    txa
    pha
    tya
    pha
    inc nmi_flag

    lda #0       ; upload sprite data
    sta $2003
    lda #2
    sta $4014

    lda #$04
    sta 1
    lda #0
    sta 0
    lda #$21
    sta 2
    lda #$8B
    sta 3
    lda #8
    sta 4
    jsr write_string

    lda scroll_y
    sta $2005
    lda scroll_x
    sta $2005

    pla
    tay
    pla
    tax
    pla
irq:
    rti
