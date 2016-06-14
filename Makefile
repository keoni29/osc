NAME = osc
OBJ = main.o osc.o
SRC = $(wildcard *.c)
OBJ = $(SRC:%.c=%.o)


CC = gcc
CFLAGS = -O0 -g3 -Wall -fmessage-length=0
CLIBS = -lSDL2 -lm

all: osc

osc: $(OBJ)
	$(CC) $^ -o $(NAME) $(CLIBS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm *.a *.o osc
