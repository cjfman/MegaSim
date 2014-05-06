#!/usr/bin/python

import sys
import time

f = open("pinOutput.txt", 'w')

LMP_START = chr(0x01)    
LMP_READY = chr(0x02)   
LMP_KILL = chr(0x03)   

LMP_PRINT   = chr(0x20) 
LMP_SETM    = chr(0x21)
LMP_SETP    = chr(0x22)
LMP_SETPN   = chr(0x23)
LMP_CLAIM   = chr(0x24)
LMP_CLAIMP  = chr(0x2B)
LMP_CLAIMPN = chr(0x2C)
LMP_WRITEM  = chr(0x25)
LMP_WRITEP  = chr(0x26)
LMP_WRITEPN = chr(0x27)
LMP_SYNC    = chr(0x28)
LMP_UNSYNC  = chr(0x29)
LMP_CLK     = chr(0x2A)
LMP_DATAM   = chr(0x40)
LMP_DATAP   = chr(0x41)
LMP_DATAPN  = chr(0x42)

LMP_BAD     = chr(0x80)
LMP_FORMAT  = chr(0x81)
LMP_RANGE   = chr(0x82)
LMP_TAKEN   = chr(0x83)
LMP_VALUE   = chr(0x84)

LMP_ACK     = chr(0xA0)
LMP_ACK1    = chr(0xA1)
LMP_ACKN    = chr(0xA2)

note = [LMP_DATAM, LMP_DATAP, LMP_DATAPN]

def printerr(p):
	sys.stderr.write(p)

def readCommand():
	return sys.stdin.read(1)

def readInt8():
	return ord(readCommand())

def readInt16():
	i = ord(readCommand()) 
	i += ord(readCommand()) << 8
	return i

def sendMessage(msg):
	sys.stdout.write(msg)
	sys.stdout.flush()

def bytesToString(l):
	return "".join([chr(x) for x in l])

def makeAndSendMessage(cmd, data):
	sendMessage(cmd + bytesToString(data))

def handleResponse(res):
	global f
	m = ""
	if   res == LMP_ACK: m = "ACK"
	elif res == LMP_BAD: m = "BAD"
	elif res == LMP_FORMAT: m = "FORMAT"
	elif res == LMP_RANGE: m = "RANGE"
	elif res == LMP_TAKEN: m = "TAKEN"
	elif res == LMP_VALUE: m = "VALUE"
	f.write(m + '\n')
	
def clearNotifications(cmd):
	if cmd == LMP_START:
		sendMessage(LMP_READY)
		readCommand()	## Clear ACK

	elif cmd == LMP_DATAM:
		addr = readInt16()
		data = readInt8()
		f.write("0x%x: %d\n"%(addr, data))

	elif cmd == LMP_DATAP:
		port = readInt8()
		data = readInt8()
		f.write("Port %d: 0x%x\n"%(port, data))

	elif cmd == LMP_DATAPN:
		pin = readInt8()
		data = readInt8()
		f.write("Pin %d: 0x%x\n"%(pin, data))

	else:
		return cmd

	return None

def cleanup(e):
	f.close()
	exit(e)

state = "PIN4"
while True:
	if state != "IDLE":
		f.write("State: " + state + '\n')

	cmd = readCommand()

	if cmd == LMP_KILL:
		cleanup(0)

	if cmd in note:
		clearNotifications(cmd)

	elif state == "PIN4":
		if cmd != LMP_START:
			cleanup(1)

		makeAndSendMessage(LMP_CLAIMPN, [4])	## Claim pin 4 on port A
		res = readCommand()						## Clear ACK
		handleResponse(res)
		state = "PIN27"

	elif state == "PIN27":
		if cmd != LMP_START:
			cleanup(1)

		makeAndSendMessage(LMP_CLAIMPN, [27])	## Claim pin 27 on port D
		res = readCommand()						## Clear ACK
		handleResponse(res)
		state = "PORTB"

	elif state == "PORTB":
		if cmd != LMP_START:
			cleanup(1)

		makeAndSendMessage(LMP_CLAIMP, [1])		## Claim port B
		res = readCommand()						## Clear ACK
		handleResponse(res)
		state = "PORTE"

	elif state == "PORTE":
		if cmd != LMP_START:
			cleanup(1)

		makeAndSendMessage(LMP_CLAIMP, [4])		## Claim port E
		res = readCommand()						## Clear ACK
		handleResponse(res)
		state = "READY"

	elif state == "READY":
		if cmd != LMP_START:
			cleanup(1)

		sendMessage(LMP_READY)
		res = readCommand()						## Clear ACK
		handleResponse(res)
		state = "WRITE4"

	elif state == "WRITE4":
		if cmd != LMP_START:
			cleanup(1)

		makeAndSendMessage(LMP_WRITEPN, [4, 0])	## Write pin 4 low
		res = readCommand()
		handleResponse(res)
		state = "WRITE27"

	elif state == "WRITE27":
		if cmd != LMP_START:
			cleanup(1)

		makeAndSendMessage(LMP_WRITEPN, [27, 1])	## Write pin 27 high
		res = readCommand()
		handleResponse(res)
		state = "WRITEB"

	elif state == "WRITEB":
		if cmd != LMP_START:
			cleanup(1)

		makeAndSendMessage(LMP_WRITEP, [1, 0xF0])	## Write port B
		res = readCommand()
		handleResponse(res)
		state = "WRITEE"

	elif state == "WRITEE":
		if cmd != LMP_START:
			cleanup(1)
		
		makeAndSendMessage(LMP_WRITEP, [4, 0x0F])	## Write port E
		state = "WAITE"
		handleResponse(res)
		state = "IDLE"
		f.write("State: " + state + '\n')

	elif state == "IDLE":
		clearNotifications(cmd)
		state = "IDLE"

cleanup(0)
