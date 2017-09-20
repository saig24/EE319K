;****************** main.s ***************
; Program written by: Chimezie Iwuanyanwu & Allen Pan
; Date Created: 1/22/2016 
; Last Modified: 2/20/2016 
; Section: Wednesday 5-6
; Instructor: Ramesh Yerraballi
; Lab number: 3
; Brief description of the program
;   If the switch is presses, the LED toggles at 8 Hz
; Hardware connections
;  PE1 is switch input  (1 means pressed, 0 means not pressed)
;  PE0 is LED output (1 activates external LED on protoboard) 
;Overall functionality of this system is the similar to Lab 2, with six changes:
;1-  the pin to which we connect the switch is moved to PE1, 
;2-  you will have to remove the PUR initialization because pull up is no longer needed. 
;3-  the pin to which we connect the LED is moved to PE0, 
;4-  the switch is changed from negative to positive logic, and 
;5-  you should increase the delay so it flashes about 8 Hz.
;6-  the LED should be on when the switch is not pressed
; Operation
;   1) Make PE0 an output and make PE1 an input. 
;   2) The system starts with the LED on (make PE0 =1). 
;   3) Wait about 62 ms
;   4) If the switch is pressed (PE1 is 1), then toggle the LED once, else turn the LED on. 
;   5) Steps 3 and 4 are repeated over and over


GPIO_PORTE_DATA_R       EQU   0x400243FC
GPIO_PORTE_DIR_R        EQU   0x40024400
GPIO_PORTE_AFSEL_R      EQU   0x40024420
GPIO_PORTE_DEN_R        EQU   0x4002451C
GPIO_PORTE_AMSEL_R      EQU   0x40024528
GPIO_PORTE_PCTL_R       EQU   0x4002452C
SYSCTL_RCGCGPIO_R       EQU   0x400FE608
SOFT_COUNT 				EQU	  1240000	; Count value from: COUNT = .062/(4*1/(80*10^6))
HARD_COUNT				EQU   1653333	; Count value from: COUNT = .062/(3*1/(80*10^6))
       IMPORT  TExaS_Init
       AREA    |.text|, CODE, READONLY, ALIGN=2
       THUMB
       EXPORT  Start
Start
 ; TExaS_Init sets bus clock at 80 MHz
		BL  TExaS_Init ; voltmeter, scope on PD3
; you initialize PE1 PE0				   FE DCBA
;										[0000 0000]
;										 7654 3210
		LDR R1, =SYSCTL_RCGCGPIO_R		; Load address of clock register into a register 
		LDR R0, [R1]					; Load value at clock register into register
		ORR R0, #0x10           		; Set the port E bit in the same register by setting the 4th bit
		STR R0, [R1]                   	; Save the resulting value into address of clocks
		NOP								; Wait for clock to settle
		NOP								
	
		LDR R1, =GPIO_PORTE_AMSEL_R     ; Load address of port E analog fuctionality register into a register
		LDR R0, [R1]                    ; Load value in analog functionality register into register
		AND R0, #0xFC					; Clear bits 0 & 1 and save value into analog functionality register to remove analog function from PE2-5
		STR R0, [R1]  					
	
		LDR R1, =GPIO_PORTE_PCTL_R      ; 4) configure as GPIO
		LDR R0, [R1]                    ; Load value in clock register into register
		AND R0, #0xFC            	    ; Clear bits 0 & 1 to set PE0 and PE1 as GPIO
		STR R0, [R1]     				
	
		LDR R1, =GPIO_PORTE_DIR_R       ; Load address of port E direction register into a register
		LDR R0, [R1]					; Load value in direction register into a register
		AND R0, #0xFC					; Set PE0 pin as an output pin by setting only the 1st bit in a register
		ORR R0, #0x01                   
		STR R0, [R1]  					; Save the same register in port E direction register
	
		LDR R1, =GPIO_PORTE_AFSEL_R     ; Load address of port E alternate function register into a register
		LDR R0, [R1]                    ; Load value in alternate function register into a register
		AND R0, #0xFC					; Clear bits 0 & 1 and save value alternate function register to remove alternate function from PE2-5
		STR R0, [R1]					
	
		LDR R1, =GPIO_PORTE_DEN_R       ; Load address of port E enable digital I/O register into a register
		LDR R0, [R1]					; Load value in enable digital I/O register into register
		ORR R0, #0x03                   ; Set bits 0 & 1 and save value into enable digital I/O register to enable digital function to PE2-5
		STR R0, [R1]
		
		B set_LED						; Turns on the LED by setting PE0 in port E data register to 1

      CPSIE  I    						; TExaS voltmeter, scope runs on interrupts

loop  
     	BL delay						; Delays 62ms in subroutine
		
		LDR R1, =GPIO_PORTE_DATA_R		; Loads data register into R0
		LDR R0, [R1]
		
		AND R2, #0						; Clears R2 to be used
		AND R2, R0, #0x02				; Gets value from switch and puts it into R2
		LSR R2, #1						; Shifts R2 by 1 to compare with the value of 0
		CMP R2, #1
		
		BNE set_LED						; Sets the LED on by default if switch is not pressed
		
		EOR R3, R0, #0x01				; Toggles LED if switch is pressed
		STR R3, [R1]
		
		B loop
	
set_LED
		LDR R1, =GPIO_PORTE_DATA_R		; Loads data register into R1
		LDR R0, [R1]
		ORR R0, #0x01					; Sets LED to on by setting bit PE0 to 1
		STR R0, [R1]
		
		B loop
		
delay
		LDR R1, =SOFT_COUNT					; Gets preset value from count
delay_loop
		SUBS R1, #1						; Subtacts count by 1 until 0 to make a delay
		BNE delay_loop
		BX LR

      ALIGN      ; make sure the end of this section is aligned
      END        ; end of file
       