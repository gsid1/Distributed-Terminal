CC = g++
CFLAGS = -Wall -std=c++14 -g
DEPS = server.h client.h clientfiles.h
CLOBJ = client 
SVOBJ = server 
peer :
	$(CC) $(CFLAGS) -o $(CLOBJ) client.cpp -lpthread
	$(CC) $(CFLAGS) -o $(SVOBJ) server.cpp -lpthread
clean:
	rm -rf *.o $(CLOBJ) $(SVOBJ)
