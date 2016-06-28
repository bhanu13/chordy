OBJS = chord.o main.o

CC = g++
DEBUG = -g
CFLAGS = -Wall -c $(DEBUG)
LFLAGS = -Wall $(DEBUG)

chord: $(OBJS)
	$(CC) $(LFLAGS) -lpthread $(OBJS) -o chord

main.o: main.cpp chord.h
	$(CC) $(CFLAGS) -lpthread main.cpp

chord.o: chord.cpp chord.h
	$(CC) $(CFLAGS) -lpthread chord.cpp

clean:
	\rm *.o *~ chord