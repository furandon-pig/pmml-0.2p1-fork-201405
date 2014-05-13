/*
 * test on event-generating commands
 */ 
print("Testing commands")

t=0 v=80 tk=2 ch=1 
{ q C i D(ch=3 v=60) w+(q-i) _E h. {^^ F(gt=z/2)} r(w) o=5 480u C(gr=20) }

t=0 tk=3
{key=-1 i A B ^C ^D q ^C ^C B% B% ^C Bb}
{tp=1 q C D E}

t=0 tk=4
q { e++ f e } & { ac=10 c+++ c--. c }
F>> G<<.

t=0 tk=5
c & e & g
[c f a]
[_b d g]++(w)
[Db Eb](v=100)& F#
{w [D5 {h C5 B4} G4]}

t=0 tk=6
for($i,C4,E4) { note(n=$i) }
note_on(c5, 100)
note_on(c6, 100, 3)
note_off(c5, 100)
note_off(c6, -1, 3)

t=0 tk=7
prog(100)
bend(-8192)
C(kp(50) rt=120u kp(60))
cpr(20)
ctrl(10, 127)
ctrl(128, 0)

t=0 tk=8
v=50
vmag(1.0)
q C D vmag(1.2) E F
vmag(1.5)
G

t=0 tk=9
excl(#(0x41, 0x10))
excl2(#(0x41, 0x10))
arbit(#(0x20, 0x30))
arbit(#(0x40, 0xf7))
seqno(2)
title("Self Test")
lyric("Chigauyou")
smpte(1,2,3,4,5)
timesig(3,4)
timesig(6,8,2)
keysig(-2,1)
meta(0x00, #(0,5))
meta(0x04, #(0x42, 0x43))
tempo(100)
c
end
