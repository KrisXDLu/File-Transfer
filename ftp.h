#include <errno.h>

#define BUFFER_SIZE 256
#define FTP "ftp"
#define YES "yes"
#define NO "no"
#define MAX_LEN 2048
#define NACK "nack"
#define ACK "ack"
#define PATH "./packet/"

void check_close(int function) {
    if (close(function) != 0) {
        perror("close");
    }
}
struct packet {
	unsigned int total_frag; // total numb of frags
	unsigned int frag_no; // seq no. of the frag
	unsigned int size; // size of data
	char * filename; 
	char filedata[1000];
};
