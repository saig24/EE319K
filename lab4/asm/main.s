;****************** main.s ***************
; Program written by: ***Your Names**update this***
; Date Created: 1/22/2016 
; Last Modified: 1/22/2016 
; Section ***Tuesday 1-2***update this***
; Instructor: ***Ramesh Yerraballi**update this***
; Lab number: 4
; Brief description of the program
;   If the switch is presses, the LED toggles at 8 Hz
; Hardware connections
;  PE1 is switch input  (1 means pressed, 0 means not pressed)
;  PE0 is LED output (1 activates external LED on protoboard) 
;Overall functionality of this system is the similar to Lab 3, with three changes:
;1-  initialize SysTick with RELOAD 0x00FFFFFF 
;2-  add a heartbeat to PF2 that toggles every time through loop 
;3-  add debugging dump of input, output, and time
; Operation
;	1) Make PE0 an output and make PE1 an input. 
;	2) The system starts with the LED on (make PE0 =1). 
;   3) Wait about 62 ms
;   4) If the switch is pressed (PE1 is 1), then toggle the LED once, else turn the LED on. 
;   5) Steps 3 and 4 are repeated over and over


SWITCH                  EQU 0x40024004  ;PE0
LED                     EQU 0x40024008	;PE1
SYSCTL_RCGCGPIO_R       EQU 0x400FE608
SYSCTL_RCGC2_GPIOE      EQU 0x00000010	; port E Clock Gating Control
SYSCTL_RCGC2_GPIOF      EQU 0x00000020	; port F Clock Gating Control
GPIO_PORTE_DATA_R       EQU 0x400243FC
GPIO_PORTE_DIR_R        EQU 0x40024400;
GPIO_PORTE_AFSEL_R      EQU 0x40024420;
GPIO_PORTE_PUR_R        EQU 0x40024510
GPIO_PORTE_DEN_R        EQU 0x4002451C;
GPIO_PORTF_DATA_R       EQU 0x400253FC
GPIO_PORTF_DIR_R        EQU 0x40025400;
GPIO_PORTF_AFSEL_R      EQU 0x40025420;
GPIO_PORTF_DEN_R        EQU 0x4002551C;
NVIC_ST_CTRL_R          EQU 0xE000E010
NVIC_ST_RELOAD_R        EQU 0xE000E014
NVIC_ST_CURRENT_R       EQU 0xE000E018
SOFT_COUNT 				EQU	1240000		; Count value from: COUNT = .062/(4*1/(80*10^6))
HARD_COUNT				EQU 1653333		; Count value from: COUNT = .062/(3*1/(80*10^6))
           THUMB
           AREA    DATA, ALIGN=4
;You MUST use these two buffers and two variables
;You MUST not change their names
;These names MUST be exported
           EXPORT DataBuffer  
           EXPORT TimeBuffer
SIZE       EQU    50
           EXPORT DataPt [DATA,SIZE=4] 
           EXPORT TimePt [DATA,SIZE=4]
DataBuffer SPACE  SIZE*4
TimeBuffer SPACE  SIZE*4
DataPt     SPACE  4
TimePt     SPACE  4
COUNT      SPACE  1

    
		ALIGN          
		AREA    |.text|, CODE, READONLY, ALIGN=2
		THUMB
		EXPORT  Start
		IMPORT  TExaS_Init
		IMPORT SysTick_Init

Start 	BL TExaS_Init  ; running at 80 MHz, scope voltmeter on PD3
; initialize Port E
; initialize Port F
; initialize debugging dump, including SysTick
			
		LDR R1, =SYSCTL_RCGCGPIO_R		; Load address of clock register into a register 
		LDR R0, [R1]					; Load value at clock register into register
		ORR R0, #0x30           		; Set the port E bit in the same register by setting the 4th bit
		STR R0, [R1]                   	; Save the resulting value into address of clocks
		NOP								; Wait for clock to settle
		NOP								 					    				
	
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
		
; PortF Initialization	

		LDR R1, =GPIO_PORTF_AFSEL_R     ; Load address of port F alternate function register into a register
		LDR R0, [R1]                    ; Load value in alternate function register into a register
		AND R0, #0xFB					; Clear bit 2 and save value alternate function register to remove alternate function from PE2-5
		STR R0, [R1]
		
		LDR R1, =GPIO_PORTF_DIR_R       ; Load address of port F direction register into a register
		LDR R0, [R1]					; Load value in direction register into a register
		AND R0, #0xFB					; Set PE2 pin as an output pin by setting only the 1st bit in a register
		ORR R0, #0x04                   
		STR R0, [R1] 
		
		LDR R1, =GPIO_PORTF_DEN_R       ; Load address of port F enable digital I/O register into a register
		LDR R0, [R1]					; Load value in enable digital I/O register into register
		ORR R0, #0x04                   ; Set bit 2 and save value into enable digital I/O register to enable digital function to PE2-5
		STR R0, [R1]
		
		
		CPSIE  I    ; TExaS voltmeter, scope runs on interrupts
		BL Debug_Init
	  	B set_LED						; Turns on the LED by setting PE0 in port E data register to 1
	  
