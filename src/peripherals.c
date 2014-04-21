// peripherals.c

#ifndef NO_PERPHS

#ifndef DEBUG_PERPH
#ifdef DEBUG
#define DEBUG_PERPH
#endif // DEBUG
#endif // DEBUG_PERPH

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <sys/select.h>
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
#define LMP_CLAIMP	0x2B
#define LMP_CLAIMPN	0x2C
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
#define LMP_TAKEN	0x83
#define LMP_VALUE	0x84
// Acknoledgments
#define LMP_ACK		0xA0
#define LMP_ACK1	0xA1
#define LMP_ACKN	0xA2

// Global Variable Instantiation
bool perph_flag = false;
bool perph_write_flag = false;

// Local Variables and Functions

// Peripheral Management
void startPeripheral(Peripheral *p);
void startPeripherals(void);

// Reading Write Commands
// These variables are needed by select(2)
fd_set rfds;				// List of file descriptors to check for commands
struct timeval wait_time;	// The maximum wait time to check for commands
int max_fd;					// The maximum value of file descriptor
//void scanPeripherals(void);	// Checks each peripherals individually
//void findBrokenPipe(void);	// Looks for a broken pipe and removes it
void readCommand(Peripheral *perph);	// Checks a peripheral's pipe for data
										// Returns false if the pipe is broken
										// true otherwise

// Message Passing
char message[MAX_MSG];								// A read buffer
void sendChar(Peripheral *p, char c);				// Send char to peripheral
void sendMessage(Peripheral *p, char* msg, int len);
char readMessage(Peripheral *p);
char readChar(Peripheral *p);

// Command Handlers
// Returns true if the call succeeds
// Returns false otherwise
void handleCommand(Peripheral *p, char cmd);
bool printHandler(Peripheral *p);
bool setmHandler(Peripheral *perph);
bool setpHandler(Peripheral *perph);
bool setpnHandler(Peripheral *perph);
bool claimHandler(Peripheral *perph);
bool claimpHandler(Peripheral *perph);
bool claimpnHandler(Peripheral *perph);
bool writemHandler(Peripheral *perph);
bool writepHandler(Peripheral *perph);
bool writepnHandler(Peripheral *perph);

