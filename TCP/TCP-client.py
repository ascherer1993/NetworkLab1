import socket
import fileinput

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

breakFlag = 1;
while (breakFlag):
	MESSAGE = raw_input("Opcode Operand1 Operand2: ")
	if MESSAGE == "q": 
		breakFlag = 0
	else: 
		if validateInput(MESSAGE):
			s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
			s.connect((TCP_IP, TCP_PORT))
			s.send(str(MESSAGE))
			data = s.recv(BUFFER_SIZE)
			s.close()
			print "received data:", data