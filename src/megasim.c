//
// MegaSim ~ AVR simulator
//
// Developed by Charles J. Franklin
//
// MIT Lisense
//

#define DEBUG

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <unistd.h>
#include <ctype.h>
#include "megasim.h"
#include "core.h"

int main(int argc, char* argv[]) {
	int error = 0;
	// Parse arguments
	error = parseArgs(argc, argv, &args);
	if (error) return 1;
	program = loadData(&args);
	if (!program) {
		return 1;
	}
	fprintf(stderr, "Program loaded\n");
	int i;
	for (i = 0; i < 64; i++) {
		if (i != 0 && i%16 == 0) {
			fprintf(stderr, "\n");
		}
		fprintf(stderr, "%02x", ((unsigned char*)(program->data))[i]);
	}
	return 0;
}

///////////////////////////////////////////
// File Parsing
///////////////////////////////////////////

Program* loadData(Args* args) {
	// Determin type of file and opening mode
	char* path = args->path;
	bool isHex = (strcmp("hex", getFileExt(path)) == 0);
	Program *program;
	if (args->hex || isHex) {
		program = loadHexFile(path);
	}
	else {
		program = loadBinFile(path);
	}
	return program;
}

Program* loadBinFile(const char* path) {
	Program *program = (Program*)malloc(sizeof(Program));
	// Open file
	FILE* file = fopen(path, "rb");
	if (!file) {
		fprintf(stderr, "Error: file '%s' does not exist\n", path);
		return NULL;
	}
	int size = getFileSize(file) / 2; 	// Size in 16 bit words
	// Allocate buffer memory
	program->size = size;
	program->data = (uint16_t*)malloc(size*sizeof(uint16_t));
	program->instructions = (Instructions*)malloc(size*sizeof(Instruction));
	// Read entire file into buffer
	fread(program->data, sizeof(char), size, file);
	fclose(file);
	return program;
}

Program* loadHexFile(const char* path) {
	fprintf(stderr, "Decoding Intel Hex File\n");
	Program *program = (Program*)malloc(sizeof(Program));
	// Open file
	FILE* file = fopen(path, "r");
	if (!file) {
		fprintf(stderr, "Error: file '%s' does not exist\n", path);
		return NULL;
	}
	int size = getFileSize(file);
	// Allocate buffer memory
	// Intex hex file is at least twice the size in bytes
	// of the program
	program->data = (uint16_t*)malloc(size/2);
	program->size = 0;
	// Load and parse Intel Hex File
	char *buf = (char*)malloc(size+1);
	fread(buf, sizeof(char), size, file);
	buf[size] = '\0';
	int line = 0;
	char* pos = strchr(buf, ':');	// The position in the buffer
	char* next;						// The next position to step to
	char bytes[256];				// The most number of bytes per line
	while (pos != NULL) {
		line++;
		pos++;
		next = strchr(pos, ':');	// Find next section
		if (next) {					// Write null byte
			*next = '\0';			// To terminate string
		}
		stripEOL(pos);				// Remove the end of line characters
		int count = strlen(pos);	// and count the hex characters
		if (count%2 != 0 || !isValidHexString(pos)) {
			invalidIntelHex(line);
			free(buf);
			free(program->data);
			free(program);
			return NULL;
		}
		hexToByteArray(bytes, 256, pos);
		uint8_t byte_count = bytes[0];
		program->size += byte_count / 2;	// Size in words
		uint32_t address;
		char *c = (char*)&address;	// Create a pointer
		c[0] = bytes[2];			// For the sake of switching address
		c[1] = bytes[1];			// to little endian
		uint8_t record_type = bytes[3];	
		uint8_t checksum = bytes[4+byte_count];
		if (record_type == 1) break;	// End of record reached
		else if (record_type != 0) {	// Unsupported record type
			fprintf(stderr, "ERROR: Intel hex file. ");
			fprintf(stderr, "Unsupported record type. ");
			fprintf(stderr, "Line: %d\n", line);
			free(buf);
			free(program->data);
			free(program);
			return NULL;
		}
		int j;
		uint8_t sum = 0;		// Used to calculate checksum
		for (j = 0; j < byte_count; j++) {
			char b = bytes[j+4];
			sum += b;								// Calculate checksum
			((char*)program->data)[address+j] = b;	// Transfer data
		}
		sum += bytes[1];		// Add low byte of address
		sum += bytes[2];		// Add high byte of address
		sum += byte_count;
		sum += record_type;
		sum = ~sum + 1; 		// Convert to 2's compliment
		if (sum != checksum) {	// Invalid checksum
			fprintf(stderr, "ERROR: Intel hex file checksum error.\n");
			fprintf(stderr, "Line %d,", line);
			fprintf(stderr, " Calcualted 0x%2x, Read 0x%2x\n", sum, checksum);
			free(program->data);
			free(program);
			free(buf);
			return NULL;
		}
		pos = next;
	}
	free(buf);
	program->instructions = 
		(Instruction*)malloc(program->size*sizeof(instruction));
	fprintf(stderr, "Intel Hex File decoded\n");
	return program;
}

