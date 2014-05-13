/*
 * PMML basic effector library
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
 * modify_notes(modification commands, ...) : modify note attribues
 *   Examples: 
 *     To increase the velocity of every note by 20:
 *          => modify_notes(v+=20) 
 *     To multiply the velocity of every note by 1.5:
 *          => modify_notes(v*=1.5)
 *     To adject all the velocities to 80:
 *          => modify_notes(v=80)
 *     To shorten the duration of every note:
 *          => modify_notes(gt*=0.8)
 */
defeff(modify_notes, "q") {
  case(note) {
    $1
  }
}

/*
 * add_notes(modification commands, ...) : adding extra notes for each note
 *   Examples: 
 *      To perform notes in octaves:
 *          => add_notes(n+=12) 
 *      To constitute a triad code for each note:
 *          => add_notes(n+=4, n+=7)
 *      To add an note with 1-octave up, half velocity and delayed by 
 *      the duration of a sixteenth note:
 *          => add_notes(n+=12 v*=0.5 dt+=s)
 */
defeff(add_notes, "q*") {
  case(note) {
    foreach($i, $*) { note($i)& }
  }
}

/*
 * modify_ctrl(controller_no, modification cmds) : modify control values
 *   Examples: 
 *     To increase the values of volume controller by 30:
 *          => modify_ctrl(Vol, val+=30)  or modify_ctrl(7, val+=30)
 */
defeff(modify_ctrl, "nq") {
  init {
    set_eff_etypes()(ctrl($1))
  }
  case(ctrl) {
    $2
  }
}

/*
 * sel_events(event_types, ...) : select specific types of events
 *   Examples:
 *     To extract only the note events:
 *          => sel_events(note)
 */
defeff(sel_events, "q*") {
  init {
    set_thru_etypes()($@)
  }
}

/*
 * rej_events(event_types, ...) : reject specific types of events
 *   Examples:
 *     To remove system exclusive messages:
 *          => rej_events(excl,arbit)
 *     To remove control change events with Controller Number 1:
 *          => rej_events(ctrl(1))
 */
defeff(rej_events, "q*") {
  init {
    set_eff_etypes()($@)
  }
}

/*
 * sel_chs(channel_numbers, ...) : select events with specific MIDI channels
 *   Examples:
 *     To extract events for Ch.2 and remove events for all the other channels:
 *          => sel_chs(2)
 *     To extract events for Channels 2,3,4,5 and 10 and remove event for all
 *     the other channels:
 *          => sel_chs(2-5,10)
 */
defeff(sel_chs, "q*") { 
  init {
    set_eff_etypes()(note, ctrl(0-191))
    set_thru_chs()($@)
  }
}

/*
 * rej_chs(channel_numbers, ...) : reject events with specific MIDI channels
 *   Examples:
 *     To remove events for Channels 2 and 4:
 *          => rej_chs(2,4)
 */
defeff(rej_chs, "q*") {
  init {
    set_eff_etypes()(note, ctrl(0-191))
    set_eff_chs()($@)
  }
}

/*
 * tr_chs(from1, to1, from2, to2, ...) : translate channel numbers
 *   Examples:
 *     To replace Channel Number 4 in events with Channel Number 3:
 *          => tr_chs(4,3)
 */
defeff(tr_chs, "n*") { 
  init {
     $map = #( for($i,1,16) {$i,} )
     while( $# >= 2 ) {
       $map[$1] = $2
       shift($*,2)
     }
  }
  case(note, ctrl(0-191)) {
    ch = $map[ch]
  }
}

/*
 * remap_chs(ch1, ch2, ..,, ch16) : translate all the channel numbers 
 *   Examples:
 *      To translate channel numbers as follows: 
 *      ch.1->ch.2, ch.2->ch.3, ch.3->ch.1 
 *          => remap(2,3,1,4,5,6,7,8,9,10,11,12,13,14,15,16)
 *
 */
defeff(remap_chs, "nnnnnnnnnnnnnnnn") {
  case(note, ctrl(0-191)) {
    ch = $*[ch]
  }
}

/*
 * tr_prog(from1, to1, from2, to2, ...) : translate program numbers 
 *   Examples:
 *     To replace Program Number 45 with 33 in program-change events:
 *          => tr_prog(45,33)
 */
