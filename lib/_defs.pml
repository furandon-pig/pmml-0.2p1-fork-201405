/*
 * Basic macro difinitions.
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

MAXINT = 0x7fffffff
MAXNEGINT = 0x80000000
REST = 0x80000000
LASTVAL = 0x80000000
bgngrp = '{'
endgrp = '}'

CUR_TRACK = 2
def(newtrack, "q") { 
    defthread($1) 
    $1 { tk = ::CUR_TRACK text(3, "("+idstr($1)+")") } ::CUR_TRACK += 1
    $1
}

def(mod12, "n") { (($1 + 12000) % 12) }
def(gap,"n") { dp=100 do= -$1 }

/*
 * definitions for control change
 */
def(defctrl, "ssirn:n") {
  evalstr($2) = $3
  evalstr($2 + "Step") = $4
  evalstr($2 + "Threshold") = $5
  if( null($6) ) {
    edef(evalstr($1), "n") { ctrl($3, $$1) }
  } 
  edef(evalstr($1 + "_pt"), "n") { ctrl_pt($3, $$1) }
  edef(evalstr($1 + "_to"), "n") {
    ctrl_to($3, $$1, evalstr($2 + "Step"), evalstr($2 + "Threshold"))
  }
  edef(evalstr($1 + "_cto"), "n:nn") { 
    ctrl_cto($3, $$1, evalstr($2 + "Step"), evalstr($2 + "Threshold"),
             null($$2) ? 0 : {$$2}, null($$3) ? 0 : {$$3})
  }
}

defctrl("mod", "Mod", 1, 15u, 1)		// 1: modulation wheel
defctrl("breath", "Breath", 2, 15u, 1)		// 2: breath control
defctrl("foot", "Foot", 4, 15u, 1)		// 4: foot controller
defctrl("pmtime", "Pmtime", 5, 15u, 1)		// 5: portamento time
defctrl("vol", "Vol", 7, 15u, 1)		// 7: volume
defctrl("pan", "Pan", 10, 15u, 1)		// 10: pan pot
defctrl("expr", "Expr", 11, 15u, 1)		// 11: expression control
defctrl("bend", "Bend", 128, 15u, 4, 0)		// 128: pitch bend
defctrl("kp", "Kp", 129, 15u, 1, 0)		// 129: key pressure
defctrl("cpr", "Cpr", 130, 15u, 1, 0)		// 130: channel pressure
defctrl("vmag", "Vmag", 132, 15u, 0.01)		// 132: velocity magnifier
defctrl("tempo", "Tempo", 192, 15u, 1, 0)	// 192: tempo
defctrl("rtempo", "Rtempo", 193, 15u, 0.01)	// 192: relative tempo

// switches
ped	= 'ctrl(64,127)'			// 64: dumper pedal
pedoff	= 'ctrl(64,0)'
Ped = 64
pm	= 'ctrl(65,127)'			// 65: portamento on/off
pmoff	= 'ctrl(65,0)'
Pm = 65
sped	= 'ctrl(67,127)'			// 67: soft pedal
spedoff	= 'ctrl(67,0)'
Sped = 67

// parameter control
def(rpc, "ii") { // rpc(RPN, data): registered parameter control (0-127)
  ctrl(100, $1 & 0x7f)
  ctrl(101, ($1 shr 8) & 0x7f)
  ctrl(6, $2 & 0x7f)
  ctrl(38, ($2 shl 7) & 0x7f)
}
def(rpcw, "ii") { // rpcw(RPN, data): registered parameter control (0-16383)
  ctrl(100, $1 & 0x7f)
  ctrl(101, ($1 shr 8) & 0x7f)
  ctrl(6, ($2 shr 7) & 0x7f)
  ctrl(38, $2 & 0x7f)
}
def(nrpc, "ii") { // nrpc(NRPN, data): non-registered PC (0-127)
  ctrl(98, $1 & 0x7f)
  ctrl(99, ($1 shr 8) & 0x7f)
  ctrl(6, $2 & 0x7f)
  ctrl(38, ($2 shl 7) & 0x7f)
}
def(nrpcw, "ii") { // nrpcw(NRPN, data): non-registered PC (0-16383)
  ctrl(98, $1 & 0x7f)
  ctrl(99, ($1 shr 8) & 0x7f)
  ctrl(6, ($2 shr 7) & 0x7f)
  ctrl(38, $2 & 0x7f)
}

def(bender_range, "i") { rpc(0, $1) }
def(fine_tune, "i")    { rpcw(0, $1+0x2000) }   // -8192 <= $1 <= 8191
def(coarse_tune, "i")  { rpc(0, $1+0x40) }      // -64 <= $1 <= 63 

// 123: all notes off
all_notes_off = 'ctrl(123, 0)'

// allocator for virtual control change numbers
::cur_vctrl = 132
def(new_vctrl) {
  ::cur_vctrl += 1
  ::cur_vctrl
}
::cur_gctrl = 193
def(new_gctrl) {
  ::cur_gctrl += 1
  ::cur_gctrl
}
   
/*
 * define commands for text events
 */
def(comment, "s")      { text(1, $1) } 
def(copyright, "s") { {tk=1  text(2, $1)} }
def(trackname, "s") { text(3, $1) }
def(instname, "s")  { text(4, $1) }
def(lyric, "s")     { text(5, $1) }
def(cue, "s")       { text(7, $1) }
if( defined($format) && {$format == 2} ) {
   def(title, "s")   { text(1, $1) }
   def(seqname, "s") { text(3, $1) }
   def(mark, "s")  { text(6, $1) } 
} else { 
   def(title, "s")   { {tk=1  text(1, $1)} }
   def(seqname, "s") { {tk=1  text(3, $1)} }
   def(mark, "s")  { {tk=1  text(6, $1)} } 
}
marker = 'mark'

/*
 * forte, piano
 */
ppp   = 'v=20'
pp    = 'v=35'
//piano = 'v=50'
p     = 'v=50'
mp    = 'v=65'
mf    = 'v=80'	// default velocity
forte = 'v=95'
ff    = 'v=110'
fff   = 'v=127'

/*
 * effector flags
 */
TimeSort = 0x00  /* obsolete */
MergeTracks = 0x01
ExpandCtrl = 0x02

/*
 * for controlling dumper pedals  <undocumented>
 */
hold_delay = z
hold = '{ped(dt+=hold_delay) R pedoff}'

/*
 * macros/effectors for implementing pseudo-ties  <undocumented>
 *   Use like: [c e(tie) g] [g e(endtie) a](gr=10)
 */
defeff($bgn_tie) {
  case(note_off) { reject }
}
tie = '$bgn_tie()'

defeff($end_tie) {
  case(note_on) { reject }
}
endtie = '$end_tie()'
midtie = '$end_tie() $bgn_tie()'

// for backward compatibility
at = 'kp'
at_to = 'kp_to'
at_cto = 'kp_cto'
eff_events = 'set_eff_etypes()'
thru_events = 'set_thru_etypes()'
set_ethru = 'del_eff_etypes()'
clr_ethru = 'add_eff_etypes()'
eff_chs = 'set_eff_chs()'
thru_chs = 'set_thru_chs()'
set_cthru = 'del_eff_chs()'
clr_cthru = 'add_eff_chs()'
