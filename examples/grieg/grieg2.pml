/*
 *  Piano Concerto Op.16         Edvard Grieg
 *      1st movement  measures 7-73    (if `times' = 1)
 *                    measures 117-170 (if `times' = 2)
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

if( !defined(times) ) { times = 1 }

r(q)   // Space for Auftakt

/***************************************************************
 * measures 7-18 and 117-128
 ***************************************************************/
===
tempo(84)
rtempo(1.0)

def(pianorh1) { 
  v=50
  {
    add_notes(n-=12 v-=30) ac=30
    q slow(0.7) [A4 C5 E5++] i. [A4 D5 F5++] s [A4 B4 D5 G5++]-.
    i [A4 D5 F5++](dr=30) q [A4 C5 E5++](v-=10) r(i)
    i. [A4 C5 E5++] s [A4 D5 F5++]
    {i dr=30 [A4 D5 F5++] slow(0.8) [A4 B4 D5 G5++]-.}
    q [A4 C5 E5++](v-=10) r(q)
    q slow(0.7) [C5 E5 G5++] i. [C5 F5 A5++] s [C5 D5 F5 B5++]-.
    i [C5 F5 A5++](dr=30) q [C5 E5 G5++]-. r(i)
    i. [C5 E5 G5++] s [C5 F5 A5++]
    {i dr=30 [C5 F5 A5++] slow(0.8) [C5 D5 F5 B5++]-.}
    q [C5 E5 G5++](v-=10) r(q)
  }
  
  v=80
  if(times==1){slow(0.8)}
  q E4 F4
  if(times==1){slow(0.7)}
  h B4 q A4 Bb4
  if(times==1){slow(0.6)}
  h E5
  q [D5--- F5] [C5--- E5] 
  if(times==1){slow(0.6)}
  [h F4--- {q/3 A4 B4 C5 i B4 D5}]
  if(times==1){slow(0.7)}
  q Ab4 A4 rtempo(1.0) C5 B4-.
  if(times==1){rtempo_to(0.8)} else{rtempo_to(0.9)}
  rtempo(1.0)
}

