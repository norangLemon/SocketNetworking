CC = g++
CFLAGS = --std=c++14
LDFLAGS = -lpthread

all: server client
server : server.cpp
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)
client : client.cpp
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)
clean :
	rm server client
