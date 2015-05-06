
CFLAGS= -Wall -Werror
LDFLAGS=-lmosquitto

all: mqtt-sys

mqtt-sys: mqtt-sys.c uthash.h
	$(CC) $(CFLAGS) -o mqtt-sys mqtt-sys.c $(LDFLAGS)
