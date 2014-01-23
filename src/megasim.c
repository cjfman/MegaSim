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
#include "megasim.h"

int main(int argc, char* argv[]) {
	if (argc != 2) {
		print_usage();
		return 1;
	}
	// Open file
	char* file_path = argv[1];
	FILE* prog_file = fopen(file_path, "rb");
	if (!prog_file) {
		fprintf(stderr, "Error: file '%s' does not exist\n", file_path);
		return 1;
	}
	// Get file size
	fseek(prog_file, 0, SEEK_END);
	int prog_size = ftell(prog_file)/sizeof(uint16_t);
	fseek(prog_file, 0, SEEK_SET);
	// Read entire file into buffer
	uint16_t* program = (uint16_t*)malloc(prog_size*sizeof(uint16_t));
	fread(program, sizeof(uint16_t), prog_size, prog_file);
	fclose(prog_file);
	// Print file Test
	int i;
	for (i = 0; i < prog_size; i++) {
		printf("%2x", program[i]);
		if (i%16 == 0) printf("\n");
	}
	return 0;
}

void print_usage(void) {
	fprintf(stderr, "Usage: megasim [file name]\n");
}

void print_error(const char* message){
	fprintf(stderr, "Error: %s\n", message);
}
