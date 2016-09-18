import socket
import fileinput

def validateInput(input):
	splitInput = input.split(' ')
	isValid = 1
	if int(input[0]) > 6:
		print "opcode must be between 0 and 6"
		isValid = 0
	return isValid

TCP_IP = '127.0.0.1'
TCP_PORT = 10010
BUFFER_SIZE = 1024

breakFlag = 1;
while (breakFlag):
	MESSAGE = raw_input("Opcode Operand1 Operand2: ")
	if validateInput(MESSAGE):
		s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
		s.connect((TCP_IP, TCP_PORT))
		s.send(str(MESSAGE))
		data = s.recv(BUFFER_SIZE)
		s.close()
		print "received data:", data