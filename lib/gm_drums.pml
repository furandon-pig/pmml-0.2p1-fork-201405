/*
 * Model-specific drums macro definitions for GM
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
 * This file is read not only by 'pmml' but also by 'm2p' to get the mapping
 * from note numbers to rhythmic instrument names.  Each line defining a macro
 * for a rhythmic instrument name must use the following format:
 *   <instrument name up to 24 characters> = 'note(n=<note number>)'
 * To comment out the above line, use '//' at the beginning of line (do not
 * use '/* .. */'), because 'm2p' only recognizes '//'.
 */

/*
 * Do not use pitch names like 'c4' for specifying note numbers so that
 * the drum instrument mapping is not affected by transposition. 
 */

/*
 * macro for each instrument
 */
AcouBD		= 'note(n=35)'
  BD2		= 'note(n=35)'
BassDrum1	= 'note(n=36)'
  BD		= 'note(n=36)'
SideStick	= 'note(n=37)'
  RimShot	= 'note(n=37)'
AcouSD		= 'note(n=38)'
  SD		= 'note(n=38)'
HandClap	= 'note(n=39)'
ElecSnare	= 'note(n=40)'
  SD2		= 'note(n=40)'
LowFloorTom	= 'note(n=41)'
  LowTom2	= 'note(n=41)'
  LT2		= 'note(n=41)'
  TMg		= 'note(n=41)'
ClosedHH	= 'note(n=42)'
HighFloorTom	= 'note(n=43)'
  LowTom1	= 'note(n=43)'
  LT1		= 'note(n=43)'
  TMf		= 'note(n=43)'
PedalHH		= 'note(n=44)'
LowTom		= 'note(n=45)'
  MidTom2	= 'note(n=45)'
  MT2		= 'note(n=45)'
  TMl		= 'note(n=45)'
OpenHH		= 'note(n=46)'
LowMidTom	= 'note(n=47)'
  MidTom1	= 'note(n=47)'
  MT1		= 'note(n=47)'
  TMm		= 'note(n=47)'
HiMidTom	= 'note(n=48)'
  HighTom2	= 'note(n=48)'
  HT2		= 'note(n=48)'
  TMi		= 'note(n=48)'
CrashCY1	= 'note(n=49)'
  CrashCY	= 'note(n=49)'
  CY		= 'note(n=49)'
HighTom		= 'note(n=50)'
  HighTom1	= 'note(n=50)'
  HT1		= 'note(n=50)'
  TMh		= 'note(n=50)'
RideCY1		= 'note(n=51)'
  RideCY	= 'note(n=51)'
ChineseCY	= 'note(n=52)'
RideBell	= 'note(n=53)'
Tambourine	= 'note(n=54)'
SplashCY	= 'note(n=55)'
Cowbell		= 'note(n=56)'
CrashCY2	= 'note(n=57)'
Vibraslap	= 'note(n=58)'
RideCY2		= 'note(n=59)'
HiBongo		= 'note(n=60)'
LowBongo	= 'note(n=61)'
MuteHiConga	= 'note(n=62)'
OpenHiConga	= 'note(n=63)'
LowConga	= 'note(n=64)'
HighTimbale	= 'note(n=65)'
LowTimbale	= 'note(n=66)'
HighAgogo	= 'note(n=67)'
LowAgogo	= 'note(n=68)'
Cabasa		= 'note(n=69)'
Maracas		= 'note(n=70)'
ShortWhistle	= 'note(n=71)'
LongWhistle	= 'note(n=72)'
ShortGuiro	= 'note(n=73)'
  Quijada	= 'note(n=73)'	// MT-32 only
LongGuiro	= 'note(n=74)'
Claves		= 'note(n=75)'
HiWoodBlock	= 'note(n=76)'
LowWoodBlock	= 'note(n=77)'
MuteCuica	= 'note(n=78)'
OpenCuica	= 'note(n=79)'
MuteTriangle	= 'note(n=80)'
OpenTriangle	= 'note(n=81)'

/*
 * macro for a group of instruments
 */
// hi-hat 	x:closed o:open p:pedal
HH 		= 'note(n=42)'	
// tom 		h:high i:mid-high m:mid l:low f:floor g:grand-floor
TM = `note(n=50)(DCm='n-=3' DCM='DCm DCX' DCl='n-=5' DCL='DCl DCX')`

Bongos		= 'note(n=60)'	// h:high l:low			
Congas		= 'note(n=63)'	// h:open-high m:muted-high l:low
Timbales	= 'note(n=65)'	// h:high l:low
Agogos		= 'note(n=67)'	// h:high l:low
// Do not use 'Whistle' because it is used as a timbre name.
Whistles	= 'note(n=71)'	// s:short l:long
Guiro		= 'note(n=73)'	// s:short l:long
WoodBlocks	= 'note(n=76)'	// h:high l:low
Cuica		= 'note(n=79)'	// x:open m:muted
Triangle	= 'note(n=81)'	// x:open m:muted

/*
 * drum characters 
 */
// High
DCh = ''
DCH = 'DCX'
// Low or Long
DCl = 'n+=1'
DCL = 'DCX DCl'
// Short
DCs = ''
DCS = 'DCX'
// Both High & Low
DCb = 'note(n+=1)'
DCB = 'DCX DCb'
// Open Hi-Hat
DCo = 'n+=4'
DCO = 'DCX DCo'
// Pedal Hi-Hat
DCp = 'n+=2'
DCP = 'DCX DCp'
// MidHigh (for Tom)
DCi = 'n-=2'
DCI = 'DCX DCi'
// Floor (for Tom)
DCf = 'n-=7'
DCF = 'DCX DCf'
// Grand Floor (for Tom)
DCg = 'n-=9'
DCG = 'DCX DCg'
// Mute
DCm = 'n-=1'
DCM = 'DCX DCm'
