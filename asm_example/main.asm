;======================================================
;Licensed under the 3-Clause BSD License
;Copyright 2021, Martin 'enthusi' Wendt / PriorArt
;Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
;
;1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
;
;2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
;
;3. Neither the name of the copyright holder nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.
;
;THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
;TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
;CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
;PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
;LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
;SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

;======================================================

; NOTE: ISAS is whitespace sensitive. The indentation in this file is required (although I don't like it)

        ISV810                  
        PUBALL                  
        CAPSON                  
        OFFBANKGROUP            
bank0 group 0

        FILE    vbdefines.asm

;some handy 8bit world macros, taken from redsquare/Kresna!
push    macro   op1
        add     -4, sp
        st.w    op1, $0[sp]
        endm

pop     macro   op1
        ld.w    $0[sp], op1
        add     4, sp
        endm

call    macro   op1
        push    r31
        jal     op1
        pop     r31
        endm

ret     macro
        jmp     [r31]
        endm

movw    macro   op1, op2
        movea   #op1'lo, r0, op2
        movhi   #op1'hi1, op2, op2
        endm

jump    macro   op1
        movw    op1, r30
        jmp     [r30]
        endm

; Start of RAM addresses
        org     $05000000

begin_variables

keypad_previous                 ds 2

end_variables

; Start of binary addresses
        org     $07000000

begin_binary

RESET_VEC:
begin_init
        ; stack pointer
        movw    $05008000, sp ;could be placed at very end of RAM as well :)

        ; Reset PSW
        ldsr r0, 5 
        
        sei
        ldsr r0, 24

        ; early mute
        movw    VSU_SSTOP, r30
        movea   $1, r0, r29
        st.h    r29, $0[r30]
        
        ; Extended WRAM warmup
        movw    $FFFF, r6
_warmup
        add     -1, r6
        bnz     _warmup

;init memory - can't hurt though technically not required
        movw VSU, r20
        movw VSU_END,r21
_loop1
        st.h r0, $0[r20]
        add     #2, r20
        cmp r20,r21
        bne _loop1

        movw WRAM, r20
        movw WRAM_END,r21
_loop2
        st.h r0, $0[r20]
        add     #2, r20
        cmp r20,r21
        bne _loop2
        
        movw VIP, r20
        movw VIP_END,r21
_loop3
        st.h r0, $0[r20]
        add     #2, r20
        cmp r20,r21
        bne _loop3
   
        movw    FRMCYC, r6
        st.h    r0, $0[r6] ;no frame delay
        
        ;nice short approach by GuyPerfect
        ; Configure the left and right column table in one go
        movhi column_table_data'hi1, r0, r10
        movea $3DC0, r0, r11  ; Start of column table
        shl   4, r11
        movea 510, r11, r12   ; End of column table
        movea 128, r0, r13    ; Remaining bytes

        ; Write all 128 bytes as halfwords to the four appropriate destinations
_column_loop
        in.b column_table_data'lo[r10], r14
        st.h r14, 0[r11]   ; Start of table, up
        st.h r14, 512[r11]
        add  1, r10        ; Breaks up the sequence of store instructions
        st.h r14, [r12]   ; End of table, down
        st.h r14, 512[r12]
        add  2, r11
        add  -2, r12
        add  -1, r13
        bnz  _column_loop

        ; Turn on the display
        mov     XP_XPEN | XP_XPRST, r29
        movw    XPCTRL, r30
        st.h    r29, [r30]
       
        movw    DP_SYNCE | DP_RE | DP_DISP, r29
        movw    DPCTRL, r30
        st.h    r29, [r30]
        
        movw palette_data, r29
        movw GPLT0, r28
        mov 8, r26
_fill_palette:
        ld.h    [r29], r25        
        st.h    r25, [r28]        
        add     2, r29              
        add     2, r28              
        add     -2, r26
        bnz     _fill_palette
        
        movw    keypad_previous, r6
        st.h    r0, [r6]
       
        movw    WCR, r6;wait state
        mov   %11, r7
        st.b    r7, [r6]
        
        ;ack pending IRQs
        movw    INTPND, r6
        movw    INTCLR, r7
        ld.h    [r6], r8
        st.h    r8, [r7]
        
        movw    INTENB, r6
        st.h    r0, [r6] ;disable all VIP IRQs
        cli

;==================================================
        ;set colors!
        movw    BRTA, r29
        movea   BRTA_DEFAULT, r0, r28
        movea   BRTB_DEFAULT, r0, r27
        movea   BRTC_DEFAULT, r0, r26
        st.h    r28, [r29]
        st.h    r27, $2[r29]
        st.h    r26, $4[r29]

        ;set up palettes including a simple fade
        movw GPLT0, r10
   
        movw (((1<<0)+(2<<2)+(3<<4))<<2),r11; %01010100,r11
        st.h r11,0[r10]
        
        movw (((0<<0)+(1<<2)+(2<<4))<<2),r11; %01010100,r11
        st.h r11,2[r10]
        
        movw (((3<<0)+(0<<2)+(1<<4))<<2),r11; %01010100,r11
        st.h r11,4[r10]
        
        movw (((2<<0)+(3<<2)+(0<<4))<<2),r11; %01010100,r11
        st.h r11,6[r10]

