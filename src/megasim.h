// megasim.h
// Developed by Charles Franklin
//
// MIT Lisense

#ifndef MEGASIM_h
#define MEGASIM_h

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <inttypes.h>
#include "core.h"

//////////////////
// Option Parsing
//////////////////

// A struct to hold options and arguments
typedef struct Args {
	bool hex; 				// Use hex file format
	char* path;				// The path to the file
	uint16_t stderr_addr;	// The address mapped to stderr
	uint16_t stdout_addr;	// The address mapped to stdout
	char** peripherals;		// A list of exacutable periferals
	int p_count;		// The number of periferals
} Args;

Args args;	// Global arguments struct

// Initializes an Args struct to default values
void initArgs(Args* args);
// Parses the arguments to the program
int parseArgs(int argc, char* argv[], Args* args);
// Parses the peripheral option
int parsePeripherals(char* p);
int parseInt(int* i, char* c);


//////////////////
// File Parsing
//////////////////

// Loads the file given in the arguments
// and allocates memory for the entire file.
// Binary files for AVR micros aren't that large.
// Returns the pointer to the data.
Program* loadData(Args* args);
Program* loadHexFile(const char* path);
Program* loadBinFile(const char* path);
const char* getFileExt(const char *filename);
int getFileSize(FILE* file);
void invalidIntelHex(int line);

//////////////////////
// String Parsing
//////////////////////

int countAlphaNumeric(char* str);
void stripEOL(char* str);
void hexToByteArray(char* buf, int max, char* hex);
bool isValidHexString(const char* str);

///////////////
// Printing
///////////////
void print_usage(void);
void print_error(const char* message);

#ifdef __cplusplus
}
#endif

#endif // MEGASIM_h
