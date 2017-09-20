// ***** 0. Documentation Section *****
// TableTrafficLight.c for (Lab 10 edX), Lab 5 EE319K
// Runs on LM4F120/TM4C123
// Program written by: Xinyuan (Allen)Pan & Chimezie Iwuanyanwu
// Date Created: 1/24/2015 
// Last Modified: 3/7/2016 
// Section WED5-6  
// Lab number: 5
// Hardware connections
// west red light connected to PE2
// west yellow light connected to PE1
// west green light connected to PE0
// south red light connected to PE5
// south yellow light connected to PE4
// south green light connected to PE3
// walk button connected to PA4 (1=pedestrian present)
// south sensor connected to PA3 (1=car present)
// west sensor connected to PA2 (1=car present)
// "walk" light connected to PF3 (built-in green LED)
// "don't walk" light connected to PF1 (built-in red LED)

// ***** 1. Pre-processor Directives Section *****
#include <stdint.h>
#include "TExaS.h"
#include "tm4c123gh6pm.h"
#include "SysTick.h"
// name the states
#define WtW_1			0
#define WtS_2			1
#define GoW_3			2
#define WtW_4			3
#define GoS_5			4
#define WtS_6			5
#define GoW_7			6
#define WtWalk_8	7
#define WtW_9			8
#define Walk_10		9
#define WtWalk_11	10
#define WtWalk_12	11
// ***** 2. Global Declarations Section *****
struct State{
	// output
	uint8_t PortE;
	uint8_t PortF;
	// time 100 = 1s
	uint32_t Time;
	// array of next state; elements are state numbers, indexes are inputs(3 bits)
	uint8_t Next[8];
};
// States defined by FSM
struct State FSM[12] = {
	{0x22, 0x02, 200, {GoS_5, 		GoS_5, 		GoS_5, 		GoS_5, 		GoS_5, 		GoS_5, 		GoS_5, 		GoS_5}},		//WtW_1
	{0x14, 0x02, 200, {GoW_3, 		GoW_3, 		GoW_3, 		GoW_3, 		GoW_3, 		GoW_3,		GoW_3, 		GoW_3}},		//WtS_2
	{0x21, 0x02, 300, {GoW_3, 		GoW_3, 		WtW_1, 		WtW_1, 		WtW_9, 		WtW_9, 		WtW_9, 		WtW_9}},		//GoW_3
	{0x22, 0x02, 200, {GoS_5, 		GoS_5, 		GoS_5, 		GoS_5, 		GoS_5, 		GoS_5, 		GoS_5, 		GoS_5}},		//WtW_4
	{0x0C, 0x02, 300, {GoS_5, 		WtS_2, 		GoS_5, 		WtS_2, 		WtS_6, 		WtS_6, 		WtS_6, 		WtS_6}},		//GoS_5
	{0x14, 0x02, 200, {Walk_10, 	Walk_10, 	Walk_10, 	Walk_10, 	Walk_10,	Walk_10, 	Walk_10, 	Walk_10}},	//WtS_6
	{0x21, 0x02, 300, {WtW_4, 		WtW_4, 		WtW_4, 		WtW_4, 		WtW_4, 		WtW_4, 		WtW_4, 		WtW_4}},		//GoW_7
	{0x24, 0x02, 200, {GoW_7, 		GoW_7, 		GoW_7, 		GoW_7, 		GoW_7, 		GoW_7, 		GoW_7, 		GoW_7}},		//WtWalk_8
	{0x22, 0x02, 200, {Walk_10, 	Walk_10, 	Walk_10, 	Walk_10, 	Walk_10, 	Walk_10, 	Walk_10, 	Walk_10}},	//WtW_9
	{0x24, 0x08, 300, {Walk_10, 	WtWalk_11,WtWalk_12,WtWalk_8, Walk_10, 	WtWalk_11,WtWalk_12,WtWalk_8}},	//Walk_10
	{0x24, 0x02, 200, {GoW_3, 		GoW_3, 		GoW_3, 		GoW_3, 		GoW_3, 		GoW_3, 		GoW_3, 		GoW_3}},		//WtWalk_11
	{0x24, 0x02, 200, {GoS_5, 		GoS_5, 		GoS_5,		GoS_5, 		GoS_5, 		GoS_5, 		GoS_5, 		GoS_5}}			//WtWalk_12
};

