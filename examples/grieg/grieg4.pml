/*
 *  Piano Concerto Op.16         Edvard Grieg
 *      1st movement  measures 171-last
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
 * measures 171-175 (Coda)
 ***************************************************************/
===
tempo(100)
rtempo(1.0)
{ r(3w+q) rtempo(1.0) r(q) rtempo_to(0.8) r(h) rtempo_to(0.5) }& 
{ aexpr(100) r(4w+h+i.) aexpr(100) r(s) aexpr_to(40)}&

piano {
  rh {
    pmag(1.0)
    v=110
    q [C6 Eb6 A6]
  }
  lh {
    v=127
    q [C5 Eb5 A5]
  }
}

orch {
  violin1 {
    arco
    v=110
    take(0, q A5(dr=70) i. B5 s C6(dr=70) i B5(dr=30) q A5 r(i)
	 i. A5(dr=80) s B5(dr=70) {i dr=30 B5 C6} h A5(dr=80)
	 q A5(dr=70) i. B5 s C6(dr=70) i B5(dr=30) q A5 r(i)
	 i. A5(dr=80) s B5(dr=70) {i dr=30 B5 C6} q A5 A5)
    h. [D4 A4 A5] r(q)
  }
  violin2 {
    arco
    v=110
    apply(0, A4 B4 C5 B4 A4 A4 B4 B4 C5 A4
	  A4 B4 C5 B4 A4 A4 B4 B4 C5 A4 A4)
    h. [F4 A4] r(q)
  }
  viola {
    arco
    v=110
    def(..::p3) { {q/3 dr=80 $1 $1 $1} }
    def(..::p3l) { {q/3 dr=100 $1 $1 $1} }
    p3([C4 Eb4]) p3([C4 Eb4]) p3([C4 Eb4]) p3([C4 Eb4])
    p3([C4 E4]) p3([C4 E4]) p3([C4 E4]) p3([C4 E4])
    p3([C4 F4]) p3([C4 F4]) p3([C4 F4]) p3([C4 F4])
    p3([C4 F4]) p3([C4 F4]) p3([C4 F4]) p3([C4 E4])
    h. [A3 D4] r(q)
  }
  cello {
    arco
    v=110
    p3(F3) p3(F3) p3(F3) p3(F3)
    p3(E3) p3(E3) p3(E3) p3(E3)
    p3(D3) p3(D3) p3(D3) p3(D3)
    p3(D#3) p3(D#3) p3(D#3) p3(E3)
    h. F3 r(q)
  }
  bass {
    arco
    v=120
    p3l(F3) p3l(F3) p3l(F3) p3l(F3)
    p3l(E3) p3l(E3) p3l(E3) p3l(E3)
    p3l(D3) p3l(D3) p3l(D3) p3l(D3)
    p3l(D#3) p3l(D#3) p3l(D#3) p3l(E3)
    h. F3 r(q)
  }
  flute {
    v=127
    apply(0, A5 B5 C6 B5 A5 A5 B5 B5 C6 A5
	  A5 B5 C6 B5 A5 A5 B5 B5 C6 A5 A5)
    h. [F5 A5] r(q)
  }
  oboe {
    v=110
    p3([C5 Eb5]) p3([C5 Eb5]) p3([C5 Eb5]) p3([C5 Eb5]) 
    p3([C5 E5]) p3([C5 E5]) p3([C5 E5]) p3([C5 E5]) 
    p3([C5 F5]) p3([C5 F5]) p3([C5 F5]) p3([C5 F5]) 
    p3([C5 F5]) p3([C5 F5]) p3([C5 F5]) p3([C5 E5]) 
    h. [A4 D5] r(q)
  }
  clarinet {
    v=110
    p3([C5 Eb5]) p3([C5 Eb5]) p3([C5 Eb5]) p3([C5 Eb5]) 
    p3([C5 Eb5]) p3([C5 Eb5]) p3([C5 Eb5]) p3([C5 Eb5]) 
    p3([C5 Eb5]) p3([C5 Eb5]) p3([C5 Eb5]) p3([C5 Eb5]) 
    p3([C5 Eb5]) p3([C5 Eb5]) p3([C5 Eb5]) p3([C5 Eb5]) 
    h. [Ab4 C5] r(q)
  }
  fagotto {
    v=110
    p3([C4 Eb4]) p3([C4 Eb4]) p3([C4 Eb4]) p3([C4 Eb4])
    p3([C4 E4]) p3([C4 E4]) p3([C4 E4]) p3([C4 E4])
    p3([C4 F4]) p3([C4 F4]) p3([C4 F4]) p3([C4 F4])
    p3([C4 F4]) p3([C4 F4]) p3([C4 F4]) p3([C4 E4])
    h. D4 r(q)
  }
  horn {
    v=127
    w F4 F4 F4 h. F4 q F4
    h. [F4 Bb4] r(q)
  }
  trumpet {
    v=110
    q A4(dr=70) i. B4 s C5(dr=90) i B4(dr=60) q A4 r(i)
    i. A4(dr=80) s B4(dr=90) {i dr=60 B4 C5} h A4(dr=80)
    q A4(dr=70) i. B4 s C5(dr=90) i B4(dr=60) q A4 r(i)
    i. A4(dr=80) s B4(dr=90) {i dr=60 B4 C5} q A4 A4
    h. [A4 D5] r(q)
  }
  trombone {
    v=127
    w [F3 C4 Eb4] [E3 C4 E4] [D3 C4 F4] h.[Eb3 C4 F4] q[E3 C4 E4]
    h. [F3 A3 D4] r(q)

  }
  timpani {
    r(4w)
    v=90
    { r(h+i.) vmag(1.0) r(s) vmag_to(0.3)}&
    h. A2(roll(speed=z/2 rsustain=0.9)) r(q)
  }
}

/***************************************************************
 * measures 176-206 (Cadenza)
 ***************************************************************/

===
tempo(84)
rtempo(1.0)

Vmag2 = new_vctrl()

defeff(vmag2_eff, "", ExpandCtrl) {
  init {
    vmag2 = 1.0
  }
  case(note) {
    v *= vmag2
  }
  case(ctrl(Vmag2)) {
    vmag2 = val
  }
}

defctrl("vmag2", "Vmag2", Vmag2, 15u, 0.01)

piano {
  pmag(1.0)
  rh {
    v=110
    ped>>
    q rtempo(0.7) F1++ F2+. rtempo_to(1.0) A2 D3 A3 D4 A4 rtempo(0.8) D5
    h rtempo(0.7) D6++ rtempo(0.8) q A5
    pedoff
    ped>>
    {h C6++ B5(dt+=z)-.}&
    q rtempo(0.8) G1 G2-.
    rtempo_to(1.0) D3-- F3-- pmag(0.7) B3 D4 F4 rtempo(1.0) B4 D5
    rtempo_to(0.8) F5
    h. vmag_to(0.3) B5(du=w)++
    pedoff

    ped>>
    { rtempo(1.0) r(h) rtempo_to(5.0) }&
    vmag(1.0)
    v=63
    i G#1+. G#2 D3 E3 B3 D3 E3 B3
    D4 E4 B4 D4 E4 B4
    D5 E5 B5 D5 E5 B5
    D6 E6 B6 D6 E6 B6 D#7
    { vmag(1.0) r(5h) vmag(1.0) r(13h) vmag_to(2.0)}& 
    { v-=5 rand_vel(5) fgrand_ntime(s)
      repeat(20) {
	F7 E7 D#7 B6
      }
    }
    rtempo(5.0)
    F7 E7 D#7 B6
    vmag(2.0)
    D7 C7 A6 B6 Ab6 E6
    rtempo(5.0)
    F6 E6 Eb6 B5 D6 C6 A5
    B5 Ab5 E5 F5 E5 Eb5 B4
    D5 C5 A4 B4 Ab4 E4
    F4 E4 Eb4 B3
    pedoff ped>>
    D4 C4 A3
    pedoff ped>>
    B3 Ab3 E3 vmag_to(1.3) rtempo_to(2.0) Eb3
    pedoff
    rtempo(1.2)
    vmag(1.0)
    v=85
    hold(h+6i)&
    [q D3++ A3 F4 A4 D5
      {h+i A5++ i F5 D5 Bb4 C5 D5  B4 rtempo(0.7) D#5(tie)}](arp(z))
    rtempo(0.5)
    [h [v=50 B2 E3 D4 G#4](gap(s)) {q D#5(endtie) rtempo(0.3) E5---(gap(s))}]
    v=80
    rtempo(0.5)
    q F5(tie)
    rtempo(0.8)
    hold(h+6i)&
    [q v=40 G2++ D3 Bb3 F4 Bb4 
      {h+i F5(endtie) v=55 rtempo(1.0)
	i D5 Bb4 F4 G4 A4 rtempo_to(0.8) E4 rtempo(0.3) G#4(tie)}]
    [h [v=30 E2 A2 G3 C#4](gap(s)) {q G#4(endtie) rtempo(0.3) A4(v=35 gap(s))}]
    v=60
    rtempo(0.6)
    q Bb4(tie)
    { hold(h+6i) r(i) hold(3i)} &
    [v=35 q {[D2+. Bb2 F3 Bb3 F4] D1}
      {h+i v=60 Bb4(endtie) rtempo(0.7)
	i vmag(1.0) C5 D5 rtempo_to(0.8) Bb4 F4 D4 E4 F4 D4 Bb3
	vmag_to(0.7) rtempo_to(0.4)}
    ](arp(z/2)) 
    
    vmag(1.0)
    {rtempo(0.7) r(h) rtempo(0.7) r(h) rtempo_to(0.5) r(q) rtempo(0.3) }&
    {w+q G#3(v=50) q A3(v=30)}&
    { r(h) v=30 q F3 E3 h D#3 }&
    { r(h) v=30 w C3 }&
    { r(h) v=30 q D#2 E2 h F2 }
    rtempo(1.0)
    r(h)
  }
  ===

  { rtempo(0.5) r(h) rtempo_to(0.8)}&

  rh {
    def(p1) {
      {z v=50 r $1 $2 $1 $2 $1 $2 $1}
      {z v=50 r $1 $2 $1 $2 $1 $2 $1}
    }
    
    pmag(0.5)
    v=80
    {p1(C5,A4) p1(C5,A4)}&
    q [E4-- E5++](hold) i. [F4-- F5++](hold) s [G4-- G5++](hold)
    {i hold q. hold}&
    i [F4-- F5++] q [E4-- E5++]-. r(i)
    {p1(D5,Bb4) p1(D5,Bb4)}&
    {q hold hold h hold}&
    i. [F4-- F5++] s [G4-- G5++] i [G4-- G5++] [A4-- A5++] h [F4-- F5++]-.
    {p1(E5,D5) p1(E5,D5)}&
    q [G4-- G5++](hold) i. [A4-- A5++](hold) s [Bb4-- Bb5++](hold)
    {i hold q. hold}&
    i [A4-- A5++] q [G4-- G5++]-. r(i)
    {p1(G5,E5) p1(G5,E5)}&
    {q hold hold h hold}&
    i. [A4-- A5++] s [B4-- B5++] i [B4-- B5++] [C#5-- C#6++] h [A4-- A5++]-.
    {p1(B5,A5) p1(B5,A5)}&
    {h hold hold}&
    q [D5 D6] i. [E5 E6] s [F5 F6] i [E5 E6] q [D5 D6]-. r(i)
    {p1(D6,B5) p1(D6,B5)}&
    {h hold hold}&
    q [F5 F6] i. [G5 G6] s [A5 A6] i [G5 G6] q [F5 F6]-. r(i)
    {p1(F6,D6)}&
    {h hold hold}&
    pmag_to(1.55)
    q [Ab5 Ab6] i. [A5 A6] s [B5 B6] i [A5 D6 F6 A6] [Ab5 Ab6]
    {q hold}&
    r(s) s [Ab5 D6 F6 Ab6] [A5 D6 F6 A6] [Bb5 D6 F6 Bb6]
    {h hold}&
    {rtempo(0.8) r(h) rtempo_to(1.0)}&
    i [A5 D6 F6 A6] [Ab5 Ab6]

    v=110
    {r(s) pmag(1.0) s [Ab5 D6 F6 Ab6] [A5 D6 F6 A6] [Bb5 D6 F6 Bb6]
      hold(q)&
      [A5 D6 F6 A6] [Ab5 D6 F6 Ab6] [A5 D6 F6 A6] [Bb5 D6 F6 Bb6]
      hold(q)&
      [A5 D6 F6 A6] [Ab5 D6 F6 Ab6] [A5 D6 F6 A6] [Bb5 D6 F6 Bb6]
      hold(i)&
      [A5 D6 F6 A6] [Ab5 Ab6]}&
    {r(s) s v=80 [Ab4 D5 F5] [A4 D5 F5] [Bb4 D5 F5]
      [A4 D5 F5] [Ab4 D5 F5] [A4 D5 F5] [Bb4 D5 F5]
      [A4 D5 F5] [Ab4 D5 F5] [A4 D5 F5] [Bb4 D5 F5]
      [A4 D5 F5] Ab4}

    { rtempo(0.8) r(h) rtempo_to(1.3) }&
    hold(h)&
    s [F4 F5 F6 F7]
    { add_notes(n-=24 v=80) s
      [E6 E7] [Eb6 Eb7] [D6 D7]
      [B5 B6] [Bb5 Bb6] [A5 A6] [Ab5 Ab6]

      hold(h)&
      [F5 F6] [E5 E6] [Eb5 Eb6] [D5 D6]
      [B4 B5] [Bb4 Bb5] [A4 A5] [Ab4 Ab5]

      hold(h)&
      [F4 F5] [E4 E5] [Eb4 Eb5] [D4 D5]
      [C#4 C#5] [C4 C5] [B3 B4] [Bb3 Bb4]
      hold(q)&
      [A3 A4] [Ab3 Ab4] [G3 G4] [F#3 F#4]
      hold(q)&
      [F3 F4] [E3 E4] [Eb3 Eb4] [D3 D4]
      hold(q)&
      { rtempo(1.3) r(q) rtempo_to(0.7) }&
      [C#3 C#4] [C3 C4] [B2 B3] [Bb2 Bb3]
    }
  }
  lh {
    v=55
    { vmag2_eff()
      vmag2pat = '{vmag2(0.8) r(q) vmag2_to(1.2) r(q) vmag2_to(0.8)}&'
      vmag2pat q/7 E1++ E2 A2 C3 E3 A3 C4 E4 C4 A3 E3 C3 A2 E2
      vmag2pat q/7 E1++ E2 A2 C3 E3 A3 C4 E4 C4 A3 E3 C3 A2 E2
      vmag2pat q/7 E1++ E2 Bb2 D3 F3 Bb3 D4 F4 D4 Bb3 F3 D3 Bb2 E2
      vmag2pat q/7 E1++ E2 Bb2 D3 F3 Bb3 D4 F4 D4 Bb3 F3 D3 Bb2 E2
      vmag2pat q/8 E1++ E2 Bb2 D3 E3 G3 Bb3 D4 E4 D4 Bb3 G3 E3 D3 Bb2 E2
      vmag2pat q/8 E1++ E2 Bb2 D3 E3 G3 Bb3 D4 E4 D4 Bb3 G3 E3 D3 Bb2 E2
      vmag2pat q/7 E1++ E2 A2 C#3 G3 A3 C#4 G4 C#4 A3 G3 C#3 A2 E2
      vmag2pat q/7 E1++ E2 A2 C#3 G3 A3 C#4 G4 C#4 A3 G3 C#3 A2 E2
      vmag2pat q/7 E1++ E2 A2 D3 F3 A3 D4 F4 D4 A3 F3 D3 A2 E2
      vmag2pat q/7 E1++ E2 A2 D3 F3 A3 D4 F4 D4 A3 F3 D3 A2 E2
      vmag2pat q/8 E1++ E2 A2 D3 F3 A3 B3 D4 F4 D4 B3 A3 F3 D3 A2 E2
      vmag2pat q/8 E1++ E2 A2 D3 F3 A3 B3 D4 F4 D4 B3 A3 F3 D3 A2 E2
      vmag2pat q/8 E1++ E2 B2 D3 F3 Ab3 B3 D4 F4 D4 B3 Ab3 F3 D3 B2 E2
      vmag2pat q/8 E1++ E2 B2 D3 F3 Ab3 B3 D4 F4(i) r(i)
      {vmag2(0.8) r(q) vmag2_to(1.2)}&
      q/8 E1++ E2 B2 D3 F3 Ab3 B3 D4 F4(i)
    }
  }
  //===
  rh {
    vmag(1.0)
    rtempo(0.9)
    v=120
    { attach(add_notes(n-=24 v-=20), an)
      hold(i+q+i.)& 
      i [A0 v=127 A1 A2 A3](disable(an))
      q [E5 A5 C6 E6] i. [F5 A5 C6 F6] s [G5 A5 C6 G6](dr=70)
      i [F5 A5 C6 F6](dr=30) q [E5 A5 C6 E6](hold)

      ped>>
      vmag(0.4)
      3z/4 A2 B2 C3 D3 E3 D3 C3 B2 A2 Ab2 G2 F#2 F2 E2 F2 F#2 G2 Ab2
      vmag_to(0.9) vmag(1.0)
      pedoff<<

      { hold(i+2q) hold(q) }&  
      i [A0 v=127 A1 A2](disable(an))
      i. [E5 A5 C6 E6] s [F5 A5 C6 F6](dr=70)
      rtempo(0.75)
      i [F5 A5 C6 F6](dr=50) [G5 A5 C6 G6](dr=50)
      rtempo(0.9)
      q [E5 A5 C6 E6]

      ped>>
      vmag(0.4)
      3z/4 A2 B2 C3 D3 E3 D3 C3 B2 A2 Ab2 G2 F#2 G2 Ab2 A2 Bb2 B2 
      vmag_to(0.9) vmag(1.0)
      pedoff<<

      hold(i+q+i.)& 
      i [C1 C2 C3](disable(an))
      q [G5 C6 E6 G6] i. [A5 C6 E6 A6] s [B5 C6 E6 B6](dr=70)
      i [A5 C6 E6 A6](dr=30) q [G5 C6 E6 G6]

      ped>>
      vmag(0.4)
      3z/4 C3 D3 E3 F3 G3 F3 E3 D3 C3 B2 Bb2 A2 Ab2 G2 Ab2 A2 Bb2 B2 
      vmag_to(0.9) vmag(1.0)
      pedoff<<

      { hold(i+2q) hold(q) }&  
      i [C1 C2 C3](disable(an))
      i. [G5 C6 E6 G6] s [A5 C6 E6 A6](dr=70)
      rtempo(0.7)
      i [A5 C6 E6 A6](dr=50) [B5 C6 E6 B6](dr=50)
      rtempo(0.9)
      q [G5 C6 E6 G6]

      ped>>
      vmag(0.4)
      3z/4 C3 D3 E3 F3 G3 F3 E3 D3 C3 B2 Bb2 A2 Ab2 G2 Ab2 A2 Bb2 B2
      C3 C#3 D3 Eb3
      vmag_to(0.9) vmag(1.0)
      pedoff<<
    }

    {rtempo(0.3) r(z) rtempo(0.9)}&
    hold(h)&
    h [E1 E2 E4 v=127 E5]&
    {z v=90 r Ab4 D5 E4 Ab4 D4 E4 Ab3 Ab4 D4 E4 Ab3 D4 E3 Ab3 B2}
    
    hold(i+h)&
    i [D2 A2 D3]--
    h [F3 F4 v=127 F5]&
    {z v=90 r A4 D5 F4 A4 D4 F4 A3 D4 F3 A3 D3 F3 A2 D3 F2} 

    hold(i+w)&
    i [G#1 G#2 D3 F3]--
    w [B3 B4 v=127 B5]&
    {z v=90 r D5 F5 B4 F5 B4 D5 F4 D5 F4 B4 D4 B4 D4 F4 B3
      F4 B3 D4 F3 D4 F3 B3 D3 B3 D3 F3 B2 F3 B2 D3 F2}
    
    {rtempo(0.3) r(z) rtempo(0.9)}&
    hold(h)&
    h [A1 A2 A4 v=127 A5]&
    {z v=90 r C#5 G5 A4 C#5 G4 A4 C#4 C#5 G4 A4 C#4 G4 A3 C#4 E3}
    
    hold(i+h)&
    i [G2 D3 G3]--
    h [Bb3 Bb4 v=127 Bb5]&
    {z v=90 r D5 G5 Bb4 D5 G4 Bb4 D4 G4 Bb3 D4 G3 Bb3 D3 G3 Bb2}
    
    hold(i+w)&
    i [C#2 C#3 G3 Bb3]--
    w [E4 E5 v=127 E6]&
    {z v=90 r G5 Bb5 E5 Bb5 E5 G5 Bb4 G5 Bb4 E5 G4 E5 G4 Bb4 E4
      Bb4 E4 G4 Bb3 G4 Bb3 E4 G3 E4 G3 Bb3 E3 Bb3 E3 G3 Bb2}
    
    hold(i+h)&
    i [D2 D3]
    h [F3 A3 D4 F4 F5 A5 D6 v=127 F6]&
    {z v=90 r A5 D6 F5 A5 D5 F5 A4 D5 F4 A4 D4 F4 A3 D4 F3}
    
    hold(i+h)&
    i [A1 A2]
    h [E3 A3 C4 E4 E5 A5 C6 v=127 E6]&
    {z v=90 r A5 C6 E5 A5 C5 E5 A4 C5 E4 A4 C4 E4 A3 C4 E3}
    
    { hold(i+h/3) h/3 hold hold q hold hold }&
    i [D1 D2]
    h/3 [A3 D4 F#4 A4 D5 F#5 v=127 A5]
    [B3 D4 F#4 B4 D5 F#5 v=127 B5]
    [C4 D4 F#4 C5 D5 F#5 v=127 C6]
    rtempo(0.7)
    q [B3 D4 F4 B4 D5 F5 v=127 B5]
    rtempo(0.6)
    [D4 D5 v=127 D6]
    rtempo(0.9)
    
    hold(w)&
    {45u v=90 r F#2 C3 Eb3 Ab3 C4 C3 Eb3 Ab3 C4 Eb4 Ab4 C5
      C4 Eb4 Ab4 C5 Eb5 Ab5 C6
      C5 Eb5 Ab5 C6 Eb6 Ab6 C7}&
    w [F#1 G#3 v=127 G#4]
    
    hold(w)&
    {45u v=90 r F2 C3 Eb3 A3 C4 C3 Eb3 A3 C4 Eb4 A4 C5
      C4 Eb4 A4 C5 Eb5 A5 C6
      C5 Eb5 A5 C6 Eb6 A6 C7}&
    w [F1 A3 v=127 A4]
    
    hold(w)&
    {50u v=70 r E2 Eb3 F#3 A3 C4 Eb3 F#3 A3 C4 Eb4 F#4 A4 C5
      Eb4 F#4 A4 C5 Eb5 F#5 A5 C6
      Eb5 F#5 A5 C6 Eb6 F#6 A6 C7}&
    w [v=80 E1 C3 v=127 C4]
    
    ped>>
    w [v=80 E1 B2 v=110 B3]&
    {z v=60 r E2 D3 F3 A3 B3 D3 F3 A3 B3 D4 F4 A4 B4
      D4 F4 A4 B4 D5 F5 A5 B5
      D5 F5 A5 B5 D6 F6 A6 B6
      {deemphasize(20)
	[B4 F5] [D5 A5] [B5 F6] [D6 A6]
	[A4 D5] [B4 F5] [A5 D6] [B5 F6]
	[F4 B4] [A4 D5] [F5 B5] [A5 D6]
	[D4 A4] [F4 B4] [D5 A5] [F5 B5]
	[B3 F4] [D4 A4] [B4 F5] [D5 A5]
	[A3 D4] [B3 F4] [A4 D5] [B4 F5]
	[F3 B3] [A3 D4] [F4 B4] [A4 D5]
	rtempo(0.9)
	[D3 A3] [F3 B3] [D4 A4] [F4 B4]
	rtempo_to(0.4)
      }}
    pedoff

    { r(q) rtempo_to(0.9) }&
    v=100
    {q. hold hold h+i+h hold }&
    q.
    B4-.(trill(1, speed=3z/4))
    D5-.(trill(2, speed=3z/4))
    h+w+i F5(trill(2,speed=3z/4 last='z v-=20 E5 F5 F#5 G5'))&
    r(h)
    i [E2 E3]
    q [D4 F4 Ab4 B4++]
    i. [C4 F4 Ab4 C5++] s [B3 F4 Ab4 D5++]
    i [C4 F4 Ab4 C5++](dr=50) q [D4 F4 Ab4 B4++] r(i)
    
    h G#5(trill(1,speed=z last='G5 G#5'))(hold)
    hold(h.)&
    slow(0.5)
    h. [F5 Ab5 B5 C6](tremolo(attack=10 decay=0.9 rsustain=0.7 speed=z))&
    {v=80 s r(i) E1 E2 D3 F3 Ab3 B3 v=90 D4 F4 Ab4 B4}

    { vmag(1.0) r(h) vmag_to(1.6) r(w) vmag_to(0.6) }&
    w [D5 Ab5 B5 F5 C6](tremolo(attack=0 decay=0.9 rsustain=0.7
				group=2 speed=z))(hold)
    { r(h) rtempo(LASTVAL) r(q) rtempo_to(0.3) }&
    v*=0.7
    h. [D5 Ab5 B5 E5 C6](tremolo(group=2 speed=z))(hold)
    rtempo(0.5)
    i [D5 B5] [C#5 Bb5] rtempo_to(0.3) rtempo(0.2) [D5 B5]
  }
}

/***************************************************************
 * measures 207-last
 ***************************************************************/

===
tempo(78)
rtempo(1.0)

orch {
  piano {
    rh {
      q [E5 E6]
      r(3q+7w)
      pmag(1.0)
      v=80
      { add_notes(n-=12 v-=20)
	du=s*0.7
	i A4 s C5 D5 q+i E5(du=q+s*0.7) s A5 Ab5 q+i E5(du=q+s*0.9)
	{vmag(1.0) r(3q) vmag_to(1.5)}&
	s C5 D5 i E5 s Ab5 C6 i A5 s C6 F6
	{rtempo(1.2) r(q) rtempo_to(0.8)}&
	i E6 s Ab6 C7 
      }
      pmag(1.0)
      {add_notes(n+=24 v+=20)
	v=90
	slow(0.8)
	4q/3 [A4 C5 E5 A5](hold)++ q/3 [A4 A5] [Ab4 Ab5]
	slow(0.9)
	4q/3 [E4 Ab4 B4 E5](hold)++ q/3 hold(i)& [E4 E5] [C4 C5]
	{ rtempo(1.0) r(q) rtempo_to(1.2) }&
	{repeat(12){{ped r(i) pedoff r(i)}}}&
	q/3 [A3 C4 E4 A4]++ [A3 A4] [Ab3 Ab4]
	[E3 Ab3 B3 E4]++ [E3 E4] [C3 C4]
	[A2 C3 E3 A3]++ [A2 A3] [Ab2 Ab3]
	[E2 Ab2 B2 E3]++ [E2 E3] [C2 C3]
	[A1 C2 E2 A2]++ [A1 C2 E2 A2] [C2 C3]
	[E2 Ab2 B2 E3]++ [E2 Ab2 B2 E3] [Ab2 Ab3]
	[A2 C3 E3 A3]++ [A2 C3 E3 A3] [C3 C4]
	[E2 Ab2 B2 E3]++ [E2 Ab2 B2 E3] [Ab2 Ab3]
	[A2 C3 E3 A3]++ [A2 C3 E3 A3] [C3 C4]
	[E3 Ab3 B3 E4]++ [E3 Ab3 B3 E4] [Ab3 Ab4]
	[A3 C4 E4 A4]++ [A3 C4 E4 A4] [C4 C5]
	rtempo(1.2)
	[E4 Ab4 B4 E5]++ [E4 E5] [Ab4 Ab5]
	q [A4 C5 E5 A5]++(hold)
	rtempo_to(1.0)
      }

      v=110
      q [A3 C4 E4 A4 C5 E5 A5 C6++](hold)
      rtempo_to(0.9)
      [A2 C3 E3 A3 C4 E4 A4 C5++](hold)
      rtempo(0.75)
      hold(q+w)&
      [A1 A2 A3](v=127)
      w [E3 A3 C4 E4 A4 C5 E5 A5++]
    }
  }
  violin1 {
    v=90
    { sexpr(50) r(h.) sexpr_to(80) r(q) sexpr_to(70)
      sexpr(70) r(h.) sexpr_to(80) r(q) sexpr_to(90) r(h) sexpr_to(70) }&
    q [A3 A4++] [Bb3 Bb4++] h [E4 E5++]
    i [A3 A4++](dr=80) [A3 A4++] [Bb3 Bb4++](dr=80) [Bb3 Bb4++] h [E4 E5++]
    q [F4 F5++] [E4 E5++]
    slow(0.8)
    q/3 [A3 A4++] [B3 B4++] [C4 C5++] i [B3 B4++] [D4 D5++]
    q. [Ab3 Ab4++] i [A3 A4++]
    i [C4 C5++](dr=50) r [Ab3 Ab4]--(dr=50) r

    { rtempo(1.0)(dt-=q) r(q) rtempo_to(1.2) }&
    v=60
    s r E4 A4 E4 r F4 A4 F4 r F4 A4 F4 r E4 B4 E4
    r Bb4 C#5 Bb4 r Bb4 D5 Bb4 r A4 C5 A4 r Ab4 E4 Ab4

    v=100
    { sexpr(80) r(w+q) sexpr_to(120) r(h.) sexpr_to(80) }&
    q A4 C5 i D5 s B4 D5 q E5
    i E5 q G5 i F5(dr=80) F5 E5(dr=80) E5 C5

    v=60
    {q dr=30 A4 Ab4(dr=100) A4 B4(dr=100) A4 Ab4 A4 B4 A4}

    r(3q+w)

    aexpr(90)
    v=127
    {q du=s r [B4 E5] [C5 E5] [B4 E5] [C5 E5] [B4 Ab5] [C5 A5] [B4 Ab5]
      h [C5 A5] [E5 C6]
    }
    { r(w-s) aexpr(90) r(s) aexpr_to(0) }&
    w A3
  }
  violin2 {
    v=60
    h [G3 G4] [Bb3 Bb4] [G3 G4] [Bb3 Bb4]
    w [A3 A4] h Eb4 i E4(dr=50) r E4(dr=50)-- r

    s r C4 E4 C4 r C4 F4 C4 r B3 F4 B3 r D4 E4 D4
    r E4 Bb4 E4 r F4 Bb4 F4 r E4 A4 E4 r E4 Ab4 E4

    { roll(attack=0 speed=s) q [E4 A4] [E4 A4] [D4 A4] Ab4 G4 Bb4 A4 Ab4 }

    s E4 E4 A4 E4 E4 E4 Ab4 E4 E4 E4 A4 E4 E4 E4 B4 E4
    E4 E4 A4 E4 E4 E4 Ab4 E4 E4 E4 A4 E4 E4 E4 B4 E4 i E4
    r(i+q+h+w)

    v=127
    {q du=s r Ab4 A4 Ab4 A4 E4 E4 E4 h E4 A4}
    w A3
  }
  viola {
    v=60
    q [C#3 C#4] [D3 D4] h [G3 G4]
    q [C#3 C#4] [D3 D4] h [G3 G4]
    w [F3 F4] h [C3 C4] i [A3 A4](dr=50) r [B3 D4](dr=50)-- r

    s r A3 C4 A3 r A3 C4 A3 r A3 B3 A3 r B3 D4 B3
    r C#4 E4 C#4 r D4 F4 D4 r C4 E4 C4 r B3 D4 B3
    
    { roll(attack=0 speed=s)
      q C4 C4 B3 [B3 E4] [Bb3 E4] [D4 F4] [C4 E4] [D4 E4] } 
    
    s C4 C4 E4 C4 B3 B3 E4 B3 C4 C4 E4 C4 D4 D4 E4 D4
    C4 C4 E4 C4 B3 B3 E4 B3 C4 C4 E4 C4 D4 D4 E4 D4 i C4
    r(i+q+h+w)

    v=127
    {q du=s r [B3 E4] [C4 E4] [B3 E4] [C4 E4] [B3 Ab4] [C4 E4 A4] [B3 Ab4]
      h [C4 E4 A4] [C4 E4 A4]}
    w A3
  }
  cello {
    v=60
    q A3 G3 h C#3
    q A3 G3 h C#3
    q D3 C4 B3 A3 Ab3 F#3 i E3(dr=50) r E2(dr=50)-- r

    {q dr=30 A3 F3 D3 Ab2 G2 F2 E2 E2}

    q A3 G3 F3 E3 C#3 D3 E3 E2
    {q dr=30 A2 A2 A2 A2 A2 A2 A2 A2 A2}
    r(3q+w)

    v=127
    {q du=s r E3 A3 E3 A3 E3 A3 E3 h A3 A3}
    w A2
  }
  bass {
    v=60
    3w+h E2 i E3(dr=50) r E3(dr=50)-- r

    {q dr=30 A3 F3 D3 Ab2 G2 F2 E2 E2}

    q A3 G3 F3 E3 C#3 D3 E3 E2
    {q dr=30 A2 A2 A2 A2 A2 A2 A2 A2 A2}
    r(3q+w)

    v=127
    {q du=s r E3 A3 E3 A3 E3 A3 E3 h A3 A3}
    w A2
  }
  flute {
    r(6w)
    v=80
    { wexpr(80) r(w+q) wexpr_to(120) r(h.) wexpr_to(70) }&
    q A5 C6 i D6 s B5 D6
    take(2, q+i E6 q G6 i F6 F6 E6 E6 C6 q A5(dr=50))
    r(q+h+3w)
    v=110
    {q du=s r [B5 E6] [C6 E6] [B5 E6]
      [C6 E6] [E6 Ab6] [E6 A6] [E6 Ab6] h [E6 A6] [A5 C6]} 
  }
  oboe {
    r(4w)
    v=90
    mod(20)
    { wexpr(60) r(w+q) wexpr_to(100) r(h.) wexpr_to(80) }&
    take(1, q A4 C5 i D5 s B4 D5 h E5 q+i F5 i E5(dr=80) E5+. C5 q A4 r r)
    v=80
    apply(2, E5 G5 F5 F5 E5 E5 C5 A4)
    mod(0)
    r(q+h+3w)
    v=110
    {q du=s r [E5 Ab5] [E5 A5] [E5 Ab5]
      [E5 A5] [Ab5 B5] [A5 C6] [Ab5 B5] h [A5 C6] [C5 E5]}
  }
  clarinet {
    r(6w)
    v=80
    [h. {h G4 q F4} C5] q [G4 B4]
    [G4 Bb4] [F4 C#5] [Eb4 C5] [F4 B4]
    q [Eb4 C5](gap(i))
    [h {q D4 Eb4(gap(i))} G4(gap(i))]
    [h {q F4 Eb4(gap(i))} G4(gap(i))]
    {q dr=30 [D4 G4] [Eb4 G4] [F4 G4] [Eb4 G4]}
    r(q+h+w)
    v=110
    {q du=s r [B4 D5] [C5 Eb5] [B4 D5]
      [C5 Eb5] [D5 G5] [Eb5 G5] [D5 G5] h [Eb5 G5] [G4 C5]}
    w C4
  }
  fagotto {
    r(4w)
    v=80
    apply(1, A3 C4 D4 B3 D4 E4 F4 E4 E4 C4 A3)
    apply(2, E4 G4 F4 F4 E4 E4 C4 A3)
    h [A2 A3](gap(i)) [A2 A3](gap(i))
    {q dr=40 [A2 A3] [A2 A3] [A2 A3] [A2 A3]}
    r(q+h+w)
    v=110
    {q du=s r [B3 E4] [C4 E4] [B3 E4]
      [C4 E4] [E4 Ab4] [E4 A4] [E4 Ab4] h [E4 A4] [A3 C4]}
    w A2
  }
  horn {
    { expr(80) r(w+h) expr(80) r(h) expr_to(100) r(h) expr_to(80) }&
    v=60
    3w+h+i C3
    r(i+q+2w)
    { expr(80) r(w+q) expr_to(120) r(h.) expr_to(80) }&
    [h. F4 {h Ab4 q G4}] q [E4 G4]
    [{q Eb4 C#4 h C4} {h F#4 C5}]
    q [C4 C5](gap(i)) h [C4 C5](gap(i)) [C4 C5](gap(i))
    {q dr=40 [C4 C5] [C4 C5] [C4 C5] [C4 C5]}
    r(q+h+w)
    v=127
    {q du=s r C4 [C4 F4] C4
      [C4 F4] [C4 C5] [F4 C5] [C4 C5] h [C4 C5] [Ab4 C5]}
    w F4
  }
  trumpet {
    r(12w)
    v=127
    {q du=s r E4 E4 E4 E4 [E4 E5] [E4 E5] [E4 E5] h [E4 E5] [A4 C5]}
    w A3
  }
  trombone {
    r(12w)
    v=127
    {q du=s r [E3 Ab3 B3] [A2 A3 C4] [E3 Ab3 B3]
      [A2 C4 E4] [E3 B3 Ab4] [A2 C4 A4] [E3 B3 Ab4]
      h [A2 C4 A4] [A3 C4 E4]}
    w [A2 A3]
  }
  timpani {
    v=30
    3w+h+i E3(roll(attack=0 decay=1 speed=50u))
    r(i+q+8w)
    v=70
    q r E3 A2 E3 A2 E3 A2 E3 h A2 A2
    w A2(roll(speed=z*0.6))
  }
}
