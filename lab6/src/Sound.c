// Sound.c
// This module contains the SysTick ISR that plays sound
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
#include <stdbool.h> 									// Allows for booleans to be used
#include "dac.h"
#include "tm4c123gh6pm.h"
#include "SysTickInts.h"
#include "Timer0A.h"
#include "Piano.h"

uint8_t Index, NotesIndex;						// Index of SineWave and music notes
uint32_t periodCheck, SineCount;			// zero frequency check and decay incrementor
bool Delay, DelayEnable, FlatWave, PianoNote; // delay between notes, delay between notes happened, 
																												// flatwave played, piano note played, select note played
// Sine wave generated to play a note
const uint8_t SineWave[32] = {8, 9, 10, 12, 13, 14, 14, 15, 15, 15, 14, 14, 13, 12, 10, 9,
															8, 6,  5,  3,  2,  1,  1,  0,  0,  0,  1,  1,  2,  3,  5, 6};

// **************Sound_Init*********************
// Initialize Systick periodic interrupts
// Called once, with sound initially off
// Input: none
// Output: none
void Sound_Init(void){
	DAC_Init();												// Initization of DAC output
	Index = 0;												// Initial index is start of wave	
	GPIO_PORTF_DEN_R |= 0x0A;					// Initialiation of PortF
	GPIO_PORTF_DIR_R |= 0x0A;
	GPIO_PORTF_AMSEL_R &= ~0x0A;
	GPIO_PORTF_AFSEL_R &= ~0x0A;
	GPIO_PORTF_PCTL_R &= ~0x0A;
	GPIO_PORTF_DATA_R |= 0x0A;
}

// **************Sound_Play*********************
// Start sound output, and set Systick interrupt period 
// Input: interrupt period
//           Units in frequency 
//           Maximum 2^32 
//           Minimum 1
//         input of zero disable sound output
// Output: none
void Sound_Play(uint32_t frequency){
	uint32_t period;									// Period used for SysTick to make frequency
	PianoNote = false;								// Not a decayed note
	periodCheck = frequency;				 	// Checks if zero frequency
	if(frequency == 0) period = 2500000;	// Blinking light when nothing is played
	else period = 2500000/frequency;
	SysTick_Init(period);							// Updates SysTickInts to knew note played
}

// **************Piano_Play*********************
// Start sound output, and set Systick interrupt period 
// Input: interrupt period, boolean selection note
//           Units in frequency
//           Maximum 2^32
//           Minimum 1
//         input of zero disable sound output
// Output: none
void Piano_Play(uint32_t frequency){
	uint32_t period;									// Period used for SysTick to make frequency
	SineCount = 1;										// Initializes the delay incremntor 
	FlatWave = false;									// Not a zero frequency played
	PianoNote = true;									// Plays decayed note
	periodCheck = frequency; 					// Checks if zero frequency
	if(frequency == 0) period = 2500000;	// Blinking light when nothing is played
	else period = 2500000/frequency;
	SysTick_Init(period);							// Updates SysTickInts to knew note played
}

// Routine played with Timer countdown finishes
void NextNote(void){ 								
		periodCheck = 0; 								// Zero frequency played
		if(Delay) {Delay = false; DelayEnable = false; NotesIndex ++;} // Allows or prevents delay between notes from occuring
		else Delay = true;
}//sian rios

// **************Music_Play*********************
// Start sound output, and set Systick interrupt period 
// Input: piano note played or not, note array, delay array, array size
void Music_Play(uint8_t PianoOn, uint32_t Notes[], uint8_t Waits[], uint8_t Size){
	uint8_t PrevNote = 50; 						// Initializes previous note with arbitrary value to see if new note is to be played
	NotesIndex = 0;										// Initial music notes index is zero
	if(PianoOn % 2 == 0) PianoNote = true; // Allows or prevents decayed note from played
	else PianoNote = false;
	Delay = false;										// No delay between notes at start
	DelayEnable = false;							// Allows delays between each note
	while(NotesIndex < Size){ 				// When there is still notes let to be played
		if(Piano_In() == 1) break;			// Stops playing song when switch 1 is pressed
		if(PrevNote != NotesIndex){ 		// If new not is to be played
			if(Delay == false && DelayEnable == false){ // If has not delayed before and is not delaying now
				DelayEnable = true;
				if(PianoNote) Piano_Play(0); // Plays decayed note or regular depeneding on input
				else Sound_Play(0);
				Timer0A_Init(&NextNote, (80000000/(Waits[NotesIndex] * 5)) * 3); // Starts the delay timer
			}
			else if(Delay){ 							//If has delayed before and is a delay now
				if(PianoNote) Piano_Play(Notes[NotesIndex]); // Plays decayed note or regular depeneding on input
				else Sound_Play(Notes[NotesIndex]);
				Timer0A_Init(&NextNote, (80000000/(Waits[NotesIndex] * 1)) * 1); // Starts the delay timer
				PrevNote = NotesIndex;			// Sets previous note pressed to current
			}
		}
	}
	TIMER0_CTL_R &= 0;								// Turns off timer
	NotesIndex = 0;										// Start of notes index becoems zero again
}

// Runs through SysTick_Handler whenever SysTick finishes counting down
void SysTick_Handler(void){
		if(NotesIndex != 0){GPIO_PORTF_DATA_R ^= 0x02; GPIO_PORTF_DATA_R &= ~(0x08);} // Red LED if song is playing
		else {GPIO_PORTF_DATA_R ^= 0x08; GPIO_PORTF_DATA_R &= ~(0x02);} 				// Green LED if song is not playing
		
		if(PianoNote){ 									// If decayed note requested
			uint32_t NoteDecay = 1000;
			if(periodCheck != 0){					// Checks if zero frequency played
				Index = (Index+1) & 0x1F;   // Outputs next in SineWave
				uint8_t DacOut = SineWave[Index]; 
				if(DacOut <= 7) 						// Decays depending if top or bottom half of wave
					if(DacOut + (SineCount/NoteDecay) > 7) DacOut = 7;
					else DacOut += (SineCount/NoteDecay);
				else 
					if(DacOut - (SineCount/NoteDecay) <= 7) DacOut = 7;
					else DacOut -= (SineCount/NoteDecay);
			if(Index == 7 && DacOut == 7) FlatWave = true; // If crest is completely decayed, then whole wave becomes flat
			if(FlatWave) DAC_Out(7);			// Outputs constant flat wave or zero frequency
			else if(SineCount/NoteDecay != 15) {DAC_Out(DacOut); SineCount++;}			// output one value each interrupt and increases decay
			}
		}
		else{
			if(periodCheck != 0){					// Checks if zero frequncy
				Index = (Index+1) & 0x1F;   // Outputs next in SineWave 
				DAC_Out(SineWave[Index]);
			}
		}
}


