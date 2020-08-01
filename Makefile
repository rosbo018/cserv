CC=gcc
CFLAGS=-g
DEPS=server.h
OBJ=server.o

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

.PHONY: serv


serv: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS)


