#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include "ftp.h"



#define INPUT_ARG_MAX_NUM 12
#define DELIM " \n"
#define MAX_BACKLOG 5
#define MAX_USERNAME 32
#define MAX_CLIENTS 10

	




int main(int argc, char* argv[]) {
	if (argc != 2) {
		printf("Usage: server <UDP listen port>\n");
		exit(1);
	}

	int listenfd, port, n;
	port = atoi(argv[1]);
	char buf[BUFFER_SIZE];
	struct sockaddr_in self, client_addr;
	memset(&self, 0, sizeof(self));
	memset(&client_addr, 0, sizeof(client_addr));


	bzero(buf, BUFFER_SIZE);
	socklen_t client_len = sizeof(client_addr);
	// int on = 1, status;

	// socket
	if ((listenfd = socket(PF_INET, SOCK_DGRAM, 0)) == -1) {
	    perror("socket");
	    exit(1);
	}

	// // we can resuse the port immediately after the server terminates
	// status = setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR,
	//     (const char *) &on, sizeof(on));
	// if (status == -1) {
	//     perror("setsockopt -- REUSEADDR");
	// }

	self.sin_family = AF_INET;
	self.sin_addr.s_addr = INADDR_ANY;
	self.sin_port = htons(port);
	memset(&self.sin_zero, 0, sizeof(self.sin_zero)); // init sin_zero

	printf("Listening on %d\n", port);

	if (bind(listenfd, (struct sockaddr *)&self, sizeof(self)) == -1) {
	    perror("bind"); // probably means port is in use
	    exit(1);
	}



	while (1) {
		if ((n = recvfrom(listenfd, buf, BUFFER_SIZE, 0, (struct sockaddr*)&client_addr, &client_len)) == -1) {
			perror("Not receiving from client\n");
			exit(-1);
		}
		buf[n] = '\0';
		printf("%s\n", buf);
		if (strcmp(buf, "ftp") == 0) {
			if (sendto(listenfd, YES, strlen(YES), 0, (struct sockaddr*)&client_addr, sizeof(client_addr)) == -1) {
				perror("Send not succsseful\n");
				exit(-1);
			}
		} else {
			if (sendto(listenfd, NO, strlen(NO), 0, (struct sockaddr*)&client_addr, sizeof(client_addr)) == -1) {
				perror("Send not succsseful\n");
				exit(-1);
			}
		}
		printf("message sent\n");
	}

	check_close(listenfd);
	return 0;

}









