/*
** server.c -- a stream socket server demo
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>

#define PORT "10022"  // the port users will be connecting to

#define BACKLOG 10	 // how many pending connections queue will hold


struct ServerResponse
	{
		unsigned char  totalMessageLength;
		unsigned char  requestID;
		unsigned char errorCode;
		unsigned long  result;
	} __attribute__((__packed__));


void sigchld_handler(int s)
{
	// waitpid() might overwrite errno, so we save and restore it:
	int saved_errno = errno;

	while(waitpid(-1, NULL, WNOHANG) > 0);

	errno = saved_errno;
}


// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}

	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int main(void)
{
	int sockfd, new_fd;  // listen on sock_fd, new connection on new_fd
	struct addrinfo hints, *servinfo, *p;
	struct sockaddr_storage their_addr; // connector's address information
	socklen_t sin_size;
	struct sigaction sa;
	int yes=1;
	char s[INET6_ADDRSTRLEN];
	char buf[1024];
	int rv;

	

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE; // use my IP

	if ((rv = getaddrinfo(NULL, PORT, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}

	// loop through all the results and bind to the first we can
	for(p = servinfo; p != NULL; p = p->ai_next) {
		if ((sockfd = socket(p->ai_family, p->ai_socktype,
				p->ai_protocol)) == -1) {
			perror("server: socket");
			continue;
		}

		if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes,
				sizeof(int)) == -1) {
			perror("setsockopt");
			exit(1);
		}

		if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
			close(sockfd);
			perror("server: bind");
			continue;
		}

		break;
	}

	freeaddrinfo(servinfo); // all done with this structure

	if (p == NULL)  {
		fprintf(stderr, "server: failed to bind\n");
		exit(1);
	}

	if (listen(sockfd, BACKLOG) == -1) {
		perror("listen");
		exit(1);
	}

	sa.sa_handler = sigchld_handler; // reap all dead processes
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_RESTART;
	if (sigaction(SIGCHLD, &sa, NULL) == -1) {
		perror("sigaction");
		exit(1);
	}

	printf("server: waiting for connections...\n");

	while(1) {  // main accept() loop
		sin_size = sizeof their_addr;
		new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
		if (new_fd == -1) {
			perror("accept");
			continue;
		}

		inet_ntop(their_addr.ss_family,
			get_in_addr((struct sockaddr *)&their_addr),
			s, sizeof s);
		
		//printf("\nserver: got connection from %s", s);

		if (!fork()) { // this is the child process
			bzero(buf, 1024);

			//Struct for return message
			struct ServerResponse response;
			response.errorCode = 0;


			long returnValue = 0;

    		int n = read(new_fd, buf, 1024);
    		if (n < 0) {
		      perror("read");
		    }
		    printf("\nserver: got connection from %s", s);
    		printf(", received %d bytes: %s\n", n, buf);

    		//This is what stores the request in variables

    		char requestSize = buf[0];
    		char requestID = buf[1];
    		char opCode = buf[2];
    		char numOfOperands = buf[3];
			short operand1 = buf[4] << 8 | buf[5];
    		short operand2 = buf[6] << 8 | buf[7];
    		
    		response.totalMessageLength = 7;
    		response.requestID = buf[1];


    		//This switch does the actual calculations and stores it in returnValue
    		switch (opCode)
    		{
    			case 0:
    				returnValue = operand1 + operand2;
    				break;
    			case 1:
    				returnValue = operand1 - operand2;
    				break;
    			case 2:
    				returnValue = operand1 | operand2;
    				break;
    			case 3:
    				returnValue = operand1 & operand2;
    				break;
    			case 4:
    				returnValue = operand1 >> operand2;
    				break;
    			case 5:
    				returnValue = operand1 << operand2;
    				break;
    			default:
    				//If an incorrect op code is received, set the errorCode to 127
    				response.errorCode = 127;

    		}
    		
    		//This puts the result in big endian. It shifs to the correct position and then ands it to get rid of all the values we don't want
    		response.result = (returnValue << 24) | ((returnValue << 8) & 0x00ff0000) | ((returnValue >> 8) & 0x0000ff00) | (returnValue >> 24);

    		// Outputs Results
    		printf("Info Received:\n");
    		printf("Size of Request: %d\n", requestSize);
    		printf("Request ID: %d\n", requestID);
    		printf("Op Code: %d\n", opCode);
    		printf("Number of Operands: %d\n", numOfOperands);
    		printf("Operand 1: %d\n", operand1);
			printf("Operand 2: %d\n", operand2);
			printf("Operation Result: %d\n\n", returnValue);


			close(sockfd); // child doesn't need the listener

			//Returns struct to the client
			if (send(new_fd, (const void *) &response, sizeof(&response), 0) == -1) {
				perror("send");
			}
			close(new_fd);
			exit(0);
		}
		close(new_fd);  // parent doesn't need this
	}

	return 0;
}