loop  	
		LDR R1, =COUNT
		LDR R0, [R1]
		CMP R0, #50
		
		BEQ Skip
		BL Debug_Capture
;heartbeat
Skip
		BL Heartbeat					; Toggles LED on microcontroller
; Delay
		BL delay						; Delays 62ms in subroutine
;input PE1 test output PE0 2480009
								
		
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


;------------Debug_Init------------
; Initializes the debugging instrument
; Input: none
; Output: none
; Modifies: none
; Note: push/pop an even number of registers so C compiler is happy
Debug_Init
		PUSH{R0, LR}			; Saves Link register to do SysTick_Init
; init SysTick
		BL SysTick_Init			; Initialize External System Clock
		
		LDR R1, =COUNT			; Sets the limit to the amount of variables to be in array
		MOV R0, #0
		STR R0, [R1]
		
		LDR R0, =DataBuffer		; Initialize DataPt by storing inital address of DataBuffer register
		LDR R1, =DataPt
		STR R0, [R1]
		
		LDR R0, =TimeBuffer		; Initialize TimePt by storing inital address of TimeBuffer register
		LDR R1, =TimePt
		STR R0, [R1]
		
		MOV R0, #0xFFFFFFFF		; Initialize DataBuffer by storing value of 0xFFFFFFFF
		LDR R1, =DataBuffer
		BL Set_Buffer
		
		MOV R0, #0xFFFFFFFF		; Initialize TimeBuffer by storing value of 0xFFFFFFFF
		LDR R1, =TimeBuffer
		BL Set_Buffer
		
		POP{R0, LR}				; Return Inital value of Link Register
		BX LR

;------------Debug_Capture------------
; Dump Port E and time into buffers
; Input: none
; Output: none
; Modifies: none
; Note: push/pop an even number of registers so C compiler is happy
Debug_Capture
		LDR R1, =TimePt					; Pointer to Time Pointer
		LDR R0, [R1]					; Value in Time Pointer (Time Buffer Address)
		LDR R2, =NVIC_ST_CURRENT_R		
		LDR R2, [R2]					; Get current time
		STR R2, [R0]
		ADD R0, #4						; Increment Time Pointer 
		STR R0, [R1]
		
		LDR R1, =GPIO_PORTE_DATA_R		; Loads data register into R1
		LDR R0, [R1]					; Data from PortF Register
		AND R2, R0, #0x01				; Get PE0 (Output value)
		MOV R3, R2	
		AND R2, R0, #0x02		
		LSL R2, #3						; Get PE1 and shift it to the 4th bit (Input value)
		ORR R3, R2	
		LDR R1, =DataPt					; Pointer to the Data Pointer
		LDR R0, [R1]					; Value in Data Pointer (Data Buffer Address)
		STR R3, [R0]					; Store input and output into databuffer
		ADD R0, #4						; Increment Data Pointer
		STR R0, [R1]					
		
		LDR R1, =COUNT			; Sets the limit to the amount of variables to be in array
		LDR R0, [R1]
		ADD R0, #1
		STR R0, [R1]
		
		BX LR

Heartbeat
		LDR R1, =GPIO_PORTF_DATA_R		; Loads data register into R1
		LDR R0, [R1]
		EOR R0, #0x04					; Toggles PF2
		STR R0, [R1]
		
		BX LR

delay
		LDR R1, =SOFT_COUNT					; Gets preset value from count 8
delay_loop
		SUBS R1, #1						; Subtacts count by 1 until 0 to make a delay 1240008
		BNE delay_loop
		BX LR

set_LED
		LDR R1, =GPIO_PORTE_DATA_R		; Loads data register into R1
		LDR R0, [R1]
		ORR R0, #0x01					; Sets LED to on by setting bit PE0 to 1
		STR R0, [R1]
		
		B loop
		
; Input R0 and R1
; R0 has the value to fill
; R1 has the starting address to fill
Set_Buffer
		MOV R2, #0
B_Loop 	
		STR R0, [R1]
		ADD R1, #4
		ADD R2, #1
		CMP R2, #50
		BNE B_Loop
		BX LR

		ALIGN                           ; make sure the end of this section is aligned
		END                             ; end of file
        