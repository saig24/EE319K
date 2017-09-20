// Lab9.c
// Runs on LM4F120 or TM4C123
// Student names: Chimezie Iwuanyanwu & Xinyuan Pan
// Last Modified: 4/14/2016 

// Analog Input connected to PE2=ADC1
// displays on Sitronox ST7735
// PF3, PF2, PF1 are heartbeats
// UART1 on PC4-5
// * Start with where you left off in Lab8. 
// * Get Lab8 code working in this project.
// * Understand what parts of your main have to move into the UART1_Handler ISR
// * Rewrite the SysTickHandler
// * Implement the s/w FiFo on the receiver end 
//    (we suggest implementing and testing this first)

#include <stdint.h>

#include "ST7735.h"
#include "TExaS.h"
#include "ADC.h"
#include "print.h"
#include "tm4c123gh6pm.h"
#include "UART.h"
#include "FiFo.h"
#include "SysTickInts.h"

//*****the first three main programs are for debugging *****
// main1 tests just the ADC and slide pot, use debugger to see data
// main2 adds the LCD to the ADC and slide pot, ADC data is on Nokia
// main3 adds your convert function, position data is no Nokia

void DisableInterrupts(void); // Disable interrupts
void EnableInterrupts(void);  // Enable interrupts

#define PF1       (*((volatile uint32_t *)0x40025008))
#define PF2       (*((volatile uint32_t *)0x40025010))
#define PF3       (*((volatile uint32_t *)0x40025020))
#define ASCII 0x30
uint32_t Data;      // 12-bit ADC
uint32_t Position;  // 32-bit fixed-point 0.001 cm
int32_t TxCounter = 0;	 // Transmit counter
char static Message [8]; // Messaage array

void PortF_Init(void){unsigned long volatile delay;
 GPIO_PORTF_DEN_R |= 0x0E;					// Initialiation of PortF
	GPIO_PORTF_DIR_R |= 0x0E;
	GPIO_PORTF_AMSEL_R &= ~0x0E;
	GPIO_PORTF_AFSEL_R &= ~0x0E;
	GPIO_PORTF_PCTL_R &= ~0x0E;
	GPIO_PORTF_DATA_R |= 0x0E;
}

// Get fit from excel and code the convert routine with the constants
// from the curve-fit
uint32_t Convert(uint32_t input){
  return (4307*input)/10000 + 90; // Formula found from taking a linear representation of ADC input
}


// final main program for bidirectional communication
// Sender sends using SysTick Interrupt
// Receiver receives using RX
int main(void){ 
	SYSCTL_RCGC2_R |= 0x35; // Turns on clock for Port A, C, E, F
	int32_t delay = SYSCTL_RCGC2_R;
  TExaS_Init();       // Bus clock is 80 MHz 
  ST7735_InitR(INITR_REDTAB);
  ADC_Init();    // initialize to sample ADC
  PortF_Init();		// Initializes Port F for Heartbeat
  UART_Init();       // initialize UART
//Enable SysTick Interrupt by calling SysTick_Init()
	SysTick_Init(80000000/40);
  EnableInterrupts();
	char letter;				// Letter that will display to screen
	char * after = &letter;	// Places null character after letter to only display letter
	*(after+1) = 0;
  while(1){
		FiFo_Get(&letter);		// Gets vaule from top of FiFo
		while(FiFo_Get(&letter) == 0){} //  Waits until FiFo is not empty
		ST7735_SetCursor(0,0);
		FiFo_Get(&letter);		
		while(letter != 0x0D){
			ST7735_OutString(&letter);		// Displays character to screen
			FiFo_Get(&letter);
		}
		ST7735_OutString(" cm");
		letter = 0;				// Resets letter
	}
}
void SysTick_Handler(void){ // every 25 ms
	PF2 ^= 0x04;      // Heartbeat
	Data = ADC_In();	// Fetches what ADC input is	
	PF2 ^= 0x04;      // Heartbeat
	Position = Convert(Data);		// Converts ADC to Position
	
	Message[0] = 0x02;				// Places start character
	Message[1] = Position / 1000;		// First decimal place
	Message[2] = 0x2E;						// Period
	Message[3] = Position / 100 - Message[1] * 10;		// Second decimal place
	Message[4] = Position / 10 - Message[3] * 10 - Message[1] * 100;	// Third decimal place
	Message[5] = Position - Message[4] * 10 - Message[3] * 100 - Message [1] * 1000; // Fourth decimal place
	Message[6] = 0x0D;	// Return character
	Message[7] = 0x03;	// End character
	
	Message[1] |= ASCII;		// Converts each decimal place to a ASCII value
	Message[3] |= ASCII;
	Message[4] |= ASCII;
	Message[5] |= ASCII;
	
	int Index = 0;
	while(Index < 8){
		UART_OutChar(Message[Index]); // Puts each part of message to UART FiFo
		Index++;
	}

	TxCounter ++;			// Transmit counter increment
	PF2 ^= 0x04;      // Heartbeat
}


uint32_t Status[20];             // entries 0,7,12,19 should be false, others true
char GetData[10];  // entries 1 2 3 4 5 6 7 8 should be 1 2 3 4 5 6 7 8
int main_fifo(void){ // Make this main to test FiFo
  FiFo_Init();
  for(;;){
    Status[0]  = FiFo_Get(&GetData[0]);  // should fail,    empty
    Status[1]  = FiFo_Put(1);            // should succeed, 1 
    Status[2]  = FiFo_Put(2);            // should succeed, 1 2
    Status[3]  = FiFo_Put(3);            // should succeed, 1 2 3
    Status[4]  = FiFo_Put(4);            // should succeed, 1 2 3 4
    Status[5]  = FiFo_Put(5);            // should succeed, 1 2 3 4 5
    Status[6]  = FiFo_Put(6);            // should succeed, 1 2 3 4 5 6
    Status[7]  = FiFo_Put(7);            // should fail,    1 2 3 4 5 6 
    Status[8]  = FiFo_Get(&GetData[1]);  // should succeed, 2 3 4 5 6
    Status[9]  = FiFo_Get(&GetData[2]);  // should succeed, 3 4 5 6
    Status[10] = FiFo_Put(7);            // should succeed, 3 4 5 6 7
    Status[11] = FiFo_Put(8);            // should succeed, 3 4 5 6 7 8
    Status[12] = FiFo_Put(9);            // should fail,    3 4 5 6 7 8 
    Status[13] = FiFo_Get(&GetData[3]);  // should succeed, 4 5 6 7 8
    Status[14] = FiFo_Get(&GetData[4]);  // should succeed, 5 6 7 8
    Status[15] = FiFo_Get(&GetData[5]);  // should succeed, 6 7 8
    Status[16] = FiFo_Get(&GetData[6]);  // should succeed, 7 8
    Status[17] = FiFo_Get(&GetData[7]);  // should succeed, 8
    Status[18] = FiFo_Get(&GetData[8]);  // should succeed, empty
    Status[19] = FiFo_Get(&GetData[9]);  // should fail,    empty
  }
}