defeff(tr_prog, "n*") {
  init {
     $map = #( for($i,1,128) {$i,} )
     while( $# >= 2 ) {
       $map[$1] = $2
       shift($*,2)
     }
  }
  case(prog) {
    val = $map[val]
  }
}

/*
 * tstretch(rate) : stretch or reduce time axis
 *   The time_stretch effector magnifies the duration D between the
 *   time of each event and the time when the effector is attached.
 *   This effector changes D to (D * rate).
 *   Examples:
 *     To stretch time by two:
 *          => tstretch(2.0)
 *     To reduce time into half:
 *          => tstretch(0.5)
 */
defeff(tstretch, "n") {
  init {
    $attach_time = t
  }
  case(all) {
    t = (t - $attach_time) * $1 + $attach_time
  }
}

/*
 * retrograde() : inverse time axis  <undocumented>
 *   The retrograde effector inverses the time axis in the period between
 *   the attaching time and detaching time.
 *   Note-on and note-off events are interchanged.
 */
defeff(retrograde) {
  init {
    $at = t
  }
  detach {
    $dt = t
  }
  case(note_on) {
    t = $dt - (t - $at) 
    note_off(n)
    reject
  }
  case(note_off) {
    t = $dt - (t - $at) 
    note_on(n)
    reject
  }
  case(all) {
    t = $dt - (t - $at) 
  }
}

/*
 * swing(period, percentage) : transform time axis so that notes on up beats
 * are delayed.
 *   Time is transformed so that the time of events located at the middle 
 *   of each period is moved to (percentage/100) * period.
 *   The time of events located on the boundary of periods is unchanged.
 *
 *   Examples:
 *     To change two eighth notes with 
 *     a quarter note and an eighth note within a triplet:
 *          => swing(q, 66) or more pericisely swing(q, 200u/3)
 *     To replace two quarter notes with 
 *     a dotted quarter note and an eighth note:
 *          => swing(h, 75)
 */
defeff(swing, "rr") {
  init {
    $attach_time = t
    $b = $2 / 50u - 1w
  }
  case(all) {
    $tr = (t - $attach_time) % $1     
    if( $tr <= $1 / 2 ) {
      t += $b * $tr
    } else {
      t += $b * ($1 - $tr)
    }
  }
}

/*
 * arp(delay_step) : apply arpeggio effects to chords
 *   The arpeggio effector delays the n-th notes in each chord 
 *   by (n-1) * delay_step.
 *
 *   Examples:
 *          => arp(s)
 *          => w [c e g ^c]
 *      In the above example, the chord will actually be played as below:
 *          => {c(w)}&
 *             {r(s)  e(w-s)  }&
 *             {r(2s) g(w-2s) }&
 *             {r(3s) ^c(w-3s)}
 *      If you want to use reverse-order arpeggios, reverse the order of 
 *      notes in the chord like:
 *          => [^c g e c]
 */
defeff(arp, "n") {
  init {
    $previous_time = -1
  }
  case(note) {
    if( $previous_time != t ) {
       $delay = 0
    } else {
       $delay += $1
    }
    $previous_time = t
    t += $delay
    gt -= $delay
  }
}

/*
 * thin_ctrl(contoller_number, value_step) : thinning on control change
 *   The thin_ctrl effector decreases the number of control-change events
 *   by removing such control-change events having control values whose
 *   difference from the value of the previous control change is less than
 *   value_step.
 *
 *   Examples:
 *     To make the interval of control values on volume controller 
 *     more than or equal to 2:
 *          => thin_ctrl(Vol,2)
 */
defeff(thin_ctrl, "nn") {  // thin_ctrl(func, val_step)
  init {
    $prev_val = rep(MAXNEGINT, 16)
    set_eff_etypes()(ctrl($1))
  }
  case(ctrl) {
    if( abs(val - $prev_val[ch]) >= $2 ) { 
       $prev_val[ch] = val
    } else {
      reject
    }
  }
}

/*
 * rand_vel(amplitude) : randomize note velocity
 *   A white random noise is added to the velocity of each note-on event.
 *   The random noise ranges from (-amplitude) to (amplitude).
 */
defeff(rand_vel, "n") {
  case(note) { v += irand(-$1, $1) }
}

