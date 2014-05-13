/*
 * Macros/effectors for transferring rhythm and velocity information
 */

/*
 *  Copyright (C) 1997,1998   Satoshi Nishimura
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

/*
 * take(id, phrase1)
 * apply(id, phrase2)
 *   These macros copy rhythm and velocity information from one phrase
 *   to another. 
 *   The `take' macro extracts note-on time, duration, and velocity
 *   information from phrase1 and stores the nformation into a 'box'
 *   (acutually, it is a set of global macros) indexed by `id'.
 *   `id' should be a non-negative integer.
 *   The 'apply' macro modifies the note-on time, duration and velocity
 *   of each note event in phrase2 so that time intervals between note
 *   events and velocity changes are identical to those of phrase1
 *   by referring to the information stored in the `id'-th box.
 *   For example, 
 *
 *      take(0, C D++ [E G(dr=80)])  ==> recognized as `C D++ [E G(dr=80)]'
 *      apply(0, E F G B)     ==> equivalent to `E F++ [G B(dr=80)]'
 *   
 *   The `X' macro represents a "phantom" note.  A phantom note in
 *   the `take' macro is not played but is treated just as a normal note 
 *   in the corresponding `apply'.  A phantom note in the `apply' macro 
 *   skips a note in the phrase given to the `take' macro.  See the example
 *   below:
 * 
 *      take(0, C [E++ G>> X--])  ==> replaced to `C [E++ G>>]'
 *      apply(0, C F G A)  ==> replaced to `C [F++ G>> A--]'
 *      apply(0, C X A B)  ==> replaced to `C [A>> B--]'
 *
 * Limitation:
 *   The time of non-note events is never modified by the 'apply' macro. 
 *   Therefore, when the phrase in the `apply' macro contains non-note
 *   event generating command (e.g. ctrl), the events may be diordered.
 */
defeff(take_sub, "q") {
  init {
    ::$1 = #()
    $t = t
    $v = v
  }
  case(note) {
    append(::$1, #(t - $t, v - $v, do))
    if( n == 0 ) { reject }
  }
}

def(take, "nq") {
  {
    $t = t
    take_sub(evalstr(sprintf("take_n%d", $1)))
    $2
    ::evalstr(sprintf("take_t%d", $1)) = t - $t
  }
}

defeff(apply_sub, "q") {
  init {
    cnt = 1
    $t = t
  }
  case(note) {
    if( #::$1 < cnt ) {
      error("apply: Too many notes")
    }
    t = $t + ::$1[cnt][1]
    v += ::$1[cnt][2]
    do = ::$1[cnt][3]
    cnt += 1
    if( n == 0 ) { reject }
  }
}

def(apply, "nq") {
  {
    $t = t
    apply_sub(evalstr(sprintf("take_n%d", $1)))
    $2
    t = $t + ::evalstr(sprintf("take_t%d", $1))
  }
}

X ='note(n=-tp)'
