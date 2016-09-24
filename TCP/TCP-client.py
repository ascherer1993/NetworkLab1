import socket
import fileinput
import struct
import time
import random

def validateInput(input):
	splitInput = input.split()
	isValid = 1
	if len(splitInput) != 3:
		print "Error: You must send 3 numbers in the format \'# # #\'.\n"
		isValid = 0
	else :
		if splitInput[0].isdigit() == False: 
			print "Error: The opcode must be between 0 and 6"
			isValid = 0		
		elif int(splitInput[0]) > 6:
			print "Error: The opcode must be between 0 and 6"
			isValid = 0
		if splitInput[1].isdigit() == False: 
			print "Error: Operand 1 must be an integer"
			isValid = 0
		if splitInput[2].isdigit() == False: 
			print "Error: Operand 2 must be an integer"
			isValid = 0
		if not isValid:
			print "\n"
	return isValid

TCP_IP = '127.0.0.1'
TCP_PORT = 10022
BUFFER_SIZE = 1024
TML = 8

breakFlag = 1;
requestID = random.randint(0,60);
while (breakFlag):
	# Displays info about how to enter operation
	print 'Format: OP # #'
	print 'OP codes:'
	print '0 : +'
	print '1 : -'
	print '2 : |'
	print '3 : &'
	print '4 : >>'
	print '5 : <<'
	# Reads in input
	MESSAGE = raw_input("Opcode Operand1 Operand2: ")
	if MESSAGE == "q": 
		breakFlag = 0
	else: 
		if validateInput(MESSAGE):

			# tcpRequest = "8" + str(requestID)
			
			# splits input into an array
			splitInput = MESSAGE.split()

			# Packs into bytes in big endian to be sent to the client
			if len(splitInput) == 3 :
				tcpRequest = struct.pack('>bbbbhh', TML, int(requestID), int(splitInput[0]), int(len(splitInput) - 1), int(splitInput[1]), int(splitInput[2]))
			
			# increments request id to distinguish requests
			requestID = requestID + 1

			# resets request id to 0 if it gets to overflow range
			if requestID > 127 :
				requestID = 0

			s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
			s.connect((TCP_IP, TCP_PORT))
			# s.send(MESSAGE)


			start = time.time() # Messures time of sending and receiving data
			s.send(tcpRequest)
			data = s.recv(BUFFER_SIZE)
			end = time.time() # End time

			# calculate time sending and receiving response
			timeElapsed = end - start

			s.close()

			unpackedStruct = struct.unpack_from('>bbbl', data)

			print "Message length: " + str(unpackedStruct[0])
			print "Request ID: " + str(unpackedStruct[1])
			print "Error Code: " + str(unpackedStruct[2])
			print "Result: " + str(unpackedStruct[3])
			print "Time Elapsed: " + str(round(timeElapsed * 1000, 2)) + " milliseconds.\n\n"

			print "-------------------------------------------------------------\n"
			
			# raw_input("Press enter to continue...\n")