// FUNCTION PROTOTYPES: Each subroutine defined
void DisableInterrupts(void); // Disable interrupts
void EnableInterrupts(void);  // Enable interrupts

// ***** 3. Subroutines Section *****
uint8_t FlashPortF(uint8_t State); // PortF LED display function

int main(void){ 
  TExaS_Init(SW_PIN_PE210, LED_PIN_PB543210); // activate grader and set system clock to 80 MHz
	SysTick_Init();
	// initialize ports
	SYSCTL_RCGC2_R |= 0x31;		    //sets clocks for PortF/E/A
	int32_t delay = SYSCTL_RCGC2_R; // delay for clock to settle
	//Initialization for PortA
	GPIO_PORTA_DEN_R |= 0x1c;
	GPIO_PORTA_DIR_R &= ~0x1c;
	GPIO_PORTA_AMSEL_R &= ~0x1c;
	GPIO_PORTA_AFSEL_R &= ~0x1c;
	GPIO_PORTA_PCTL_R &= ~0x1c;
	//Initialization for PortE
	GPIO_PORTE_DEN_R |= 0x3F;
	GPIO_PORTE_DIR_R |= 0x3F;
	GPIO_PORTE_AMSEL_R &= ~0x3F;
	GPIO_PORTE_AFSEL_R &= ~0x3F;
	GPIO_PORTE_PCTL_R &= ~0x3F;
	//Initialization for PortF
	GPIO_PORTF_DEN_R |= 0x0A;
	GPIO_PORTF_DIR_R |= 0x0A;
	GPIO_PORTF_AMSEL_R &= ~0x0A;
	GPIO_PORTF_AFSEL_R &= ~0x0A;
	GPIO_PORTF_PCTL_R &= ~0x0A;
	// the starting state is GoW_3
	uint8_t State = GoW_3;
  EnableInterrupts();
  while(1){
		// output
    GPIO_PORTE_DATA_R = FSM[State].PortE;	// Outputs the LED on the board 
		uint8_t Flash = FlashPortF(State);		// Displays the LED on PortF
		// delay
		SysTick_Wait10ms(FSM[State].Time*((Flash-1)*(Flash-1)));	// Delays specified by the FSM
		// input
		uint8_t Input = (GPIO_PORTA_DATA_R >> 2) & 0x07; // shift it to bit[2:0] and clear the useless bits
		// next state
		State = FSM[State].Next[Input];				// Updates the next state
  }
}
uint8_t FlashPortF(uint8_t State){ // Displays the LED on PortF
	uint8_t Flash = ((FSM[State].PortF & 0x02) >> 1) & ((FSM[State].PortE & 0x20) >> 5) & ((FSM[State].PortE & 0x04) >> 2) & (((GPIO_PORTA_DATA_R & 0x08) >> 3) | ((GPIO_PORTA_DATA_R & 0x04) >> 2));
	GPIO_PORTF_DATA_R &= 0;
	SysTick_Wait10ms(Flash*(FSM[State].Time/8));
	GPIO_PORTF_DATA_R |= FSM[State].PortF;
	SysTick_Wait10ms(Flash*(FSM[State].Time/8));
	GPIO_PORTF_DATA_R &= 0;
	SysTick_Wait10ms(Flash*(FSM[State].Time/8));
	GPIO_PORTF_DATA_R |= FSM[State].PortF;
	SysTick_Wait10ms(Flash*(FSM[State].Time/8));
	
	GPIO_PORTF_DATA_R &= 0;
	SysTick_Wait10ms(Flash*(FSM[State].Time/8));
	GPIO_PORTF_DATA_R |= FSM[State].PortF;
	SysTick_Wait10ms(Flash*(FSM[State].Time/8));
	GPIO_PORTF_DATA_R &= 0;
	SysTick_Wait10ms(Flash*(FSM[State].Time/8));
	GPIO_PORTF_DATA_R |= FSM[State].PortF;
	SysTick_Wait10ms(Flash*(FSM[State].Time/8));
	
	return Flash;
}
	
