// Lab8.c
// Runs on LM4F120 or TM4C123
// Student names: Chimezie Iwuanyanwu & Xinyuan Pan
// Last Modified: 4/12/2016 

// Analog Input connected to PE2=ADC1
// displays on Sitronox ST7735
// PF3, PF2, PF1 are heartbeats

#include <stdint.h>

#include "ST7735.h"
#include "TExaS.h"
#include "ADC.h"
#include "print.h"
#include "tm4c123gh6pm.h"
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
// Initialize Port F so PF1, PF2 and PF3 are heartbeats
void PortF_Init(void){
	 
	GPIO_PORTF_DEN_R |= 0x0E;					// Initialiation of PortF
	GPIO_PORTF_DIR_R |= 0x0E;
	GPIO_PORTF_AMSEL_R &= ~0x0E;
	GPIO_PORTF_AFSEL_R &= ~0x0E;
	GPIO_PORTF_PCTL_R &= ~0x0E;
	GPIO_PORTF_DATA_R |= 0x0E;
}
uint32_t Data;        // 12-bit ADC
uint32_t Position;    // 32-bit fixed-point 0.001 cm
int main1(void){      // single step this program and look at Data
  TExaS_Init();       // Bus clock is 80 MHz 
  ADC_Init();         // turn on ADC, set channel to 1
  while(1){                
    Data = ADC_In();  // sample 12-bit channel 1
  }
}

int main2(void){
	SYSCTL_RCGC2_R |= 0x31;																								// Turns on clock for Port A, E, F
	int32_t delay = SYSCTL_RCGC2_R;
  TExaS_Init();       // Bus clock is 80 MHz 
  ADC_Init();         // turn on ADC, set channel to 1
  ST7735_InitR(INITR_REDTAB); 
  PortF_Init();
	
  while(1){           // use scope to measure execution time for ADC_In and LCD_OutDec           
    PF2 = 0x04;       // Profile ADC
		Data = ADC_In();  // sample 12-bit channel 1
    PF2 = 0x00;       // end of ADC Profile
    ST7735_SetCursor(0,0);
    PF1 = 0x02;       // Profile LCD
    LCD_OutDec(Data); 
    ST7735_OutString("    ");  // these spaces are used to coverup characters from last output
    PF1 = 0;          // end of LCD Profile
  }
}

uint32_t Convert(uint32_t input){
  return (4307*input)/10000 + 90; // Formula found from taking a linear representation of ADC input
}
int main3(void){
	SYSCTL_RCGC2_R |= 0x31;																								// Turns on clock for Port A, E, F
	int32_t delay = SYSCTL_RCGC2_R;
  TExaS_Init();         // Bus clock is 80 MHz 
  ST7735_InitR(INITR_REDTAB); 
  PortF_Init();
  ADC_Init();         // turn on ADC, set channel to 1
  while(1){
		PF2 ^= 0x04;      // Heartbeat
    Data = ADC_In();  // sample 12-bit channel 1
    PF3 = 0x08;       // Profile Convert
    Position = Convert(Data); 
    PF3 = 0;          // end of Convert Profile
    PF1 = 0x02;       // Profile LCD
    ST7735_SetCursor(0,0);
    LCD_OutDec(Data); ST7735_OutString("    "); // Prints raw ADC input
    ST7735_SetCursor(6,0);
    LCD_OutFix(Position); // Prints convereted input
    PF1 = 0;          // end of LCD Profile
		
  }
}   
uint32_t ADCMail, ADCStatus;
int main(void){
	SYSCTL_RCGC2_R |= 0x31;																								// Turns on clock for Port A, E, F
	int32_t delay = SYSCTL_RCGC2_R;
	ADCStatus = 0; ADCMail = 0; // Initializes ADC status and input
  TExaS_Init();         // Bus clock is 80 MHz 
  PortF_Init();
  ADC_Init();         // turn on ADC, set channel to 1
	SysTick_Init(80000000/40); 	// Sets frequency to 40 Hz sampling
	ST7735_InitR(INITR_REDTAB); 
  while(1){
		while(ADCStatus == 0){} // Waits till status is set to print
		ADCStatus = 0;					// Sets status to wait for print
		Data = ADCMail;
		PF3 = 0x08;       // Profile Convert
		Position = Convert(Data);  // Converts ADC input to position 
		PF3 = 0;          // end of Convert Profile
		
		PF1 = 0x02;       // Profile LCD
		ST7735_SetCursor(0,0);
		LCD_OutFix(Position); // Prints convereted input
		ST7735_OutString(" cm"); 	
		PF1 = 0;          // end of LCD Profile
	}
}
void SysTick_Handler(void){
	PF2 ^= 0x04;      // Heartbeat
	PF2 ^= 0x04;      // Heartbeat
	ADCMail = ADC_In();	// Fetches what ADC input is
	ADCStatus = 1;			// Sets status to print
	PF2 ^= 0x04;      // Heartbeat
}