/* fgrand_vel(amplitude) : similar to rand_vel but uses fgrand() */
defeff(fgrand_vel, "n") {
  case(note) { v += fgrand(-$1, $1) }
}

/*
 * rand_ntime(amplitude) : randomize note time
 *   A white random noise is added to the time of each note event.
 *   The random noise ranges from (-amplitude) to (amplitude).
 *
 *   This effector changes only the time of note events and does not
 *   modify the time of other types of events such as control changes.  
 *   Therefore, it is possible that resultant notes are played with different 
 *   control values or a program number from those of the original notes.
 */
defeff(rand_ntime, "n") {
  case(note) { t += irand(-$1, $1) }
}

/* fgrand_ntime(amplitude) : similar to rand_ntime but uses fgrand() */
defeff(fgrand_ntime, "n") {
  case(note) { t += fgrand(-$1, $1) }
} 

/*
 * rand_ctrl(controller_no, amplitude) : randomize control 
 * values
 *   A white random noise is added to the value of each control-change event
 *   whose controller number is equal to controller_no.
 *   The random noise ranges from (-amplitude) to (amplitude).
 */
defeff(rand_ctrl, "nn", ExpandCtrl) {
  init {
    set_eff_etypes()(ctrl($1))
  }
  case(ctrl) { val += irand(-$2, $2) }
}

/* fgrand_ctrl(ctl, amplitude) : similar to rand_ctrl but uses fgrand() */
defeff(fgrand_ctrl, "nn", ExpandCtrl) {
  init {
    set_eff_etypes()(ctrl($1))
  }
  case(ctrl) { val += fgrand(-$2, $2) }
}


/*
 * dump(): print out all the events to this effector for debugging purposes
 */
