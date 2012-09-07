CC = gcc
DEBUGFLAGS = -g -Wall
CFLAGS = -D_REENTRANT $(DEBUGFLAGS) -D_XOPEN_SOURCE=500
LDFLAGS = -lpthread -pthread

all: client server server_single_threaded server_multi_threaded

client:	client.c
server:	server.c
server_single_threaded: server_single_threaded.c
server_multi_threaded: server_multi_threaded.c

clean:
	rm -f client server
