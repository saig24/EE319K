// FiFo.c
// Runs on LM4F120/TM4C123
// Provide functions that implement the Software FiFo Buffer
// Last Modified: 4/14/2016 
// Student names: Chimezie Iwuanyanwu & Xinyuan Pan

#include <stdint.h>
#include "FiFo.h"
// Declare state variables for FiFo
//        size, buffer, put and get indexes
#define FIFO_SIZE 10
int32_t static PutI; // Index to put new
                
int32_t static GetI; // Index of oldest 

int32_t Fifo[FIFO_SIZE];	// FiFo array


// *********** FiFo_Init**********
// Initializes a software FIFO of a
// fixed size and sets up indexes for
// put and get operations
void FiFo_Init() {
	PutI = 0;	// Start position and end is same to signal empty
	GetI = 0;
}

// *********** FiFo_Put**********
// Adds an element to the FIFO
// Input: Character to be inserted
// Output: 1 for success and 0 for failure
//         failure is when the buffer is full
uint32_t FiFo_Put(char data) {
	if ( ((PutI+1)%FIFO_SIZE) == GetI) {
          return(0);		// Return 0 if FiFo full
   }
   Fifo[PutI] = data;		// Puts data into FiFO
   PutI = (PutI+1)%FIFO_SIZE;
   return(1);						// Return 1 since put successful


}

// *********** FiFo_Get**********
// Gets an element from the FIFO
// Input: Pointer to a character that will get the character read from the buffer
// Output: 1 for success and 0 for failure
//         failure is when the buffer is empty
uint32_t FiFo_Get(char *datapt)
{
	if (GetI == PutI) {
          return(0);		// Return 0 if FIFo empty
   }
   *datapt = Fifo[GetI];	// Gets data from FiFo and puts in variable
   GetI = (GetI+1)%FIFO_SIZE;
   return(1);						// Return 1 since get successful


}



