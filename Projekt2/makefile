# Program:	Webklient, IPK projekt 1
# Autor: 	Adam Ormandy
# 

all: ./client/client ./server/server

./client/client: ./src/client.o ./src/funkcie.o
	g++ ./src/client.o ./src/funkcie.o -o ./client/client -std=gnu++11  -static-libstdc++
./src/client.o: ./src/client.cpp
	g++ -c ./src/client.cpp -std=gnu++11 -o ./src/client.o  


./server/server: ./src/server.o ./src/funkcie.o
	g++ ./src/server.o  ./src/funkcie.o -o ./server/server -std=gnu++11  -static-libstdc++
./src/server.o: ./src/server.cpp
	g++ -c ./src/server.cpp -std=gnu++11 -o ./src/server.o
	

./src/funkcie.o: ./src/funkcie.cpp ./src/funkcie.h
	g++ -c ./src/funkcie.cpp -std=gnu++11 -o ./src/funkcie.o


.PHONY: clean
clean:
	rm -f ./client/client ./server/server  ./src/*.o ./client/server* ./server/client*

