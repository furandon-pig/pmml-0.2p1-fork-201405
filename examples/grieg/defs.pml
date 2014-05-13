/*
 *  Piano Concerto Op.16         Edvard Grieg
 *      macro and thread definitions
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

include("gm")
include("take")

timesig(4,4)
keysig(1,1)

acped = '{ped R pedoff}'
def(slow, "n") { {rtempo($1) r(s) rtempo(1.0)}& }

arco = Strings
pizz = PizzicatoStr
trem = TremoloStr

// Define expression controllers directed to a group of channels
defctrl("sexpr", "StringsExpr", new_gctrl(), 15u, 1)
defctrl("wexpr", "WoodwindExpr", new_gctrl(), 15u, 1)
defctrl("bexpr", "BrassExpr", new_gctrl(), 15u, 1)
defctrl("aexpr", "AllExpr", new_gctrl(), 15u, 1)
defctrl("pmag", "PianoVmag", new_gctrl(), 15u, 0.01)

defeff(gexp_effector, "") {
  case(ctrl(StringsExpr)) {
    ch=2 tk=stk   ctrl_any(11, val)
    ch=3 tk=stk+1 ctrl_any(11, val)
    ch=4 tk=stk+2 ctrl_any(11, val)
    ch=5 tk=stk+3 ctrl_any(11, val)
    ch=6 tk=stk+4 ctrl_any(11, val)
    reject
  }
  case(ctrl(WoodwindExpr)) {
    ch=7 tk=wtk    ctrl_any(11, val)
    ch=8 tk=wtk+1  ctrl_any(11, val)
    ch=9 tk=wtk+2  ctrl_any(11, val)
    ch=11 tk=wtk+3 ctrl_any(11, val)
    reject
  }
  case(ctrl(BrassExpr)) {
    ch=12 tk=btk   ctrl_any(11, val)
    ch=13 tk=btk+1 ctrl_any(11, val)
    ch=14 tk=btk+2 ctrl_any(11, val)
    reject
  }
  case(ctrl(AllExpr)) {
    ch=2 tk=stk   ctrl_any(11, val)
    ch=3 tk=stk+1 ctrl_any(11, val)
    ch=4 tk=stk+2 ctrl_any(11, val)
    ch=5 tk=stk+3 ctrl_any(11, val)
    ch=6 tk=stk+4 ctrl_any(11, val)
    ch=7 tk=wtk    ctrl_any(11, val)
    ch=8 tk=wtk+1  ctrl_any(11, val)
    ch=9 tk=wtk+2  ctrl_any(11, val)
    ch=11 tk=wtk+3 ctrl_any(11, val)
    ch=12 tk=btk   ctrl_any(11, val)
    ch=13 tk=btk+1 ctrl_any(11, val)
    ch=14 tk=btk+2 ctrl_any(11, val)
    reject
  }
  case(ctrl(PianoVmag)) {
    tk=2 ch=1 ctrl_any(Vmag, val) 
    tk=3 ch=1 ctrl_any(Vmag, val)
    reject
  }
}
gexp_effector()
    
// layout of tracks
defthread(piano, orch)
newtrack(piano::rh)     { ch=1 vol(100) pan(63) mod(0) Piano1 }
newtrack(piano::lh)     { ch=1 }
newtrack(orch::violin1) { ch=2 ::stk=tk vol(100) pan(60) mod(0) }
newtrack(orch::violin2) { ch=3 vol(100) pan(70) mod(0) }
newtrack(orch::viola)   { ch=4 vol(100) pan(45) mod(0) }
newtrack(orch::cello)   { ch=5 vol(100) pan(80) mod(0) }
newtrack(orch::bass)    { ch=6 vol(120) pan(90) mod(0) tp= -12 }
newtrack(orch::flute)   { ch=7 ::wtk=tk vol(85) pan(50) mod(0) Flute }
newtrack(orch::oboe)    { ch=8 vol(85) pan(60) mod(0) Oboe }
newtrack(orch::clarinet){ ch=9 vol(85) pan(70) mod(0) Clarinet tp= -3 }
newtrack(orch::fagotto) { ch=11 vol(85) pan(80) mod(0) Bassoon }
newtrack(orch::horn)    { ch=12 ::btk=tk
                          vol(100) pan(50) mod(0) FrenchHorn tp= -8 }
newtrack(orch::trumpet) { ch=13 vol(100) pan(63) mod(0) Trumpet }
newtrack(orch::trombone){ ch=14 vol(100) pan(70) mod(0) Trombone }
newtrack(orch::timpani) { ch=15 vol(100) pan(40) mod(0) Timpani }

/*
 * Control values to be kept track of:
 *  piano    - v, pmag
 *  strings  - v, sexpr (or aexpr), arco or pizz
 *  woodwind - v, wexpr (or aexpr) 
 *  brass    - v, bexpr (or aexpr)
 *  timpani  - v
 *  Global   - tempo, rtempo
 */

// effector for reducing velocity of the accompanying notes in each chord
defeff(deemphasize, "n") {
  init {
    $prev_note = #()
  }
  case(note) {
    $save = #(t,n,v,nv,do)

    if( !$prev_note ) {
      reject
    } else {
      if( t == $prev_note[1] ) {
	v = $prev_note[3] - $1
      } else {
	v = $prev_note[3]
      }
      t = $prev_note[1]
      n = $prev_note[2]
      nv = $prev_note[4]
      do = $prev_note[5] 
    }

    $prev_note = $save
  }

  wrap {
    t = $prev_note[1]
    n = $prev_note[2]
    v = $prev_note[3]
    nv = $prev_note[4]
    do = $prev_note[5] 
    note
  }
}
