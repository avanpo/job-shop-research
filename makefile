LIBS = -lm
CC = gcc
CFLAGS = -g -Wall

all: program

program: 
	$(CC) $(CFLAGS) $(LIBS) -o program sol.c search.c schedule.c graph.c instance.c
clean:
	rm program
