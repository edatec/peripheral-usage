# Makefile for LVD test
# Copyright (c) EDATEC

CC=gcc
CFLAGS=-Wall -std=gnu99 -g
iniparser_dir=/home/pi/test/iniparser
LDFLAGS=-lgpiod ${iniparser_dir}/build/libiniparser.a -I${iniparser_dir}/src/
DEB_GEN_PATH := ..
DEB_SRC_NAME := ed-lvd
DEB_PACKAGE_NAME := ed-lvd

.PHONY: nop
nop:
	@echo "no source code to build"

.PHONY: install
install:
	@echo "nothing to do to install"

lvd: lvd.c
	$(CC) lvd.c -o lvd $(CFLAGS) $(LDFLAGS)

.PHONY: deb
deb:
	dpkg-buildpackage -us -uc

.PHONY: all
all: lvd deb

.PHONY: clean
clean:
	rm -f $(DEB_GEN_PATH)/*.deb $(DEB_GEN_PATH)/*.buildinfo $(DEB_GEN_PATH)/*.changes ./*.deb
	rm -rf $(DEB_GEN_PATH)/$(DEB_SRC_NAME)_*.dsc $(DEB_GEN_PATH)/$(DEB_SRC_NAME)_*.tar.xz
	rm -f  debian/files debian/*.substvars debian/debhelper-build-stamp
	rm -rf debian/.debhelper debian/$(DEB_PACKAGE_NAME)
