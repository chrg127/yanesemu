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
.byte $0F,$15,$26,$37 ; bg0 purple/pink
.byte $0F,$09,$19,$29 ; bg1 green
.byte $0F,$01,$11,$21 ; bg2 blue
.byte $0F,$00,$10,$30 ; bg3 greyscale
.byte $0F,$18,$28,$38 ; sp0 yellow
.byte $0F,$14,$24,$34 ; sp1 purple
.byte $0F,$1B,$2B,$3B ; sp2 teal
.byte $0F,$12,$22,$32 ; sp3 marine

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

    jsr waitvblank  ; wait again...
    jsr clear_spbuf ; clear oam and nametables
    jsr clearnt

    lda $2002   ; reset latch
    lda #$3F
    sta $2006   ; set v = 3F00
    lda #$00
    sta $2006
    ldx #8
loadpal_loop:   ; fill background palette
    lda #$0F
    sta $2007
    lda #$00
    sta $2007
    lda #$10
    sta $2007
    lda #$30
    sta $2007
    dex
    bne loadpal_loop

    lda #$20    ; reset v
    sta $2006
    lda #$00
    sta $2006

    lda #$00
    sta scroll_x
    sta scroll_y
    jsr write_helloworld    ; and write a 'hello world'
    jsr waitvblank
    lda #$20
    sta $2006
    lda #$00
    sta $2006
    sta $2005
    lda #$80    ;turn on nmi
    sta $2000
    lda #%00001010
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

hellostring: .byte $07, $08, $09, $09, $0A, $00, $0B, $0A, $0C, $09, $0D
;hellostring: .byte $0E, $0F, $10, $11, $00, $09, $12, $13, $13, $12, $0C, $0A

; writes a hello world at the center of the screen
; assumes we are at the start of the game
write_helloworld:
    lda $2002
    lda #$21    ; set starting pos to somewhat into the center
    sta $2006
    lda #$EC
    sta $2006
    ldx #$00
write_loop:
    lda hellostring,x
    sta $2007
    inx
    cpx #12
    bne write_loop
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
    lda spritebuf
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

