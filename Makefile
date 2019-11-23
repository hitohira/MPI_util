MAIN = main
OBJS = main.o print.o distance.o split.o timer.o

CC = mpicc
CFLAGS = -O3 -xCORE-AVX2

all: $(MAIN)

$(MAIN): $(OBJS)
	$(CC) -o $(MAIN) $^

.SUFFIXES: .c .o
.c.o:
	$(CC) $(CFLAGS) -c $<

.PHONY: clean
clean:
	rm $(PROGRAM) *.o
