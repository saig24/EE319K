; IO.s
; Student names: Xinyuan Pan & Chimezie Iwuanyanwu
; Last modification date: 3/29
; Runs on TM4C123

; EE319K lab 7 device driver for the switch and LED.
; You are allowed to use any switch and any LED,
; although the Lab suggests the SW1 switch PF4 and Red LED PF1

; As part of Lab 7, students need to implement these three functions

; negative logic SW2 connected to PF0 on the Launchpad
; red LED connected to PF1 on the Launchpad
; blue LED connected to PF2 on the Launchpad
; green LED connected to PF3 on the Launchpad
; negative logic SW1 connected to PF4 on the Launchpad

        EXPORT   IO_Init
        EXPORT   IO_Touch
        EXPORT   IO_HeartBeat

GPIO_PORTF_DATA_R  EQU 0x400253FC
GPIO_PORTF_DIR_R   EQU 0x40025400
GPIO_PORTF_AFSEL_R EQU 0x40025420
GPIO_PORTF_PUR_R   EQU 0x40025510
GPIO_PORTF_DEN_R   EQU 0x4002551C
GPIO_PORTF_LOCK_R  EQU 0x40025520
GPIO_PORTF_CR_R    EQU 0x40025524
GPIO_PORTF_AMSEL_R EQU 0x40025528
GPIO_PORTF_PCTL_R  EQU 0x4002552C
GPIO_LOCK_KEY      EQU 0x4C4F434B  ; Unlocks the GPIO_CR register
PF0       EQU 0x40025004
PF1       EQU 0x40025008
PF2       EQU 0x40025010
PF3       EQU 0x40025020
PF4       EQU 0x40025040
LEDS      EQU 0x40025038
RED       EQU 0x02
BLUE      EQU 0x04
GREEN     EQU 0x08
SWITCHES  EQU 0x40025044
SW1       EQU 0x10                 ; on the left side of the Launchpad board
SW2       EQU 0x01                 ; on the right side of the Launchpad board
SYSCTL_RCGCGPIO_R  EQU 0x400FE608
DELAY_CYCLE EQU 266666
    
        AREA    |.text|, CODE, READONLY, ALIGN=2
        THUMB



;------------IO_Init------------
; Initialize GPIO Port for a switch and an LED
; Input: none
; Output: none
; This is a public function
; Invariables: This function must not permanently modify registers R4 to R11
IO_Init
	   LDR	R2, =SYSCTL_RCGCGPIO_R
	   LDR	R1, [R2]
	   ORR	R1, R1, #0x20
	   STR	R1, [R2]
	   NOP
	   NOP	   
	   LDR	R2, =GPIO_PORTF_DIR_R
	   LDR	R1, [R2]
	   BIC	R1, R1, #0x10	;PF4 is input / switch
	   ORR	R1, R1, #0x02	;PF1 is output / led
	   STR 	R1, [R2]
	   LDR	R2, =GPIO_PORTF_DEN_R
	   LDR	R1, [R2]
	   ORR	R1, R1, #0x12
	   STR 	R1, [R2]
	   LDR	R2, =GPIO_PORTF_AFSEL_R
	   LDR	R1, [R2]
	   BIC	R1, R1, #0x12
	   STR 	R1, [R2]
	   LDR	R2, =GPIO_PORTF_AMSEL_R
	   LDR	R1, [R2]
	   BIC	R1, R1, #0x12
	   STR 	R1, [R2]
	   ; activate the pull up resistor for negative logic switch
	   LDR	R2, =GPIO_PORTF_PUR_R
	   LDR	R1, [R2]
	   ORR	R1, R1, #0x10
	   STR 	R1, [R2]
    
    BX  LR
;* * * * * * * * End of IO_Init * * * * * * * *

;------------IO_HeartBeat------------
; Toggle the output state of the LED.
; Input: none
; Output: none
; This is a public function
; Invariables: This function must not permanently modify registers R4 to R11
IO_HeartBeat
	  LDR	R2, =GPIO_PORTF_DATA_R
	  LDR	R1,	[R2]
	  EOR	R1, #0x02				;PF1
	  STR	R1, [R2];	  
    
    BX  LR                          ; return
;* * * * * * * * End of IO_HeartBeat * * * * * * * *

;------------IO_Touch------------
; First: wait for the release of the switch
; and then: wait for the touch of the switch
; Input: none
; Input: none
; Output: none
; This is a public function
; Invariables: This function must not permanently modify registers R4 to R11
IO_Touch
    ; note PF4 is negative logic
	PUSH {R4,LR}
	LDR   R2, =GPIO_PORTF_DATA_R
wait_release	
	LDR   R1, [R2]
	; leave only PF4
	AND   R1, #0x10
	CMP   R1, #0x00
	BEQ   wait_release ; already touched, now wait for release (due to bounce)
	; if released meaning it started bouncing. wait for 10ms
	BL    delay10ms
	; now detect release
wait_touch
	LDR   R1, [R2]
	AND   R1, #0x10
	CMP   R1, #0x10
	BEQ   wait_touch ; already released, now wait for touch (due to bounce)
	MOV   R0, #10
	BL    delay10ms
    POP {R4,LR}
    BX  LR                          ; return
	
;* * * * * * * * End of IO_Touch * * * * * * * *

; private function to delay 10 ms
delay10ms
	LDR   R0, =DELAY_CYCLE
notDone
	SUBS  R0,#1
	BNE   notDone
	BX  LR
;********** End of delay10ms *********

    ALIGN                           ; make sure the end of this section is aligned
    END                             ; end of file