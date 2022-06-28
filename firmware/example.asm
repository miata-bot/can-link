.creator: "connor"
.date: "2022-06-28 20:28:41.421534Z"
.version: 1
.comment: "Example LED Strip program"

; Syntax explanation
; comments are delimited by ;
; Each line should contain two instructions split by a -
; This allows both strips to be updated in lock-step with eachother.
; If only one channel is needed, a NOP may be put in the other channel's instruction.

; some instructions may only be run on channel 1. In this case the instruction in
; channel 2 MUST be `NOP`. Failing to do this will result in an assembly error.

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

.main:
  CLS            - CLS              ; clear both channels
  LD X #ff0000ff - LD Y #00ff00ff   ; load red into X and green into Y with full brightness
  FILL X         - FILL Y           ; fill channel 1 with red, fill channel 2 with green
  FLUSH          - FLUSH            ; blit both channels

  CLS            - CLS              ; clear both channels
  LD X #0000ffff - LD Y #ff0000ff   ; load blue and red
  SET 0 X        - SET 0 Y          ; set the pixel colors at address 0
  FLUSH          - FLUSH            ; blit both channels

; Routine to fill both channels one pixel at a time
  CLS            - CLS              ; clear both channels before beginning the loop
  LD X #ff0000ff - LD Y #ff0000ff   ; load red into both operand registers
  LD ZA 0        - LD ZB 0          ; load 0 into both general purpose registers
  LD LP 300      - NOP              ; load 300 into the counter register
.loop:
  FLUSH          - FLUSH            ; blit both channels
  SET ZA X       - SET ZB Y         ; set the pixel referenced in the Z register to red
  INC ZA         - INC ZB           ; increment Z register
  DEC LP         - NOP              ; dec counter
  LD TM 10      - NOP               ; set a timer for 10 ms
.sleep:
  JNZ TM .sleep  - NOP              ; sleep until 10 ms timer is up
  JNZ LP .loop   - NOP              ; jump to the beginning of the loop if LP is not zero

  HALT           - HALT             ; halt both channels
