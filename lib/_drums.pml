/*
 * Macros for rhythm sections  <undocumented>
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
 * drums(length [rept=n], note-cmd1, string1, note-cmd2, string2, ...)
 *   For each charactor in "stringn", a note specified by "note-cmdn" is
 *   played with the note length of "length".  "note-cmd" is a note
 *   command for striking a drum (e.g., C2, note(n=32), D4#++), which is 
 *   usually defined as a macro in a model-specific macro library for drums. 
 *   When the "rept=n" option is specified, the same pattern is repeated 
 *   n times.  
 *
 *   Example:
 *     drums(i rept=4,
 *     BD,    "X.......",
 *     SD,    "..x...x.",
 *     HH,    "xxxxxxxo",
 *     Bongo, "hhllhhll",
 *     Guiro, "l.ssl.ss")
 *
 *   The Meanning of Drums Charactors
 *     "." or " " : rest
 *     "x"        : strike it with normal velocity
 *     "X"        : strike it with higher velocity
 *     "1" to "8" : strike it with more specific velocity ("8" is the highest) 
 *     "a"        : get commands for modifing notes from the
 *                  next extra argument in the "drums" macro. 
 *                     (ex.)  drums(i, c4,  "a...a...", dt+=z, dt+=.5z)
 *                            => h c4(dt+=z) c4(dt+=.5z)
 *     "n"        : similar to "a" but the previous arugment is reused. 
 *                     (ex.)  drums(i, c4,  "a...n. .", dt+=z)
 *                            => h c4(dt+=z) c4(dt+=z)
 *     others     : see definitions in the model-specific macro library
 *
 *   Users can define (or redefine) the meanning of a drum character by
 *   defining a "DC?" macro where "?" is the drum character.  The macro
 *   contents should be a list of commands for modifing notes.
 *     (ex.)  DCd = 'v=70 dt+=z' 
 *            
 */
def(drums, "qq*") {
  { 
    rept = 1
    $1     // note length , rept  etc.
    shift
    [
      while( $# > 0 ) {
	if( $# < 2 ) { error("drums: bad arg") } 
	{  // begin group
	  repeat(rept) {
	    local::$ag = 0
	    for($i,1,strlen($2)) {
	      local::$c = substr($2, $i, 1)
	      if( $c == " " || $c == "." ) {
		R
	      } else {
		$1(evalstr("DC"+$c))
	      }
	    }
	  }
	}
	shift($*,$ag + 2)
      }
    ]
  }  
}

/*
 * set_drums(macro_for_drums_note, commands)
 *   set_drums binds note-modification commands to indivisual rhythmic 
 *   instruments.
 *
 *   Example:
 *     To set default velocity (i.e. the velocity when the "x" drum 
 *     charactor is used) for the bass drum:
 *       => set_drums(BD, v=100)
 *     To delay all the notes concerning the snare drum by 20 ticks:
 *       => set_drums(SD, dt+=20)
 *     To define drum characters local to specific rhythmic instruments:
 *       => set_drums(SD, DCX='v+=20' DCr='roll()' )
 */
/*
def(set_drums, "qq") {
  edef($1,"") { eval(eval($1))($2) }
}
*/
def(set_drums, "qq") {
  $1 = `eval($1)($2)`
}

/*
 * drums_no(macro_for_drums_note)
 *   drums_no extracts the note number from a macro defining a drums note
 *   (ex.)  drums_no(BD) -> 36
 */
def(drums_no, "q") { 
  nth_token($1, 5)
}

/*
 * definition of drum characters
 */
// basic drum characters
DCx = ''
DCX = 'v+=40'

// drum characters for finer velocity control  
DC0 = 'v=0'
DC1 = 'v=35'
DC2 = 'v=55'
DC3 = 'v=70'
DC4 = 'v=80'
DC5 = 'v=90'
DC6 = 'v=100'
DC7 = 'v=110'
DC8 = 'v=120'
DC9 = 'v=127'

// get a modifier from the next argument
DCa = '$*[$ag + 3] $ag += 1'
DCn = '$*[$ag + 2]'
