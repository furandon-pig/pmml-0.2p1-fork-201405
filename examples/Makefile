PMML = ../comp/pmml -I../lib

.SUFFIXES: .pml .mid
.pml.mid:
	$(PMML) $<

MIDFILES = etude1.mid handel.mid menuet1.mid menuet2.mid rand1.mid rand2.mid

all:	$(MIDFILES)

clean:
	rm -f $(MIDFILES)
