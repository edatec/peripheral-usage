# Makefile for LVD test
# Copyright (c) EDATEC

CC=gcc
CFLAGS=-Wall -std=gnu99 -g
LDFLAGS=-lgpiod

all: lvd

lvd: lvd.c
	$(CC) lvd.c -o lvd $(CFLAGS) $(LDFLAGS)

clean:
	rm -rf lvd