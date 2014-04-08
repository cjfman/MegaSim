// peripherals.h

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <sys/mman.h>
//#include <sys/types.h>
//#include <sys/wait.h>
#include "peripherals.h"
#include "core.h"

// OS X uses MAP_ANON
// Linux has depricated MAP_ANON and prefers MAP_ANONYMOUS
#ifdef MAP_ANONYMOUS
#define MAP_ANON MAP_ANONYMOUS
#endif

// Listener Message Protocol
#define LMP_START 	0x01	// OUT
#define LMP_READY 	0x02	// IN
#define LMP_KILL	0x03	// OUT
// Requests
#define LMP_PRINT	0x20	// IN
#define LMP_SETM	0x21
#define LMP_SETP	0x22
#define LMP_SETPN	0x23
#define LMP_CLAIM	0x24
#define LMP_WRITEM	0x25
#define LMP_WRITEP	0x26
#define LMP_WRITEPN	0x27
#define LMP_SYNC	0x28
#define LMP_UNSYNC	0x29
#define LMP_CLK		0x2A
// Data
#define LMP_DATAM	0x40
#define LMP_DATAP	0x41
#define LMP_DATAPN	0x42
// Errors
#define LMP_BAD		0x80
#define LMP_FORMAT	0x81
#define LMP_RANGE	0x82
// Acknoledgments
#define LMP_ACK		0xA0
#define LMP_ACK1	0xA1
#define LMP_ACKN	0xA2

bool perph_flag = false;
void startPeripheral(Peripheral *p);
// Message Passing
char message[MAX_MSG];
void sendChar(Peripheral *p, char c);
void sendMessage(Peripheral *p, char* msg, int len);
char readMessage(Peripheral *p);

// Command Handlers
void handleCommand(Peripheral *p, char cmd);
void printHandler(Peripheral *p);
void setmHandler(Peripheral *perph);

int openPeripherals(char** perphs, int p_count) {
	signal(SIGPIPE, SIG_IGN);
	int i;
	int errors = 0;
	peripherals = (Peripheral**)malloc(p_count*sizeof(Peripheral*));
	perph_errors = (bool*)malloc(p_count*sizeof(bool));
	for (i = 0; i < p_count; i++) {
		perph_errors[i] = true;
		int rd_pipefd[2];
		int wr_pipefd[2];
		if (pipe(rd_pipefd) == -1 || pipe(wr_pipefd) == -1) {
			errors++;
			continue;
		}
		// Allocate space for the peripheral
		// Use shared memory space so that the peripheral
		// can report that it has started successfully
		Peripheral *perph = (Peripheral*)mmap(NULL, sizeof(Peripheral),
			PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANON, -1, 0);
		if (perph == MAP_FAILED) {
			errors++;
			continue;
		}
		peripherals[num_perphs] = perph;
		perph->live = true; 	// Assume launch is successful
		// Start fork
		int cpid = fork();
		if (cpid == -1) {
			free(peripherals[num_perphs]);
			errors++;
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
			execvp(perphs[i], argv);
			// Exec has returned, there is an error
			perph->live = false;
			exit(1);
		}
		// Parent Process
		close(wr_pipefd[0]);	// Close unused read
		close(rd_pipefd[1]); 	// Close unused write
		perph->name = perphs[i];
		perph->wrfd = wr_pipefd[1];
		perph->rdfd = rd_pipefd[0];
		perph->cpid = cpid;
		num_perphs++;
		perph_errors[i] = false;
		startPeripheral(perph);
	}
	perph_flag = (num_perphs > 0);
	return errors;
}

int closePeripherals(void) {
	int i, exit_status;
	for (i = 0; i < num_perphs; i++) {
		Peripheral *perph = peripherals[i];
		if (perph->live) {
			sendChar(perph, LMP_KILL);
			waitpid(perph->cpid, &exit_status, 0);
		}
		else {
			kill(perph->cpid, SIGKILL);
		}
		munmap(perph, sizeof(Peripheral));
	}
	free(peripherals);
	free(perph_errors);
	return 0;
}