end_init

begin_genhex
generate_hexfont
        ;CHAR_TBL
        
        movw CHAR_TBL,r11 ;destination
        movw hexfontdata, r24
        mov 15,r23 ;how often to repeat MSB
        
        mov 0, r14; which character to use MSB
   
_charsetloop
        mov 0, r18; which character to use LSB
        mov 15,r15 ;how often to repeat LSB
_digitloop    
        ;msb source
        mov r24, r10 ;source
        mov r14,r16
        ;get offset in hexfontdata
        andi 1,r16,r17 ;0(2,4) or 1(3,5..)?
        add r17,r10
        shr 1,r16
        shl 4, r16 ;*8
        add r16,r10
        
        ;lsb source
        mov r24, r20 ;source
        mov r18,r21
        ;get offset in hexfontdata
        andi 1,r21,r22 ;0(2,4) or 1(3,5..)?
        add r22,r20
        shr 1, r21
        shl 4, r21 ;*8
        add r21,r20
        
        mov 8, r13
_charloop    
        in.b [r10],r12 ;msb
        st.b r12, 0[r11]
        
        in.b [r20],r12 ;lsb
        st.b r12, 1[r11]
        
        add 2,r10
        add 2,r20
        add 2,r11
        add -1,r13
        bne _charloop
    ;----
        add 1, r18
        add -1, r15
        bp _digitloop
        
        add 1, r14
        add -1, r23
        bp _charsetloop
        ret
    
hexfontdata
        LIBBIN hexset.dat
        EVEN 4
end_genhex

        EVEN    4
FONT_CHARSET:
        LIBBIN   priorart_fontprep.dat
FONT_CHARSET_END 


        EVEN 4

palette_data:
        dh      %0000000011100100
        dh      %0000000011100001
        dh      %0000000000100111
        dh      %0000000011000110
        dh      %0000000011100100
        dh      %0000000000111001
        dh      %0000000001001110
        dh      %0000000010010011
;-------------------------------------------------------------
column_table_data ;still aligned
        db $FE, $FE, $FE, $FE, $FE, $FE, $FE, $FE
        db $FE, $FE, $FE, $FE, $FE, $FE, $FE, $FE
        db $FE, $FE, $FE, $FE, $FE, $FE, $FE, $FE
        db $FE, $FE, $FE, $FE, $FE, $FE, $FE, $FE
        db $FE, $FE, $FE, $FE, $FE, $FE, $FE, $FE
        db $FE, $FE, $FE, $FE, $FE, $FE, $FE, $FE
        db $FE, $FE, $FE, $FE, $FE, $FE, $FE, $FE
        db $FE, $FE, $FE, $FE, $FE, $FE, $E0, $BC
        db $A6, $96, $8A, $82, $7A, $74, $6E, $6A
        db $66, $62, $60, $5C, $5A, $58, $56, $54
        db $52, $50, $50, $4E, $4C, $4C, $4A, $4A
        db $48, $48, $46, $46, $46, $44, $44, $44
        db $42, $42, $42, $40, $40, $40, $40, $40
        db $3E, $3E, $3E, $3E, $3E, $3E, $3E, $3C
        db $3C, $3C, $3C, $3C, $3C, $3C, $3C, $3C
        db $3C, $3C, $3C, $3C, $3C, $3C, $3C, $3C

end_binary

;--------------------------------------------------------
; Header
;--------------------------------------------------------

        org     $FFFFFDE0       ; title
        db      "Test ROM"

        org     $FFFFFDF4       ; reserved
        db      $00, $00, $00, $00, $00

        org     $FFFFFDF9       ; dev code
        db      "AG"

        org     $FFFFFDFB       ; game code
        db      "0000"

        org     $FFFFFDFF       ; ROM vers
        db      $00

;-------------------------------------------------------------------------------
; IRQ

        org     $FFFFFE00       ; Key Interrupt
        reti

        org     $FFFFFE10       ; Timer Interrupt
        ; push r30
        ; movea   #Timer_Interrupt'lo, r0, r30
        ; movhi   #Timer_Interrupt'hi1, r30, r30
        ; jmp [r30]
        reti

        org     $FFFFFE20       ; Expansion Port Interrupt
        reti

        org     $FFFFFE30       ; Link Port Interrupt
        reti

        org     $FFFFFE40       ; VIP Interrupt
        ;just ack here
        push r6
        push r7
        movw    INTPND, r6
        movw    INTCLR, r7
        ld.h    $0[r6], r6
        st.h    r6, $0[r7]
        pop r7
        pop r6
        reti

        ; Reset Vector 
        org     $FFFFFFF0 
        jump    RESET_VEC
