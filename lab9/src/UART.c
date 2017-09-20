// UART.c
// Runs on LM4F120/TM4C123
// Provide functions that setup and interact with UART
// Last Modified: 4/14/2016 
// Student names: Chimezie Iwuanyanwu & Xinyuan Pan

#include <stdint.h>
#include "tm4c123gh6pm.h"
#include "FiFo.h"

int32_t RxCounter = 0;	// Receive counter
uint32_t UART_Error = 0; // Overflow counter

void EnableInterrupts(void);  // Enable interrupts

// UART initialization function 
// Input: none
// Output: none
void UART_Init(void){ 
	SYSCTL_RCGCUART_R |= 0x02; // activate UART1
	FiFo_Init();							// Initializes FiFo
  UART1_CTL_R &= ~0x0001;      // disable UART
  UART1_IBRD_R = 50; 					// Baudrate 
// IBRD=int(80000000/(16*100,000)) = int(50)
  UART1_FBRD_R = 0;  
// FBRD = round(0 * 64) = 0
	UART1_IFLS_R  =0;
	UART1_IFLS_R |= UART_IFLS_RX4_8; // FiFo Interrupt at 1/2 full
	UART1_IM_R |= 0x10;						// Allows FiFo Interrupt
	UART1_LCRH_R = 0x0070;  // 8-bit length, enable FIFO
  UART1_CTL_R = 0x0301;   // enable RXE, TXE and UART
  GPIO_PORTC_PCTL_R =
            (GPIO_PORTC_PCTL_R&0xFF00FFFF)+0x00220000; 
	GPIO_PORTC_AMSEL_R &= ~0x30; // No analog on PC4-5
	GPIO_PORTC_AFSEL_R |= 0x30; // alt funct on PC4-5
  GPIO_PORTC_DEN_R |= 0x30;   // digital I/O on PC4-5
	NVIC_PRI1_R = (NVIC_PRI1_R&0xFF1FFFFF)|0x00600000; 	
	NVIC_EN0_R = NVIC_EN0_INT6;		// Enable level 6 interrupt
}

//------------UART_InChar------------
// Wait for new input,
// then return ASCII code
// Input: none
// Output: char read from UART
// *** Need not be busy-wait if you know when to call
char UART_InChar(void){  
 while((UART1_FR_R&0x0010) != 0){}  // Waits for new input
  // wait until RXFE is 0
  return((uint8_t)(UART1_DR_R&0xFF));	// Gets vaule 
}

//------------UART_OutChar------------
// Wait for new input,
// then return ASCII code
// Input: none
// Output: char read from UART
void UART_OutChar(char data){  
while((UART1_FR_R&0x0020) != 0){}  // Waits for new input
  // wait until TXFF is 0
  UART1_DR_R = data; 							// Store data into UART FiFo
}

void UART1_Handler(void){ char letter;
	GPIO_PORTF_DATA_R ^= 0x02;      // Heartbeat
	GPIO_PORTF_DATA_R ^= 0x02;      // Heartbeat
	while((UART1_FR_R & UART_FR_RXFE) == 0){
		letter = UART_InChar();			// Gets vaule from UART FiFo
		UART_Error += FiFo_Put(letter); // Counts times the FiFo is full
	}
	RxCounter++;	// Counts time data received
	UART1_ICR_R = 0x10;		// Acknowledges handler handled...
	GPIO_PORTF_DATA_R ^= 0x02;      // Heartbeat
}
