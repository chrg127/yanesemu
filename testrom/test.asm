; constants
MAPPER = 0
MIRROR = 1
SRAM   = 0

.segment "HEADER"
.byte 'N', 'E', 'S', $1A ; ID
.byte $02                ; 16kib PRG chunk count
.byte $01                ; 8kib CHR chunk count
.byte MIRROR | (SRAM << 1) | ((MAPPER & $f) << 4)
.byte (MAPPER & %11110000)
.byte $0, $0, $0, $0, $0, $0, $0, $0

.segment "TILES"
.incbin "testbg.chr"
.incbin "testspr.chr"

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
scroll_x:       .res 1      ; x scroll position
scroll_y:       .res 1      ; y scroll position
nmi_flag:       .res 1

.segment "RAM"
spritebuf:      .res 256        ; sprite data to be uploaded by DMA
nmt_update:     .res 256        ; nametable update entry buffer for PPU update
palette:        .res 32         ; palette buffer for PPU update

.segment "RODATA"
paltab:
.byte $13,$15,$26,$37 ; bg0 purple/pink
.byte $0F,$09,$19,$29 ; bg1 green
.byte $0F,$01,$11,$21 ; bg2 blue
.byte $0F,$00,$10,$30 ; bg3 greyscale
.byte $13,$18,$28,$38 ; sp0 yellow
.byte $0F,$14,$24,$34 ; sp1 purple
.byte $0F,$1B,$2B,$3B ; sp2 teal
.byte $0F,$12,$22,$32 ; sp3 marine
hellostring: .byte $04, $04, $04, $09, $0A, $00, $0B, $0A, $0C, $09, $0D

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

    lda $2002   ; reset latch
    lda #$3F
    sta $2006   ; set v = 3F00
    lda #$00
    sta $2006
    ldx #0
loadpal_loop:   ; fill background palette
    lda paltab,x
    sta $2007
    inx
    cpx #32
    bne loadpal_loop

    lda #$20    ; reset v
    sta $2006
    lda #$00
    sta $2006

    lda #$00
    sta scroll_x
    sta scroll_y
    jsr write_helloworld    ; and write a 'hello world'
    jsr create_sprites
    jsr waitvblank
    lda #$20
    sta $2006
    lda #$00
    sta $2006
    sta $2005
    lda #$80    ;turn on nmi
    sta $2000
    lda #%00011110
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
    rts

waitvblank:
    lda $2002
    bpl waitvblank
    rts

clear_spbuf:
    ldx #$00
    lda #$ff                ;y = ff, below bottom of screen
clear_spbuf_loop:        ;puts all sprites off screen
    sta spritebuf, x
    inx
    bne clear_spbuf_loop
    rts

clearnt:
    lda $2002   ; set v = 2000
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

joypad_poll:
    rts

gamecode:
    rts

; writes a hello world at the center of the screen
; assumes we are at the start of the game
write_helloworld:
    lda #$20
    sta $02
    lda #$00
    sta $03
    lda #<hellostring
    sta $00
    lda #>hellostring
    sta $01
    lda #12
    sta $04
    jsr write_string

    lda $2002
    lda #$23
    sta $2006
    lda #$00
    sta $2006
    ldx #$FF
    lda #$aa
pal_loop:
    sta $2007
    dex
    bne pal_loop
    rts

; 00 = pointer to string, 02: position, 04: string size (< 256)
; void write_string(u8 *str, u8 pos[2], u8 size)
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

sprite1: .byte  4, 4, 1, 0
sprite2: .byte  60, 2, 0, 8
sprite3: .byte  60, 3, 0, 16
sprite4: .byte  60, 4, 0, 24
sprite5: .byte  60, 5, 0, 32
sprite6: .byte  60, 6, 0, 40
sprite7: .byte  60, 7, 0, 48
sprite8: .byte  60, 8, 0, 56

create_sprites:
    ldx #0

    lda #<sprite1
    sta $00
    lda #>sprite1
    sta $01
    jsr create_sprite

    lda #<sprite3
    sta $00
    lda #>sprite3
    sta $01
    jsr create_sprite

    lda #<sprite4
    sta $00
    lda #>sprite4
    sta $01
    jsr create_sprite

    lda #<sprite5
    sta $00
    lda #>sprite5
    sta $01
    jsr create_sprite

    lda #<sprite6
    sta $00
    lda #>sprite6
    sta $01
    jsr create_sprite

    lda #<sprite7
    sta $00
    lda #>sprite7
    sta $01
    jsr create_sprite

    lda #<sprite8
    sta $00
    lda #>sprite8
    sta $01
    jsr create_sprite

    ldx #252
    lda #<sprite2
    sta $00
    lda #>sprite2
    sta $01
    jsr create_sprite

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
    pha     ; save registers
    txa
    pha
    tya
    pha
    inc nmi_flag
    lda #$00
    sta $2003
    lda #2
    sta $4014

    ; should write to $2000 and $2005
    lda scroll_x
    sta $2005
    lda scroll_y
    sta $2005

    pla     ; restore registers
    tay
    pla
    tax
    pla
irq:
    rti


