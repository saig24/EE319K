// Piano.c
// This software configures the off-board piano keys
// Runs on LM4F120 or TM4C123
// Program written by: Chimezie Iwuanyanwu & Xinyuan Pan
// Date Created: 8/25/2014 
// Last Modified: 3/23/2015 
// Section 5-6pm     TA: Wooseok Lee
// Lab number: 6
// Hardware connections
// TO STUDENTS "REMOVE THIS LINE AND SPECIFY YOUR HARDWARE********

// Code files contain the actual implemenation for public functions
// this file also contains an private functions and private data
#include <stdint.h>
#include "tm4c123gh6pm.h"

// **************Piano_Init*********************
// Initialize piano key inputs, called once 
// Input: none 
// Output: none
void Piano_Init(void){	//Port E Initialzation
	GPIO_PORTE_DEN_R |= 0x0F;
	GPIO_PORTE_DIR_R &= ~0x0F;
	GPIO_PORTE_AFSEL_R &= ~0x0F;
	GPIO_PORTE_AMSEL_R &= ~0x0F;
}

// **************Piano_In*********************
// Input from piano key inputs 
// Input: none 
// Output: 0 to 4 depending on keys
// 0x01 is just Key0, 0x02 is just Key1, 0x04 is just Key2
uint32_t Piano_In(void){
  return GPIO_PORTE_DATA_R & 0x0F; // Returns which keys are played 
}
