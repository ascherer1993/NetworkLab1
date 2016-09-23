/*
** talker.c -- a datagram "client" demo
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <inttypes.h>
#include <time.h>

#define SERVERPORT "10022"	// the port users will be connecting to
struct ClientRequest
{
	uint8_t TotalMessageLength:8;
	uint8_t RequestID:8;
	uint8_t OpCode:8;
	uint8_t NumberOfOperands:8;
	uint16_t Operand1:16;
	uint16_t Operand2:16;
}__attribute__((__packed__));
char** str_split(char* a_str, const char a_delim)
{
	char** result    = 0;
	size_t count     = 0;
	char* tmp        = a_str;
	char* last_comma = 0;
	char delim[2];
	delim[0] = a_delim;
	delim[1] = 0;

/* Count how many elements will be extracted. */
	while (*tmp)
	{
		if (a_delim == *tmp)
		{
			count++;
			last_comma = tmp;
		}
		tmp++;
	}

/* Add space for trailing token. */
	count += last_comma < (a_str + strlen(a_str) - 1);

/* Add space for terminating null string so caller
knows where the list of returned strings ends. */
	count++;

	result = malloc(sizeof(char*) * count);

	if (result)
	{
		size_t idx  = 0;
		char* token = strtok(a_str, delim);

		while (token)
		{
			*(result + idx++) = strdup(token);
			token = strtok(0, delim);
		}
		*(result + idx) = 0;
	}

	return result;
}
int main(int argc, char *argv[])
{
	int sockfd;
	struct addrinfo hints, *servinfo, *p;
	int rv;
	int numbytes;

	if (argc != 3) {
		fprintf(stderr,"usage: talker hostname message\n");
		exit(1);
	}

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_DGRAM;

	if ((rv = getaddrinfo(argv[1], argv[2], &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}

// loop through all the results and make a socket
	for(p = servinfo; p != NULL; p = p->ai_next) {
		if ((sockfd = socket(p->ai_family, p->ai_socktype,
			p->ai_protocol)) == -1) {
			perror("talker: socket");
			continue;
		}
		break;
	}

	if (p == NULL) {
		fprintf(stderr, "talker: failed to create socket\n");
		return 2;
	}
	time_t t;
	srand((unsigned) time(&t)); //initialize random number generator
	uint8_t requestID = rand() % 255;
	//read input
	int keepGoing = 1;
	while (1) {
		keepGoing = 0;
		uint8_t opcode;
		uint16_t operand1;
		uint16_t operand2;
		printf("Opcode: ");
		scanf("%hu", &opcode);

		printf("Operand 1: ");
		scanf("%hu", &operand1);

		printf("Operand 2: ");
		scanf("%hu", &operand2);

		if (requestID < 255) {
			requestID = requestID + 1;
		}
		else {
			srand((unsigned) time(&t)); //reseed random number generator
			requestID = rand() % 255;
		}
		struct ClientRequest request = {
			sizeof(request),
			requestID,
			opcode,
			2,
			htons(operand1),
			htons(operand2)
		};
		printf("\trequest %hu sent to : %s:%s\n", requestID, argv[1], argv[2]);
		clock_t start = clock(), diff;
		if ((numbytes = sendto(sockfd, &request, sizeof(request), 0, p->ai_addr, p->ai_addrlen)) == -1) {
			perror("talker: sendto");
			exit(1);
		}
		char response[7];
		bzero(response, 7);
		int n = read(sockfd, response, 7);
		if (n < 0) {
			perror("read");
		}
		else {
			uint8_t responseID;
			uint8_t errorcode;
			uint32_t result;
			memcpy(&responseID, response+1, 1);
			memcpy(&errorcode, response+2, 1);
			memcpy(&result, response+3, 4);
			result = ntohl(result);
			diff = clock() - start;
			int msec = diff * 1000 / CLOCKS_PER_SEC;
			printf("\tresult for request %hu: %u\n", responseID, result);
			printf("\tTime taken: %ds %dms\n", msec/100, msec%100);
			printf("\tError code: %hu\n", errorcode);
		}
	}
	freeaddrinfo(servinfo);

	printf("talker: sent %d bytes to %s\n", numbytes, argv[1]);
	close(sockfd);
	return 0;
}
