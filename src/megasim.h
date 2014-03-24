// megasim.h
// Developed by Charles Franklin
//
// MIT Lisense

#ifndef MEGASIM_h
#define MEGASIM_h

#include <stdio.h>
#include <inttypes.h>
#include "bool.h"
#include "core.h"

//////////////////
// Option Parsing
//////////////////

// A struct to hold options and arguments
typedef struct Args {
	bool hex; 	// Use hex file format
	char* path;	// The path to the file
} Args;

Args args;	// Global arguments struct

// Initializes an Args struct to default values
void initArgs(Args* args);
// Parses the arguments to the program
int parseArgs(int argc, char* argv[], Args* args);


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


#endif