void forceClosePeripherals(void) {
	int i;
	for (i = 0; i < num_perphs; i++) {
		kill(peripherals[i]->cpid, SIGKILL);
		munmap(peripherals[i], sizeof(Peripheral));
	}
	free(peripherals);
	free(perph_errors);
}

void memoryNotification(Peripheral* perph, uint16_t addr, uint8_t data) {
	char c[4];
	c[0] = LMP_DATAM;
	c[1] = addr & 0x00FF;
	c[2] = addr >> 8;
	c[3] = data;
	sendMessage(perph, c, 4);
}

char readCommand(Peripheral *p) {
	char c = '\0';
	read(p->rdfd, &c, 1);
	return c;
}

char readChar(Peripheral *p) {
	char c = '\0';
	read(p->rdfd, &c, 1);
	return c;
}

char readBytes(Peripheral *p, int msize) {
	int count = read(p->rdfd, message, msize);	// Read message
	if (count < msize) return 0;				// Only partial message
	return count;
}

char readMessage(Peripheral *p) {
	char msize;
	int count = read(p->rdfd, &msize, 1);	// Read message size
	if (count != 1) return 0;				// Message size not specified
	count = read(p->rdfd, message, msize);	// Read message
	if (count < msize) return 0;			// Pipe didn't contain full message
	message[count] = '\0';					// Null terminate
	return count;
}

void sendChar(Peripheral *p, char c) {
	sendMessage(p, &c, 1);
}

void sendMessage(Peripheral *p, char* msg, int len) {
	int count = write(p->wrfd, msg, len);
	if (count <= 0) {
		p->live = false;
		switch(count) {
		case EPIPE:
			fprintf(stderr, "Received EPIPE."); 
			break;
		default:
			fprintf(stderr, "Received error %d", count);
			break;
		}
		fprintf(stderr, " Removing %s\n", p->name);
	}
}

void startPeripheral(Peripheral *perph) {
	fprintf(stderr, "%s...", perph->name);
	char cmd = '\0';			// The command received
	while (cmd != LMP_READY && perph->live) {
		sendChar(perph, LMP_START);	// Attempt to start peripheral
		/*
		int atmpt;					// The number of attempts to 
									// talk to perph
		for (atmpt = 0; cmd == '\0' && atmpt < 5; atmpt++) {
			sleep(1);		// Give peripheral 1 second to respond
		}	
		// */
		/*
		char c[100];
		read(perph->rdfd, c, 100);
		fprintf(stderr, "TEST: %s\n", c);
		// */
		cmd = readCommand(perph);	
		if (cmd == '\0') {
			perph->live = false;
			break;
		}
		handleCommand(perph, cmd);
	}
	if (perph->live) {
		fprintf(stderr, "launched\n");
	}
	else {
		fprintf(stderr, "failed\n");
	}
}

void startPeripherals(void) {
	int i;
	// Start each peripheral
	for (i = 0; i < num_perphs; i++) {
		Peripheral *perph = peripherals[i];
		startPeripheral(perph);
	}
}

void handleCommand(Peripheral *p, char c) {
	switch (c) {
	case LMP_READY:
		sendChar(p, LMP_ACK);
		break;
	case LMP_PRINT:
		printHandler(p);
		break;
	case LMP_SETM:
		setmHandler(p);
		break;
	case LMP_SETP:
	case LMP_SETPN:
	case LMP_CLAIM:
	case LMP_WRITEM:
	case LMP_WRITEP:
	case LMP_WRITEPN:
	case LMP_SYNC:
	case LMP_UNSYNC:
	case LMP_CLK:
	case LMP_DATAM:
	case LMP_DATAP:
	case LMP_DATAPN:
	default:			// Bad Command
		sendChar(p, LMP_BAD);
		break;
	}
}

void printHandler(Peripheral *p) {
	int len = readMessage(p);
	if (len == 0) {
		sendChar(p, LMP_FORMAT);
		return;
	}
	sendChar(p, LMP_ACK);
	fprintf(stderr, "%s says: %s\n", p->name, message);
}

void setmHandler(Peripheral *perph) {
	int count = readBytes(perph, 2);
	if (count != 2) return;				// Invalid message
	uint16_t addr = *((uint16_t*)message);
	mem_listeners[addr] = perph;
	sendChar(perph, LMP_ACK);
}
