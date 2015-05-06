PREFIX=/usr/local
BINDIR=$(PREFIX)/bin
MANDIR=$(PREFIX)/share/man/man1

CFLAGS= -Wall -Werror
LDFLAGS=-lmosquitto

all: mqtt-sys

mqtt-sys: mqtt-sys.c uthash.h
	$(CC) $(CFLAGS) -o mqtt-sys mqtt-sys.c $(LDFLAGS)

install: mqtt-sys mqtt-sys.1
	install -m 755 mqtt-sys $(BINDIR)/
	install -m 644 mqtt-sys.1 $(MANDIR)/

