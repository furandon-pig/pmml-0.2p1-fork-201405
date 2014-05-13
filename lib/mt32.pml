/*
 * Model Specific Definitions for Roland MT-32
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
 * define model specific control changes
 */
defctrl("pan", "Pan", 10, 15u, 1)		// 10: pan pot
defctrl("expr", "Expr", 11, 15u, 1)		// 11: expression control

/* values for panpot control */
right7 = 0
right6 = 9
right5 = 18
right4 = 27
right3 = 36
right2 = 45
right1 = 54
center = 63
left1  = 72
left2  = 81
left3  = 90
left4  = 99
left5  = 108
left6  = 117
left7  = 127

/*
 * initial bender range in semitones
 */
init_bender_range = 12		// initial bender range is +-12 semitones

/*
 * exclusive messages
 */
device_id = 16		// this can be changed later by users

def(mt32excl, "ia") {  // mt32excl(address, data_array)
  local::dat = #(0x41, device_id, 0x16, 0x12, 
  		 $1 shr 16, $1 shr 8, $1, @$2)
  local::checksum = 0
  for($i, 5, #dat) { checksum += int(dat[$i]) & 0x7f }
  append(dat, - checksum)
  excl(dat)
}

defctrl("msvol", "MasterVol", new_gctrl(), 15u, 1) 
defeff(mt32_mastervol_effector, "", ExpandCtrl) {
  case(ctrl(MasterVol)) {
    if( val < 0 ) {
      warn("msvol: master volume out of range")
      val = 0
    } elsif( val > 100 ) {
      warn("msvol: master volume out of range")
      val = 100 
    }
    mt32excl(0x100016, #(int(val)))   
  }
}
mt32_mastervol_effector()       // attach the above effector 

def(mt32_master_tune, "n") {  // Master tune ($1 = frequency)
  /* Although the MT32's manual says that the tune range is 432.1Hz -
   * 457.6Hz, the actual range of my MT32 seems to be 427.5 - 452.6 ! */
  local::$d = int(floor(($1-427.5) * 127 / (452.6-427.5)+ .5))
  if( $d < 0 ) {
    warn("master_tune: frequency out of range")
    $d = 0
  } elsif( $d > 127 ) {
    warn("master_tune: frequency out of range")
    $d = 127
  }
  mt32excl(0x100000, #($d))
}

def(mt32_reverb, "iii") {  // reverb(Mode:0-3, Time:0-7, Level:0-7)
  mt32excl(0x100001, $*)
}

RevRoom = 0
RevHall = 1
RevPlate = 2
RevDelay = 3

def(mt32_reverb_off, "") {  // Turn off current channel's reverb
		       // Valid until the next program change
  if( ch == 10 ) {
    mt32excl(0x030106, #(0))
  } else {
    mt32excl(0x030006 + ((ch-2) shl 4), #(0))
  }
}

def(mt32_reverb_on, "") {  // Turn on current channel's reverb
  if( ch == 10 ) {
    mt32excl(0x030106, #(1))
  } else {
    mt32excl(0x030006 + ((ch-2) shl 4), #(1))
  }
}

def(mt32_partial_reserve, "iiiiiiiii") { // change partial reserve
  if( $1+$2+$3+$4+$5+$6+$7+$8+$9 > 32 ) {
    error("partial_reserve: total number of partials exceeds 32")
  }
  mt32excl(0x100004, $*)
}

def(mt32_display_string, "s") { // display string on the LCD panel
  local::dat = #()
  local::len = strlen($1)
  for($i, 1, 0x14) {
    if( $i <= len ) {
      append(dat, charcode(substr($1, $i, 1)))
    } else {
      append(dat, 0x20)
    }
  }
  mt32excl(0x200000, dat)
}

def(mt32_all_reset, "") {  // all parameters reset
  mt32excl(0x7f0000, #(0))
}

/*
 * include instrument name file
 */
include("la_inst")

/*
 * include rhythmic instrument name file
 */
include("gm_drums")
