/*
 *  Menuet G-major by J. S. Bach   
 *     with velocity and tempo control
 */
timesig(3,4)
tempo(160)

newtrack(rh) { v=80 }
newtrack(lh) { o=3 v=60 }

def(slow, "n") { {rtempo($1) r(s) rtempo(1.0)}& }

rh {
  for($i,1,2) {
    if( $i == 1 ) {
      { rtempo(0.8) r(q) rtempo(1.0) }&
      { vmag(0.9) r(q) vmag(0.6) r(h) vmag_to(0.8) r(h.)
	vmag(0.8) r(q) vmag(0.7) r(h) vmag_to(0.9) r(h.)
	vmag(0.8) r(3h.) vmag_to(0.7)
      }&
    } else {
      slow(0.8)
      { vmag(0.9) r(q) vmag(0.8) r(h) vmag_to(0.9) r(h.)
	vmag(0.9) r(q) vmag(0.7) r(h) vmag_to(1.0) r(h.)
	vmag(0.9) r(q) vmag(0.8) r(3h.-q) vmag_to(0.6)
      }&
    }
    q ^D>. i G A B ^C  q gr=80 ^D gr=30 G-- G--
    ^ q gr=80 E i gr=100 C D E F#
    slow(0.9) q gr=80 G _ rtempo(1.0) gr=30 G-- G-- rtempo_to(0.9)
    slow(0.8) q gr=100 ^C(gr=80) i ^D ^C B A  q B(gr=80) i ^C B A G
    if( $i == 1 ) {
      q F#(gr=80) i G A B G  rtempo(1.0) h. A(grace(s B) gr=90) rtempo_to(0.95)
    } else {
      q A(gr=80) i B A rtempo(1.0) G F#  h. G(gr=90) rtempo_to(0.8)
    }   
  }
}

lh {
  vmag(1.0)
  h. [ {h G q A-.} B-- ^D-- ]
  B-. ^C B-. A G(gr=80) q ^D B G q ^D(gr=80)-.
  vmag(0.8) i D ^C B A vmag_to(1.0)

  h B q A q G(gr=80)-. {gr=30 -. B G} h. ^C q B(gr=80)-.
  vmag(1.0) i ^C B A G vmag_to(0.9)
  { vmag(0.9) r(3h.) vmag_to(0.8) }&
  h A q F# h G q B
  q ^C ^D(gr=50) D(gr=50) h G q _G(gr=80)
}

===
rh {
  slow(0.7)
  vmag(0.7)
  ^ q B(gr=80) i G A B G  q A(gr=80) i D E F# D
  q G(gr=80) i E F# G D  q C#(grace(z. C# D)) i _B C# q _A-.(gr=80)
  { vmag(0.7) r(h.) vmag_to(0.9) }&
  i _A _B C# D E F#
  {gr=40 q G vmag(0.8) F# E q F#+. rtempo(1.0) _A C# vmag_to(0.7)}
  h. D rtempo_to(0.8)

  slow(0.8) vmag(0.8)
  _ q ^D i G F# q G(gr=50)-.  q ^E i G F# q G(gr=50)-.
  q gr=40 ^D ^C B  i gr=100 A G F# G q A(gr=50)-.
  { vmag(0.7) r(h.) vmag_to(0.9) }&
  i D E F# G A B
  {gr=40 q ^C B A} vmag_to(0.7) vmag(0.8)
  rtempo(1.0) i B+. ^D {gr=40 q G F#}  h. [_B-- D-- G+.] rtempo_to(0.6)
}

lh {
  vmag(0.75)
  h. G F# {gr=40 q E G E} h A q _A(gr=80)-.
  h. A {gr=40 q vmag(0.8) B ^D ^C# ^D F# A vmag_to(0.7)} q
  vmag(0.9) ^D D ^C vmag_to(0.8)

  [{h B q B(gr=80)} {r(q) h ^D}] [{h ^C q ^C(gr=80)} {r(q) h ^E}]
  q B A G h ^D r(q)
  [h. D {r(h) q F#}] {gr=40 q E G F#} q G {gr=40 _B D G D} _G
}
