/*
 * Roll, tremolo and trill effectors
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
 * roll([optional_parameters])
 *   The roll effector divide notes into a series of short notes
 *   for simulating "rolling" in drums or "tremolo" in strings.
 *   As for chords, all the notes in the chords are played for every 
 *   resultant short note.
 *
 *   Optional_parameters:
 *     speed = n     time interval between resulatant notes 
 *                    (default = z, i.e., 32nd notes)
 *     attack = n    extra velocity increment for the first note 
 *                   (default = 30)
 *     decay = n     velocity decay factor
 *                   The velocity of each note is determined by multiplying 
 *                   this value to the velocity of the previous note.
 *                   (default = 0.9)
 *     sustain = n   sustained velocity (absolute velocity)
 *                   When note velocity is decayed to less than this value, 
 *                   this value is used as the velocity rather than
 *                   decaying further. If this value is zero, the sustained
 *                   velocity is determined by the 'rsustain' parameter below.
 *                   (default = 0)
 *     rsustain = n  sustained velocity (relative value to the original 
 *                   note velocity)
 *                   Same as 'sustain' except that the sustained velocity is
 *                   measured relatively to the original note velocity,
 *                   i.e., the sustained velocity is the original velocity
 *                   times this value.
 *                   To make this parameter effective, the 'sustain' parameter
 *                   above must be zero.  (default = 0.7)
 *   Examples:
 *      To apply off-the-shelf rolling effects:
 *          => roll()
 *      To get faster rolls:
 *          => roll(speed = z/2)
 *      To applay rolloing effects without any velocity modifications:
 *          => roll(attack=0 decay=1)
 */
roll_speed = z
roll_attack = 30
roll_decay = 0.9
roll_sustain = 0
roll_rsustain = 0.7

defeff(roll, ":q") {
  init {
    speed = roll_speed
    decay = roll_decay
    attack = roll_attack
    sustain = roll_sustain
    rsustain = roll_rsustain
    $1
  }

  case(note) {
    $time = 0u
    $sus = sustain > 0 ? sustain : rsustain * v
    while( $time < do ) {
      $g = ($time + speed) > do ? do - $time : speed 
      note( dt+=$time do=$g if($time==0){v+=attack} )&
      $time += speed
      v *= decay
      if( v < $sus ) { v = $sus }
    }
    reject
  }
}

/*
 * tremolo([optional_parameters])
 *   The tremolo effector divide notes into a series of short notes
 *   for simulating "tremolo" in pianos or marimbas.
 *   As for chords, notes in the chords are grouped into two groups, 
 *   which are played alternately.
 *
 *   For single notes, the tremolo effector behaves just like as the roll 
 *   effector.  It is recommended to use the roll effector if chords are
 *   not included in the input stream, because the roll effector is much
 *   faster for compiling the source code.
 *
 *   Optional_parameters:
 *     The tremolo effector accepts all the parementers of the roll effector.
 *
 *     group = n      the number of notes in the second group
 *                    (default = 1)
 *
 *   Examples:
 *      To apply off-the-shelf tremolo effects:
 *          => tremolo()
 *      By this attachment, 
 *          => i C [D F] [C E G]
 *      will be played as:
 *          => z C C C C  D F D F  [C E] G [C E] G
 *             (actually velocities will be modified but we omittied it)
 *      If you prefer playing the last chord as:
 *          => [G E] C [G E] C
 *      then, reorder the notes in the chord in the original text like this:
 *          => i C [D F] [G E C]
 *      On the other hand, if you prefer playing the last chord as:
 *          => C [E G] C [E G]
 *      then, change the group parameter to 2 when attaching the effector like:
 *          => tremolo(group=2)
 */
tremolo_speed = z
tremolo_group = 1
tremolo_decay = 1.0
tremolo_attack = 0
tremolo_sustain = 0
tremolo_rsustain = 1.0

