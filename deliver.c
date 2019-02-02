#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include "ftp.h"


void check_close(int function);

int main(int argc, char* argv[]) {

	if (argc != 3) {
		printf("Source Usage: deliver <server address> <server port number>\n");
	}

	char fname[100], proto[100];
	char input[BUFFER_SIZE];
	int sockfd,  n, status;
	char buf[BUFFER_SIZE];
	struct sockaddr_in serv_addr;
    struct addrinfo *servinfo, hints;
    socklen_t serv_addr_size;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC; // use AF_INET6 to force IPv6
	hints.ai_socktype = SOCK_STREAM;


	status = getaddrinfo(argv[1], argv[2], &hints, &servinfo);
	if (status < 0) {
		printf("Lookup failed\n");
		exit(1);
	}

	serv_addr_size = sizeof(serv_addr);

	sockfd = socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol);
    if (sockfd < 0) {
        perror("ERROR opening socket");
    	exit(1);
    }


    printf("Please input: ftp <file name>\n");
    scanf("%s %s", proto, fname);


    if (access(fname, F_OK) == -1) {
    // file doesn't exist
    	printf("file %s doesn\'t exists\n", fname);
    	exit(-1);
	} 

	// FILE *fp = fopen(fname, "rb");
	// fseek(fp, 0, SEEK_END);
	// unsigned int filesize = ftell(fp);
	// rewind(fp);


	if (sendto(sockfd, proto, strlen(proto), 0, servinfo->ai_addr, servinfo->ai_addrlen) <= 0) {
		perror("write error\n");
		exit(-1);
	}

	if (recvfrom(sockfd, buf, BUFFER_SIZE, 0, (struct sockaddr *) &serv_addr, &serv_addr_size) <= 0) {
		perror("write error\n");
		exit(-1);
	}
	if (strcmp(buf, YES) == 0) {
		printf("A file transfer can start.\n");
	} else {
		exit(-1);
	}
	freeaddrinfo(servinfo);
	check_close(sockfd);
	// fclose(fp);
	return 0;

}





