/*
 * GM-specific definitions
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

reset_all_ctrls = 'ctrl(121,0)'

/* values for panpot control */
if( defined(gm_reverse_panpot) ) {
  right7 = 0
  right6 = 10
  right5 = 19
  right4 = 28
  right3 = 37
  right2 = 46
  right1 = 55
  center = 64
  left1  = 73
  left2  = 82
  left3  = 91
  left4  = 100 
  left5  = 109
  left6  = 118
  left7  = 127
} else {
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
}

/*
 * initial bender range in semitones
 */
init_bender_range = 2		// initial bender range is +-2 semitones

/* 
 * GM system on
 */
gm_system_on = 'excl(#(0x7e, 0x7f, 0x09, 0x01))'

/*
 * include instrument name file
 */
include("gm_inst")

/*
 * include rhythmic instrument name file
 */
include("gm_drums")
