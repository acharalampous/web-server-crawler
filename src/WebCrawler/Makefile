CC = gcc
CFLAGS = -g -Wall
THR = -lpthread
OBJECTS = main.o acutil.o uQueue.o threadPool.o myCrawler.o jobExecutor.o worker.o postingList.o txInfo.o WordList.o trie.o
EX = mycrawler

$(EX): $(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS) -o $(EX) $(THR)

main.o: main.c
	$(CC) $(CFLAGS) -c main.c $(THR)

acutil.o: acutil.c
	$(CC) $(CFLAGS) -c acutil.c $(THR)

uQueue.o: uQueue.c
	$(CC) $(CFLAGS) -c uQueue.c

threadPool.o: threadPool.c
	$(CC) $(CFLAGS) -c threadPool.c $(THR)

myCrawler.o: myCrawler.c
	$(CC) $(CFLAGS) -c myCrawler.c $(THR)

jobExecutor.o: jobExecutor.c
	$(CC) $(CFLAGS) -c jobExecutor.c

worker.o: worker.c
	$(CC) $(CFLAGS) -c worker.c

postingList.o: postingList.c
	$(CC) $(CFLAGS) -c postingList.c

txInfo.o: txInfo.c
	$(CC) $(CFLAGS) -c txInfo.c

WordList.o: WordList.c
	$(CC) $(CFLAGS) -c WordList.c

trie.o: trie.c
	$(CC) $(CFLAGS) -c trie.c

.PHONY: clean

clean:
	rm -f $(EX) $(OBJECTS)

