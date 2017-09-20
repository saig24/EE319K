// Sound.h
// This module contains the SysTick ISR that plays sound
// Runs on LM4F120 or TM4C123
// Program written by: put your names here
// Date Created: 8/25/2014 
// Last Modified: 10/5/2014 
// Section 1-2pm     TA: Wooseok Lee
// Lab number: 6
// Hardware connections

// Header files contain the prototypes for public functions
// this file explains what the module does

// **************Sound_Init*********************
// Initialize Systick periodic interrupts
// Called once, with sound initially off
// Output: none
void Sound_Init(void);

// **************Sound_Play*********************
// Start sound output, and set Systick interrupt period 
// Input: interrupt period
//           Units in frequency
//           Maximum 2^32
//           Minimum 1
//         input of zero disable sound output
// Output: none
void Sound_Play(uint32_t frequency);

// **************Piano_Play*********************
// Start sound output, and set Systick interrupt period 
// Input: interrupt period
//           Units in frequency
//           Maximum 2^32
//           Minimum 1
//         input of zero disable sound output
// Output: none
void Piano_Play(uint32_t frequency);

void NextNote(void); // Routine when timer interrupt occurs
// **************Music_Play*********************
// Start sound output, and set Systick interrupt period 
// Input: piano note played or not, note array, delay array, array size
void Music_Play(uint8_t PianoOn, uint32_t Notes[], uint8_t Waits[], uint8_t Size);

