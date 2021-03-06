#!/usr/bin/python

import sys
import time

LMP_START = chr(0x01)    
LMP_READY = chr(0x02)   
LMP_KILL = chr(0x03)   
LMP_PRINT = chr(0x20) 
LMP_SETM = chr(0x21)
LMP_SETP = chr(0x22)
LMP_SETPN = chr(0x23)
LMP_CLAIM = chr(0x24)
LMP_WRITEM = chr(0x25)
LMP_WRITEP = chr(0x26)
LMP_WRITEPN = chr(0x27)
LMP_SYNC = chr(0x28)
LMP_UNSYNC = chr(0x29)
LMP_CLK = chr(0x2A)
LMP_DATAM = chr(0x40)
LMP_DATAP = chr(0x41)
LMP_DATAPN = chr(0x42)
LMP_BAD = chr(0x80)
LMP_FORMAT = chr(0x81)
LMP_RANGE = chr(0x82)
LMP_ACK = chr(0xA0)
LMP_ACK1 = chr(0xA1)
LMP_ACKN = chr(0xA2)

def readCommand():
	return sys.stdin.read(1)

def sendMessage(msg):
	sys.stdout.write(msg)
	sys.stdout.flush()

state = "START"
while True:
	cmd = readCommand()

	if cmd == LMP_KILL:
		break

	if state == "START":
		if cmd != LMP_START:
			exit(1)

		#sys.stderr.write("Received Start\n")
		state = "READY"
		msg = "Hello World Peripheral!"
		sendMessage(LMP_PRINT + chr(len(msg)) + msg)

	if state == "READY":
		sendMessage(LMP_READY)
	
	readCommand()	## Clear ACK

exit(0)
