CC = gcc
CFLAGS = -g3 -Wall
THR = -lpthread
OBJECTS = main.o acutil.o sQueue.o threadPool.o myServer.o
EX = myhttpd

$(EX): $(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS) -o $(EX) $(THR)

main.o: main.c
	$(CC) $(CFLAGS) -c main.c $(THR)

acutil.o: acutil.c
	$(CC) $(CFLAGS) -c acutil.c $(THR)

sQueue.o: sQueue.c
	$(CC) $(CFLAGS) -c sQueue.c

threadPool.o: threadPool.c
	$(CC) $(CFLAGS) -c threadPool.c $(THR)

myServer.o: myServer.c
	$(CC) $(CFLAGS) -c myServer.c $(THR)

.PHONY: clean

clean:
	rm -f $(EX) $(OBJECTS)
