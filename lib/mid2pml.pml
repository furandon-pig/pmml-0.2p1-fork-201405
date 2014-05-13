/*
 * Simple MIDI file to PMML translator
 *
 * usage:  pmml -i mid2pml -l MIDI_file
 *
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

defeff(mid2pml) {
  init {
    def(put_header) {
      if( tk != prev_tk ) {
        printf("/* Track No. %d */\n", tk)
        printf("tk=%d\n", tk)
        prev_tk = tk

        measure_length = w
        beat_length = q
        measure_base = 1
        time_base = time_offset
        tsigidx = 1
      }

      while( tsigidx <= #tsigs && {tsigs[tsigidx][1] <= t} ) {
        local::$st = tsigs[tsigidx][1]
	if( $st >= time_offset ) {
          measure_base += ceil(($st - time_base) / measure_length)
          time_base = $st
        }
        measure_length = tsigs[tsigidx][2]
        beat_length = tsigs[tsigidx][3]
	tsigidx += 1
      } 

      if( t >= time_base ) {
        local::$dt = (t - time_base) % measure_length
        printf("t=%8gu /* %03d:%1d:%03d */  ", t * 1920.0, 
	  floor((t - time_base) / measure_length) + measure_base,
	  floor($dt / beat_length) + 1, $dt % beat_length)
      } else {
        printf("t=%8gu /* %9d */  ", t * 1920.0, (t - time_base) * 1920.0)
      }
    }

    prev_tk = -1
    measure_length = w
    beat_length = q
    measure_base = 1
    time_offset = 0u
    $1
    time_base = time_offset
    tsigs = #()
    tsigidx = 1
  }
 
  case(note) {
    put_header()
    printf("ch=%-2d    %-3s(v=%d gt=%gu", ch, notename(n), v, gt * 1920.0)
    if( nv != -1 ) {
      printf(" nv=%d)\n", nv)
    } else {
      printf(")\n")
    }
  }

  case(ctrl(1)) {
    put_header()
    printf("ch=%-2d  mod(%d)\n", ch, val)
  }
  case(ctrl(2)) {
    put_header()
    printf("ch=%-2d  breath(%d)\n", ch, val)
  }
  case(ctrl(4)) {
    put_header()
    printf("ch=%-2d  foot(%d)\n", ch, val)
  }
  case(ctrl(5)) {
    put_header()
    printf("ch=%-2d  pmtime(%d)\n", ch, val)
  }
  case(ctrl(7)) {
    put_header()
    printf("ch=%-2d  vol(%d)\n", ch, val)
  }
  case(ctrl(10)) {
    put_header()
    printf("ch=%-2d  pan(%d)\n", ch, val)
  }
  case(ctrl(11)) {
    put_header()
    printf("ch=%-2d  expr(%d)\n", ch, val)
  }
  case(ctrl(64)) {
    put_header()
    printf("ch=%-2d  ctrl(Ped, %d)\n", ch, val)
  }
  case(ctrl(65)) {
    put_header()
    printf("ch=%-2d  ctrl(Pm, %d)\n", ch, val)
  }
  case(ctrl(67)) {
    put_header()
    printf("ch=%-2d  ctrl(Sped, %d)\n", ch, val)
  }
  case(prog) {
    put_header()
    printf("ch=%-2d  prog(%d)\n", ch, val)
  }
  case(bend) {
    put_header()
    printf("ch=%-2d  bend(%d)\n", ch, val)
  }
  case(kp) {
    put_header()
    printf("ch=%-2d  kp(%d) (n=%s)\n", ch, val, notename(n)) 
  }
  case(cpr) {
    put_header()
    printf("ch=%-2d  cpr(%d)\n", ch, val)
  }
  case(tempo) {
    put_header()
    printf("       tempo(%d)\n", val)
  }
  case(ctrl) {
    put_header()
    printf("ch=%-2d  ctrl(%d, %d)\n", ch, etype, val)
  }

  case(text) {
    put_header()
    printf("       text(%d, \"%s\")\n", etype - 0x100, val)
  }
    
  case(seqno) {
    put_header()
    printf("       seqno(%d)\n", val)
  }

  case(smpte) {
    put_header()
    printf("       smpte(%d,%d,%d,%d,%d)\n",
	   val[1], val[2], val[3], val[4], val[5])
  }

  case(timesig) {
    put_header()
    p3 = (val[3] shl val[2])/96
    p4 = val[4]
    if( p4 != 8 ) {
    	printf("       timesig(%d,%d,%d,%d)\n", 
        	   val[1], 1 shl val[2], p3, p4)
    } elsif( p3 != 1 ) {
    	printf("       timesig(%d,%d,%d)\n", 
        	   val[1], 1 shl val[2], p3)
    } else {
    	printf("       timesig(%d,%d)\n", 
        	   val[1], 1 shl val[2])
    }
    if( tk == 1 ) {
      append(tsigs, #(t, rational(val[1], 1 shl val[2]), val[3] * q / 24))
    }
  }

  case(keysig) {
    put_header()
    printf("       keysig(%d,%d)\n", 
	   val[1] shl 24 shr 24, val[2])  // shifting is for sign expansion
  }

  case(meta) { 
    put_header()
    printf("       meta(%d, #(", etype - 0x100)
    for($i, 1, #val) { 
      printf("0x%02x", val[$i])
      if( $i != #val ) { printf(",") }
    }
    printf("))\n")
  }

  case(excl) {
    put_header()
    if( #val && {val[#val] == 0xf7} ) { 
      printf("       excl(#(")
      for($i, 1, #val-1) { 
        printf("0x%02x", val[$i])
        if( $i != #val-1 ) { printf(",") }
      }
    } else {
      printf("       excl2(#(")
      for($i, 1, #val) { 
        printf("0x%02x", val[$i])
        if( $i != #val ) { printf(",") }
      }
    }
    printf("))\n")
  }
     
  case(arbit) {
    put_header()
    printf("       arbit(#(")
    for($i, 1, #val) { 
      printf("0x%02x", val[$i])
      if( $i != #val ) { printf(",") }
    }
    printf("))\n")
  }
}

//mid2pml(time_offset = 3000)
mid2pml()
