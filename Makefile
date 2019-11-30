MAIN = main
OBJS = main.o print.o distance.o split.o timer.o pingpong.o

CC = mpicc
CFLAGS = -O3 -xCORE-AVX2 -std=c99 

all: $(MAIN)

$(MAIN): $(OBJS)
	$(CC) -o $(MAIN) $^

.SUFFIXES: .c .o
.c.o:
	$(CC) $(CFLAGS) -c $<

.PHONY: clean
clean:
	rm $(MAIN) *.o
