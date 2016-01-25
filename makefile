CC = gcc
CFLAGS = -g -O3 -Wall
LIBS = -lm

all: sol instances/instgen

sol: sol.c search.c schedule.c graph.c instance.c
	$(CC) $(CFLAGS) $(LIBS) -o sol sol.c search.c schedule.c graph.c instance.c

instances/instgen: instances/instgen.c
	$(CC) $(CFLAGS) $(LIBS) -o instances/instgen instances/instgen.c

clean:
	rm -f sol instances/instgen
