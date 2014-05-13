BINDIR  = /usr/local/bin
LIBDIR  = /usr/local/lib/pmml
PMMLPATH = .;$(LIBDIR)
PROG	= m2p
CC	= gcc
SHELL	= /bin/sh
DFLAGS	= 
CCOPTS	= -O
CFLAGS	= $(CCOPTS) $(DFLAGS) -DPMML_PATH='"$(PMMLPATH)"' 
LDFLAGS	=
RCSFLAGS =
SRCS	= m2p.c readsmf.c evsplit.c linefit.c oututil.c eplout.c mplout.c
OBJS	= $(SRCS:.c=.o)
COMMOBJS = ../common/smf.o ../common/util.o
HDRS	= m2p.h
RCSFILES = $(SRCS) $(HDRS)

$(PROG): $(OBJS) $(COMMOBJS)
	$(CC) -o $(PROG) $(OBJS) $(COMMOBJS) $(LDFLAGS) -lm

$(COMMOBJS):
	cd ../common; $(MAKE) 'CC=$(CC)' 'CCOPTS=$(CCOPTS)'

install: $(PROG)
	install -s -m755 $(PROG) $(BINDIR)

clean	:
	rm -f $(PROG) $(OBJS) core mon.out gmon.out

depend:
	makedepend $(SRCS)

# DO NOT DELETE THIS LINE -- make depend depends on it.

m2p.o: m2p.h ../common/util.h
readsmf.o: m2p.h ../common/util.h ../common/smf.h
evsplit.o: m2p.h ../common/util.h
linefit.o: m2p.h ../common/util.h
oututil.o: m2p.h ../common/util.h
eplout.o: m2p.h ../common/util.h
mplout.o: m2p.h ../common/util.h
