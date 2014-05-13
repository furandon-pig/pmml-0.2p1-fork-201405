BINDIR  = /usr/local/bin
LIBDIR  = /usr/local/lib/pmml
PMMLPATH = .;$(LIBDIR)
CC	= gcc
PROG	= pmml
SHELL	= /bin/sh
CCOPTS  = -O
DFLAGS	= #-DDEBUG -DPRINT_REF -DDEBUG2 -DFREE_TEST
CFLAGS	= $(CCOPTS) $(DFLAGS) -DPMML_PATH='"$(PMMLPATH)"'
LDFLAGS	=
RCSFLAGS =
SRCS	= pmml.c input.c error.c table.c expr.c macro.c parseut.c \
  dict.c array.c token.c builtin.c parse.c output.c \
  smfout.c load.c rational.c
OBJS	= $(SRCS:.c=.o) keyword.o 
COMMOBJS = ../common/smf.o ../common/util.o 
CLIBOBJS = ../c_lib/random.o
HDRS	= pmml.h rational.h

$(PROG): $(OBJS) $(COMMOBJS) $(CLIBOBJS)
	$(CC) -o $(PROG) $(OBJS) $(COMMOBJS) $(CLIBOBJS) $(LDFLAGS) -lm

keyword.c keyword.h table2.c: keywords
	./mkkw.sh

$(COMMOBJS):
	cd ../common; $(MAKE) 'CC=$(CC)' 'CCOPTS=$(CCOPTS)'

$(CLIBOBJS):
	cd ../c_lib; $(MAKE) 'CC=$(CC)' 'CCOPTS=$(CCOPTS)'

test	: $(PROG)
	cd tests; $(MAKE)

clean	:
	rm -f $(PROG) $(OBJS) core mon.out gmon.out out.mid test.mid
	cd ../c_lib; $(MAKE) clean
	cd tests; $(MAKE) clean

install: $(PROG)
	install -s -m755 $(PROG) $(BINDIR)

depend:
	makedepend $(SRCS)

# DO NOT DELETE THIS LINE -- make depend depends on it.

pmml.o: pmml.h rational.h keyword.h ../common/util.h ../common/smf.h
input.o: pmml.h rational.h keyword.h ../common/util.h
error.o: pmml.h rational.h keyword.h ../common/util.h ../common/smf.h
table.o: pmml.h rational.h keyword.h ../common/util.h table2.c
expr.o: pmml.h rational.h keyword.h ../common/util.h
macro.o: pmml.h rational.h keyword.h ../common/util.h
parseut.o: pmml.h rational.h keyword.h ../common/util.h
dict.o: pmml.h rational.h keyword.h ../common/util.h
array.o: pmml.h rational.h keyword.h ../common/util.h
token.o: pmml.h rational.h keyword.h ../common/util.h
builtin.o: pmml.h rational.h keyword.h ../common/util.h
parse.o: pmml.h rational.h keyword.h ../common/util.h
output.o: pmml.h rational.h keyword.h ../common/util.h
smfout.o: pmml.h rational.h keyword.h ../common/util.h ../common/smf.h
load.o: pmml.h rational.h keyword.h ../common/util.h ../common/smf.h
rational.o: pmml.h rational.h keyword.h ../common/util.h
