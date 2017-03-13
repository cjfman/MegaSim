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
#include <getopt.h>
#include <ctype.h>
#include <math.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>
#include "megasim.h"
#include "core.h"
#include "devices.h"
#include "decoder.h"
#include "handlers.h"
#include "peripherals.h"
#include "debugterm.h"

#ifdef NO_HARDWARE
#define NO_PERPHS
#define NO_TIMERS
#define NO_PORTS
#endif	// NO_HARDWARE

int main(int argc, char* argv[]) {
	signal(SIGINT, intHandler);
	int error = 0;
	// Parse arguments
	error = parseArgs(argc, argv, &args);
	if (error) return 1;
	stderr_addr = args.stderr_addr;
	stdout_addr = args.stdout_addr;

	// Setup core
	coredef = (CoreDef*)malloc(sizeof(CoreDef));		// Allocate core
	memcpy(coredef, &default_core, sizeof(CoreDef));	// Load core
	/*
	if (coredef->default_addrs) {
		setDefaultAddresses(coredef);
	}
	*/
	setupMemory();

	// Load Program
	program = loadData(&args);
	if (!program) {
		return 1;
	}
	fprintf(stderr, "Program loaded. %d bytes\n", program->size*2);
	decodeAllInstructions();
	fprintf(stderr, "All instructions decoded\n");

	// Start Peripherals
#ifndef NO_PERPHS
	if (args.p_count) {
		fprintf(stderr, "Using the following peripherials\n");
		openPeripherals(args.peripherals, args.p_count);
	}
#endif // NO_PERPHS

	// Start simulator
	fprintf(stderr, "Starting simulator\n");
	error = runAVR();
	if (error) {
		printError(error);
	}

	// Free memory
	teardownMemory();
	free(program->data);
	free(program->instructions);
	free(program);
#ifndef NO_PERPHS
	closePeripherals();
#endif
	return error;
}

////////////////////////////////////////
// Signal Handlers
////////////////////////////////////////

void intHandler(int val) {
	//fprintf(stderr, "\nINT Signal Caught\n");
	//exit(0);
	if (!debug_mode) {
		break_now = true;
	}
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
	args->stderr_addr = 0x00E0;
	args->stdout_addr = 0x00E1;
#ifndef NO_PERPHS
	args->peripherals = NULL;
	args->p_count = 0;
#endif // NO_PERPHS
}

static struct option long_options[] = {
	/* These options set a flag*/
	/* These options don't set a flag*/
	{"hex", 	no_argument, 		0, 'h'},
	{"mapout", 	required_argument,	0, 'o'},
	{"maperr", 	required_argument,	0, 'e'},
#ifndef NO_PERPHS
	{"perphs",	required_argument,	0, 'p'},
#endif // NO_PERPHS
	/*end*/
	{0, 0, 0, 0}
};

int parseArgs(int argc, char* argv[], Args* args) {
	initArgs(args);
	int oindex = 0;
	int c;
	while ((c = getopt_long(argc, argv, "ho:e:p:", long_options, &oindex)) != -1) {
		switch (c) {
		case 'h':
			args->hex = true;
			break;
		case 'o':
			errno = 0;
			if ((args->stdout_addr = strtol(optarg, NULL, 0)) == 0) {
				if (errno != 0) {
					fprintf(stderr, "-o must be number\n");
				}
				else {
					fprintf(stderr, "-o cannot be 0");
				}
				return 1;
			}
			break;
		case 'e':
			errno = 0;
			if ((args->stderr_addr = strtol(optarg, NULL, 0)) == 0) {
				if (errno != 0) {
					fprintf(stderr, "-e must be number\n");
				}
				else {
					fprintf(stderr, "-e cannot be 0");
				}
				return 1;
			}
			break;
		case 'p':
#ifndef NO_PERPHS
			parsePeripherals(optarg);
#else
			fprintf(stderr, "ALERT: Peripherals disabled!\n");
#endif
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

#ifndef NO_PERPHS
int parsePeripherals(char* p) {
	char *pos;
	char *next;
	// Count commas
	int count = 0;
	for (pos = p; *pos != '\0'; pos++) {
		count += (*pos == ',');
	}

	// Allocate Memory
	args.peripherals = (char**)malloc((count + 1)*sizeof(char**));
	int i;
	pos = p;
	// Get comma seperated entries
	for (i = 0; i < count; i++) {
		next = strchr(pos, ',');
		*(next++) = '\0';
		args.peripherals[i] = pos;
		pos = next;
	}
	// Get last entry
	args.peripherals[i] = pos;
	args.p_count = count + 1;
	return 1;
}
#endif // NO_PERPHS

//////////////////////////////////////////////////
// Printing
//////////////////////////////////////////////////

void print_usage(void) {
	fprintf(stderr, "Usage: megasim [options] [file name]\n");
}

void print_error(const char* message) {
	fprintf(stderr, "Error: %s\n", message);
}
