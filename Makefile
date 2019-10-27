CC = c99 
CFLAGS = -Wall  

all: client server 
	echo "Compile Done."

server.o client.o: helper.h 

both: server.o client.o 

clean:
	rm -f both
	rm -f *.o 
.PHONY: all clean 
