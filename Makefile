##
# sccpp 0.1
##

CC=clang #gcc
CFLAGS=-I /usr/include -Wall -g -pthread -D_REENTRANT -DORTP_INET6 -I/usr/local/include -L/usr/local/lib -lortp -lpthread -lasound
DEPS = message.h utils.h phone.h
OBJ = main.o sccpp.o message.o phone.o rtp.o

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
