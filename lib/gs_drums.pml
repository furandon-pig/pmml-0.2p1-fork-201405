/*
 * Model-specific drums macro definitions for GS
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

include("gm_drums")

/*
 * macro for each instrument
 */

SnareRoll 	= 'note(n=25)'	// SC-88 only
FingerSnap 	= 'note(n=26)'	// SC-88 only
HighQ 		= 'note(n=27)'
Slap 		= 'note(n=28)'
ScratchPush	= 'note(n=29)'
ScratchPull	= 'note(n=30)'
Sticks		= 'note(n=31)'
SquareClick	= 'note(n=32)'
MetroClick	= 'note(n=33)'
MetroBell	= 'note(n=34)'

Shaker		= 'note(n=82)'
JingleBell	= 'note(n=83)'
BellTree	= 'note(n=84)'
Castanets	= 'note(n=85)'
MuteSurdo	= 'note(n=86)'
OpenSurdo	= 'note(n=87)'

/*
 * macro for a group of instruments
 */
Surdo		= 'note(n=87)'	// x:open m:muted
