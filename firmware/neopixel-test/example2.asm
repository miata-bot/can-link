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

; CLS [r]        - clear a channel in [r]
; SET [a] [v]    - set a pixel at [a]ddress to [v]alue
; SET [a] [r]    - set a pixel at [a]ddress to a value pointed to by [r]egister
; SET [r1] [v]   - set a pixel at address referenced by [r1] to [v]alue
; SET [r1] [r2]  - set a pixel at address referenced by [r1] to value pointed to by [r2]
; FLUSH          - Blit the current changes

; Program Flow
; JP [r]         - unconditional jump to [r]
; J[n]Z [r]      - jump if [not] zero to [r]
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
  FLUSH

.redBegin:
  LD  X #ff0000ff   ; load red full brightness into X register
  FILL CHN X        ; fill channel 0 with red
  FLUSH             ; blit both channels
  LD TM 1000         ; set a timer for 100 ms

.redDelay:
  JNZ TM .redDelay     ; sleep until 100 ms timer is up

  CLS CHN           ; clear channel 0
  FLUSH

  LD  X #00ff00ff   ; load green full brightness into X register
  FILL CHN X        ; fill channel 0 with green
  FLUSH             ; blit both channels

  LD TM 1000         ; set a timer for 100 ms
.greenDelay:
  JNZ TM .greenDelay     ; sleep until 100 ms timer is up

  CLS CHN                ; clear channel 0
  FLUSH

  LD  X #0000ffff   ; load blue full brightness into X register
  FILL CHN X        ; fill channel 0 with blue
  FLUSH             ; blit both channels
  LD TM 1000         ; set a timer for 100 ms
.blueDelay:
  JNZ TM .blueDelay     ; sleep until 100 ms timer is up

  CLS CHN                ; clear channel 0
  FLUSH
  LD ZB 1
  ; JNZ ZB .redBegin
  ; HALT

; Routine to fill one pixel at a time
  LD X #ff0000ff   ; load red into X
  LD ZA 0          ; load 0 beginning address into ZA
  LD LP 40         ; load 40 into the counter register
  
.fillLoop:
  FLUSH             ; blit all channels
  SET CHN ZA X      ; set the pixel referenced in the Z register to red
  INC ZA            ; increment address
  DEC LP            ; decrement loop counter
  LD TM 500          ; set a timer for 10 ms
.fillSleep:
  JNZ TM .fillSleep     ; sleep until 10 ms timer is up
  JNZ LP .fillLoop      ; restart continue running the fill loop until it gets to zero
  HALT              ; stop the program