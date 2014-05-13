include("gm")

defthread(melody_part, chord_part, drums_part)

scale1 = #(g4, a4,  b4,  c5, d5,  e5,  g5, a5) 
scale2 = #(g4, a4,  bb4, c5, c#5, d#5, f5, g4)
scale3 = #(g4, a4,  c5,  d5, e5,  f5,  g5, a5)
scale4 = #(g4, ab4, bb4, b4, c#5, eb5, f5, g5)

rhythms = #( #(q), #(q/3, q/3, q/3), 
             #(2q/3, q/3), #(i,i) )

def(rand_phrase) {
  foreach($i, rhythms[irand(#rhythms)]) {
    note(n = $1[frand(8)+1]  l = $i) 
  }
}

srand()

repeat(8) {
  melody_part {
    repeat(4) { rand_phrase(scale1) } 
    repeat(4) { rand_phrase(scale2) }
    repeat(4) { rand_phrase(scale3) }
    repeat(4) { rand_phrase(scale4) }
  }
  chord_part {
    w
    [c3 e3 {h b3 b3}]
    [a2 g3 {h c#4 c#4}]
    [d3 f3 {h c4 c4}]
    [g2 f3 {h b3 b3}]
  } 
  drums_part {
    ch=10 v=60
    CrashCY &
    if( irand(3) == 1 ) {
      {r(3w+h.) q/3 r MT1 LT1 }&
    }
    repeat(4) {
      drums(q/3 v=30,
	    BD,     "X...........",
	    SD,     "......X.....",
	    RideCY, "X..X.xX..X.x")
    }
  }
}
