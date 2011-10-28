##
# sccpp 0.1
##

CC=gcc
CFLAGS=-I /usr/include -Wall -pthread
DEPS = message.h utils.h
OBJ = sccpp.o message.o

%.o: %.c
	$(CC) $< -c -o $@ $(CFLAGS)

sccpp: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS)

clean:
	rm -f sccpp
	rm *.o

install: sccpp
	install -d "/usr/bin"
	install -m 755 sccpp "/usr/bin"

uninstall:
	rm -rf "/usr/bin/sccpp"
