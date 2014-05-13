PMML = ../../comp/pmml -I../../lib

.SUFFIXES: .pml .mid
.pml.mid:
	$(PMML) $<

grieg.mid: grieg1.mid grieg2a.mid grieg3.mid grieg2b.mid grieg4.mid
	$(PMML) grieg

grieg2a.mid: grieg2.pml
	$(PMML) -o $@ -e'times=1' grieg2
grieg2b.mid: grieg2.pml
	$(PMML) -o $@ -e'times=2' grieg2

clean:
	rm -f *.mid
