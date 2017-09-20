// Lab6.c
// Runs on LM4F120 or TM4C123
// Use SysTick interrupts to implement a 4-key digital piano
// MOOC lab 13 or EE319K lab6 starter
// Program written by: Chimezie Iwuanyanwu & Xinyuan Pan
// Date Created: 1/24/2015 
// Last Modified: 3/23/2015 
// Section 5-6pm     TA: Wooseok Lee
// Lab number: 6
// Hardware connections
// TO STUDENTS "REMOVE THIS LINE AND SPECIFY YOUR HARDWARE********


#include <stdint.h>
#include <stdbool.h>																										// Allows for booleans to be used
#include "tm4c123gh6pm.h"
#include "Sound.h"
#include "Piano.h"
#include "TExaS.h"
#define A 440																														// Each note based on piano frequency
#define B 494
#define C 261
#define D 293
#define E 329
#define F 349
#define G 392
#define Gb 196

uint32_t TwinkleNotes[42] = {	C, C, G, G, A, A, G, F, F, E, E, D, D, C, // Notes in order for Twinkle Twinkle Little Star
															G, G, F, F, E, E, D, G, G, F, F, E, E, D,
															C, C, G, G, A, A, G, F, F, E, E, D, D, C}; 
uint8_t  TwinkleWaits[42] = {	4, 4, 4, 4, 4, 4, 2, 4, 4, 4, 4, 4, 4, 2,	// Delay for each note in Twinkle Twinkle Little Star
															4, 4, 4, 4, 4, 4, 2, 4, 4, 4, 4, 4, 4, 2,	// Each unit represents 1/unit of a second
															4, 4, 4, 4, 4, 4, 2, 4, 4, 4, 4, 4, 4, 2}; 
uint32_t CenaNotes[63] = 		{	E, D, C, B, B, A, G, G,Gb, G,Gb, G,Gb, G,	// Notes in order for SOMETHING THAT CAN'T BE SEEN
															Gb, G,Gb, G,Gb,G,Gb, G,Gb, 0, G, B, F, G,
															G, G, 0, B,A, F, 0, G, G, B, A, G,Gb, 0,
															G, B, F, 0,G, G, G, 0, B, A, F, 0, G, G,
															0, G, B, F,0, G, 0};
uint8_t  CenaWaits[63] = 		{ 2, 2, 2, 2, 2, 2, 2, 32,32,32,32,32,32,32,// Delay for each note for John Cena theme song
															32,32,32,32,32,32,32,32,32, 8, 8,16,16, 2,
															16,16,16, 8,16,16,16, 2,16, 2, 2, 2, 2, 8,
															8, 16,16,16,2, 16,16,16, 8,16,16,16, 2,16,
															8, 8, 16,16,16,16,2};
// basic functions defined at end of startup.s
void DisableInterrupts(void); 																					// Disable interrupts
void EnableInterrupts(void);  																					// Enable interrupts

int main(void){      
  TExaS_Init(SW_PIN_PE3210,DAC_PIN_PB3210,ScopeOn);    									// bus clock at 80 MHz
	SYSCTL_RCGC2_R |= 0x3A;																								// Turns on clock for Port B, E, F
	int32_t delay = SYSCTL_RCGC2_R; 																			// Delay for clock to stablizie
	
	GPIO_PORTD_DEN_R |= 0x08;																							// Initialiation of PortD pin 3 for Texas_Init
	GPIO_PORTD_DIR_R |= ~(0x08);
	GPIO_PORTD_AMSEL_R &= ~0x08;
	GPIO_PORTD_AFSEL_R &= ~0x08;
	GPIO_PORTD_PCTL_R &= ~0x08;
	
	Piano_Init();																													// Initializes for key inputs
	Sound_Init();																													// Initializes for sound to be played
	EnableInterrupts();																										// Allows for SysTick and Timer interrupts to happen
	
	while(1){
		uint8_t prevNote = 8;																								// Previous note played before new key check
		if(Piano_In() == 4){																								// Plays normal Piano Keys if 3rd switch pressed
			while(1){
				uint8_t currNote = Piano_In();																 	// updates current note pressed
				if(currNote != prevNote){																				// Checks to see if a new key is pressed
					switch(currNote){																							// Plays note depending on which switch pressed
						case 1 : Sound_Play(C); break; // Note C
						case 2 : Sound_Play(D); break; // Note D
						case 3 : Sound_Play(E); break; // Note E
						case 4 : Sound_Play(F); break; // Note F
						case 5 : Sound_Play(G); break; // Note G
						case 6 : Sound_Play(A); break; // Note A
						case 7 : Sound_Play(B); break; // Note B
						default : Sound_Play(0); break; // Plays nothing as default
					}
				}
				prevNote = currNote;																						// Changes the previous note to current 
			} 
		}
		else if(Piano_In() == 2){ 																					// Plays decayed Piano Keys if 2nd switch pressed
			while(1){
				uint8_t currNote = Piano_In();
				if(currNote != prevNote){																				// Checks to see if a new key is pressed
					switch(currNote){																							// Plays note depending on which switch pressed
						case 1 : Piano_Play(C); break; // Decayed Note C
						case 2 : Piano_Play(D); break; // Decayed Note D
						case 3 : Piano_Play(E); break; // Decayed Note E
						case 4 : Piano_Play(F); break; // Decayed Note F
						case 5 : Piano_Play(G); break; // Decayed Note G
						case 6 : Piano_Play(A); break; // Decayed Note A
						case 7 : Piano_Play(B); break; // Decayed Note B
						default : Piano_Play(0); break; // Plays nothing as default
					}
				}
				prevNote = currNote;																						// Changes the previous note to current 
			} 
		}
		else if (Piano_In() == 1){ 																					// Plays songs if 1st switch pressed
			uint8_t ChangeSound = 0; 																					// Changes whether song is played in decayed keys or normal; Normal at Start
			while(1){
				uint8_t currNote = Piano_In();
				if(currNote != prevNote){																				// Checks to see if a new key is pressed
					switch(currNote){																							// Option depending on which switch pressed
					case 1 : ChangeSound ++; 
									 if(ChangeSound % 2 == 0) Piano_Play(C);							// Changes key sound and plays notification
									 else Sound_Play(C);  break; 													// Changes key sound and plays notification
					case 2 : Music_Play(ChangeSound, CenaNotes, 		CenaWaits, 		(sizeof(CenaNotes)/sizeof(CenaNotes[0]))); 				break; // Plays John Cena theme with type of notes specified
					case 4 : Music_Play(ChangeSound, TwinkleNotes, 	TwinkleWaits, (sizeof(TwinkleNotes)/sizeof(TwinkleNotes[0]))); 	break; // Plays Twinkle Twinkle Little Star with type of notes specified
					default : Sound_Play(0); break; 															// Plays nothing as default
					}
				}
				prevNote = currNote;																						// Changes the previous option to current 
			}
		}
	}	
}



