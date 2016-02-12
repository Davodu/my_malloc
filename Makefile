CC=gcc
CFLAGS=-O3 -fPIC
WDIR=/Users/davies/Desktop/HW1

all: thread_test_malloc_free

thread_test_malloc_free: thread_test_malloc_free.c
	     $(CC) $(CFLAGS) -I$(WDIR) -L$(WDIR) -Wl,-rpath,$(WDIR) -o $@ thread_test_malloc_free.c -lmymalloc -lpthread

clean:
	rm -f *~ *.o thread_test_malloc_free

clobber:
	rm -f *~ *.o