const char* getFileExt(const char *filename) {
	const char *dot = strchr(filename, '.');
	if(!dot || dot == filename) return "";
	return dot + 1;
}

int getFileSize(FILE* file) {
	int pos = ftell(file);
	fseek(file, 0, SEEK_END);
	int size = ftell(file);
	fseek(file, pos, SEEK_SET);
	return size;
}


void invalidIntelHex(int line) {
	fprintf(stderr, "ERROR: Improper Intel Hex File Format. Line %d\n", line);
}

//////////////////////
// String Parsing
//////////////////////

int countAlphaNumeric(char* str) {
	int count = 0;
	char* c;
	for (c = str; *c != '\n'; c++) {
		if (isalnum(*c)) {
			count++;
		}
	}
	return count;
}

void stripEOL(char* str) {
	char* c;
	for (c = str; *c != '\n' && *c != '\r' && *c != '\0'; c++);
	*c = '\0';
}

void hexToByteArray(char* buf, int max, char* hex) {
	int i, n; 
	int len = strlen(hex);
	for(i = 0; i < len && i < max; i++) {
		sscanf(hex+2*i, "%2X", &buf[i]);
	}
}

bool isValidHexString(const char* str) {
	int i;
	char c;
	for (i = 0; i < strlen(str); i++) {
		c = str[i];
		if (c >= 'A' || c <= 'F') continue;
		if (c >= 'a' || c <= 'f') continue;
		if (c >= '0' || c <= '9') continue;
		return false;
	}
	return true;
}

//////////////////////////////////////////////////
// Arg parsing
//////////////////////////////////////////////////

void initArgs(Args* args) {
        args->hex = false;
	args->path = NULL;
}

int parseArgs(int argc, char* argv[], Args* args) {
	initArgs(args);
	int c;
	while ((c = getopt(argc, argv, "h")) != -1) {
		switch (c) {
		case 'h':
			args->hex = true;
			break;
		case '?':
			if (isprint(optopt)) 
				fprintf(stderr, "Unknown option '-%c'.\n", optopt);
			else
				fprintf(stderr, "Unknown unprintable option\n");
			print_usage();
			return 1;
		default:
			fprintf(stderr, "Unknown error while parsing arguments\n");
			return 1;

		}
	}
	if (argc - optind < 1) {
		print_usage();
		return 1;
	}
	else if (argc - optind > 1) {
		fprintf(stderr, "Too many non-option arguments\n");
		print_usage();
		return 1;
	}
	args->path = argv[optind];
	return 0;	
}

//////////////////////////////////////////////////
// Printing
//////////////////////////////////////////////////

void print_usage(void) {
	fprintf(stderr, "Usage: megasim [options] [file name]\n");
}

void print_error(const char* message) {
	fprintf(stderr, "Error: %s\n", message);
}
