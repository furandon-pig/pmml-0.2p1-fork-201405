/*
 * fix some anormalities in the output file when linking some MIDI files 
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

defeff($fix_link_problem) {
  init {
    prev_timesig = #()
    prev_keysig = #()
    prev_copyright = 0
    prev_trkname = 0
    prev_trkname_tk = 0
  }
  case(timesig) {
    if( val == prev_timesig ) {
      reject
    }
    prev_timesig = val
  }
  case(keysig) {
    if( val == prev_keysig ) {
      reject
    }
    prev_keysig = val
  }
  case(smpte, seqno) {
    if( t != 0 ) {
      reject
    }
  }
  case(text(2)) {  /* copyright notice event */
    t = 0
    if( val == prev_copyright ) {
      reject
    }
    prev_copyright = val
  }
  case(text(3)) {  /* track/sequence name */
    t = 0
    if( tk == prev_trkname_tk && val == prev_trkname ) {
      reject
    }
    prev_trkname = val
    prev_trkname_tk = tk
  }
  case(meta) { }    // To keep the sequential order of meta events 
}

$fix_link_problem()
