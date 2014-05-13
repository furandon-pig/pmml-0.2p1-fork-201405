#
# Makefile for PMML
#

#------ configuration parameters ------

# The directory to which PMML library files are installed.
LIBDIR  = /usr/local/lib/pmml

# The directory to which executable files are installed.
BINDIR  = /usr/local/bin

# Command name of the C compiler
CC	= gcc

# C compiler options
CCOPTS	= -O
# for DOS, use the next line
#CCOPTS	= -O -DDOS

# search path for PMML input files
PMMLPATH = .;$(LIBDIR)

#-----------

SUBDIRS = c_lib common comp m2p lib
MAKEOPTS= 'LIBDIR=$(LIBDIR)' 'BINDIR=$(BINDIR)' \
	'CC=$(CC)' 'CCOPTS=$(CCOPTS)' 'PMMLPATH=$(PMMLPATH)'
SHELL	= /bin/sh

all:
	for i in $(SUBDIRS); do \
	  (echo $$i; cd $$i; $(MAKE) $(MAKEOPTS)); \
	done

test:
	(cd comp; $(MAKE) $(MAKEOPTS) test)

install:
	mkdir -p $(LIBDIR)
	mkdir -p $(BINDIR)
	for i in $(SUBDIRS); do \
	  (echo $$i; cd $$i; $(MAKE) $(MAKEOPTS) install); \
	done

clean:
	for i in $(SUBDIRS); do \
	  (echo $$i; cd $$i; $(MAKE) $(MAKEOPTS) clean); \
	done
