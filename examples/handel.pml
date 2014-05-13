/*
 *   Try to uncomment some lines to get various effects!
 */
tempo(100)

// swing(q, 70)

newtrack(righthand) { v=80 ch=1 prog(1) bend(0) }
newtrack(lefthand) { v=60 ch=2 prog(1) bend(0) }

righthand {
  // add_notes(n+=12 dt+=s)
  // trill(1)
  // grace(note(n-=1))
  // retrograde()
  // modify_notes(bend(irand(-2000,2000)))

  h G5 q. E5 i F5 h G5 C5
  i D5 E5 F5 G5 q F5 E5 w D5
  i E5 F5 G5 A5 q G5 G5 h C6 G5
  q F5 E5 q. D5 i C5 h C5 [C5 E5 G5 C6](arp(z))
}

lefthand {
  // tremolo()
  // roll()

  h [C4 E4] [C4 E4] [C4 E4] [C4 E4]
  [{i B3 C4 D4 E4} G4] [{q D4 C4} G4] i [B3 G4] F#4 G4 A4 G4 F E4 D4
  h [C4 G4] q [C4 G4] [D4 F4] h [E4 G4] [E4 C4] 
  [{h. C4 q B3} {q A4 G4 h F4}] w [C4 E4]
}
