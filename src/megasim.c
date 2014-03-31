//
// MegaSim ~ AVR simulator
//
// Developed by Charles J. Franklin
//
// MIT Lisense
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <unistd.h>
#include <getopt.h>
#include <ctype.h>
#include <math.h>
#include "megasim.h"
#include "core.h"
#include "devices.h"
#include "decoder.h"

int main(int argc, char* argv[]) {
	int error = 0;
	// Parse arguments
	error = parseArgs(argc, argv, &args);
	if (error) return 1;
	coredef = &default_core;

	// Load Program
	program = loadData(&args);
	if (!program) {
		return 1;
	}
	fprintf(stderr, "Program loaded. %d bytes\n", program->size*2);
	/*
	int i;
	for (i = 0; i < 64; i++) {
		if (i != 0 && i%16 == 0) {
			fprintf(stderr, "\n");
		}
		fprintf(stderr, "%02x", ((unsigned char*)(program->data))[i]);
	}
	// */

	// Setup core
	setupMemory();
	decodeAllInstructions();
	fprintf(stderr, "All instructions decoded\n");
	
	// Start simulator
	fprintf(stderr, "Starting simulator\n");
	error = runAVR();
	if (error) {
		fprintf(stderr, "Unrecoverable Error: %d\n", error);
	}

	// Free memory
	teardownMemory();
	free(program->data);
	free(program->instructions);
	free(program);
	return error;
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
	size = fmin(size, coredef->prog_mem_size);
	// Allocate buffer memory
	program->size = size;
	program->data = (uint16_t*)malloc(coredef->prog_mem_size*2);
	program->instructions = 
		(Instruction*)malloc(coredef->prog_mem_size*sizeof(Instruction));
	memset(program->data, 0, coredef->prog_mem_size*2);
	// Read entire file into buffer
	fread(program->data, sizeof(uint16_t), size, file);
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
	program->data = (uint16_t*)malloc(coredef->prog_mem_size*2);
	program->instructions = 
		(Instruction*)malloc(coredef->prog_mem_size*sizeof(Instruction));
	memset(program->data, 0, coredef->prog_mem_size*2);
	// Load and parse Intel Hex File
	char *buf = (char*)malloc(size+1);
	fread(buf, sizeof(char), size, file);
	buf[size] = '\0';
	int line = 0;
	char* pos = strchr(buf, ':');	// The position in the buffer
	char* next;						// The next position to step to
	char bytes[256];				// The most number of bytes per line
	while (pos != NULL && program->size < coredef->prog_mem_size) {
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
		int max_trans = 		// Don't copy past the end of memory
			fmin(byte_count, coredef->prog_mem_size - program->size);
		program->size += max_trans;
		for (j = 0; j < max_trans; j++) {
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

static struct option long_options[] = {
	/* These options set a flag*/
	/* These options don't set a flag*/
	{"hex", no_argument, 0, 'h'},
	/*end*/
	{0, 0, 0, 0}
};

int parseArgs(int argc, char* argv[], Args* args) {
	initArgs(args);
	int oindex = 0;
	int c;
	while ((c = getopt_long(argc, argv, "h", long_options, &oindex)) != -1) {
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
