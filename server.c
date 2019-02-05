#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include "ftp.h"


	
void read_packets(char * data, struct packet* p) {
	int num = 0;
	int left = 0;
	char* temp[4];
	for (int i = 0; i < MAX_LEN; i++) {
		if (data[i] == ':') {
			temp[num] = malloc(sizeof(char)*(i-left));
			memcpy(temp[num], data+left, i-left);
			left = i + 1;
			num++;
		}
		if (num == 4) break;
	}
	p->total_frag = atoi(temp[0]);
	p->frag_no = atoi(temp[0]);
	p->size = atoi(temp[0]);
	for (int i = 0; i < 3; i++) free(temp[i]); 
	p->filename = temp[3];
	memcpy(&(p->filedata), data+left, p->size);
}



int main(int argc, char* argv[]) {
	if (argc != 2) {
		printf("Usage: server <UDP listen port>\n");
		exit(1);
	}

	int listenfd, port, n;
	port = atoi(argv[1]);
	char buf[BUFFER_SIZE];
	char input[MAX_LEN];
	struct sockaddr_in self, client_addr;
	socklen_t serv_addr_size = sizeof(self);
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

			

			FILE* fp = NULL;
			char* filepath;
			int num = 0, prev = -1;
			int n = 0, flag = 0;
			char data[MAX_LEN];
			unsigned int offset;
			struct packet p;
			int *received;

			while (1) {
				if ((n = recvfrom(listenfd, data, BUFFER_SIZE, 0, (struct sockaddr *) &self, &serv_addr_size)) < 0) { 
					printf("No packet received\n");
				} else {
					read_packets(data, &p);
					if (!flag) {
						// first packet
						flag = 1;
						filepath = malloc(sizeof(char) * (strlen(PATH) + strlen(p.filename)));
						strcpy(filepath, PATH);
						strcat(filepath, p.filename);
						fp = fopen(filepath, "w");
						if (fp == NULL) {
							perror("Fail to open file\n");
							exit(1);
						}
						received = malloc(sizeof(int)*p.total_frag);
					}

					if (p.frag_no - prev == 1 || received[p.frag_no]) {
						if (sendto(listenfd, ACK, strlen(ACK), 0, (struct sockaddr*) &client_addr, sizeof(client_addr)) <= 0) {
							perror("send ack error\n");
							exit(-1);
						}
						if (received[p.frag_no]) {
							continue;
						} else {
							prev++;
						}
					} else {
						if (sendto(listenfd, NACK, strlen(NACK), 0, (struct sockaddr*) &client_addr, sizeof(client_addr)) <= 0) {
							perror("send nack error\n");
							continue;
						}
					}
					received[p.frag_no] = 1;
					long int offset = sizeof(char) * p.frag_no * MAX_LEN;
					fseek(fp, offset, SEEK_CUR);
					fwrite(p.filedata, sizeof(char), p.size, fp);
					free(p.filename);
					// free(p.filedata);
					if (p.frag_no + 1 == p.total_frag) break;
				}
			}
		
		} else {
			if (sendto(listenfd, NO, strlen(NO), 0, (struct sockaddr*)&client_addr, sizeof(client_addr)) == -1) {
				perror("Send not succsseful\n");
				exit(-1);
			}
			printf("message sent succssefully\n");
		}
	}
	check_close(listenfd);
	return 0;

}









