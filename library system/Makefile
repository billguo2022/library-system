CC=g++
CFLAGS=-Wall -std=c++11

all: serverM serverS serverL serverH client

serverM: serverM.cpp
	$(CC) $(CFLAGS) -o serverM serverM.cpp

serverS: serverS.cpp
	$(CC) $(CFLAGS) -o serverS serverS.cpp

serverL: serverL.cpp
	$(CC) $(CFLAGS) -o serverL serverL.cpp

serverH: serverH.cpp
	$(CC) $(CFLAGS) -o serverH serverH.cpp

client: client.cpp
	$(CC) $(CFLAGS) -o client client.cpp

clean:
	rm -f serverM serverS serverL serverH client
