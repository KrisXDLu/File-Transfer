
all: server deliver

server: ftp.h server.c
	gcc server.c -o server

deliver: ftp.h deliver.c
	gcc deliver.c -o deliver

clean:
	rm *.o