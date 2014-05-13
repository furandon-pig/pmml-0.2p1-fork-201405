/*
 *  Piano Concerto Op.16         Edvard Grieg
 *      1st movement  measures 74-116
 */

/* The following copyright notice applies to this PMML code and
   musical expressions described therein.  The copyright of the music 
   itself is already expired. */ 
/*
 *  Copyright (C) 1997   Satoshi Nishimura
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

include("defs")

/***************************************************************
 * measures 74-88
 ***************************************************************/
===
tempo(124)
rtempo(1.0)

piano {
  rh {
    pmag(1.0)
    v=110
    q [C6 E6 G6 C7]
  }
  lh {
    v=127
    q [C1 C2]
  }
}
orch {
  violin1 {
    sexpr(110)
    {
      arco
      v=100
      roll(attack=0 decay=0 sustain=100 speed=q/6)
      q/6 [G4 E5 C6]++ C6 q/3 C6 C6 C6 C6 B5
      G5++ G5 G5 G5 G5 B5
      C6++ C6 B5 G5++ G5 B5 C6++ C6 B5 G5++ G5 B5
      q C6++(disable(roll)) r {h -- C5 sexpr(110) C5 C5 sexpr_to(127)}

      sexpr(110)
      q/6 [A4 F5 D6]++ D6 q/3 D6 D6 D6 D6 C#6
      A5++ A5 A5 A5 A5 C#6
      D6++ D6 C#6 A5++ A5 C#6 D6++ D6 C#6 A5++ A5 C#6
      q D6(disable(roll)) r
      {h -- D5 D5 D5 E5 sexpr(110) E5 E5 F5 sexpr_to(127)}
    }

    { v=120 dr=50 sexpr(110)
      r(h) q [A4 C5 F5] R r(h) q [A4 C5 F5] R
      dr=30 R [A4 C5 F5] R [G4 C5 E5] R [F#4 Eb5] R [G4 C5 E5]
    }

    pizz v=80 sexpr(80)
    {tempo(80) r(h) tempo(110) r(h) tempo(110) r(w) tempo_to(100)}&
    r(h) q F4 r E4 r Eb4 r 
  }
  violin2 {
    {
      arco
      v=80
      roll(attack=0 decay=0 sustain=80 speed=q/6)
      q/3 C5++ C5 C5 C5 C5 B4 G4++ G4 G4 G4 G4 B4
      C5++ C5 B4 G4++ G4 B4 C5++ C5 B4 G4++ G4 B4
      q C5++(disable(roll)) r {h -- G4 A4 A4}
      
      q/3 D5++ D5 D5 D5 D5 C#5 A4++ A4 A4 A4 A4 C#5
      D5++ D5 C#5 A4++ A4 C#5 D5++ D5 C#5 A4++ A4 C#5
      q D5++(disable(roll)) r {h -- A4 G4 B4 B4 C5 C5 C5}
    }

    r(4w)
    pizz v=60
    r(h) q C4 r B3 r A3 r 
  }
  viola {
    {
      arco
      v=80
      roll(attack=0 decay=0 sustain=80 speed=q/6)
      q/6 [C3 G3 E4]++ 5q/6 [G3 E4] i [G3 E4](disable(roll)) r
      q/6 [C3 G3 F4]++ 5q/6 [G3 F4] i [G3 F4](disable(roll)) r
      {disable(roll) q dr=30 ++
	[C3 G3 E4] [C3 G3 F4] [C3 G3 E4] [C3 G3 F4] [C3 G3 E4] r }
      {h -- E4 E4 F4}
      q/6 [D3 A3 F4]++ 5q/6 [A3 F4] i [A3 F4](disable(roll)) r
      q/6 [D3 A3 G4]++ 5q/6 [A3 G4] i [A3 G4](disable(roll)) r
      {disable(roll) q dr=30 ++
	[D3 A3 F4] [D3 A3 G4] [D3 A3 F4] [D3 A3 G4] [A3 F4] r }
      {h -- F4 F4 F4 G4 G4 A4 A4}
    }

    { v=100 dr=50
      r(h) q [A3 C4 F4] R r(h) q [A3 C4 F4] R
      dr=30 R [A3 C4 F4] R [G3 C4 E4] R [F#3 Eb4] R [G3 C4 E4]
    }

    pizz v=60
    r(h) q A3 r G3 r F#3 r 
  }
  foreach($i, #('cello', 'bass')) { $i {
    arco
    v=100
    h C3 C3 {q dr=30 C3 C3 C3 C3}
    v=127
    q C4(dr=50) i. D4(dr=90) s E4(dr=50)
    {i dr=50 D4 C4 B3 C4 B3 A3 Ab3 A3 G3 F3 E3 F3}
    v=100
    h D3 D3 {q dr=30 D3 D3 D3 D3}
    v=127
    q D4(dr=50) i. E4(dr=90) s F4(dr=50)
    {i dr=50 E4 D4 C#4 D4 C4 B3 Bb3 B3 A3 G3 F#3 G3
      F#3 E3 Eb3 E3 D3 C3 B2 C3 B2 A2 Ab2 A2 G2 F2 E2 F2 }

    { v=110 dr=50
      r(h) q A2 R r(h) q A2 R
      dr=30 R A2 R A2 R A2 R A2
    }

    pizz v=60
    r(h) q A3 r B3 r B2 r 
  }}

  flute {
    v=100
    wexpr(80)
    take(0, 
	 4q/3 C6++ q/3 C6 B5 4q/3 G5++ q/3 G5 B5
	 q/3 C6++ C6 B5 G5++ G5 B5 C6++ C6 B5 G5++ G5 B5)
    {wexpr(70) r(w) wexpr(70) r(w) wexpr_to(90)}&
    [{w G5 A5} {2w C6}]
    wexpr(80)
    apply(0, D6 D6 C#6 A5 A5 C#6 D6 D6 C#6 A5 A5 C#6 D6 D6 C#6 A5 A5 C#6)
    {wexpr(70) r(2w+h) wexpr(70) r(w.) wexpr_to(90)}&
    [{w A5 h G5 w B5 w+h C6} {2w D6 w. E6 h F6}]

    { dr=50 r(h) q [C6 F6] R r(h) q [C6 F6] R
      dr=30 R [C6 F6] R [C6 E6] R [C6 Eb6] R [C6 E6] }
  }
  clarinet {
    v=100
    apply(0, Eb5 Eb5 D5 Bb4 Bb4 D5 Eb5 Eb5 D5 Bb4 Bb4 D5 Eb5 Eb5 D5 Bb4 Bb4 D5)
    [{w Bb4 C5} {2w Eb5}]
    apply(0, F5 F5 E5 C5 C5 E5 F5 F5 E5 C5 C5 E5 F5 F5 E5 C5 C5 E5)
    [{w C5 h Bb4 w D5 w+h Eb5} {2w F5 w. G5 h Ab5}]

    { dr=50 r(h) q [Eb5 Ab5] R r(h) q [Eb5 Ab5] R
      dr=30 R [Eb5 Ab5] R [Eb5 G5] R [Eb5 F#5] R [Eb5 G5] }
  }
  foreach($i, #('oboe', 'fagotto')) { $i {
    {
      v=100
      if( $i == 'fagotto' ) { tp-=12 }
      h [C5 E5] [{q A4 B4} F5]
      {q dr=50 [C5 E5] [G4 D5] [C5 E5] [G4 D5]}
      [{2w C5} {w. E5 h F5}]
      h [D5 F5] [{q B4 C#5} G5]
      {q dr=50 [D5 F5] [A4 E5] [D5 F5] [A4 E5]}
      2w [D5 F5] [{w. E5 h F5} {w G5 A5}]
    }
  }}
  oboe {
    { dr=50 r(h) q [F5 A5] R r(h) q [F5 A5] R
      dr=30 R [F5 A5] r [E5 G5] r [Eb5 F#5] r [E5 G5] } 
  }
  fagotto {
    { dr=50 r(h) q A2 R r(h) q A2 R
      dr=30 R A2 r A2 r A2 r A2 } 
  }
  horn {
    bexpr(80)
    v=120
    h Eb4 {q dr=70 F4 G4} {q dr=50 Eb4 G4 Eb4 G4 Eb4} r(q+h+w)
    h F4 {q dr=70 G4 A4} {q dr=50 F4 A4 F4 A4 F4} r(q+h+3w)
    r(4w)
    { expr(127) r(q) expr_to(60) r(2w-q) expr_to(40) }&
    w Ab4 G4(dr=110)
  }
  trumpet {
    v=100
    h [C4 C5] [G3 G4]
    {q dr=50 [C4 C5] [G3 G4] [C4 C5] [G3 G4] [C4 C5]} r(q+h+w)
    h [D4 D5] [A3 A4]
    {q dr=50 [D4 D5] [A3 A4] [D4 D5] [A3 A4] [D4 D5]} r(q+h+3w)
    v=127 expr(127)
    mod(10)
    {deemphasize(30)
      h.. [C4 C5] i/3 {dr=80 [C4 C5] [C4 C5] [C4 C5]}
      h.. [C4 C5] i/3 {dr=80 [C4 C5] [C4 C5] [C4 C5]}
      h [C4 C5] [C4 C5] [C4 C5] [C4 C5]
    }
    mod(0)
  }
  trombone {
    v=100
    h [C3 G3 E4] [C3 G3 F4]
    {q dr=50 [C3 G3 E4] [C3 G3 F4] [C3 G3 E4] [C3 G3 F4] [C3 G3 E4]} r(q+h+w)
    h [D3 A3 F4] [D3 A3 G4]
    {q dr=50 [D3 A3 F4] [D3 A3 G4] [D3 A3 F4] [D3 A3 G4] [D3 A3 F4]} r(q+h+3w)

    { dr=50 r(h) q [A3 C4 F4] R r(h) q [A3 C4 F4] R
      dr=30 R [A3 C4 F4] r [G3 C4 E4] r [F#3 C4 Eb4] r [G3 C4 E4] } 
  }
  timpani {
    r(10w)
    v=80
    r(h) q A2 R r(h) q A2 R R A2 R A2 R A2 R A2
  }
}

/***************************************************************
 * measures 89-101
 ***************************************************************/
===
{ tempo(72) r(h) tempo_to(80) }&

piano {
  pmag(1.0)
  rh {
    v=60
    { h hold hold hold hold hold hold hold hold }&
    q/6 E3 B3 E4 G4 B4 E5 G5 E5 B4 G4 E4 B3
    q/7 D3+. B3 E4 G4 B4 E5 G5 i G6(dr=30) r
    q/6 C#3+. B3 E4 G4 B4 E5 G5 E5 B4 G4 E4 B3
    q/8 B2+. G3 D4 E4 G4 D5 E5 G5 i G6(dr=30) r
    q/7 Bb2+. E3 G3 C#4 E4 G4 C#5 E5 C#5 G4 E4 C#4 G3 E3
    q/8 A2+. E3 G3 C4 E4 G4 C5 E5 i E6(dr=30) r
    q/7 G2+. E3 G3 B3 E4 G4 B4 E5 B4 G4 E4 B3 G3 E3
    h/19 F#1+. F#2 C#3 E3 G3 Bb3 C#4 E4 G4 Bb4 C#5
         E5 G5 Bb5 C#6 E6 G6 Bb6 C#7
    i E7(dr=30) r(2w-i)
    { h hold hold hold hold hold hold hold hold }&
    q/6 F3+. C4 F4 Ab4 C5 F5 Ab5 F5 C5 Ab4 F4 C4
    q/7 Eb3+. C4 F4 Ab4 C5 F5 Ab5 i Ab6(dr=30) r
    q/6 D3+. C4 F4 Ab4 C5 F5 Ab5 F5 C5 Ab4 F4 C4
    q/8 C3+. Ab3 Eb4 F4 Ab4 Eb5 F5 Ab5 i Ab6(dr=30) r
    q/7 B2+. F3 Ab3 D4 F4 Ab4 D5 F5 D5 Ab4 F4 D4 Ab3 F3
    q/8 Bb2+. F3 Ab3 C#4 F4 Ab4 C#5 F5 i F6(dr=30) r
    q/7 Ab2+. F3 Ab3 C4 F4 Ab4 C5 F5 C5 Ab4 F4 C4 Ab3 F3
    h/19 G1+. G2 D3 F3 Ab3 B3 D4 F4 Ab4 B4
         D5 F5 Ab5 B5 D6 F6 Ab6 B6 D7
    i F7(dr=30) r(2w-i)
    {t-=h tempo(LASTVAL) r(h) tempo_to(90)}&
    hold(w)&
    v=80
    w/35 F#1++ F#2 C#3 F#3 A3 C#4 F4 Ab4
    A4 C#5 F5 Ab5 A5 C#6 F6 F#6 A6 C#7 F7 Ab7++
    F#7 C#7 A6 F#6 C#6 A5 F#5 C#5
    A4 F#4 C#4 A3 F#3 C#3 F#2
  }
}
orch {
  violin1 {
    arco
    sexpr(60)
    ..::rollparam = 'speed=q/3 attack=0 decay=0.9 rsustain=0.9'
    { v=60 roll(rollparam)
      q G4 G4 G4 G4 G4 G4 G4 G4
      G4 G4 G4 G4 G4 G4 G4 G4
      G4 Bb4 Bb4 Bb4 Ab4 F#4 F4 E4
      Ab4 Ab4 Ab4 Ab4 Ab4 Ab4 Ab4 Ab4
      Ab4 Ab4 Ab4 Ab4 Ab4 Ab4 Ab4 Ab4
      Ab4 B4 B4 B4 A4 G4 F#4 F4
    }
    v=100
    take(2, q dr=50 F#4++ i. dr=60 Ab4++ s A4 {i dr=30 v+=25 Ab4 F#4} r(q))
  }
  violin2 {
    arco
    { v=60 roll(rollparam)
      q/3 [G3 E4] E4 E4 q E4 E4 E4 E4 E4 E4 E4
      E4 E4 E4 E4 E4 E4 E4 E4
      E4 F4 G4 G4 C#4 C#4 C4 C4
      F4 F4 F4 F4 F4 F4 F4 F4
      F4 F4 F4 F4 F4 F4 F4 F4
      F4 F#4 Ab4 Ab4 D4 D4 C#4 C#4
    }
    v=80
    apply(2, C#4 C#4 C#4 C#4 C#4)
  }
  viola {
    pizz
    v=80
    q E3 r(q+h)
    arco
    h E4 D4 C#4 C4 B3 Bb3
    { v=60 roll(rollparam) q Bb3 C#4 C#4 C#4 Ab3 Bb3 Ab3 G3 }
    q Ab3(dr=30) r(q+h)
    h F4 Eb4 D4 C#4 C4 B3
    { v=60 roll(rollparam) q B3 D4 D4 D4 A3 B3 A3 Ab3 }
    v=80
    apply(2, A3 A3 A3 A3 A3)
  }
  cello {
    arco
    v=80
    h E3 D3 C#3 B2 Bb2 A2 G2 F#2
    v=100
    { sexpr(60) r(w) sexpr_to(80) r(w) sexpr_to(60) }&
    q C4 C#4 h G4 q Ab4 F#4 C4 E4
    v=80
    h F3 Eb3 D3 C3 B2 Bb2 Ab2 G2
    v=100
    { sexpr(60) r(2w) sexpr_to(100) }&
    q C#4 D4 h Ab4 q A4 G4 C#4 F4
    apply(2, F#3 Ab3 A3 Ab3 F#3)
  }
  bass {
    pizz
    v=80
    q E3 r D3 r C#3 r B2 r Bb2 r A2 r G2 r F#2 r
    arco
    q C3 Bb2 h E2 q F2 F#2 i Ab2 Bb2 q C3
    q F2(dr=30) r
    pizz
    q Eb3 r D3 r C3 r B2 r Bb2 r Ab2 r G2 r
    arco
    q C#3 B2 h F2 q F#2 G2 i A2 B2 q C#3
    h+q F#2 r(q)
  }
  flute {
    v=100
    mod(30)
    def(..::expat1) {
      { expr(40) r(s) expr_to(80) r(q-s) expr(80) r(q)
	expr_to(100) r(q) expr_to(80) r(q)
	expr(80) r(q) expr_to(90) r(q) expr_to(80) r(q) expr(80) r(q)
	expr_to(40)}&
    }
    expat1()
    take(0, q E5 i. F#5 s G5 i F#5 q E5 i B4
	 i. E5 s F#5(dr=70) {i dr=50 F#5 G5} h E5)
    r(4w)
    expat1()
    apply(0, F5 G5 Ab5 G5 F5 C5 F5 G5 G5 Ab5 F5)
    mod(0)
  }
  oboe {
    r(4w)
    mod(30)
    { expr(80) r(w) expr_to(100) r(w) expr_to(80) r(q) expr_to(40) }&
    v=100
    q C5 C#5 h G5 q Ab5 Gb5 C5 E5
    q F5 r(q+h+3w)
    { expr(80) r(2w) expr_to(120) }&
    q C#5 D5 h Ab5 q A5 G5 C#5 F5
    q F#5
    mod(0)
  }
  clarinet {
    r(4w)
    v=70
    { expr(80) r(w) expr_to(100) r(w) expr_to(80) r(q) expr_to(40) }&
    r(h) [{h Db5 q Cb5 Db5 Cb5 Bb4 Cb5} {w Fb5 h+q Eb5}]
    r(q+h+3w)
    { expr(80) r(2w) expr_to(120) }&
    r(h) [{h D5 q C5 D5 C5 B4 C5} {w F5 h E5 q E5}]
  }
  fagotto {
    r(4w)
    v=70
    { add_notes(n-=12) 
      { expr(80) r(w) expr_to(100) r(w) expr_to(80) r(q) expr_to(40) }&
      q C4 Bb3 h E3 q F3 F#3 i Ab3 Bb3 q C4
      q F3 r(q+h+3w)
      { expr(80) r(2w) expr_to(120) }&
      q C#4 B3 h F3 q F#3 G3 i A3 B3 q C#4
      q F#3
    }
  }
  horn {
    r(2w)
    mod(30) expat1() v=100
    apply(0, C5 D5 Eb5 D5 C5 Ab4 C5 D5 D5 Eb5 C5)
    mod(0)
    { expr(80) r(w) expr_to(100) r(w) expr_to(80) r(q) expr_to(40) }&
    v=60 r(h) [{h F#4 q E4 F#4 E4 D#4 E4} {w A4 h+q G#4}]
    r(q+h+w)
    mod(30) expat1() v=100
    apply(0, C#5 Eb5 E5 Eb5 C#5 A4 C#5 Eb5 Eb5 E5 C#5) 
    mod(0)
    { expr(80) r(2w) expr_to(120) }&
    v=60 r(h) [{h G4 q F4 G4 F4 E4} {w Bb4 h A4}]
    v=100
    apply(2, F4 F4 F4 F4 F4)& apply(2, A4 A4 A4 A4 A4)
  }
  trumpet {
    r(12w)
    v=100 expr(120)
    apply(2, F#4 Ab4 A4 Ab4 F#4)
  }
  trombone {
    r(12w)
    v=100 expr(120)
    apply(2, F#3 F#3 F#3 F#3 F#3)&
    apply(2, C#3 C#3 C#3 C#3 C#3)& 
    apply(2, F#2 F#2 F#2 F#2 F#2) 
  }
}
  
/***************************************************************
 * measures 102-116
 ***************************************************************/
===

piano {
  rh {
    vmag(1.0)
    v=100 q A4 B4 h E5
    v=70 i A5 A5 B5 B5 h E6++
    v=100 q B4 C#5 h F#5
    v=70 i B5 B5 C#6 C#6 h F#6
    v=90 i C#6 C#6 Eb6 Eb6 h Ab6++
    v=110 i D6 D6 E6 E6 h A6++
    v=90
    q/6 r F3 A3 Eb4 F4 A4
    Eb5 F5 A5 Eb6 F6 A6
    F7 A6 F6 Eb6 A5 F5
    Eb5 A4 F4 Eb4 A3 F3
    q E3++
  }
  lh {
    v=60
    { i r acped r acped r q. acped }&
    z E2++(du=q) A2 C#3 G3 i/3 A3 C#4 G4
    z D2++(du=q) A2 B2 G3 i/3 A3 D4 G4
    z C#2++(du=h) A2 E3 G3 i/3 A3 E4 G4 i A4(dr=30) r(i)

    v=40
    { q hold hold h hold }&
    z E3++ A3 C#4 G4 i/3 A4 C#5 G5
    z D3++ A3 B3 G4 i/3 A4 D5 G5
    z C#3++ A3 E4 G4 i/3 A4 E5 G5 i A5(dr=30) r(i)

    v=60
    { i r acped r acped r q. acped }&
    z F#2++(du=q) B2 Eb3 A3 i/3 B3 Eb4 A4 
    z E2++(du=q) B2 C#3 A3 i/3 B3 E4 A4 
    z Eb2++(du=h) B2 F#3 A3 i/3 B3 F#4 A4 i B4(dr=30) r(i)

    v=40
    { q hold hold h hold }&
    z F#3++ B3 Eb4 A4 i/3 B4 Eb5 A5 
    z E3++ B3 C#4 A4 i/3 B4 E5 A5 
    z Eb3++ B3 F#4 A4 i/3 B4 F#5 A5 i B5(dr=30) r(i)

    v=55
    { q hold hold h hold }&
    z Ab3++ C#4 F4 B4 i/3 C#5 F5 B5 
    z F#3++ C#4 Eb4 B4 i/3 C#5 F5 B5 
    z F3++ C#4 Ab4 B4 i/3 C#5 Ab5 B5 i C#6(dr=30) r(i)

    v=70
    { q hold hold h hold }&
    z A3++ D4 F#4 C5 i/3 D5 F#5 C6 
    z G3++ D4 E4 C5 i/3 D5 G5 C6 
    z F#3++ D4 A4 C5 i/3 D5 A5 C6 i D6(dr=30) r(i)

    v=90
    hold(w)&
    q/6 F1++ F2 C3 F3 A3 Eb4
    F4 A4 Eb5 F5 A5 Eb6
    F6 Eb6 A5 F5 Eb5 A4
    F4 Eb4 A3 F3 C3 F2
    q [E1 E2]++ r(q+h)

    v=110
    { add_notes(n-=12) 
      {q hold hold hold hold hold }&
      { rtempo(0.8) r(q/3) rtempo(1.1)}& 
      q/3 [E6 E7]++ [E6 E7] [Eb6 Eb7]
      [B5 B6]++ [B5 B6] [A5 A6]
      [E5 E6]++ [E5 E6] [Eb5 Eb6]
      [B4 B5]++ [B4 B5] [A4 A5]
      rtempo(1.0)
      q [E5 Ab5 B5 E6]++ r(q+h)
      { rtempo(0.6) r(q/3) rtempo(1.1)}& 
      s [E6 E7]++ [F6 F7] [E6 E7] [Eb6 Eb7]
      [B5 B6]++ [C6 C7] [B5 B6] [A5 A6]
      [E5 E6]++ [F5 F6] [E5 E6] [Eb5 Eb6]
      [B4 B5]++ [C5 C6] [B4 B5] [A4 A5]
      { rtempo(1.0) r(3w) rtempo(1.0) r(w) rtempo_to(0.7) }&
      q [E5 Ab5 B5 E6]++
    }
  }
}
orch {
  violin1 {
    sexpr(60)
    v=60
    r(w)
    take(0, q dr=20 G4 G4 dr=100 G4 r)
    r(w)
    apply(0, A4 A4 A4)
    sexpr(70)
    apply(0, B4 B4 B4) 
    sexpr(80)
    apply(0, C5 C5 C5)
    
    v=80
    { sexpr(80) r(w) sexpr_to(120) }&
    h. C5 q A4 q Ab4++(dr=50) r(q+h)
    { sexpr(80) r(w) sexpr_to(120) }&
    w A4 q Ab4++(dr=50) r(q+h)
    { sexpr(80) r(w) sexpr_to(120) }&
    w A4
    { r(q) sexpr_to(70) r(w-q+w) sexpr(60) r(2w) sexpr_to(40) }&
    q Ab4++(dr=50) r(q+h) r(w)
    w E4 D4
  }
  violin2 {
    v=80
    r(w)
    apply(0, C#4 D4 E4)
    r(w)
    apply(0, Eb4 E4 F#4)
    apply(0, F4 F#4 Ab4)
    apply(0, F#4 G4 A4)

    v=80
    h. A4 q D#4 E4++(dr=50) r(q+h) 
    h. E4 q D#4 E4++(dr=50) r(q+h) 
    h. E4 q D#4 

    dr=70 q E4 i. F4 s G4
    i F4(dr=40) dr=100 q E4 i F4
    w F#4 G4 G#4
  }
  viola {
    v=60
    r(w)
    apply(0, A3 A3 A3)
    r(w)
    apply(0, B3 B3 B3)
    apply(0, C#4 C#4 C#4)
    apply(0, D4 D4 D4)

    v=80
    h. D#4 q A3 G#3++(dr=50) r(q+h) 
    w A3 q G#3++(dr=50) r(q+h) 
    w A3

    dr=70 q G#3 i. A3 s B3
    i A3(dr=40) dr=100 q+i G#3
    w A3 2w E3
  }
  foreach($i, #('cello', 'bass')) { $i {
    v=60
    r(w)
    apply(0, E3 D3 C#3)
    r(w)
    apply(0, F#3 E3 Eb3)
    apply(0, Ab3 F#3 F3)
    apply(0, A3 G3 F#3)

    v=80
    w F3 q E3++(dr=50) r(q+h) 
    w F3 q E3++(dr=50) r(q+h) 
    w B2 

    w E3 D3 C3 B2
  }}
  flute {
    r(7w)
    v=100
    take(1, dr=70 q [Ab5 E6] i. [A5 F6] s [B5 G6]
	 i [A5 F6](dr=40) q [Ab5 E6] r(i))
    r(w)
    apply(1, [Ab5 E6] [A5 F6] [B5 G6] [A5 F6] [Ab5 E6])
  }
  oboe {
    r(6w)
    { wexpr(80) r(w) wexpr_to(110) }&
    v=110
    q C6 i B5 A5 G5 F5 E5 Eb5
    wexpr(90)
    v=127
    apply(1, [Ab4 E5] [A4 F5] [B4 G5] [A4 F5] [Ab4 E5])
    r(w)
    apply(1, [Ab4 E5] [A4 F5] [B4 G5] [A4 F5] [Ab4 E5])
  }
  clarinet {
    r(6w)
    v=110
    q Eb5 i D5 C5 Bb4 Ab4 G4 F#4
    v=127
    apply(1, [B4 G5] [C5 Ab5] [D5 Bb5] [C5 Ab5] [B4 G5])
    r(w)
    apply(1, [B4 G5] [C5 Ab5] [D5 Bb5] [C5 Ab5] [B4 G5])
    r(w)

    { wexpr(90) r(w) wexpr(90) r(3w) wexpr_to(50) }&
    v=100
    take(2, dr=70 q [B3 G4] i. [C4 Ab4] s [D4 Bb4]
	 i [C4 Ab4](dr=40) dr=100 [q+i B3 {q G4 i Ab4}])
    w [C4 A4] [C#4 Bb4] [D4 B4]
  }
  fagotto {
    r(7w)
    v=127
    apply(1, [Ab2 E3] [A2 F3] [B2 G3] [A2 F3] [Ab2 E3])
    r(w)
    apply(1, [Ab2 E3] [A2 F3] [B2 G3] [A2 F3] [Ab2 E3])
    r(w)

    v=100
    apply(2, [Ab2 E3] [A2 F3] [B2 G3] [A2 F3] [Ab2 {E3 F3}])
    w [A2 F#3] [Bb2 G3] [B2 Ab3]
  }
  horn {
    r(7w)
    v=110
    {q/3 dr=50 [C4 C5] [C4 C5] [C4 C5] [C4 C5] [C4 C5] [C4 C5]
      [C4 C5] [C4 C5] [C4 C5] [C4 C5] [C4 C5] [C4 C5]}
    r(w)
    {q/3 dr=50 [C4 C5] [C4 C5] [C4 C5] [C4 C5] [C4 C5] [C4 C5]
      [C4 C5] [C4 C5] [C4 C5] [C4 C5] [C4 C5] [C4 C5]}
    r(2w)
    v=90
    take(4, q [C4 C5]+.(dr=50) i. [C4 C5](dr=90) s [C4 C5](dr=50)
	 i [C4 C5](dr=40) q [C4 C5]+.(dr=80) i [C4 C5](dr=50))
    r(w)
    v=60
    apply(4, [C4 C5] [C4 C5] [C4 C5] [C4 C5] [C4 C5] [C4 C5]) 
  }
  trumpet {
    r(7w)
    bexpr(110)
    v=110
    {q/3 dr=50 [E4 E5] [E4 E5] [E4 E5] [E4 E5] [E4 E5] [E4 E5]
      [E4 E5] [E4 E5] [E4 E5] [E4 E5] [E4 E5] [E4 E5]}
    r(w)
    {q/3 dr=50 [E4 E5] [E4 E5] [E4 E5] [E4 E5] [E4 E5] [E4 E5]
      [E4 E5] [E4 E5] [E4 E5] [E4 E5] [E4 E5] [E4 E5]}
    r(w)
    v=70 bexpr(80)
    {q/3 dr=50 [E4 E5] [E4 E5] [E4 E5] [E4 E5] [E4 E5] [E4 E5]
      [E4 E5] [E4 E5] [E4 E5] [E4 E5] [E4 E5] [E4 E5]}
    r(w)
    v=80
    apply(4, [E4 E5] [E4 E5] [E4 E5] [E4 E5] [E4 E5] [E4 E5])
  }
  trombone {
    r(7w)
    v=127 
    E2(w)&
    apply(1, [Ab3 E4] [A3 F4] [B3 G4] [A3 F4] [Ab3 E4])
    r(w)
    E2(w)&
    apply(1, [Ab3 E4] [A3 F4] [B3 G4] [A3 F4] [Ab3 E4])
  }
  timpani {
    r(7w)
    v=80
    q/3 E3+++ E3 E3 E3 E3 E3 E3 E3 E3 E3 E3 E3
    r(w)
    q/3 E3+++ E3 E3 E3 E3 E3 E3 E3 E3 E3 E3 E3
    r(w)
    v=60
    {r(2w) vmag(1.0) r(2w) vmag_to(0.7)}&
    E3(4w roll())
    vmag(1.0)
  }
}
