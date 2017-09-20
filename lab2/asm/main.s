;****************** main.s ***************
; Program written by: Chimezie Iwuanyanwu & Allen Pan
; Date Created: 1/22/2016 
; Last Modified: 2/14/2016 
; Section: Wednesday 5-6
; Instructor: Ramesh Yerraballi
; Lab number: 2
; Brief description of the program
; The overall objective of this system an interactive alarm
; Hardware connections
;  PF4 is switch input  (1 means SW1 is not pressed, 0 means SW1 is pressed)
;  PF3 is LED output (1 activates green LED) 
; The specific operation of this system 
;    1) Make PF3 an output and make PF4 an input (enable PUR for PF4). 
;    2) The system starts with the LED OFF (make PF3 =0). 
;    3) Delay for about 100 ms
;    4) If the switch is pressed (PF4 is 0), then toggle the LED once, else turn the LED OFF. 
;    5) Repeat steps 3 and 4 over and over

GPIO_PORTF_DATA_R       EQU   0x400253FC
GPIO_PORTF_DIR_R        EQU   0x40025400;X
GPIO_PORTF_AFSEL_R      EQU   0x40025420;X
GPIO_PORTF_PUR_R        EQU   0x40025510;X
GPIO_PORTF_DEN_R        EQU   0x4002551C;X
GPIO_PORTF_AMSEL_R      EQU   0x40025528;X
GPIO_PORTF_PCTL_R       EQU   0x4002552C;X
SYSCTL_RCGCGPIO_R       EQU   0x400FE608;X
COUNT EQU 400000

       AREA    |.text|, CODE, READONLY, ALIGN=2
       THUMB
       EXPORT  Start
Start
		LDR R1, =SYSCTL_RCGCGPIO_R		; Load address of clock register into a register 
		LDR R0, [R1]					; Load value at clock register into register
		ORR R0, #0x20           		; Set the port E bit in the same register by setting the 4th bit
		STR R0, [R1]                   	; Save the resulting value into address of clocks
		NOP								; Wait for clock to settle
		NOP								
	
		LDR R1, =GPIO_PORTF_AMSEL_R     ; Load address of port E analog fuctionality register into a register
		LDR R0, [R1]                    ; Load value in analog functionality register into register
		AND R0, #0xE7					; Clear bits 2-5 and save value into analog functionality register to remove analog function from PE2-5
		STR R0, [R1]  					
	
		LDR R1, =GPIO_PORTF_PCTL_R      ; 4) configure as GPIO
		LDR R0, [R1]                    ; Load value in clock register into register
		AND R0, #0xE7            	    ; 0 means configure Port F as GPIO
		STR R0, [R1]     				
	
		LDR R1, =GPIO_PORTF_DIR_R       ; Load address of port E direction register into a register
		LDR R0, [R1]					; Load value in direction register into a register
		AND R0, #0xE7					; Set PE2 pin as an output pin by setting only the 3rd bit in a register
		ORR R0, #0x08                   
		STR R0, [R1]  					; Save the same register in port E direction register
	
		LDR R1, =GPIO_PORTF_AFSEL_R     ; Load address of port E alternate function register into a register
		LDR R0, [R1]                    ; Load value in alternate function register into a register
		AND R0, #0xE7					; Clear bits 2-5 and save value alternate function register to remove alternate function from PE2-5
		STR R0, [R1]					
	
		LDR R1, =GPIO_PORTF_DEN_R       ; Load address of port E enable digital I/O register into a register
		LDR R0, [R1]					; Load value in enable digital I/O register into register
		ORR R0, #0x18                   ; Set bits 2-5 and save value into enable digital I/O register to enable digital function to PE2-5
		STR R0, [R1]
	
		LDR R1, =GPIO_PORTF_PUR_R       ; [1110 0111]
		LDR R0, [R1]					;  7654 3210
		ORR R0, #0x10                   ;    FE DCBA
		STR R0, [R1]
		
		B clear
loop  
		BL delay
		
		LDR R1, =GPIO_PORTF_DATA_R
		LDR R0, [R1]
		
		AND R2, #0
		AND R2, R0, #0x10
		CMP R2, #0
		
		BNE clear
		
		EOR R3, R0, #0x08
		STR R3, [R1]
		
		B loop
	
clear
		LDR R1, =GPIO_PORTF_DATA_R
		LDR R0, [R1]
		AND R0, #0xF7
		STR R0, [R1]
		
		B loop
		
delay
		LDR R1, =COUNT
delay_loop
		SUBS R1, #1
		BNE delay_loop
		BX LR
		
		ALIGN      ; make sure the end of this section is aligned
		END        ; end of file
       