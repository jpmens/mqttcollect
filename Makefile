PREFIX=/usr/local
BINDIR=$(PREFIX)/bin
MANDIR=$(PREFIX)/share/man/man1

CFLAGS= -Wall -Werror
LDFLAGS=-lmosquitto

all: mqttcollect README.md

mqttcollect: mqttcollect.c uthash.h json.o utstring.h ini.o
	$(CC) $(CFLAGS) -o mqttcollect mqttcollect.c json.o ini.o $(LDFLAGS)

json.o: json.c json.h
ini.o: ini.c ini.h

install: mqttcollect mqttcollect.1
	install -m 755 mqttcollect $(BINDIR)/
	install -m 644 mqttcollect.1 $(MANDIR)/

README.md: mqttcollect.pandoc
	pandoc -w markdown mqttcollect.pandoc -o README.md
