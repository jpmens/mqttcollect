PREFIX=/usr/local
BINDIR=$(PREFIX)/bin
MANDIR=$(PREFIX)/share/man/man1

CFLAGS= -Wall -Werror
LDFLAGS=-lmosquitto

all: mqtt-sys

mqtt-sys: mqtt-sys.c uthash.h json.o utstring.h ini.o
	$(CC) $(CFLAGS) -o mqtt-sys mqtt-sys.c json.o ini.o $(LDFLAGS)

json.o: json.c json.h
ini.o: ini.c ini.h

install: mqtt-sys mqtt-sys.1
	install -m 755 mqtt-sys $(BINDIR)/
	install -m 644 mqtt-sys.1 $(MANDIR)/

README.txt: mqtt-sys.1
	nroff -man mqtt-sys.1 | col -b > README.txt
