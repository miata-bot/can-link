; SECTION "Header" 0x100

; Syntax explanation
; comments are delimited by ;

; a pixel [v]alue is expected to be a 32 bit unsigned integer. IE: 0xRRGGBBAA
; syntax sugar is provided by the assembler to evaluate html color codes. IE: #ff0000ff

; Instructions:
; NOP            - no operation

; General purpose register access
; LD [r] [v]     - load a [v]alue into a [r]egister.
; INC [r]        - increment a [r]egister.
; DEC [r]        - decrement a [r]egister.

; LED Output

; CLS            - clear a channel
; SET [a] [v]    - set a pixel at [a]ddress to [v]alue
; SET [a] [r]    - set a pixel at [a]ddress to a value pointed to by [r]egister
; SET [r1] [v]   - set a pixel at address referenced by [r1] to [v]alue
; SET [r1] [r2]  - set a pixel at address referenced by [r1] to value pointed to by [r2]
; FLUSH          - Blit the current changes a channel

; Program Flow
; JP [r]         - unconditional jump to [r]. (note: this may only be in channel 1. channel 2 should contain a NOP)
; J[n]Z [r]      - jump if [not] zero to [r]. (note: this may only be in channel 1. channel 2 should contain a NOP)
; HALT           - stop the current channel

; 32 bit Registers:
;
; PC - program counter - readonly, incremented every instruction
; X  - normally used for channel 1 instructions
; Y  - normally used for channel 2 instructions
; ZA  - general purpose channel 1 register
; ZB  - general purpose channel 2 register
; LP  - general purpose loop register
; TM  - general purpose timer register. decremented once every millisecond
; CHN - RGB channel to control

.main:
  LD  CHN 0         ; load 0 into channel register
  CLS CHN           ; clear channel 0
  LD  X #ff0000ff   ; load red full brightness into X register
  FILL CHN X        ; fill channel 0 with red

  INC CHN           ; increment the channel, works for 2 channels currently
  CLS CHN           ; clear channel 1
  LD  Y  #00ff00ff  ; load green into Y with full brightness
  FILL CHN Y        ; fill channel 1 with green

  FLUSH             ; blit both channels

  LD  CHN 0         ; load 0 into channel register
  CLS CHN           ; clear channel 0
  INC CHN           ; increment to channel 1
  CLS CHN           ; clear channel 1

  LD X #0000ffff    ; load blue into X register
  LD Y #ff0000ff    ; load red into Y register

  LD ZA 0           ; pixel address
  LD ZB 0           ; pixel address

  LD  CHN 0         ; load 0 into channel register
  SET CHN ZA X       ; set pixel address 0 on channel 0

  INC CHN           ; increment to channel 1
  SET CHN ZB Y       ; set the pixel colors at address 0 on channel 1
  FLUSH             ; blit both channels

; clear both channels
  LD  CHN 0         ; load 0 into channel register
  CLS CHN           ; clear channel 0
  INC CHN           ; increment to channel 1
  CLS CHN           ; clear channel 1

; Routine to fill both channels one pixel at a time
  LD X #ff0000ff   ; load red into X
  LD Y #ff0000ff   ; load red into Y
  LD ZA 0          ; 
  LD ZB 0          ; load 0 into all general purpose registers
  LD LP 300        ; load 300 into the counter register

.loop:
  FLUSH             ; blit all channels
  SET CHN ZA X      ; set the pixel referenced in the Z register to red
  INC CHN           ; increment the channel  
  SET CHN ZB Y      ; set the pixel referenced in the Z register to red
  INC ZA            ; 
  INC ZB            ; increment Z register
  DEC LP            ; dec counter
  LD TM 10          ; set a timer for 10 ms

.sleep:
  ; JNZ TM .sleep     ; sleep until 10 ms timer is up
  JNZ LP .loop      ; jump to the beginning of the loop if LP is not zero

  HALT              ; halt all channels
