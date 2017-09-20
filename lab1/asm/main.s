;****************** main.s ***************
; Program written by: Chimezie Iwuanyanwu (cci273)
; Date Created: 1/22/2015 
; Last Modified: 2/3/2016 
; Section Wednesday 5-6
; Instructor: Ramesh Yerraballi
; Lab number: 1
; Brief description of the program
; The overall objective of this system is a digital lock
; Hardware connections
;  PE3 is switch input  (1 means switch is not pressed, 0 means switch is pressed)
;  PE4 is switch input  (1 means switch is not pressed, 0 means switch is pressed)
;  PE5 is switch input  (1 means switch is not pressed, 0 means switch is pressed)
;  PE2 is LED output (0 means door is locked, 1 means door is unlocked) 
; The specific operation of this system is to 
;   unlock if all three switches are pressed

GPIO_PORTE_DATA_R       EQU   0x400243FC 
GPIO_PORTE_DIR_R        EQU   0x40024400 
GPIO_PORTE_AFSEL_R      EQU   0x40024420 
GPIO_PORTE_DEN_R        EQU   0x4002451C 
GPIO_PORTE_AMSEL_R      EQU   0x40024528 
GPIO_PORTE_PCTL_R       EQU   0x4002452C 
SYSCTL_RCGCGPIO_R       EQU   0x400FE608 

      AREA    |.text|, CODE, READONLY, ALIGN=2
      THUMB
      EXPORT  Start
Start  
	MOV R0, #1
	MOV R1, #2
	MOV R2, #4
	MOV R3, #8
	MOV R4, #16
	PUSH {R1, R4, R3, R0}
	POP  {R3, R1, R2, R0}



	LDR R1, =SYSCTL_RCGCGPIO_R		; Load address of clock register into a register 
    LDR R0, [R1]					; Load value at clock register into register
    ORR R0, #0x10           		; Set the port E bit in the same register by setting the 4th bit
    STR R0, [R1]                   	; Save the resulting value into address of clocks
	NOP								; Wait for clock to settle
	NOP								
	
	LDR R1, =GPIO_PORTE_AMSEL_R     ; Load address of port E analog fuctionality register into a register
    LDR R0, [R1]                    ; Load value in analog functionality register into register
    AND R0, #0xC3					; Clear bits 2-5 and save value into analog functionality register to remove analog function from PE2-5
	STR R0, [R1]  					
	
	LDR R1, =GPIO_PORTE_PCTL_R      ; 4) configure as GPIO
	LDR R0, [R1]                    ; Load value in clock register into register
    AND R0, #0xC3            	    ; 0 means configure Port F as GPIO
    STR R0, [R1]     				
	
	LDR R1, =GPIO_PORTE_DIR_R       ; Load address of port E direction register into a register
    LDR R0, [R1]					; Load value in direction register into a register
	AND R0, #0xC3					; Set PE2 pin as an output pin by setting only the 3rd bit in a register
	ORR R0, #0x04                   
    STR R0, [R1]  					; Save the same register in port E direction register
	
	LDR R1, =GPIO_PORTE_AFSEL_R     ; Load address of port E alternate function register into a register
    LDR R0, [R1]                    ; Load value in alternate function register into a register
    AND R0, #0xC3					; Clear bits 2-5 and save value alternate function register to remove alternate function from PE2-5
    STR R0, [R1]					
	
	LDR R1, =GPIO_PORTE_DEN_R       ; Load address of port E enable digital I/O register into a register
    LDR R0, [R1]					; Load value in enable digital I/O register into register
	ORR R0, #0x3C                   ; Set bits 2-5 and save value into enable digital I/O register to enable digital function to PE2-5
    STR R0, [R1] 					
	
loop   
    LDR R1, =GPIO_PORTE_DATA_R		; Load address of port E data register into a register
    LDR R0, [R1]					; Load value in data register into a register
	
	EOR R2, R0, #0x20				; Exclusive OR bit 5 of value in data register and store the value into a register
	EOR R3, R0, #0x10				; Exclusive OR bit 4 of value in data register and store the value into a register				
	EOR R4, R0, #0x08				; Exclusive OR bit 3 of value in data register and store the value into a register
	
	LSR R2, #3						; Logical shift right register with bit 5 by 3
	LSR R3, #2						; Logical shift right register with bit 4 by 2
	LSR R4, #1						; Logical shift right register with bit 3 by 1
	
	AND R2, R3						; AND one shifted register with other the other shifted registers
	AND R2, R4						
	
	AND R0, #0xFB					; Save the value of AND'd registers into the port E data register
	ORR R0, R2
							
	STR R0, [R1]
	B loop							; Loop back to repeat the algorithm 
	
      ALIGN        					; make sure the end of this section is aligned
      END          					; end of file