defeff(dump, "", ExpandCtrl) {
  init {
    def(printhdr, "") {
      printf("tk=%d t=%-14s", tk, 
	     sprintf("%d+%d/%d", intg(t), num(t), den(t)))
    }
  }
  case(note_on) {
    printhdr()
    printf("NoteOn    ch=%d n=%s v=%g\n", ch, notename(n), v);
  }
  case(note_off) {
    printhdr()
    printf("NoteOff   ch=%d n=%s nv=%g\n", ch, notename(n), nv);
  }
  case(prog) {
    printhdr()
    printf("ProgChg   ch=%d val=%d\n", ch, val);
  }
  case(cpr) {
    printhdr()
    printf("ChanPres  ch=%d val=%g\n", ch, val);
  }
  case(bend) {
    printhdr()
    printf("PitchBend ch=%d val=%g\n", ch, val);
  }
  case(kp) {
    printhdr()
    printf("KeyPres   ch=%d n=%s val=%g\n", ch, notename(n), val);
  }
  case(tempo) {
    printhdr()
    printf("Tempo     val=%g\n", val);
  }
  case(ctrl) {
    printhdr()
    printf("Control   ch=%d ctrl=%d val=", ch, etype);
    print(val)
  }

  case(text) {
    printhdr()
    printf("Text      type=%d str=\"%s\"\n", etype - 0x100, val);
  }
  case(seqno) {
    printhdr()
    printf("SeqNo     val=%d\n", val);
  }
  case(smpte) {
    printhdr()
    printf("SMPTE     hr=%d mn=%d se=%d fr=%d ff=%d\n", 
	   val[1], val[2], val[3], val[4], val[5]);
  }
  case(timesig) {
    printhdr()
    printf("TimeSig   nn=%d dd=%d cc=%d bb=%d\n", 
	   val[1], val[2], val[3], val[4]);
  }
  case(keysig) {
    printhdr()
    printf("KeySig    sf=%d mi=%d\n",
	   val[1] shl 24 shr 24, val[2]);  // shifting is for sign expansion
  }
  case(meta) { 
    printhdr()
    printf("Meta      type=%02x data=[", etype - 0x100);
    for($i, 1, #val) { 
      printf("%02x", val[$i])
      if( $i != #val ) { printf(" ") }
    }
    printf("]\n")
  }

  case(excl,arbit) {
    printhdr()
    if( etype == 0x190 ) {
      printf("Excl      data=[");
    } else {
      printf("Arbit     data=[");
    }
    for($i, 1, #val) { 
       printf("%02x", val[$i])
       if( $i != #val ) { printf(" ") }
    }
    printf("]\n")
  }
}

/*
 * auto_accent(period, accent1, accent2, ...)
 *   The auto_accent effector increases the velocity of notes located at
 *   the begining of each period.  The duration of the period is specified
 *   the first parameter.  The rest parametsers spefify velocity increments.
 *   If more than one increment is specified, the increment values are used 
 *   in a round-robin fashion.
 *
 *   To handle syncopated notes, velocity is also increased if a note 
 *   started during the previous preiod continues until the current preiod.
 *
 *   Examples:
 *      We assume 4/4 time signature in the examples below.
 *
 *      To increase velocity by 20 for notes located at the begging of
 *      each measure:
 *          => auto_accent(w, 20)
 *      To increase velocity by 40 for notes located at the begging of
 *      each measure and increase it by 20 for notes located at the middle
 *      of each measure:
 *          => auto_accent(h, 40, 20)
 */
auto_accent_torelance = z/2

defeff(auto_accent, "rnn*") {  // auto_accent(period, accent1, accent2, ...)
  init {
    $attach_time = t
  }
  case(note) {
    $tl = (t - $attach_time) % $1     
    $ti = floor((t - $attach_time) / $1)
    if( $tl < auto_accent_torelance ) { v += $*[$ti % ($#-1) + 2] }
    elsif( $tl + du > $1 ) {  // syncopation condition
      v += $*[($ti+1) % ($#-1) + 2]
    }
  }
}

/*
 * quantize(step [, optional_paramenters])
 *   The quantize effector adjusts event time to the nearest multiple
 *   of "step". 
 *
 *   Optional_parameters:
 *     etype = 'event_type, event_type, ...'
 *                    type of events to which quantization is applied
 *                    The format of event_type is compatible with 
 *                    the set_eff_etypes command.  (default = 'note')
 *     strength = n   strength of adjustment
 *                    If n = 100, the resultant event time becomes
 *                    just a multiple of "step".  If n < 100, the time 
 *                    change is mitigated to n%.  (default = 100)
 *     window = n     width of the window where quantization is active
 *                    Quantaization is performed only if the distance 
 *                    to the nearest multiple is less than (n/100) * (step/2).
 *                    (default = 100)
 *     keepdur = 0/1  If keepdur = 0, the time of note-off events is
 *                    also quantized using the same rule as note-on.
 *                    If keepdur = 1, the time of note-off events is
 *                    adjusted so that the duration of each note is
 *                    maintained.  (defualt = 1)
 *   Examples:
 *      To quantize note events with eighth-note steps:
 *          => quantize(i)
 *      To quantize all kinds of events with sixteenth-note steps:
 *          => quantize(s, etype='all')
 */
quantize_etype = 'note'
quantize_strength = 100
quantize_window = 100
quantize_keepdur = 1

defeff(quantize, "r:q") {
  init {
    $attach_time = t
    etype = quantize_etype
    strength = quantize_strength
    window = quantize_window
    keepdur = quantize_keepdur
    $2
    set_eff_etypes()(etype)
    $w = $1 * window / 200
  }
  case(note) {
    $org_t = t
    $dt = (t - $attach_time) % $1
    if( $dt < $w ) {
      t -= $dt * strength / 100
    } elsif( $dt >= $1 - $w ) {
      t += ($1 - $dt) * strength / 100
    }
    if( !keepdur ) {
      $dt = ($org_t + do - $attach_time) % $1
      do += $org_t - t
      if( $dt < $w ) {
        do -= $dt * strength / 100
      } elsif( $dt >= $1 - $w ) {
        do += ($1 - $dt) * strength / 100
      }
    }
  }
  case(all) {
    $dt = (t - $attach_time) % $1
    if( $dt < $w ) {
      t -= $dt * strength / 100
    } elsif( $dt >= $1 - $w ) {
      t += ($1 - $dt) * strength / 100
    }
  }
}

/*
 * grace(grace_notes) : insert grace notes at the beginnig of each note
 * agrace(grace_notes) : insert grace notes at the end of each note (After)
 * bgrace(grace_notes) : insert grace notes before each note (Before)
 *   <undocumented>
 *   These effectors attaches grace notes to each note.
 *   The 'grace' effector shrinks the duration of the original note at its
 *   beginning part by the length of the grace_notes sequence, and pad
 *   the grace_notes sequence there.
 *   The 'agrace' effector shrinks the duration of the original note at its
 *   last part, and pad the grace_notes sequence there.
 *   The 'bgrace' effector keeps the orignal note as it is, and insert
 *   the grace_notes sequence just before the original note.  The grace notes
 *   may sound overlapped with the previous note.
 *
 *   The note length of grace notes is 'z' by default; however, it can be
 *   changed by specifying note length in the argument.  The velocity of
 *   grace notes are decremented by the value of the 'ac' register .
 * 
 *   Examples:
 *     To attach a 32nd grace note 'D' to a quarter note 'E':
 *        => q E(grace(D))    // equivalent to D--(z) E(q-z)
 *        => q E(agrace(D))   // equivalent to E(q-z) D--(z)
 *        => q E(bgrace(D))   // equivalent to t-=z D--(z) E(q)
 *     To attach two 64th notes:
 *        => grace(z/2 C D)
 *     To attach a grace note semitone higher than the original note:
 *        => grace(note(n+=1))
 */
defeff(grace, "q") {
  case(note) { 
    $t = t 
    {dr=100 z -- $1}
    do -= t - $t
  }
}

defeff(agrace, "q") {
  case(note) {
    { $t=t  rej_events(all) z $1 ..::$du=t-$t }&
    { t=t+do-$du dr=100 z -- $1 }&
    do -= $du
  }
}

defeff(bgrace, "q") {
  case(note) {
    { $t=t  rej_events(all) z $1 ..::$du=t-$t }&
    { t-=$du dr=100 z -- $1 }&
  }
}


/*
 * chord_apply(modification commands, ...)
 *   <undocumented>
 *   The chord_apply effector applies 'modification commands' for the
 *   corresponding note in each chord.
 *   The first argument is applied to the last-designated note
 *   (which usually has the highest pitch) in each chord, the second
 *   argument is applied to the second note from the last, and so on.
 *
 *   The last argument of chord_apply may be a word 'ditto' indicating
 *   that the modification commands just before 'ditto' are repeated
 *   infinitely.
 *
 *   When `$i' is used in the modification commands, it is replaced with
 *   the index number of notes in each chord
 *   counted from the last-designated note in each chord.
 *
 *   Examples:
 *      To increase the velocity of the last-designated note in each chord
 *      by 20:
 *          => chord_apply(v+=20)
 *      To decrease the velocity of the second note from the last in each
 *      chord by 10:
 *          => chord_apply(,v-=10)
 *      To decrease the velocity of notes except the last-designated note
 *      in each chord by 10:
 *          => chord_apply(,v-=10,ditto)
 *      To trill the last-designated note in each chord:
 *          => chord_apply(trill(1))
 *      To decrease the velocity of notes in each chord linearly:
 *          => chord_apply(v-=$i*10, ditto)
 */
defeff(chord_apply, "q*") {
  init { 
    def(flush_notes, "a") {
      local::$i = #$list[ch]
      foreach($p, $list[ch]) {
	{
	  t = $prev_time[ch]
	  n = $p[1]
	  v = $p[2]
	  nv = $p[3]
	  do = $p[4]
	  if($i <= #$1) { $1[$i] }
	  elsif( dittoflag ) { $1[#$1] }
	  note
	  $i -= 1
	}&
      }
      $list[ch] = #()
    }

    $list = rep(16, #())  // active note list (per channel)
    $prev_time = rep(16, 0)  // time of previous event (per channel)
    if( $# && {`$[$#]` == 'ditto'} ) {
      dittoflag = 1
      shift($*,-1)
    } else {
      dittoflag = 0
    }
  }

  case(note) { 
    if( t != $prev_time[ch] ) {
      flush_notes($*)
    }
    append($list[ch], #(n, v, nv, do))
    $prev_time[ch] = t
    reject
  }

  wrap {
    flush_notes($*)
  }
}

def(slim_chord, "n:r") {
  if( $# == 1 ) {
    chord_apply(, v-=$1, ditto)
  } else {
    chord_apply(dt+=$2, v-=$1, ditto)
  }
}
