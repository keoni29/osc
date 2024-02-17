NAME = osc
OBJ = main.o osc.o
SRC = $(wildcard *.c)
OBJ = $(SRC:%.c=%.o)


CC = gcc
CFLAGS = -O0 -Wall -fmessage-length=0
CLIBS = -lSDL2 -lm -lconfig

all: osc

osc: $(OBJ)
	$(CC) $^ -g -o $(NAME) $(CLIBS)

%.o: %.c
	$(CC) -g $(CFLAGS) -c $< -o $@

clean:
	rm -f *.a *.o $(NAME)
