OBJS = chord.o main.o

CC = g++
DEBUG = -g
CFLAGS = -Wall -c $(DEBUG)
LFLAGS = -Wall $(DEBUG)

chord: $(OBJS)
	$(CC) $(LFLAGS) $(OBJS) -o chord -lpthread 

main.o: main.cpp chord.h
	$(CC) $(CFLAGS) main.cpp -lpthread  

chord.o: chord.cpp chord.h
	$(CC) $(CFLAGS) chord.cpp -lpthread

clean:
	\rm *.o *~ chord

