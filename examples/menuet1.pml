/*
 *  Menuet G-major by J. S. Bach   
 *     No velocity or tempo control
 */
timesig(3,4)
tempo(160)

newtrack(rh) { v=80 }
newtrack(lh) { o=3 v=60 }

rh {
  for($i,1,2) {
    q ^D i G A B ^C  q ^D G G
    ^ q E i C D E F#  q G _ G G 
    q ^C i ^D ^C B A  q B i ^C B A G
    if( $i == 1 ) {
      q F# i G A B G  h. A(grace(s B))
    } else {
      q A i B A G F#  h. G
    }   
  }
}

lh {
  h. [ {h G q A} B ^D ] 
  B ^C B A G q ^D B G q ^D i D ^C B A

  h B q A q G B G h. ^C q B i ^C B A G
  h A q F# h G q B
  q ^C ^D D h G q _G
}

===
rh {
  ^ q B i G A B G  q A i D E F# D
  q G i E F# G D  q C#(grace(C# D)) i _B C# q _A
  i _A _B C# D E F#  q G F# E
  q F# _A C#  h. D

  _ q ^D i G F# q G  q ^E i G F# q G
  q ^D ^C B  i A G F# G q A
  i D E F# G A B  q ^C B A
  i B ^D q G F#  h. [_B D G]
}

lh {
  h. G F# q E G E h A q _A
  h. A q B ^D ^C# ^D F# A ^D D ^C

  [{h B q B} {r(q) h ^D}] [{h ^C q ^C} {r(q) h ^E}]
  q B A G h ^D r(q)
  [h. D {r(h) q F#}] q E G F# G _B D G D _G
}
