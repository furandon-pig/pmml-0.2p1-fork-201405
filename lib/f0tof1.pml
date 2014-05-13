/*
 * Convert format-0 MIDI file to format-1 MIDI file
 *   In the output file, Track 1 will contain a tempo map, 
 *   Track 2 will contain exclusive messages, 
 *   and Track 3 thru. 18 will contain messages per channel.
 *
 * usage:  pmml -i f0tof1 -o format-1-file  -l format-0-file 
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

$format = 1
if( !defined($resolution) )	{ $resolution = 0 }
if( !defined($no_run_stat) )	{ $no_run_stat = 0 }
if( !defined($pack_tracks) )	{ $pack_tracks = 0 }
if( !defined($no_retrigger) )	{ $no_retrigger = 0 }
if( !defined($ch_mask) )	{ $ch_mask = 0 }
if( !defined($track_list) )	{ $track_list = "" }
if( !defined($pos_begin) )	{ $pos_begin = "" }
if( !defined($pos_end) )	{ $pos_end = "" }

$smfout($outfile, $resolution, $format, 
	$no_run_stat, $pack_tracks, $no_retrigger, 
	$ch_mask, $track_list, $pos_begin, $pos_end)

defeff(diverse_tracks) {
  case(note, ctrl(0-191)) {
    tk = ch + 2
  }
  case(excl, arbit) {
    tk = 2
  }
  case(all) {  // other types
    tk = 1
  }
}

diverse_tracks()
