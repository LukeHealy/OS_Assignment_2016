CC = gcc
CFLAGS = -Wall -g -std=gnu99 -lrt -lpthread
OBJ = pmms.o file_io.o
EXEC = pmms

$(EXEC) : $(OBJ)
	$(CC) $(OBJ) -o $(EXEC) -lrt -lpthread

pmms.o : pmms.c
	$(CC) -c pmms.c $(CFLAGS) -lrt -lpthread

file_io.o : file_io.c
	$(CC) -c file_io.c $(CFLAGS)

clean:
	rm -f $(EXEC) $(OBJ)
