/*
 * Effector for changing temperament using pitch-bend controllers
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
 * temperament(pitcharray, tonic [, nch])
 *
 * Arguments:
 *   pitcharray    Array of 12 integers each of which represents
 *                 a pitch-bend value for a different interval from the
 *                 tonic note.
 *   tonic         MIDI note number of the tonic note.
 *                 Octave is not significant.
 *   nch           Number of MIDI channels allocated for each of
 *                 the MIDI channel in the input events.
 *                 Since pitch-bend control is on a per-channel basis,
 *                 it is impossible to control the fine pitch of each note
 *                 within a chord using a single channel.  By setting nch
 *                 to more than 1, MIDI channles ch, ch+1, ..., ch+nch-1
 *                 are dynamically allocated for the input note event whose
 *                 channel is ch, so that up to nch notes can be
 *                 simultaneously played with correct pitch.
 *                 By default, nch is 1; therefore, no chord is allowed.
 * Limitations:
 *   - The difference between '#' and 'b' is not recognized.
 * Example:
 *   temperament(Pure, F, 3)
 *
 * The change_temperament and change_tonic macros can be used for
 * dynamically changing 'pitcharray' and 'tonic' parameters respectively.
 */

/* virtual controllers for changing 'pitcharray' and 'tonic' */
$change_temp_ctrl = new_gctrl() 
def(change_temperament, "a") { ctrl($change_temp_ctrl, $1) }
$change_tonic_ctrl = new_gctrl() 
def(change_tonic, "i") { ctrl($change_tonic_ctrl, $1) }

defeff(temperament, "ai:i", MergeTracks) {
  init {
    bendtbl = $1
    $tonic = $2
    nch = $# > 2 ? {$3} : 1 
    nt = rep(16, -1)   // note number of the playing note of each channel
  }

  case(note_on) { 
    // find an empty channel (if not found, use the original channel)
    for($i, ch, ch + nch - 1) {
      if( nt[$i] == -1 ) { ch = $i  break }
    }
    bend(bendtbl[mod12(n - $tonic) + 1])
    nt[ch] = n
  }

  case(note_off) { 
    for($i, ch, ch + nch - 1) {
      if( nt[$i] == n ) { ch = $i  break }
    }
    nt[ch] = -1
  }

  case(ctrl($change_temp_ctrl)) {
    bendtbl = val
  }

  case(ctrl($change_tonic_ctrl)) {
    $tonic = val
  }
}

// Sample Arrays of pitch-bend values
//  (Bender range of +-2 semitones is assumed)
Equal = #(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0)
Pure = #(0, 481, 160, 641, -561, -80, -400, 80, 561, -641, 721, -481)

