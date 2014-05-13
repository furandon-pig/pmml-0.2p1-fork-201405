/*
 *  Piano Concerto Op.16         Edvard Grieg
 *      1st movement  measures 1-6
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
 * measures 1-6
 ***************************************************************/
tempo(84)

piano {
  rh {
    r(w)
    {add_notes(n+=24 v+=20)
      v=80
      rtempo(0.6)
      q. [A4 C5 E5 A5](hold)++ s [A4 A5](hold) [Ab4 Ab5](hold)
      rtempo_to(0.8)
      q. [E4 Ab4 B4 E5](hold)++ s hold(i)& [E4 E5] [C4 C5]
      rtempo_to(1.0)
      i [A3 C4 E4 A4](hold) rtempo_to(1.1) s hold(i)& [A3 A4] [Ab3 Ab4]-.
      i [E3 Ab3 B3 E4](hold) s hold(i)& [E3 E4] [C3 C4]-.
      i [A2 C3 E3 A3](hold) s hold(i)& [A2 A3] [Ab2 Ab3]-.
      rtempo(1.1) 
      i [E2 Ab2 B2 E3](hold)
      s hold(i)& [E2 E3] rtempo_to(0.7) [C2 C3]
      h.. [A1 A2](hold)++&
    }
    { rtempo(0.7) r(q)
      rtempo(0.15)
      z/2 A0(v=127) A1(v=100) v=80 E2 A2 rtempo_cto(0.6,0,1) C3 E3 A3 C4
      A2 C3 E3 A3 C4 E4 A4 C5
      A3 C4 E4 A4 C5 E5 A5 C6
      A4 C5 E5 A5 rtempo(0.6) C6 E6 A6 C7++ rtempo_to(0.25) }&
    r(h..)
    rtempo(0.3)
    i v=127 ped>> [F1 F2 F3]
    v=100
    z. [[F2 A2 D3 F3] [A3 D4 A4(v=127)]]
    rtempo(1.0)
    h..-z. [[F3 A3 D4 F4] [A4 D5(v=127) A5++]] pedoff
    rtempo(0.3)
    i v=127 ped>> [G#1 G#2 G#3]
    v=100
    z. [[G#2 B2 E3 G#3] [B3 E4 B4(v=127)]]
    w-z. [[G#3 B3 E4 G#4] rtempo(0.6) [B4 E5(v=127) B5++]] pedoff
  }
}

orch {
  aexpr(100)
  violin1 {pizz r(w) i [/*E4 C5*/ A5](v=127)}   // Omit some notes to keep 
  violin2 {pizz r(w) i [/*A4*/ C5 E5](v=127)}   // the total number of notes
  viola {pizz r(w) i [/*A3 E4*/ A4 ](v=127)}    // within 32 notes.
  cello {pizz r(w) i [A2 /*E3*/ C4](v=127)}
  bass {pizz r(w) i A2(v=127)}
  flute {r(w)
    i [E6 A6](dr=20 v=100)
  }
  oboe { r(w)
    i [A5 C6](dr=20 v=100)
  }
  clarinet { r(w)
    i [Eb5 G5](dr=20 v=100) 
  }
  fagotto { r(w)
    i [A2 E3](dr=20 v=100)
  }
  horn { r(w)
    i [C4 F4 Ab4 C5](dr=20 v=100)
  }
  trumpet { r(w)
    i [/*A3*/ A4](dr=20 v=100)
  }
  trombone { r(w)
    i [A2 E3 C4](dr=20 v=100)
  }
  timpani {
    v=110
    w
    vmag(0.3)
    A2(roll(speed=45u rsustain=0.9))
    vmag_cto(1.0, 1, 0)
    q
    v=127
    A2
  }
}