def(pianolh1) {
  pmag(1.0)
  v=50
  {t-=i hold(q+i)& [A2 E3]}& r(w)
  {t-=i hold(q+i)& [A2 E3]}& r(w)
  {t-=i hold(q+i)& [C3 G3]}& r(w)
  {t-=i hold(q+i)& [C3 G3]}& r(w)
  
  v=40 pmag(0.8)
  hold(q)& {q/5 E2++ B2 E3 Ab3 D4}
  hold(q)& {q/5 D2++ B2 F3 A3 D4}
  hold(h)& {q/6 G#1++ G#2 D3 F3 B3 D4 pmag_pt(1.1) F4 D4 B3 F3 D3 G#2}
  pmag_pt(0.9)
  hold(q)& {q/5 A2++ E3 A3 C#4 G4}
  hold(q)& {q/5 G2++ E3 Bb3 D4 G4}
  hold(h)& {q/6 C#2++ C#3 G3 Bb3 E4 G4 pmag_pt(1.2) Bb4 G4 E4 Bb3 G3 C#3}
  pmag_pt(0.9)
  hold(q)& {q/5 D3++ A3 D4 F4 A4}
  hold(q)& {q/5 A2++ F3 C4 F4 A4}
  hold(h)& {q/6 D2++ A2 D3 F3 A3 B3 q/4 D4 B3 A3 F3}
  hold(q)& {q/6 F#2++ C3 Eb3 Ab3 C4 Eb4}
  hold(q)& {q/6 F2++ B2 Eb3 A3 B3 Eb4}
  hold(q)& {q/6 E2++ E3 F#3 C4 D#4 F#4}
  pmag_pt(0.8)
  hold(q)& {q/6 E2++ E3 Ab3 B3 E4 Ab4}
  pmag_cto(0.6)
}
  
orch {
  if(times==2) {
    rh {
      pianorh1()
    }
    lh { 
      pianolh1()
    } 
  }

  if(times==1) { aexpr(80) } else { aexpr(50) }
  violin1 {
    arco v=80
    take(0, r(h+i) i du=z C4 D4 E4 q C4 R i. C4(du=i) s D4 i D4 E4 q E4 R 
	     i R E4 F4 G4 q E4 R i. E4(du=i) s F4 i F4 G4)
    take(1, du=s v-=10 i r E4 r F4 r B4 r B4 r A4 r Bb4 r E5 r E5
	     r F5 r E5 r A4 r(q) r Ab4 r A4 r C5 r B4)
    v=80
    {add_notes(n+=12 v-=40) q E4 F4 h B4 q A4 Bb4 h E5 
      q F5 E5 q/3 A4 B4 C5 i B4 D5}
    v=80
    take(3, q. Ab4 i [A4 X] dr=50 [C5 X] r Ab4 r)
  }
  violin2 {
    arco v=60
    apply(0, A3 A3 B3 A3 A3 A3 A3 B3 C4 C4 C4 D4 C4 C4 C4 C4 D4)
    apply(1, D4 D4 F4 F4 G4 G4 Bb4 Bb4 A4 C5 F4 Eb4 Eb4 E4 E4)
    v=80
    { roll(speed=z attack=0 decay=1)
      s r D4 E4 Ab4 A4 D4 F4 A4 B4 D4 F4 B4 B4 D4 F4 B4
      r G4 A4 C#5 D5 G4 Bb4 D5 E5 G4 Bb4 E5 E5 G4 Bb4 E5
      r F4 A4 D5 E5 A4 C5 E5} 
    i F5 r(i+q)
    apply(3, Eb4 [Eb4 A4] [E4 A4] D4)
  }
  viola {
    arco v=60
    apply(0, E3 F#3 Ab3 E3 E3 F#3 F#3 G3 G3 G3 A3 B3 G3 G3 A3 A3 B3)
    apply(1, Ab3 A3 D4 D4 C#4 D4 G4 G4 F4 F4 A3 C4 C4 A4 Ab4)
    v=80
    { roll(speed=z attack=0 decay=1)
      s r Ab3 D4 E4 F4 A3 D4 F4 F4 B3 D4 F4 F4 B3 D4 F4
      r C#4 G4 A4 Bb4 D4 G4 Bb4 Bb4 E4 G4 Bb4 Bb4 E4 G4 Bb4
      r D4 F4 A4 C5 E4 A4 C5}
    i D5 r(i+q)
    apply(3, C4 [C4 X] [E4 X] B3)
  }
  cello {
    arco v=60
    apply(0, A2 A2 A2 A2 A2 A2 A2 G2 C3 C3 C3 C3 C3 C3 C3 C3 B2)
    apply(1, E3 D3 Ab2 Ab3 A3 G3 C#3 C#4 D4 A3 D3 F#3 F3 E3 E3)
    v=80
    q E3 D3 h Ab2 q A3 G3 h C#3 q D4 A3 D3 r
    apply(3, F#3 [F3 X] [E3 X] E2) 
  }
  bass {
    arco v=80
    apply(0, A2 A2 A2 A2 A2 A2 A2 G2 C3 C3 C3 C3 C3 C3 C3 C3 B2)
    apply(1, E3 D3 Ab2 Ab3 A3 G3 C#3 C#4 D4 A3 D3 F#3 F3 E3 E3)
    q E3 D3 h Ab2 q A3 G3 h C#3 q D4 A3 D3 r
    apply(3, F#3 [F3 X] [E3 X] E2) 
  }
  oboe {
    if(times==1) {
      v=80 take(0, q dr=80 E5 i. F5 s G5(dr=50) i F5(dr=30) E5(dr=150) r(q))&
      v=50 apply(0, A4 A4 A4 A4 A4)
      v=80 take(1, i. dr=80 E5 s F5 i dr=30 F5 G5 q dr=100 E5 r)&
      v=50 apply(1, A4 A4 A4 A4 A4)
      v=80 apply(0, G5 A5 B5 A5 G5)&
      v=50 apply(0, C5 C5 C5 C5 C5)
      v=80 apply(1, G5 A5 A5 B5 G5)&
      v=50 apply(1, C5 C5 C5 C5 C5)
    } else { r(4w) }
    r(4w)
    v=80
    aexpr(50)
    take(3, q E4 F4 h B4 q A4 Bb4 h E5 aexpr_pt(110)
	  q F5 E5 aexpr_cto(80) q/3 slow(0.7) A4 B4 C5 i B4 D5 aexpr(80)
	  q G#4 aexpr_pt(70) r(3q) aexpr_cto(50))
  }
  flute {
    if(times==1) {
      v=60
      apply(0, E5 F5 D5 F5 E5)& apply(0, C5 D5 B4 D5 C5)
      apply(1, E5 F5 F5 D5 E5)& apply(1, C5 D5 D5 B4 C5)
      apply(0, G5 A5 F5 A5 G5)& apply(0, E5 F5 D5 F5 E5)
      apply(1, G5 A5 A5 F5 G5)& apply(1, E5 F5 F5 D5 E5)
    } else { r(4w) }
    r(4w)
    v=60
    apply(3, E5 F5 B5 A5 Bb5 E6 F6 E6 A5 B5 C6 B5 D6 Ab5) 
  }
  clarinet {
    if(times==1) {
      v=80
      apply(0, G4 Ab4 Bb4 Ab4 G4)
      apply(1, G4 Ab4 Ab4 Bb4 G4)
      apply(0, Bb4 C5 D5 C5 Bb4)
      apply(1, Bb4 C5 C5 D5 Bb4) 
      v=100
      aexpr(60)
      mod(10)
      take(2, q G4 Ab4 h D5 q C5 C#5 h G5 aexpr_pt(100)
	   q Ab5 G5 aexpr_cto(60) q/3 slow(0.7) C5 D5 Eb5 i D5 F5 aexpr(60)
	   q. B4 i C5 q aexpr_pt(80) Eb5 D5 aexpr_cto(40)<<)
      mod(0)
    } else { r(8w) }
    v=80
    {h F4 Ab4 Bb4 C#5 q C5 C5 C5}&
    {q B3 C4 h F4 q E4 F4 h Bb4 q Ab4 Ab4 Ab4}
    r(q+w)
  }
  fagotto {
    if(times==1) {
      v=50
      apply(0, E4 F4 D4 F4 E4)& apply(0, C4 D4 B3 D4 C4) 
      apply(1, E4 F4 F4 D4 E4)& apply(1, C4 D4 D4 B3 C4)
      apply(0, G4 A4 F4 A4 G4)& apply(0, E4 F4 D4 F4 E4)
      apply(1, G4 A4 A4 F4 G4)& apply(1, E4 F4 F4 D4 E4)
      v=100
      mod(10)
      apply(2, E3 F3 B3 A3 Bb3 E4 F4 E4 A3 B3 C4 B3 D4 Ab3 A3 C4 B3)
      mod(0)
    } else { r(8w) }
    v=80
    apply(3, E3 F3 B3 A3 Bb3 E4 F4 E4 A3 B3 C4 B3 D4 Ab3)&
    {h B2 D3 E3 G3 q D4 C4 F3}
    r(q+w)
  }
  horn {
    if(times==1) {
      v=50
      apply(0, F4 F4 F4 F4 F4)
      apply(1, F4 F4 F4 F4 F4)
      apply(0, Ab4 Ab4 Ab4 Ab4 Ab4)
      apply(1, Ab4 Ab4 Ab4 Ab4 Ab4)
    } else { r(4w) }
    r(4w)
    v=80
    h r [G4 Bb4] q C4 C4 h [Eb4 C5] q [C#4 F4] [C#4 F4] [C#4 F4]
    r(q+w)
  }
}

/**********/
if(times==1) {
/***************************************************************
 * measures 19-30
 ***************************************************************/
===
piano {
  rh {
    pianorh1()
    v=95
    q slow(0.7) [E4-- E5] [F4-- F5] slow(0.7) h [B4-- B5]>.
    q [A4-- A5] [Bb4-- Bb5] slow(0.6) h [E5-- E6]>.
    q [D6--- F6] [C6--- E6] slow(0.6) [h F5--- {q/3 A5 B5 C6 i B5 D6}]
    v=80
    take(0, q [Ab4++ C5 Eb5 Ab5] dr=30 i r [C4 Eb4 A4++]--
	    [E4 A4 C5++] r [B3 D4 Ab4++] r)
  }
  lh {
    pianolh1()
    v=50
    pmag(0.8)
    hold(q)& {q/5 E3++ B3 E4 Ab4 D5}
    hold(q)& {q/5 D3++ B3 F4 A4 D5}
    hold(h)& {q/6 G#2++ G#3 D4 F4 B4 D5 pmag_pt(1.1) F5 D5 B4 F4 D4 G#3}
    pmag_pt(0.9)
    hold(q)& {q/5 A3++ E4 A4 C#5 G5}
    hold(q)& {q/5 G3++ E4 Bb4 D5 G5}
    hold(h)& {q/6 C#3++ C#4 G4 Bb4 E5 G5 pmag_pt(1.2) Bb5 G5 E5 Bb4 G4 E4}
    pmag_pt(0.9)
    hold(q)& {q/5 D4++ A4 D5 F5 A5}
    hold(q)& {q/5 A3++ F4 C5 F5 A5}
    pmag_pt(0.8)
    hold(h)& {q/6 D2++ A2 D3 F3 A3 B3 q/4 D4 B3 A3 F3}
    pmag_cto(0.6)
    apply(0, [F#3 C4 Eb4] [F2 F3] [E2 E3] [E2 E3])
  } 
}

orch {
  aexpr(60)
  violin1 {
    v=80
    take(0, i A4 r(i+q+2h)
	 du=z i. C4(du=i) s D4 i D4 E4 q E4 r(q+2h) 
	 du=z i. E4(du=i) s F4 i F4 G4 q E4 r(q+h+3w) )
    aexpr(60) q E4 F4 h B4 q A4 Bb4 h E5
    aexpr_to(90) q F5 E5 A4 aexpr_to(40) r(q+h+q) i Ab4(dr=30) r
  }
  violin2 {
    v=60
    apply(0, C4 A3 A3 A3 B3 C4 C4 C4 C4 D4 E4)
    h D4 F4 G4 Bb4 q A4 C5 F4 r(q+h+q) i E4(dr=30) r
  }
  viola {
    v=60
    apply(0, A3 E3 F#3 F#3 G3 G3 G3 A3 A3 B3 Ab3)
    q Ab3 A3 h D4 q C#4 D4 h G4 h F4 q A3 r(q+h+q) i [B3 D4](dr=30) r
  }
  cello {
    v=60
    apply(0, A2 A2 A2 A2 G2 C3 C3 C3 C3 B2 E3)
    q E3 [D3 F3] h [Ab2 B3] q A3 [G3 Bb3] h [C#3 E4]
    q [D4 F4] [A3 E4] [D3 A3] r(q+h+q) i E3(dr=30) r
  }
  bass {
    v=80
    apply(0, A2 A2 A2 A2 G2 C3 C3 C3 C3 B2 E3)
    pizz
    v=60
    q E3 D3 Ab2 r A3 G3 C#3 r D4 A3 D3 r r(h+q) arco i E3(dr=30) r
  }
}

/**********/
}
/***************************************************************
 * measures 31-42 and 129-140
 ***************************************************************/
===
tempo(100)(dt-=z)
tempo_to(116)(dt+=h)

piano {
  pmag(1.0)
  rh {
    v=80
    def(p1) { z $1(v-=30) s $2(dr=30) r(z) }
    t-=z
    { deemphasize(20)
      p1([C6 F6], [A5 E6]) p1([F5 C6], [E5 A5]) p1(C6, B5) p1(Ab5, E5)
      p1([C5 F5], [A4 E5]) p1([F4 C5], [E4 A4]) p1(C5, B4) p1(Ab4, E4)
      p1(C4, F4) p1(Eb4, E4) p1(Ab4, A4) p1([E4 B4], [A4 C5])
      p1([A4 Eb5], [C5 E5]) p1([C5 Ab5], [E5 A5])
      p1([E5 B5], [A5 C6]) p1([A5 Eb6], [C6 E6]) r(z)
    }

    def(piano::p2) { {i/3 $1(v-=20) $2(v-=15) $3(v-=10)} i $4(dr=30) }
    r(i)
    p2(F6, C7, E7, [B6 D7])
    p2(D6, A6, C7, [Ab6 B6])
    p2(B5, F6, A6, [E6 Ab6])
    p2(Ab5, E6, G6, [D6 F6])
    p2(F5, C6, E6, [B5 D6])
    p2(D5, A5, C6, [Ab5 B5])
    p2(B4, F5, A5, [E5 Ab5])
    i [E5 E6](dr=50)
    
    t-=z
    { deemphasize(20)
      p1([C6 F6], [A5 E6]) p1([F5 C6], [E5 A5]) p1(C6, B5) p1(Ab5, E5)
      p1([C5 F5], [A4 E5]) p1([F4 C5], [E4 A4]) p1(C5, B4) p1(Ab4, E4)
      p1(C4, F4) p1(Eb4, E4)
      if(times==1) {
	p1(Ab4, A4) p1([E4 B4], [A4 C5])
	p1([A4 D5], [C5 Eb5]) p1([C5 Ab5], [Eb5 A5])
	p1([Eb5 B5], [A5 C6]) p1([G5 D6], [C6 Eb6])
	p1([A5 G6], [D6 F#6]) r(z)
      } else {
	p1(B4, C5) p1([A4 Eb5], [C5 E5])
	p1([C5 Ab5], [E5 A5]) p1([E5 B5], [A5 C6])
	p1([A5 Eb6], [C6 E6]) p1([C6 Ab6], [E6 A6])
	p1([F#5 E6], [B5 Eb6]) r(z)
      }
    }

    if(times==2) {tp-=3}

    p2(Eb6, Bb6, D7, [A6 C7])
    p2(C6, G6, Bb6, [F#6 A6])
    p2(A5, Eb6, G6, [D6 F#6])
    p2(F#5, D6, F6, [C6 Eb6])
    p2(Eb5, Bb5, D6, [A5 C6])
    p2(C5, G5, Bb5, [F#5 A5])
    p2(A4, Eb5, G5, [D5 F#5])
    i [D5 D6](dr=50)

    [{i Eb6 D6} {i/3 D6++ C6 Bb5 Bb5++ A5 G5}]
    t-=z
    { deemphasize(20)
      p1(Bb5, A5) p1(F#5, D5)
      p1([Eb5 F#5], [E5 G5]) p1([G5 Bb5], [E5 D6])
      p1([Eb5 F#5], [E5 G5]) p1([G5 Bb5], [E5 D6]) r(z)
    }

    [{i Bb6 A6} {i/3 A6++ G6 F6 F6++ E6 D6}]
    t-=z
    { deemphasize(20)
      p1(F6, E6) p1(C#6, A5)
      p1([A5 C#6], [B5 D6]) p1([D6 F6], [B5 A6])
      p1([A5 C#6], [B5 D6]) p1([D6 F6], [B5 A6]) z [A5 C#6]
    }
    { deemphasize(20)
      s [B5 D6](dr=50)
      slow(0.8) [D7 F7]++ [C#7 E7] [C7 Eb7]
      [B6 D7] [Bb6 C#7] [A6 C7] [Ab6 B6]
      [G6 Bb6] [F#6 A6] [F6 Ab6] [E6 G6]
      [Eb6 F#6] [D6 F6] [C#6 E6] [C6 Eb6]
      tempo(LASTVAL)
      [B5 D6] [Bb5 C#6] [A5 C6] [Ab5 B5]
      [G5 Bb5] [F#5 A5] [F5 Ab5] [E5 G5]
      [Eb5 F#5] [D5 F5] [C#5 E5] [C5 Eb5]
      [B4 D5] [Bb4 C#5] [A4 C5] [Ab4 B4]
      tempo_to(70)
    }
  }
  lh {
    v=50 i
    pmag(1.0)
    [A2 Eb3](hold) {dr=50 [E3 C4] [F#3 A3 D4]++ [Ab3 B3 E4]}
    [A2 Eb3](hold) {dr=50 [E3 C4] [F#3 A3 D4]++ [Ab3 B3]}
    pmag(0.8)
    [A2 Eb3](hold) [E3 C4] [A3 C4] [Ab3 A3 C4]
    [G3 A3 C4] [F#3 A3 C4] [F3 A3 C4] [E3 A3 C4]
    pmag_to(1.2)
    i [E2 E3](hold)+++
    {q hold hold hold hold hold hold hold }&
    p2(E6, C6, F5, [B5 D6])
    p2(C6, A5, D5, [Ab5 B5])
    p2(A5, F5, B4, [E5 Ab5])
    p2(G5, E5, Ab4, [D5 F5])
    p2(E5, C5, F4, [B4 D5])
    p2(C5, A4, D4, [Ab4 B4])
    p2(A4, F4, B3, [E4 Ab4])
    i [E2 E3](dr=50)

    pmag(1.0)
    i {dr=30 [A2 Eb3](hold) [E3 C4]} {dr=50 [F#3 A3 D4]++ [Ab3 B3 E4]}
    {dr=30 [A2 Eb3](hold) [E3 C4]} {dr=50 [F#3 A3 D4]++ [Ab3 B3]}
    pmag(0.8)
    [A2 Eb3](hold) [E3 C4]
    if(times==1) {
      [A3 C4] [Ab3 A3 C4] [G3 A3 C4] [F#3 A3 C4] [F3 A3 C4] [Eb3 G3 C4]
    } else {
      [C4 E4] [B3 C4 E4] [A3 C4 E4] [Ab3 C4 E4] [G3 C4 E4] [F#3 C4 E4]
    }
    pmag_to(1.2)
    if(times==2) {tp-=3}
    i [D2 D3](hold)+++
    {q hold hold hold hold hold hold hold }&
    p2(F6, Bb5, Eb5, [A5 C6])
    p2(Bb5, G5, C5, [F#5 A5])
    p2(G5, Eb5, A4, [D5 F#5])
    p2(F5, D5, F#4, [C5 Eb5])
    p2(D5, Bb4, Eb4, [A4 C5])
    p2(Bb4, G4, C4, [F#4 A4])
    p2(G4, Eb4, A3, [D4 F#4])
    r(i)

    pmag(1.0)
    {
      dr=30 i
      [G3 C#4](hold) [D4 Bb4] [E4 G4 C5]++ [F#4 A4 C5]
      G3 [D4 E4 Bb4] G3 [D4 E4 Bb4]
      [D4 Ab4](hold) [A4 F5] [B4 D5 G5]++ [C#5 E5 G5]
      pmag(1.0)
      D4 [A4 B4 F5] D4 [A4 B4 F5]
      pmag_to(1.2)
      {q+i hold}& D4 [A4 B4 F5] [B4 F5 A5] r(i+h)
      pmag_to(1.0)
      {q+i hold}& [D2 D3] [A3 B3 F4] [B3 F4 A4] r(i+h)
      pmag_to(0.6)
    }
    
  }
}
orch {
  sexpr(60)
  violin1 {
    v=80
    take(0, q dr=50 A3 r A3 r A3 r(q+h) Ab3
	 i r du=s C6 q B5 i r G5 q F5 i r C5 B4 A4 Ab4 X++)
    if(times==1) {
      apply(0, A3 A3 A3 F#4 Bb5 A5 F5 Eb5 Bb4 A4 G4 F#4)
    } else {
      apply(0, A3 A3 A3 D#4 G5 F#5 D5 C5 G4 F#4 E4 D#4)
      tp-=3
    }
      
    r(h+i) i
    { du=s
      Bb4(bgrace(A4)) D5(bgrace(C#5)) Bb4(bgrace(A4)) D5(bgrace(C#5)) 
      r(i+q+i)
      F5(bgrace(E5)) A5(bgrace(G#5)) F5(bgrace(E5)) A5(bgrace(G#5))
    }
    pizz
    sexpr(80)
    r(i) A5(v=127)
  }
  violin2 {
    v=60
    apply(0, A3 A3 A3 D4 A5 Ab5 E5 D5 A4 Ab4 F4 E4)
    if(times==1) {
      apply(0, A3 A3 A3 C4 G5 F#5 D5 C5 G4 F#4 Eb4 D4)
    } else {
      apply(0, A3 A3 A3 B4 E5 D#5 B4 A4 E4 D#4 C4 B3)
      tp-=3
    }
    r(h+i) i
    { du=s
      G4(bgrace(F#4)) Bb4(bgrace(A4)) G4(bgrace(F#4)) A4
      r(i+q+i)
      D5(bgrace(C#5)) F5(bgrace(E5)) D5(bgrace(C#5)) F5(bgrace(E5))
    }
    pizz
    r(i) F5(v=127)
  }
  viola {
    v=60
    apply(0, A3 A3 A3 B3 D5 D5 Ab4 Ab4 D4 D4 B3 B3 E4)
    if(times==1) {
      apply(0, A3 A3 A3 A3 C5 C5 F#4 F#4 C4 C4 A3 A3)
    } else {
      apply(0, A3 A3 A3 F#3 A4 A4 D#4 D#4 A3 A3 F#3 F#3)
      tp-=3
    }
    pizz
    r(h+i) i
    E4 E4 E4 F4 r(i+q+i) B4 B4 B4 B4 r(i) B4(v=127)
  }
  cello {
    v=60
    apply(0, A2 A2 A2 E3 X X X X X X X X E3)
    if(times==1) {
      apply(0, A2 A2 A2 D3)
    } else {
      apply(0, A2 A2 A2 B2)
      tp-=3
    }
    pizz
    r(h+i) i
    D4 G3 D4 D3 r(i+q+i) A4 D4 A4 D4 r(i) D4(v=127)
  }
  bass {
    v=80
    apply(0, A2 A2 A2 E3 X X X X X X X X E3)
    if(times==1) {
      apply(0, A2 A2 A2 D3)
    } else {
      apply(0, A2 A2 A2 B2)
      tp-=3
    }
  }
  flute {
    v=80
    take(0, r(w+q) [A4 C5](h.) i r dr=50 [C6 E6] [B5 D6] 
	 r r [F5 A5] [E5 Ab5] r r [C5 E5] [B4 D5] r(i+h))
    if(times==1) {
      apply(0, [A4 C5] [Bb5 D6] [A5 C6] [Eb5 G5] [D5 F#5]
	    [Bb4 D5] [A4 C5])
    } else {
      apply(0, [C5 E5] [G5 B5] [F#5 A5] [C5 E5] [B4 Eb5]
	    [G4 B4] [F#4 A4])
      tp-=3
    }
  }
  oboe {
    v=80
    {wexpr(60) r(w) wexpr(40) r(w) wexpr_to(100) wexpr(80) r(w) wexpr_to(70)}&
    take(1, r(w+i) i E4 A4 Ab4 G4 F#4 F4 E4 
         dr=50 if(times==1) {r F5 F5 r r B4 B4 r r F4 F4 r} else {r(3h)} r(h))
    {wexpr(60) r(w) wexpr(40) r(w) wexpr_to(100) wexpr(80) r(w) wexpr_to(70)}&
    if(times==1) {
      apply(1, E4 A4 Ab4 G4 F#4 F4 Eb4 Eb5 Eb5 A4 A4 Eb4 Eb4)
    } else {
      v-=10
      apply(1, E4 C5 B4 A4 G#4 G4 F#4)
      tp-=3
    }
  }
  clarinet {
    v=80
    apply(0, [C4 Eb4]
	  if(times==1) {[Eb5 G5] [D5 F5] [Ab4 C5] [G4 B4] [Eb4 G4] [D4 F4]}
          else {[Ab5 X] [Ab5 X] [D5 X] [D5 X] [Ab4 X] [Ab4 X]})
    if(times==1) {
      apply(0, [C4 Eb4] [C#5 F5] [C5 Eb5] [F#4 Bb4] [F4 A4]
	    [C#4 F4] [C4 Eb4])
    } else {
      apply(0, [Eb4 G4] [Eb5 X] [Eb5 X] [A4 X] [A4 X] [Eb4 X] [Eb4 X])
      tp-=3
    }
  }
  fagotto {
    v=80
    apply(1, E3 A3 Ab3 G3 F#3 F3 E3 if(times==1){F4 F4 B3 B3 F3 F3})
    if(times==1) {
      apply(1, E3 A3 Ab3 G3 F#3 F3 Eb3 Eb4 Eb4 A3 A3 Eb3 Eb3)
    } else {
      apply(1, E3 C4 B3 A3 G#3 G3 F#3)
      tp-=3
    }
  }
  horn {
    v=80
    r(2w)
    { expr(120) r(q) expr_to(80) }&
    w+q [C4 C5](if(times==2) {du=w+h+i})
    r(q+h+2w)
    if(times==2) {tp-=3}
    { expr(120) r(q) expr_to(80) }&
    w+q [Bb3 Bb4](if(times==2) {du=w+h+i})
  }
  trumpet {
    if(times==2) {tp-=3}
  }
  trombone {
    if(times==2) {tp-=3}
  }
}

/***************************************************************
 * measures 43-52 and 141-150
 ***************************************************************/
===
tempo(84)

piano {
  rh {
    pmag(0.8)
    v=90
    i slow(0.5)
    C5 A4 q/3 slow(0.8) G4(bgrace(G4 A4)) F4 A4 i C5 F5-- slow(0.8) F5(q+i)
    C5 q/3 slow(0.8) B4(bgrace(B4 C5)) A4 C5 i F5 A5-- slow(0.8) A5(q+i)
    F5 q/3 slow(0.8) E5(bgrace(E5 F5)) D5 F5-- 
    slow(0.7)
    for($i,1,2) { q/6 A5(if($i==1){++}) B5 A5 F5 D5 F5 }
    pmag_to(1.0)
    for($i,1,2) { q/6 C6(if($i==1){++}) D6 C6 A5 F5 A5 }
    for($i,1,2) { q/6 F6(if($i==1){++}) G6 F6 C6 A5 C6 }
    pmag_to(1.2)
    tempo(LASTVAL)(dt+=h)
    i slow(0.6) A6++ r(w-i)
    tempo_to(70)
    pmag_cto(0.8, 0, 1)
    i [D6--- F6--- G6 D7---] r(w-i)
  }
  lh {
    v=50
    repeat(16) {
      hold(q)& q/6 G2++ D3 F3 C4 F3 D3
    }
    hold(w)& s G1++ G2 z C3 D3 F3 A3
    C3++ D3 F3 A3 C4 D4 F4 A4
    C4++ D4 F4 A4 C5 D5 F5 A5
    C5++ D5 F5 A5 C6 D6 F6 A6
    i hold(i.)& [B4 D5 F5 G5]--- r(w-i)
  }
}
orch {
  sexpr(65)
  violin1 {
    v=80
    arco
    h+i D4 i D4 D4 D4 h+i F4 i F4 F4 F4 h+s A4 s A4 A4 A4 A4 A4 A4 A4
    sexpr(70)
    C5 C5 A4 A4 A4 A4 C5 C5 F5 F5 C5 C5 C5 C5 F5 F5 i sexpr_to(100) A5 r(w-i)
    sexpr(40)
    q if(times==1){G3} else {R}
    expr_to(70) tempo(LASTVAL) B3 C4 D4 sexpr_to(50)<< tempo_to(55)
    tempo(55)
    E4 r(q+h+q+i) i F#4 h F4
    r(w+q+i) i F#4 if(times==1) {q D4 D#4} else {q A4 G4}
  }
  violin2 {
    v=60
    arco
    h+i C4 i C4 C4 C4 h+i D4 i D4 D4 D4 h+s F4 s F4 F4 F4 F4 F4 F4 F4
    A4 A4 F4 F4 F4 F4 A4 A4 C5 C5 A4 A4 A4 A4 C5 C5 i F5 r(w-i)
    if(times==1) {h G3 q A3 B3 C4} else {q R B3 C4 D4 E4} 
    r(q+h+q+i) i+h D4
    r(w+q+i) i C4 h B3
  }
  viola {
    v=60
    arco
    h+i F3 i F3 F3 F3 h+i C4 i C4 C4 C4 h+s C4 s C4 C4 C4 C4 C4 C4 C4
    F4 F4 C4 C4 C4 C4 F4 F4 A4 A4 F4 F4 F4 F4 A4 A4 i C5 r(w-i)
    if(times==1) {w+q G3} else {q R G3 A3 B3 G3}
    r(q+h+q+i) [i+h D3 {i C4 h B3}]
    r(w+q+i) q [D4 D3] i E3 F3 F#3 
  }
  cello {
    v=60
    arco
    h+i G2 i G2 G2 G2 h+i G2 i G2 G2 G2 h+i G2 i G2 G2 G2
    G2 G2 G2 G2 G2 G2 G2 G2 G2 r(w-i)
    if(times==1) {w G2} else {w [G2 G3]}
    v=90 mod(20)
    expr(30)
    q slow(0.8) C4 expr_to(90)(dt-=i)
    q/3 slow(0.7) B3
    A3 expr_to(110) G3 i rtempo(0.8) A3 E4 D4 B3 
    q expr_to(90) A3 i expr_to(90) G3 expr_to(50) r(i+h)
    expr(50)
    q slow(0.8) C4 expr_to(100)(dt-=i)
    q/3 slow(0.7) Bb3
    A3 expr_to(127) G3 i rtempo(0.8) Bb3 F4 Eb4 C4 
    q expr_to(100) Bb3 i Ab3 expr_to(50) r(i+h)
    mod(0)
  }
  bass {
    v=80
    if(times==1) {w G2 G2}
    else {h+i G2 i G2 G2 G2 h+i G2 i G2 G2 G2}
    h+i G2 i G2 G2 G2
    G2 G2 G2 G2 G2 G2 G2 G2 G2 r(w-i)
    q G3 F3 E3 D3
    C3 r(q+h+q+i) i D3 h G3
    r(w+q+i) i D3 h G3
  }
  flute {
    v=80
    r(3w) wexpr(50)
    h C6 F6 wexpr_to(80) w A6 q wexpr_to(50) G6 r(h.)
    r(w+q+i)
    {v=100 mod(30)
      { expr(50) r(i) expr_to(80) r(q) expr(80) r(q) expr_to(50) }&
      i B5 q A5 G5 r(w+q+i)
      { expr(50) r(i) expr_to(80) r(q) expr(80) r(q) expr_to(50) }&
      i D6 q A5 G5
    }
    mod(0)
  }
  oboe {
    r(h) i
    mod(30)
    expr(80)
    v=100
    C5 A4 q/3 G4(bgrace(z G4 A4)) F4 A4 i C5 F5(dr=70) F5(q+i)
    C5 q/3 B4(bgrace(z B4 C5)) A4 C5 i F5 A5(dr=70) expr_to(100) A5(h.)
    mod(0)
    r(w)
    w+q v=80 [D5 F5] r(h.)
  }
  clarinet {
    v=80
    r(3w)
    h Eb5 Ab5 w [Eb5 C6] q [D5 Bb5] r(h.)
    r(w+q+i)
    v=60
    { expr(50) r(i) expr_to(80) r(q) expr(80) r(q) expr_to(50) }&
    i D5 q C5 Bb4
    r(w+q+i)
    { expr(50) r(i) expr_to(80) r(q) expr(80) r(q) expr_to(50) }&
    i F5 q C5 Bb4 
  }
  fagotto {
    v=80
    r(3w)
    h C4 F4 w [C4 A4] q [B3 G4] r(h.)
    expr(80)
    {expr(50) r(q) expr_to(80) r(h.) expr(80) r(q) expr_to(60)}&
    h C3 F2 q. if(times==1){C2}else{C3} r(i+h)
    {expr(50) r(q) expr_to(100) r(h.) expr(100) r(q) expr_to(70)}&
    q C3 G2 h Eb2 q. Ab2 r(i+h)
  }
  horn {
    v=60
    bexpr(80)
    r(4w)
    w+q [Bb4 Db5] r(h.)
    {bexpr(50) r(q) bexpr_to(80) r(h.) bexpr(80) r(q) bexpr_to(60)}&
    h C5 C#5 q. C5 r(i+h)
    {bexpr(50) r(q) bexpr_to(100) r(h.) bexpr(100) r(q) bexpr_to(70)}&
    q B4 Bb4 h Eb5 q. B4 r(i+h)
  }
  trombone {
    v=60
    r(6w)
    [{w+q+i C3} {h ++ G3 q A3 Ab3 q+i G3} {w+q+i C4}] r(i+h)
    [{q +. C3 D3 h Eb3 q+i Ab2} {w G3 q+i Eb3} {q C4 Bb3 h Db4 q+i C4}] r(i+h)
  }
}

/***************************************************************
 * measures 53-73 and 151-170
 ***************************************************************/
===

piano {
  pmag(1.0)
  rh {
    v=80
    { rtempo(0.7) r(q) rtempo_to(1.0) rtempo(0.8) r(s) rtempo(1.0) r(3s)
      r(i) slow(0.8) }&
    { pmag(0.9) r(h) pmag_to(1.1) r(h.) pmag_to(0.8) }&
    q C5 q/5 B4 C5 B4 A4 G4 i A4 E5 D5 B4
    { rtempo(1.0) r(q+q/3) rtempo_to(0.7) rtempo(0.8) r(2q/3)
      rtempo(1.0) r(h) rtempo_to(0.7) }&
    q A4 q/3 G4 {deemphasize(20) [C5 D5 F#5 D6] [C5 D5 F#5 B5]
    q [A4 B4 D5 A5] i [G4 G5] r}

    { deemphasize(20) 
      { rtempo(0.8) r(q) rtempo_to(1.1) rtempo(0.7) r(s) rtempo(1.1) r(3s)
	slow(0.8) r(q) rtempo(1.0) r(q) rtempo_to(0.6) }&
      { pmag(1.0) r(h) pmag_to(1.3) r(h.) pmag_to(0.8) }&
      q [C5 C6] q/5 [Bb4 Bb5] [C5 C6] [Bb4 Bb5] [A4 A5] [G4 G5]
      i [Bb4 Bb5] [F5 F6] [Eb5 Eb6] [C5 C6]
      { rtempo(1.0) r(q+q/3) rtempo_to(0.7) rtempo(0.8) r(2q/3)
	r(s) rtempo(1.0) r(h-s) rtempo_to(0.7) }&
      q [Bb4 C5 Eb5 Bb5](dt-=3z/2 arp(z/2) deemphasize(20)) q/3 [Ab4 Ab5]
      [D5 D6] [F#5 F#6](arp(z/2) deemphasize(20))
      q [A5 B5 D6 A6] i [G5 G6] r
    }

    v=90
    { tempo(60) rtempo(0.8) r(i) rtempo_to(1.0) r(w-i+i) rtempo_to(1.1) }&
    { pmag(0.8) r(q) pmag_to(1.0) r(h) pmag_to(0.9) }&
    q C5 q/3 A4 B4 C5 [{s v-=30 r dr=150 Bb4 r E4} {i E5 D5}] q C5
    { pmag(0.9) r(q) pmag_to(1.1) r(h) pmag_to(1.0) }&
    q D5 q/3 B4 C#5 D5 [{s v-=30 r dr=150 C5 r F#4} {i F#5 E5}] q D5
    { rtempo(1.1) r(q) rtempo(1.1) r(2q/3) rtempo_to(0.8) r(q/3) rtempo(1.0)}&
    { pmag(1.0) r(q+2q/3) pmag_to(1.2) }&
    q E5 q/3 C5 E5 C6+. i B5 A5 q/3 G5 E5 C5
    { r(q) slow(0.8) r(q) rtempo(0.8) }&
    { pmag(1.2) r(w) pmag_to(1.0)}&
    q A4 i D5-. B4-. q [A4(dt-=z) D5(dt+=z v-=30)] G4--

    { tempo(65) rtempo(0.8) r(i) rtempo_to(1.0) r(w-i+i) rtempo_to(1.1) }&
    { pmag(1.0) r(5w) pmag_to(1.3) }&
    v=90
    q [C5 C6] q/3 [A4 A5] [B4 B5] [C5 C6]
    i [E5 E6] [D5 D6] q [C5 C6]-.  
    q [D5 D6] q/3 [B4 B5] [C#5 C#6] [D5 D6]
    i [F#5 F#6] [E5 E6] q [D5 D6]-.
    { rtempo(1.1) r(q) rtempo(1.1) r(2q/3) rtempo_to(0.9) r(q/3)
      rtempo(1.0) r(q) rtempo(1.1)}&
    q [E5 E6] q/3 [C5 C6] [E5 E6] [C6 C7]+.
    i [B5 B6]++ [A5 A6] q/3 [G5 G6] [E5 E6] [C5 C6]
    { rtempo(1.1) r(q) rtempo(1.1) r(q) rtempo_to(0.9) }&
    q [A4 A5] i [D5 D6] [B4 B5]
    { r(q/3) rtempo(1.1) r(2q/3+3q) rtempo_to(1.3)
      rtempo(1.1) r(q/3) rtempo(1.3) r(2q/3+3q) rtempo_to(1.5)
      rtempo(1.3) r(q/3) rtempo(1.5) r(2q/3+q)
    }&
    q/3 [A4 A5]
    { v-=20 
      [F#4 B4 D5 F#5] [G4 B4 D5 G5]
      [Ab4 B4 D5 Ab5] [F#4 B4 D5 F#5] [G4 B4 D5 G5]
      [Ab4 B4 D5 Ab5] [F#4 B4 D5 F#5] [G4 B4 D5 G5]
      [Ab4 C5 E5 Ab5] [F#4 C5 E5 F#5] [G4 C5 E5 G5]
      r [Ab4 C#5 E5 Ab5] [A4 C#5 E5 A5]
      [Bb4 C#5 E5 Bb5] [Ab4 C#5 E5 Ab5] [A4 C#5 E5 A5]
      [Bb4 C#5 E5 Bb5] [Ab4 C#5 E5 Ab5] [A4 C#5 E5 A5]
      [Bb4 D5 F5 Bb5] [Ab4 D5 F5 Ab5] [A4 D5 F5 A5]
      r [Bb4 Eb5 F#5 Bb5] [B4 Eb5 F#5 B5]
      [C5 E5 G5 C6] [Bb4 E5 G5 Bb5] [B4 E5 G5 B5]
    }

    { rtempo(1.4) r(q/3) rtempo(1.5) r(2q/3+2q) rtempo_to(2.3) r(q+q)
      rtempo(2.3) r(h.) rtempo_to(1.2) }&
    v=127
    { pmag(1.0) r(s) pmag(0.7) r(w-s) pmag_to(1.0) }&
    r [C#5 C#6] [D5 D6] [E5 E6] [F5 F6] [Ab5 Ab6]
    [A5 A6] [C#6 C#7] [D6 D7] [E6 E7] [F6 F7] [Ab6 Ab7]
    [A6 A7] [E6 E7] [F6 F7] [C#6 C#7] [D6 D7] [Ab5 Ab6]
    [A5 A6] [E5 E6] [F5 F6] [C#5 C#6] [D5 D6] [A5 A6]
    i [A5 A6] r
    [B3 F4 A4] r [B4 F5 A5] r [B3 F4 A4] r
    [B4 F5 A5] r [B5 F6 A6] r [B4 F5 A5] r [B3 F4 A4] r

    v=90
    {rtempo(1.0) r(q/5) rtempo(1.7)}&
    q/5 [D3 F3 B3]++ C4 [D3 F3 B3] C4 [D3 F3 B3]
    {rtempo(1.0) r(q/5) rtempo(1.7)}&
    q/5 [D4 F4 B4]++ C5 [D4 F4 B4] C5 [D4 F4 B4]
    if(times==2) {
      {rtempo(1.0) r(q/5) rtempo(1.7)}&
      q/5 [D4 F4 B4]++ C5 [D4 F4 B4] C5 [D4 F4 B4]
    }
    {rtempo(1.0) r(q/5) rtempo(1.7)}&
    q/5 [D5 F5 B5]++ C6 [D5 F5 B5] C6 [D5 F5 B5]

    if(times==1) {
      { pmag(0.8) r(w+q) pmag_to(1.0) }&
      v=110
      {rtempo(1.0) r(s) rtempo(1.5) r(i.) r(w-i.) rtempo(1.5)
	r(i.) rtempo_to(0.2) }&
      q/6 [D6 F6 B6]++ C7
      repeat(12) {q/6 [D6 F6 B6] C7}
      [D6 F6 B6] z C7 [D6 F6 B6] A6++ B6++
    } else {
      { pmag(0.8) r(w) pmag_to(1.0) }&
      v=110
      {rtempo(1.0) r(s) rtempo(1.5) r(i.) r(h.-q) rtempo(1.5)
	r(q) rtempo_to(0.2) }&
      q/6 [D5 F5 B5]++ C6 [D5 F5 B5] C6 [D5 F5 B5] C6 
      { add_notes(n+=12)
	q/6 [D5 F5 B5]++ C6
	repeat(6) {q/6 [D5 F5 B5] C6}
	[D5 F5 B5] z C6 [D5 F5 B5] A5++ B5++
      }
    }
  }
  lh {
    v=50
    {q hold hold hold hold}&
    q/3 C3+. G3 E4 F#3 G3 E4
    C3+. A3 F4 Ab3 C4 F4
    {q hold q/3 hold 2q/3 hold q hold hold}&
    C3+. G3 Eb4 E4
    [D3 C4] [C4 F#4]
    G2+. D3 Bb3 B3 D4 G4
    {q hold hold hold hold}&
    C3+. G3 Eb4 G2 D3 Bb3
    s Eb2+. Bb2 C#3 G3 Bb3 C#4 G4 r
    {q hold q/3 hold 2q/3 hold q hold hold}&
    q/3 Ab2+. Eb3 F#3 C4
    D3+. [C4 F#4 Bb4]
    G2+. D3 Bb3 B3 D4 G4

    v=60
    {q hold hold hold hold}&
    E2+. C3 Bb3 A3 C3 F2
    E2+. C3 Ab3 A3 C3 F2
    {q hold hold hold hold}&
    F#2+. D3 C4 B3 D3 G2
    F#2+. D3 Bb3 B3 D3 G2
    {q hold hold hold hold}&
    Ab2+. E3 D4 C4 E3 A2
    B2+. G3 Eb4 E4 G3 C3
    {q hold hold hold hold}&
    D3+. C4 G4 q F#4-.
    q/3 G2+. F3 C4 q B3-.

    v=60
    def(p3) {{q hold}& q/3 $1++ $2-. $2-.}
    def(p3a) {{q hold}& q/3 $1++
      if(times==1) {[$2 $3]-. [$2 $3]-.}
      else {$3-. $2-.}
    }
    def(p4) {{q hold}& q/3 $1-. $1-. $1-.}
    { swing(q, 55)
      p3a(E2, C3, [Bb3 C4]) p3a(F2, C3, [A3 C4])
      p3a(E2, C3, [Ab3 C4]) p3a(F2, C3, [A3 C4])
      p3a(F#2, D3, [C4 D4]) p3a(G2, D3, [B3 D4])
      p3a(F#2, D3, [Bb3 D4]) p3a(G2, D3, [B3 D4])
      p3a(Ab2, E3, [D4 E4]) p3a(A2, E3, [C4 E4])
      p3a(B2, G3, [Eb4 G4]) p3a(C3, G3, [E4 G4])

      p3([D2++ D3], [G3 C4 D4 G4])
      {2q/3 hold q/3 hold}&
      q/3 [F3 C4 D4 F4]-. [F3 C4 D4 F4]-. [F#3 C4 D4 F#4]-.
      p3([G1++ G2], [F3 B3 D4 F4]) p4([F3 B3 D4 F4])
      p4([F3 B3 D4 F4]) p4([E3 G3 C4 E4])
      p3([A1 A2]++, [G3 C#4 E4 G4]) p4([G3 C#4 E4 G4])
      p4([G3 C#4 E4 G4]) p4([F3 A3 D4 F4])
      p3([B1 B2]++, [A3 Eb4 F#4 A4]) p4([G3 B3 E4 G4])
    }

    v=110
    hold(2w)&
    q/3 [D2 D3]++ [B3 F4 A4 B4] [B3 F4 A4 B4]
    repeat(7) { p4([B3 F4 A4 B4]) }
    i [B3 F4 A4 B4](hold(du=q)&) r

    v=110
    {h hold hold h. hold}&
    [G2 D3 A3] r
    [G3 D4 A4] r [G2 D3 A3] r
    [G3 D4 A4] r [G4 D5 A5] r
    [G3 D4 A4] r [G2 D3 A3] r

    {h. hold w+q hold}&
    i [G1 G2]++ r(w-i)
    if(times==1) {
      v=127
      s r [G4 G5]<< [F4 F5] [D4 D5]
      [B3 B4] [G3 G4] [F3 F4] [D3 D4]
      [B2 B3] [G2 G3] [F2 F3] [D2 D3]
      [B1 B2] [G1 G2] [F1 F2] [D1 D2]
    }
  }
}
orch {
  sexpr(35)
  violin1 {
    v=80
    h E4 F4 q+q/3 E4 2q/3 if(times==1) {F#4} else {C5}
    {r(q) sexpr(35) r(q) sexpr_to(0)}&
    h if(times==1) {D4} else {B4}
    {sexpr(35) r(h.) sexpr_to(55) r(q) sexpr_to(35)}&
    q Eb4 G4 Bb4 Eb5 q+q/3 Eb5 2q/3 D5
    {r(q) sexpr(35) r(q) sexpr_to(0)}&
    h D5
    if(times==1) {
      { r(h) sexpr(40) r(h) sexpr_to(35) r(h) sexpr(45) r(h) sexpr_to(40)}&
      r(h) q Ab4 A4 r(h) q Bb4 B4
      { r(h) sexpr(40) r(q) sexpr_to(55) r(q+w) sexpr_to(40) }&
      r(h) q Eb5 h E5 q F#4 A4 G4
    } else {
      r(4w)
    }
    sexpr(60)
    if(times==1) {
      take(0, dr=30 q Bb3 C4 r(h) C4 B3 r(h) D4 C4 G4 E4 A4 r(q))
    } else {
      {tp+=3 take(0, dr=30 q G4 F#4 r(h) A4 Ab4 r(h) B4 A4 C5 C#5 F#4 r(q))}
    }
    {sexpr(60) r(h+2w) sexpr_to(90) }&
    h+q D5 q C5 h. E5 q D5 F#5 E5
    A5
  }
  violin2 {
    v=60
    if(times==1) {
      w+q+q/3 C4 2q/3 C5 h B4
      q C4 D4 h G4 h F#4 h D4
      r(h) q Ab3 A3 r(h) q Bb3 B3
      r(h) q Eb4 h E4 q D4 h F4
      apply(0, G3 A3 A3 B3 B3 C4 Eb4 E4 C4)
      h+q B3 q C4 h. C#4 q D4 Eb4 E4
      F4
    } else {
      { tp+=3 h C#4 D4 q C#4 q/3 E4 2q/3 Eb4 h B3
	q A3 B3 h E4 Eb4 E4
	r(4w)
	{apply(0, E4 D4 F#4 E4 G#4 F#4 E4 E4)}& r(3w)
	{v=100 q. E4 i Eb4} h+q D4 q C#4 h. E4 q D4 F#4 E4
	G#4
      }
    }
  }
  viola {
    v=60
    if(times==1) {
      h G3 q A3 Ab3 q+q/3 G3 [2q/3+h D3 {2q/3 C4 h B3}]
      q G3 Bb3 h C#4 h C4 B3
      r(h) h C3 r(h) h D3
      r(h) h G3 h. C4 q B3
      {apply(0, C3 C3 D3 D3 E3 E3 G3 G3)}& r(3w)
      {v=100 q. G3 i F#3} h+q F3 q E3 h. G3 q F3 A3 G3
      B3
    } else {
      { tp+=3 w+q+q/3 A3 2q/3 A3 h Ab3
	q E3 G3 h Bb3 A3 Ab3 
	r(4w)
	apply(0, A3 A3 B3 B3 C#4 C#4 C4 C#4 A3)
	h+q Ab3 q A3 h. Bb3 q B3 C4 C#4
	D4
      }
    }
  }
  cello {
    v=60
    if(times==1) {
      h C3 F2 q+q/3 C2 2q/3 D2 h G2
      q C3 G2 h Eb2 q+q/3 Ab2 2q/3 D2 h G2
      r(h) q E2 F2 r(h) q F#2 G2
      r(h) q B2 C3 h D3 G2
    } else {
      { tp+=3 [w+q+q/3 A2 {h E3 q F#3 F3 q+q/3 E3}] 2q/3+h B2
	q A2 E2 h C2 q+q/3 F2 2q/3 B2 h E2 }
      r(4w)
    }
    {
      if(times==2) {tp+=12}
      apply(0, E2 F2 F#2 G2 Ab2 A2 B2 C3 D3)
      h+h G2 h+h A2 h B2
      q D3
    }
  }
  bass {
    v=80
    if(times==1) {
      q C3 r(3q+7w)
    } else {
      {tp+=3 v=60 h A3 D3 q+q/3 A2 2q/3 B2 h E3}
      r(6w)
    }
    {
      if(times==2) {tp+=12}
      apply(0, E2 F2 F#2 G2 Ab2 A2 B2 C3 D3)
      h+h G2 h+h A2 h B2
      q D3
    }
  }
  flute {
    v=60
    r(8w)
    { r(h) wexpr(80) r(q) wexpr(80) r(q) wexpr_to(50) 
      r(h) wexpr(80) r(q) wexpr(80) r(q) wexpr_to(50) 
      r(h) wexpr(90) r(q) wexpr(90) r(q) wexpr_to(60) }& 
    r(h) q Ab5 A5 r(h) Bb5 B5 r(h) Eb6 E6
  }
  oboe {
    v=80
    r(8w)
    if(times==1) {
      r(h) q Ab4 A4 r(h) Bb4 B4 r(h) Eb5 E5
    } else {
      {tp+=3 r(h) h A4 r(h) B4 r(h) E5}
    }
  }
  clarinet {
    v=80
    r(8w)
    if(times==1) {
      r(h) h Eb5 r(h) F5 r(h) Bb5
    } else {
      {tp+=3 r(h) q G#4 A4 r(h) q A#4 B4 r(h) q D#5 E5}
    }
  }
  fagotto {
    v=80
    r(4w)
    if(times==1) {
      mod(30)
      expr(60)
      q Bb3 A3 Ab3 A3
      expr(70)
      C4 B3 Bb3 B3
      {expr(80) r(w) expr_to(100) r(h.) expr(100) r(q.) expr_to(70) }&
      D4 C4 Eb4 E4 G4 F#4
      i F4 C#4 E4 D4
      q C4 r(q)
      mod(0)
    } else {
      r(4w+h)
    }
    if(times==2) {
      {tp+=3 h A3 r B3 r E4}&
    }
    q E3 F3 r(h) F#3 G3 r(h) B3 C4
  }
  horn {
    if(times==1) {
      r(8w)
      { r(h) expr(80) r(q) expr(80) r(q) expr_to(50) 
	r(h) expr(80) r(q) expr(80) r(q) expr_to(50) 
	r(h) expr(90) r(q) expr(90) r(q) expr_to(60) }& 
      v=60
      r(h) h Ab4 r(h) Bb4 r(h) Eb5 
    } else {
      r(4w)
      {
	v=70
	tp+=3
	mod(30)
	expr(60)
	q Eb4 D4 C#4 D4
	expr(70)
	F4 E4 Eb4 E4 
	{expr(80) r(w) expr_to(100) r(h.) expr(100) r(q.) expr_to(70) }&
	G4 F4 Ab4 A4 C5 B4
	i Bb4 F#4 A4 G4 
	q F4 r(q)
	mod(0)
      }
    }
  }
}

