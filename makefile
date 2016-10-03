CFLAGS = -G -Wall -O3 `libgcrypt-config --cflags`
LIBS = -lsqlite3 `libgcrypt-config --libs`

OBJS = aes_crypt.o base64.o sqlite.o
DEPS = aes_crypt base64

main:$(OBJS)
	gcc -o $@ $(OBJS) $(CFLAGS) $(LIBS)  main.c

$(OBJS):
	gcc -c $(CFLAGS) $(LIBS) $*.c
clean:
	rm -f *.o
	rm main
