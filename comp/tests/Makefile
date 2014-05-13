PMML	= ../pmml -I../../lib
SHELL	= /bin/sh
COMPARE = cmp

TESTS	= test1 test2 test3
MIDS    = $(TESTS:=.mid)

all : $(TESTS)

$(TESTS) :
	rm -f $@.mid
	$(PMML) $@
	$(COMPARE) $@.mid $@.ref

clean :
	rm -f $(MIDS)

ci	:
	shar *.pml *.ref > test.shar
	ci -l -f $(RCSFLAGS) test.shar
