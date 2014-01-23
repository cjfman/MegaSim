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

int main(int argc, char* argv[]) {
	int error = 0;
	// Parse arguments
	error = parseArgs(argc, argv, &args);
	if (error) return 1;
	Program *program = loadData(&args);
	if (!program) {
		return 1;
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
	if (args->hex || isHex) {
		return loadHexFile(path);
	}
	else {
		return loadBinFile(path);
	}
}

Program* loadBinFile(const char* path) {
	Program *program = (Program*)malloc(sizeof(Program));
	// Open file
	FILE* file = fopen(path, "r");
	if (!file) {
		fprintf(stderr, "Error: file '%s' does not exist\n", path);
		return NULL;
	}
	int size = getFileSize(file);
	program->size = size/2;
	// Allocate buffer memory
	program->data = (uint16_t*)malloc(size);
	// Read entire file into buffer
	fread(program->data, sizeof(char), size, file);
	fclose(file);
	return program;
}

Program* loadHexFile(const char* path) {
	return NULL;
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