defeff(tremolo, ":q") {
  init { 
    def(gen_notes, "nna") {  // gen_notes(bgn_time, end_time, notes_array)
       if( $1 < $2 && #$3 > 0 ) {
          local::$time = $1
          local::$even = 1
          local::$rv = 1.0
          while( $time < $2 ) {
            $g = ($time + speed) > $2 ? $2 - $time : speed 
            if( #$3 <= group ) {
              for($i, 1, #$3) 
            } elsif( $even ) {
              for($i, 1, #$3-group)
            } else {
              for($i, #$3-group+1, #$3)
            } {
              { t=$time gt=$g n=$3[$i][1] 
                v= $3[$i][2] * $rv
		if($time==$1){v+=attack} elsif(v<sustain) {v=sustain}
                note }&
            }
            $even = 1 - $even
            $time += speed
            $rv *= decay
            if(sustain == 0) {if($rv < rsustain) { $rv = rsustain }}
          }
       }
    }

    speed = tremolo_speed
    group = tremolo_group
    decay = tremolo_decay
    attack = tremolo_attack
    sustain = tremolo_sustain
    rsustain = tremolo_rsustain
    $1
    $list = rep(16, #())  // active note list (per channel)
    $prev_time = rep(16, 0)  // time of previous event (per channel)
  }

  case(note_on) { 
    gen_notes($prev_time[ch], t, $list[ch])

    append($list[ch], #(n, v))
    $prev_time[ch] = t
    reject
  }

  case(note_off) { 
    gen_notes($prev_time[ch], t, $list[ch])

    $newlist = #() 
    foreach($i, $list[ch]) {
      if( $i[1] != n ) {  
        append($newlist, $i)
      }
    }
    $list[ch] = $newlist
    $prev_time[ch] = t
    reject
  }
}

/*
 * trill(interval [, optional_parameters])        <undocumented>
 *  The trill effector divide a note into a series of short notes in which
 *  two pitches are played alternately.
 *
 *  Arguments:
 *    interval     The interval between the two pitches in semitones.
 *
 *  Optional parmeters:
 *    speed = n    note length of each trilling note (default = z)
 *                 To obtain natural trilling effects, the trill effector
 *                 may use the note length shorter than this value.
 *    rvel = n     velocity of trilling notes relative to the velocity of
 *                 the original note (default = 0.7)
 *    first = 'note sequence'
 *                 Sequence of notes for the beginning part of trill.
 *                 Any number of notes can be contained in the sequence.
 *                 The notes can be specified in either an absolute pitch form
 *                 like 'D E F' or a relative pitch from like 'note(n+=1)
 *                 note() note(n-=1)'.  The length of each note is by default
 *                 equal to the value of the `speed' parameter, but it can be
 *                 overriddern by specifiying note length in the sequence.
 *                 (default = '')
 *    last = 'note sequence'
 *                 Sequence of notes for the ending part of trill.
 *                 For other  
 *                 
 *  Examples:
 *     To make trilling effects with halftone interval:
 *         => trill(1)
 *     To make a wholetone trill begin with the upper note:
 *         => trill(2, first = 'note(n+=2)')
 *         or you may use an absolute pitch like "trill(2, first='E')" if the
 *         pitch is known.
 *     To make a wholetone trill ending with 'C D' (like turning):
 *         => trill(2, last = 'C D')
 *         With the above effector attachment, D(q) note will be replaced to 
 *             i./7 D E D E D E D z C D
 */                
 
/* sub effector for finding the pitch of first and last notes in a sequence */
defeff(trill_sub, "") {
  init {
    $f = -1
    $l = -1
  }
  case(note) {
    if( $f == -1 ) { $f = n }
    $l = n
    reject
  }
  case(all) { reject }
  wrap {
    ..::..::$f = $f
    ..::..::$l = $l
  }
}

trill_speed = z
trill_rvel = 0.7

defeff(trill, "n:q") {  // $1 - trilling interval in semitones (>0)
  init {
    speed = trill_speed  // length of each trilling note (maximum value)
    rvel = trill_rvel    // relative velocity of trilling notes
    first = ''    // initial sequence
    last = ''     // ending sequence
    $2 
  }
  case(note) {
    $fcond = 0   // Trill should begin with (0) primary or (1) secondary note.
    $lcond = -1  // Trill should end with (0) primary or (1) secondary note,
                 // or (-1) free.
    $flen = 0u
    $llen = 0u
    if( first ) {
      { $t=t l=speed trill_sub() first ..::$flen=t-$t }&
      if( $l == n || $l > n + $1 ) { $fcond = 1 }
    }
    if( last ) {
      { $t=t l=speed trill_sub() last ..::$llen=t-$t }&
      if( $f == n || $f > n + $1 ) { $lcond = 1 }
      elsif( $f == n + $1 || $f < n ) { $lcond = 0 }
    }
    $tdur = do - $flen - $llen
    if( $tdur < 0 ) {
      error("trill: Too long first or last sequence")
    }
    $nnote = int(ceil($tdur / speed))
    if( $lcond >= 0 ) {
      if( $fcond == $lcond ) {  // the number of notes should be odd.
	if( ($nnote & 1) == 0 ) { $nnote += 1 }
      } else {  // the number of notes should be even.
	if( ($nnote & 1) == 1 ) { $nnote += 1 }
      }
    }
    dr=100
    
    /* output the first part */ 
    {l=speed first}
    
    /* output the trilling part */
    {
      l = $tdur / $nnote
      $v = v * rvel
      if( first ) { v = $v }
      for($i, 0, $nnote - 1) {
	note( if( $fcond ) { n += $1 } )
	$fcond = ! $fcond
	v = $v
      }
    }
      
    /* output the last part */
    {l=speed last}
    
    reject
  }
}

