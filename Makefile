CC=gcc
FLAG=-g
OBJS=command.c
CLIENT=client.c
SERVER=server.c

all:
	$(CC)  -c $(OBJS) 
	$(CC)  $(CLIENT) command.o -o client
	$(CC)  $(SERVER) command.o -o server -lpthread

clean:
	rm -f command.o
	rm -f client
	rm -f server
