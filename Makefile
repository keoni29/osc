NAME := osc

CC := gcc
CFLAGS := -O0 -g3 -Wall -fmessage-length=0
CLIBS := -lpulse-simple

all: osc

osc: main.o osc.o
	$(CC) main.o osc.o -o $(NAME) $(CLIBS)

main.o: main.c
	$(CC) $(CFLAGS) -c main.c -o main.o

osc.o: osc.c
	$(CC) $(CFLAGS) -c osc.c -o osc.o


clean:
	rm *.a *.o osc
