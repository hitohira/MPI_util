MAIN = main
OBJS = main.o print.o distance.o split.o timer.o pingpong.o locate.o
AROBJS = timer.o split.o locate.o

CC = mpicc
CFLAGS = -O3 -xCORE-AVX2 -std=c99 

all: $(MAIN) mpi_util.a

mpi_util.a: $(AROBJS)
	ar rcs $@ $^

$(MAIN): $(OBJS)
	$(CC) -o $(MAIN) $^

.SUFFIXES: .c .o
.c.o:
	$(CC) $(CFLAGS) -c $<

.PHONY: clean
clean:
	rm $(MAIN) *.o *.a
