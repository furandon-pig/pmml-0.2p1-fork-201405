/*
 * GS-specific definitions (not tested)
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
def(pmctrl, "i")  { ctrl(84, $1) }		// 84: portamento control
defctrl("reverb", "Reverb", 91, 15, 1)		// 91: reverb send level
defctrl("chorus", "Chorus", 93, 15, 1)		// 93: chorus send level
defctrl("delay", "Delay", 94, 15, 1)		// 94: delay send level

all_sound_off = 'ctrl(120,0)'
reset_all_ctrls = 'ctrl(121,0)'

/* NRPCs */
def(vib_rate, "i")    {nrpc(0x1008, $1 + 64)}	// vibrato rate (-64 - 63) 
def(vib_depth, "i")   {nrpc(0x1009, $1 + 64)}	// vibrato depth (-64 - 63) 
def(vib_delay, "i")   {nrpc(0x100a, $1 + 64)}	// vibrato delay (-64 - 63) 
def(tvf_cutoff, "i")  {nrpc(0x1020, $1 + 64)}	// TVF cutoff freq (-64 - 63) 
def(tvf_reso, "i")    {nrpc(0x1021, $1 + 64)}	// TVF resonance (-64 - 63) 
def(env_attack, "i")  {nrpc(0x1063, $1 + 64)}	// envelope attack (-64 - 63) 
def(env_decay, "i")   {nrpc(0x1064, $1 + 64)}	// envelope decay (-64 - 63) 
// set per-note drums parameters   (ex.) drums_pitch(drums_no(BD), 10)
def(drums_pitch, "ii") {nrpc(0x1800 + $1, $2 + 64)} // drums pitch (-64 - 63) 
def(drums_level, "ii") {nrpc(0x1a00 + $1, $2)}  // drums level (0-127)
def(drums_pan, "ii")   {nrpc(0x1c00 + $1, $2)}  // drums pan (1-127, 0 for rnd)
def(drums_reverb, "ii"){nrpc(0x1d00 + $1, $2)}	// drums reverb (0-127)
def(drums_chorus, "ii"){nrpc(0x1e00 + $1, $2)}	// drums chorus (0-127)
def(drums_delay, "ii") {nrpc(0x1f00 + $1, $2)}	// drums delay (0-127)

/* values for panpot control */
left7 = 0
left6 = 10
left5 = 19
left4 = 28
left3 = 37
left2 = 46
left1 = 55
center = 64
right1  = 73
right2  = 82
right3  = 91
right4  = 100 
right5  = 109
right6  = 118
right7  = 127

/*
 * xprog(prog_num, bank_num): extended program change
 */
bank_num_lsb = 0

def(bank, "i") {
  ctrl(0, $1)
  ctrl(32, bank_num_lsb)
}

def(xprog, "ii") {
  bank($2)
  prog($1)
}

sc55map = 'bank_num_lsb=1  bank(0)'
sc88map = 'bank_num_lsb=2  bank(0)'

/*
 * initial bender range in semitones
 */
init_bender_range = 2		// initial bender range is +-2 semitones

/* 
 * GM system on
 */
gm_system_on = 'excl(#(0x7e, 0x7f, 0x09, 0x01))'

/*
 * GS-specific exclusive messages
 */
device_id = 16		// this can be changed later by users

def(gs_excl, "ia") {  // gs_excl(address, data_array)
  local::dat = #(0x41, device_id, 0x42, 0x12, 
  		 $1 shr 16, $1 shr 8, $1, @$2)
  local::checksum = 0
  for($i, 5, #dat) { checksum += int(dat[$i]) & 0x7f }
  append(dat, - checksum)
  excl(dat)
}

def(gs_reset, "") {
  gs_excl(0x40007f, #(0))
}
def(sc88_mode_set, "i") {  // 0: single module mode  1: double module mode
  gs_excl(0x00007f, #($1))
}

/*
 * virtual control changes for master volume and master pan 
 * using exclusive messages 
 */
defctrl("msvol", "MasterVol", new_gctrl(), 15u, 1) 
defctrl("mspan", "MasterPan", new_gctrl(), 15u, 1) 

defeff(gs_effector, "", ExpandCtrl) {
  case(ctrl(MasterVol)) {
    if( val < 0 ) {
      warn("msvol: master volume out of range")
      val = 0
    } elsif( val > 127 ) {
      warn("msvol: master volume out of range")
      val = 127 
    }
    gs_excl(0x400004, #(int(val)))   
    reject
  }
  case(ctrl(MasterPan)) {
    if( val < 0 ) {
      warn("msvol: master pan out of range")
      val = 0
    } elsif( val > 127 ) {
      warn("msvol: master pan out of range")
      val = 127 
    }
    gs_excl(0x400006, #(int(val)))   
    reject
  }
}

gs_effector()       // attach the above effector 

/*
 * reverb, chorus, and delay
 */
def(gs_set_reverb, "i*") { // gs_set_reverb(macro, [, charac, pre-lpf, 
		           //   level, time, feedback, dummy, predelay] )
  gs_excl(0x400130, $*)
}

def(gs_set_chorus, "i*") { // gs_set_chorus(macro, [, pre-rpf, level, feedback,
		           //   delay, rate, depth, snd_to_rev, snd_to_delay] )
  gs_excl(0x400138, $*)
}

def(gs_set_delay, "i*")  { // gs_set_delay(macro, [, pre-lpf, time-cent,
			   //   time-left, time-right, lev-cent, lev-left,
			   //   lev-right, level, feedback, snd_to_rev] )
  gs_excl(0x400150, $*)
}

def(gs_set_eqlzer, "i*") { // gs_set_eqlzer(l-freq, l-gain, h-freq, h-gain)
  gs_excl(0x400200, $*)
}

/* 
 * display string on the LCD panel
 */
def(gs_display_string, "s") {
  local::dat = #()
  local::len = strlen($1)
  for($i, 1, 32) {
    if( $i <= len ) {
      append(dat, charcode(substr($1, $i, 1)))
    } elsif( $i <= 16 ) {
      append(dat, 0x20)
    }
  }
  gs_excl(0x100000, dat)
}

/*
 * gs_write_bitmap(page_no, string1, string2, ..., string16) stores 
 * a bitmap image to one of the 16 image buffers in a GS module.
 * "stringN" contains a 16-character string representing the bitmap 
 * of the N-th line.  Charactor "*" or "x" truns on its corresponding dot. 
 * If page_no is 1, the bitmap is immediately displayed.
 */
def(gs_write_bitmap, "issssssssssssssss") { 
  local::$ad = 0x100000 + ((($1 + 1) shl 7) & 0xf00) + ((~$1 & 1) shl 6)
  shift
  local::dat = rep(64,0)
  for($i,1,16) {
    local::$d = 0
    local::$ofs = 0
    for($col, 1, 16) {
      if( local::$c = substr($*[$i], $col, 1)  $c == "*" || $c == "x" ) {
         $d |= 1
      }
      if( $col % 5 == 0 ) {
	 dat[$i + $ofs] = $d
         $d = 0
         $ofs += 16
      }
      $d shl= 1
    }
    $d shl= 3
    dat[$i + 48] = $d
  }
  gs_excl($ad, dat)
}

def(gs_display_page, "i") {  // gs_display_page(page_no)
  gs_excl(0x102000, #($1))
}
def(gs_display_time, "i") {  // gs_display_time(time)
  gs_excl(0x102001, #($1))
}

/*
 * include instrument name file
 */
include("gs_inst")

/*
 * include rhythmic instrument name file
 */
include("gs_drums")
