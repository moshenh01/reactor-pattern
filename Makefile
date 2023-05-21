CC = gcc
FLAGS = -Wall -g

all: st_reactor.so react_server client

react_server: react_server.o
	$(CC) $(FLAGS) -o react_server react_server.o -lpthread ./st_reactor.so

st_reactor.so: reactor.o
	$(CC) $(FLAGS) -shared -o st_reactor.so reactor.o


client: client.o
	$(CC) $(FLAGS) -o client client.o

react_server.o: react_server.c reactor.h
	$(CC) $(FLAGS) -c -fPIC react_server.c

client.o: client.c
	$(CC) $(FLAGS) -c client.c

reactor.o: reactor.c reactor.h
	$(CC) $(FLAGS) -c -fPIC reactor.c

clean:
	rm -f *.o *.so react_server client