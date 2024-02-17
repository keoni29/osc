NAME = osc

CC = gcc
OBJDIR = build
SRCDIR = src
BINDIR = bin
INCDIR = inc

INC = -I$(INCDIR)
SRC = $(shell find $(SRCDIR)/ -type f -name '*.c')
OBJ = $(patsubst $(SRCDIR)/%.c,$(OBJDIR)/%.o,$(SRC))
BIN = $(addprefix $(BINDIR)/,$(NAME))

CFLAGS = -c -Wall -g $(INC)
LDFLAGS = -g -lSDL2 -lm -lconfig

all: $(BIN)

$(BIN): $(OBJ)
		@mkdir -p "$(@D)"
		$(CC) $(LDFLAGS) $(OBJ) -o $@

$(OBJDIR)/%.o: $(SRCDIR)/%.c
		@mkdir -p "$(@D)"
		$(CC) $(CFLAGS) $< -o $@

.PHONY: clean purge
clean:
		@rm -f $(OBJ) $(BIN) && echo "clean succesful" || echo "clean failed"

purge:
		@rm -rf $(OBJDIR) $(BINDIR) && echo "purge succesful" || echo "purge failed"
