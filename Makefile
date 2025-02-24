CC = gcc
CFLAGS = -Wall -O2 -pthread

all: matrixmult_par matrixmult_ser

matrixmult_par: matrixmult_par.c
    $(CC) $(CFLAGS) -o matrixmult_par matrixmult_par.c

matrixmult_ser: matrixmult_ser.c
    $(CC) $(CFLAGS) -o matrixmult_ser matrixmult_ser.c

clean:
    rm -f matrixmult_par matrixmult_ser