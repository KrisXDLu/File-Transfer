#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "ftp.h"


void check_close(int function);

int main(int argc, char* argv[]) {

	if (argc != 3) {
		printf("Source Usage: deliver <server address> <server port number>\n");
	}

	char fname[100], proto[100];
	char input[1000];
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

	FILE *fp = fopen(fname, "rb");
	fseek(fp, 0, SEEK_END);
	unsigned int filesize = ftell(fp);
	rewind(fp);
	unsigned total_frags = (filesize % 1000 == 0) ? filesize/1000 : filesize/1000 + 1;

	
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
	int num = 0;
	char pkt[MAX_LEN];
	unsigned int offset;

	while ((n = fread(input, sizeof(char), 1000, fp)) > 0) {
		offset = sprintf(pkt, "%d:%d:%d:%s:", total_frags, num, n, fname);
		memcpy(pkt + offset, input, n);		
		if (sendto(sockfd, pkt, strlen(pkt), 0, servinfo->ai_addr, servinfo->ai_addrlen) <= 0) {
			perror("write error\n");
			exit(-1);
		}
		if (recvfrom(sockfd, buf, BUFFER_SIZE, 0, (struct sockaddr *) &serv_addr, &serv_addr_size) <= 0) {
			if (errno == EAGAIN) {
				printf("Timeout: resending packet\n");
				fseek(fp, -n, SEEK_CUR);
				continue;
			} else {
				perror("Unknown error");
				exit(1);
			}
		}
		if (!strcmp(buf, NACK)) {
			fseek(fp, -n, SEEK_CUR);
			printf("NACK: resending packet\n");
		}
		if (strcmp(buf, ACK)) {
			printf("Unknow responds, terminating connection\n");
			exit(1);
		}
		num++;
	}
	freeaddrinfo(servinfo);
	check_close(sockfd);
	fclose(fp);
	return 0;

}





