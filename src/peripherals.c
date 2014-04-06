// peripherals.h

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include "peripherals.h"

// OS X uses MAP_ANON
// Linux has depricated MAP_ANON and prefers MAP_ANONYMOUS
#ifdef MAP_ANONYMOUS
#define MAP_ANON MAP_ANONYMOUS
#endif

int perph_flag = false;

int openPeripherals(char** perphs, int p_count) {
	int i;
	int errors = 0;
	peripherals = (Peripheral**)malloc(p_count*sizeof(Peripheral*));
	perph_errors = (int*)malloc(p_count*sizeof(int));
	for (i = 0; i < p_count; i++) {
		int rd_pipefd[2];
		int wr_pipefd[2];
		if (pipe(rd_pipefd) == -1 || pipe(wr_pipefd) == -1) {
			perph_errors[errors] = i;
			continue;
		}
		// Allocate space for the peripheral
		// Use shared memory space so that the peripheral
		// can report that it has started successfully
		Peripheral *perph = (Peripheral*)mmap(NULL, sizeof(Peripheral),
			PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANON, -1, 0);
		peripherals[num_perphs] = perph;
		perph->live = false; 	// Assume false
		// Start fork
		int cpid = fork();
		if (cpid == -1) {
			perph_errors[errors] = i;
			free(peripherals[num_perphs]);
			continue;
		}
		else if (cpid == 0) {
			// Child Process
			close(wr_pipefd[1]);	// Close unused write
			close(rd_pipefd[0]);	// Close unused read
			dup2(wr_pipefd[0], 0);	// Replace stdin
			dup2(rd_pipefd[1], 1); 	// Replace stdout
			char** argv;
			argv = (char**)malloc(2*sizeof(char*));
			argv[0] = perphs[i];
			argv[1] = (char*)malloc(2*sizeof(char));
			strcpy(argv[1], "\0");
			perph->live = true;
			execvp(perphs[i], argv);
			// Exec has returned, there is an error
			perph->live = false;
			exit(1);
		}
		// Parent Process
		perph->name = perphs[i];
		perph->wr_pipefd = wr_pipefd[1];
		perph->rd_pipefd = rd_pipefd[0];
		perph->cpid = cpid;
		close(wr_pipefd[0]);	// Close unused read
		close(rd_pipefd[1]); 	//Close unused write
		num_perphs++;
	}
	return errors;
}

int closePeripherals(void) {
	int i, exit_status;
	for (i = 0; i < num_perphs; i++) {
		waitpid(peripherals[i]->cpid, &exit_status, 0);
		free(peripherals[i]);
	}
	free(peripherals);
	free(perph_errors);
	return 0;
}
