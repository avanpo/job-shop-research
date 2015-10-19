LIBS = -lm
CC = gcc
CFLAGS = -g -Wall

all: sol

sol: 
	$(CC) $(CFLAGS) $(LIBS) -o sol sol.c search.c schedule.c graph.c instance.c
clean:
	rm sol
