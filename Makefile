CFLAGS = -g -Wall
LIBS = -lX11

xlock: xlock.o
	$(CC) $(CFLAGS) -o xlock xlock.o $(LIBS)
