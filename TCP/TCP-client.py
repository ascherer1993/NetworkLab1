import socket
import fileinput
import struct

def validateInput(input):
	splitInput = input.split()
	isValid = 1
	if len(splitInput) < 2:
		print "Error: You must send at least two numbers."
		isValid = 0		
	else:
		if len(splitInput) > 3:
			print "Error: You may send a max of three numbers"
			isValid = 0		
		if int(splitInput[0]) > 6:
			print "Error: opcode must be between 0 and 6"
			isValid = 0
		if splitInput[1].isdigit() == False: 
			print "Error: Operand 1 must be an integer"
			isValid = 0
		if len(splitInput) == 3 and splitInput[2].isdigit() == False: 
			print "Error: Operand 2 must be an integer"
			isValid = 0
	return isValid

TCP_IP = '127.0.0.1'
TCP_PORT = 10022
BUFFER_SIZE = 1024
TML = 8

breakFlag = 1;
requestID = 0;
while (breakFlag):
	MESSAGE = raw_input("Opcode Operand1 Operand2: ")
	if MESSAGE == "q": 
		breakFlag = 0
	else: 
		if validateInput(MESSAGE):

			# tcpRequest = "8" + str(requestID)
			

			splitInput = MESSAGE.split()

			# print struct.pack('>h', int(splitInput[1]))
			# tcpRequest = tcpRequest + splitInput[0]
			# tcpRequest = tcpRequest + str(len(splitInput) - 1)
			# tcpRequest = tcpRequest +  str(struct.pack('>H', int(splitInput[1])))

			# lower case b instead of c for unsigned char (integers)
			if len(splitInput) == 3 :
				tcpRequest = struct.pack('>cccchh', "8", str(requestID), str(splitInput[0]), str(len(splitInput) - 1), int(splitInput[1]), int(splitInput[2]))
			else :
				tcpRequest = struct.pack('>cccchh', "8", str(requestID), str(splitInput[0]), str(len(splitInput) - 1), int(splitInput[1]), int(0))
			requestID = requestID + 1
			print tcpRequest

			# tcpRequest = tcpRequest + str(struct.unpack("1H", splitInput[1]))


			s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
			s.connect((TCP_IP, TCP_PORT))
			# s.send(MESSAGE)
			s.send(tcpRequest)
			data = s.recv(BUFFER_SIZE)
			s.close()
			print "received data:", data