/////////////////////////////////
// Open / Close Peripherals
////////////////////////////////
int openPeripherals(char** perphs, int p_count) {
	signal(SIGPIPE, SIG_IGN);	// Catch SIGPIPE signal
	FD_ZERO(&rfds);				// Clear file descriptor list
	
	// Memory Allocation
	peripherals = (Peripheral**)malloc(p_count*sizeof(Peripheral*));
	perph_errors = (bool*)malloc(p_count*sizeof(bool));

	// Peripheral Initialization
	int i;
	int errors = 0;
	for (i = 0; i < p_count; i++) {
		perph_errors[i] = true;	// Assume an error will occur
		int rd_pipefd[2];		// Create read and write pipes
		int wr_pipefd[2];
		if (pipe(rd_pipefd) == -1 || pipe(wr_pipefd) == -1) {
			// One of the pipes failed
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
		peripherals[num_perphs] = perph;	// Add to peripheral array
		perph->live = true; 				// Assume launch is successful
		// Fork the program
		int cpid = fork();
		if (cpid == -1) {
			// Fork failed
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
		perph->can_write = false;
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

void startPeripheral(Peripheral *perph) {
	fprintf(stderr, "%s...", perph->name);
	char cmd = '\0';			// The command received
	while (cmd != LMP_READY && perph->live) {
		sendChar(perph, LMP_START);	// Attempt to start peripheral
		cmd = readChar(perph);	
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


// Notifications /////////////////////////////////////////////////////

void memoryNotification(Peripheral* perph, uint16_t addr, uint8_t data) {
	char c[4];
	c[0] = LMP_DATAM;
	c[1] = addr & 0x00FF;
	c[2] = addr >> 8;
	c[3] = data;
	sendMessage(perph, c, 4);
}

void portNotification(Peripheral* perph, uint16_t port, uint8_t data) {
	char c[3];
	c[0] = LMP_DATAP;
	c[1] = port & 0xFF;
	c[2] = data;
	sendMessage(perph, c, 3);
}

void pinNotification(Peripheral* perph, uint16_t pin, uint8_t data) {
	char c[3];
	c[0] = LMP_DATAPN;
	c[1] = pin & 0xFF;
	c[2] = data;
	sendMessage(perph, c, 3);
}

// Peripheral Write Checking /////////////////////////////////////////

/*
void checkPeripherals(void) {
	wait_time.tv_sec = 0;	// Do not wait
    wait_time.tv_usec = 0;	// non-blocking call
	// Check all peripheral pipes at once for write commands
    int retval = select(max_fd + 1, &rfds, NULL, NULL, &wait_time);
	if (retval == 0) {
		// There is nothing to read
		return;
	}
    else if (retval == -1) {
		// One of the pipes is broken
		// Find out which one
		findBrokenPipe();
	}
	else {
		// At least one of the pipes has data
		// Check each one
		scanPeripherals();
	}
}
// */

void checkPeripherals(void) {
	// Check each peripheral for commands
	int i;
	for (i = 0; i < num_perphs; i++) {
		Peripheral *perph = peripherals[i];
		if (!perph->live || !perph->can_write) continue;	// Don't check
		readCommand(perph);
	}
}

/*
void findBrokenPipe(void) {
	FD_ZERO(&rfds);		// Clear the list of pipes to check
	max_fd = 0;			// reset max fd
	int i;
	for (i = 0; i < num_perphs; i++) {
		Peripheral *perph = peripherals[i];		// Check each peripheral
		if (!perph->live || !perph->can_write) 
			continue;							// Don't check
		bool res = readCommand(perph);			// by checking for read data
		if (!res) continue;						// Read failed
		FD_SET(perph->rdfd, &rfds);				// Read succeeded
		max_fd = fmax(max_fd, perph->rdfd);		// recalculate max fd
	}
}
// */

void readCommand(Peripheral *perph) {
	char cmd = '\0';			// The command received
	while (cmd != LMP_READY && perph->live) {
		sendChar(perph, LMP_START);	// Attempt to start peripheral
		cmd = readChar(perph);	
		if (cmd == '\0') {
			perph->live = false;
			break;
		}
		handleCommand(perph, cmd);
	}
	/*
	wait_time.tv_sec = 0;			// Do not wait
	wait_time.tv_usec = 0;			// Non-blocking
	fd_set test_set;				// Create fd set with one entry
	FD_ZERO(&test_set);
	FD_SET(perph->rdfd, &test_set);
	int ret = select(perph->rdfd + 1, &test_set, NULL, NULL, &wait_time);
	if (ret == -1) {
		// Read failed
		fprintf(stderr, "Peripheral %s has broken pipe. ", perph->name);
		fprintf(stderr, "Removing\n");
		perph->live = false;
		return false;
	}
	else if (ret > 0) {
		// There is data to read
		handleCommand(perph, readChar(perph));
	}
	return true;
	// */
}

// Message Passing //////////////////////////////////////////////////////

char readChar(Peripheral *p) {
	char c = '\0';
	read(p->rdfd, &c, 1);
	return c;
}

char readChars(Peripheral *p, int msize) {
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
	int count = write(p->wrfd, msg, len);		// Perform write
	if (count <= 0) {
		// Error received
		p->live = false;
		switch(errno) {
		case EPIPE:
			fprintf(stderr, "Broken Pipe."); 
			break;
		default:
			fprintf(stderr, "Received error %d", count);
			break;
		}
		fprintf(stderr, " Removing %s\n", p->name);
	}
}

void handleCommand(Peripheral *p, char c) {
#ifdef DEBUG_PERPH
	//if (c != LMP_READY) {
		fprintf(stderr, "%s sent command 0x%X\n", p->name, c);
//	}
#endif // DEBUG_PERPH
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
		setpHandler(p);
		break;
	case LMP_SETPN:
		setpnHandler(p);
		break;
	case LMP_WRITEM:
		writemHandler(p);
		break;
	case LMP_WRITEP:
		writepHandler(p);
		break;
	case LMP_WRITEPN:
		writepnHandler(p);
		break;
	case LMP_CLAIM:
		claimHandler(p);
		break;
	case LMP_CLAIMP:
		claimpHandler(p);
		break;
	case LMP_CLAIMPN:
		claimpnHandler(p);
		break;
	case LMP_SYNC:
	case LMP_UNSYNC:
	case LMP_CLK:
	case LMP_DATAM:
	case LMP_DATAP:
	case LMP_DATAPN:
	default:			// Bad Command
		sendChar(p, LMP_BAD);
#ifdef DEBUG_PERPH
		fprintf(stderr, "%s sent bad command 0x%X\n", p->name, c);
#endif // DEBUG_PERPH
		break;
	}
}

bool printHandler(Peripheral *p) {
	int len = readMessage(p);		// Read print message
	if (len == 0) {					// There was no message
		sendChar(p, LMP_FORMAT);
		return false;
	}
	sendChar(p, LMP_ACK);
	fprintf(stderr, "%s says: %s\n", p->name, message);
	return true;
}

bool setmHandler(Peripheral *perph) {
	int count = readChars(perph, 2);		// Get address
	if (count != 2) { 						// Invalid address
		sendChar(perph, LMP_FORMAT);
		return false;				
	}
	uint16_t addr = *((uint16_t*)message); 	// Format address
	if (mem_listeners[addr] != NULL) {
		sendChar(perph, LMP_TAKEN);			// The address was taken
		return false;
	}
	mem_listeners[addr] = perph;		 	// Set Listener
	sendChar(perph, LMP_ACK);
	return true;
}

bool setpHandler(Peripheral *perph) {
	int count = readChars(perph, 1);		// Get port number
	if (count != 1) {
		sendChar(perph, LMP_FORMAT);		// No data
		return false;
	}
	uint8_t port_id = (uint8_t) message[0];	// Format port
	Port *port;
	if (port_id >= num_ports || (port = ports[port_id]) == NULL) {
		// Port does not exist
		sendChar(perph, LMP_RANGE);
		return false;
	}
	if (port->listener != NULL || port->pin_listener) {
		// Port is taken
		sendChar(perph, LMP_TAKEN);
		return false;
	}
	port->listener = perph;					// Set listener
	sendChar(perph, LMP_ACK);
	return true;
}

bool setpnHandler(Peripheral *perph) {	
	int count = readChars(perph, 1);		// Get pin number
	if (count != 1) {
		sendChar(perph, LMP_FORMAT);		// No data
		return false;
	}
	uint8_t pin_id = (uint8_t) message[0];	// Format number
	Pin *pin;
	if (pin_id > coredef->num_pins || (pin = pins[pin_id - 1]) == NULL) {
		// Pin does not exist
		sendChar(perph, LMP_RANGE);
		return false;
	}
	Port *port = pin->port;
	if (pin->listener != NULL || port->listener != NULL) {
		// Pin or port is taken
		sendChar(perph, LMP_TAKEN);
		return false;
	}
//*
#ifdef DEBUG_PERPH
	fprintf(stderr, "%s set listener for pin %d\n", perph->name, pin_id);
#endif
// */
	pin->listener = perph;
	port->pin_listener = true;
	sendChar(perph, LMP_ACK);
	return true;
}

bool claimHandler(Peripheral *perph) {
	// Claiming an address is the same as setting a listener
	// with the addition of setting a flag
	if (!setmHandler(perph)) {
		return false;			// Set listener failed
	}
	perph->can_write = true;
	perph_write_flag = true;
	FD_SET(perph->rdfd, &rfds);				// Add to check set
	max_fd = fmax(max_fd, perph->rdfd);		// recalculate max fd
	return true;
}

bool claimpHandler(Peripheral *perph) {
	if (!setpHandler(perph)) {
		return false;
	}
	perph->can_write = true;
	perph_write_flag = true;
	FD_SET(perph->rdfd, &rfds);				// Add to check set
	max_fd = fmax(max_fd, perph->rdfd);		// recalculate max fd
	return true;
}

bool claimpnHandler(Peripheral *perph) {
	if (!setpnHandler(perph)) {
		return false;
	}
	perph->can_write = true;
	perph_write_flag = true;
	FD_SET(perph->rdfd, &rfds);				// Add to check set
	max_fd = fmax(max_fd, perph->rdfd);		// recalculate max fd
	return true;
}

bool writemHandler(Peripheral *perph) {
	int count = readChars(perph, 3);
	if (count != 3) { 						// Invalid message
		sendChar(perph, LMP_FORMAT);		// Need a 2 byte address
		return false;						// and one data byte
	}
	uint16_t addr = *((uint16_t*)message);	// Format address
	if (mem_listeners[addr] != perph) {
		sendChar(perph, LMP_TAKEN);			// The address was taken
		return false;
	}
	main_mem[addr] = message[2];			// Save data
	sendChar(perph, LMP_ACK);
	return true;
}

bool writepHandler(Peripheral *perph) {
	int count = readChars(perph, 2);	
	if (count != 2) {
		sendChar(perph, LMP_FORMAT);		// Invalid
		return false;						// Need port number and data byte
	}
	uint8_t port_id = (uint8_t) message[0];	// Get port number
	Port *port;
	if (port_id >= num_ports || (port = ports[port_id]) == NULL) {
		sendChar(perph, LMP_RANGE);			// Port does not exist
		return false;
	}
	if (port->listener != perph) {
		// Port is taken
		sendChar(perph, LMP_TAKEN);
		return false;
	}
	int i;
	for (i = 0; i < 8; i++) {
		// Loop through each pin
		writePin(port->pin_map[i] + 1, (message[1] >> i) & 0x01);
	}
	sendChar(perph, LMP_ACK);
	portNotification(perph, port_id, *port->pin);
	return true;
}

bool writepnHandler(Peripheral *perph) {	
	int count = readChars(perph, 2);	
	if (count != 2) {
		sendChar(perph, LMP_FORMAT);	// Invalid
		return false;					// Requires pin number and value
	}
	int pin_id = message[0];
	Pin *pin;
	if (pin_id > coredef->num_pins || (pin = pins[pin_id - 1]) == NULL) {
		sendChar(perph, LMP_RANGE);		// Pin does not exist
		return false;
	}
	Port *port = pin->port;
	if (pin->listener != perph && port->listener != perph) {
		// Pin or port is taken
		sendChar(perph, LMP_TAKEN);
		return false;
	}
	PinState state = message[1];
	if (state > 3) {
		sendChar(perph, LMP_VALUE);		// Invalid pin value
		return false;
	}
	if (!writePin(pin_id, state)) {
		sendChar(perph, LMP_RANGE);		// Pin does not exist
		return false;
	}
	sendChar(perph, LMP_ACK);
	return true;
}

#endif  // NO_PERPHS
