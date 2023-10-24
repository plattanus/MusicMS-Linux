FLAG=-Wall -Werror
STD=-std=gnu99
CC=gcc
all:
	$(CC) $(STD) $(FLAG) client.c -o client -lpthread -lsqlite3
	$(CC) $(STD) $(FLAG) server.c -o server -lpthread -lsqlite3
clean:
	rm client server
	