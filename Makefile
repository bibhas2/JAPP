CC=gcc
CFLAGS=-std=c99 
OBJS=Parser.o
HEADERS=Parser.h

all: libjapp.a test

%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c -o $@ $<
libjapp.a: $(OBJS) 
	ar rcs libjapp.a $(OBJS)
test: $(OBJS) test.o
	gcc -o test test.o -L../Cute -L. -ljapp -lcute -lm
clean:
	rm $(OBJS)
	rm libjapp.a
	
