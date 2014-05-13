/*
 * Instrument names for GM
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
 * from program numbers to instrument names.  Each line defining a macro
 * for an instrument name must use the following format:
 *   <instrument name up to 24 characters> = 'prog(<program number>)'
 * To comment out the above line, use '//' at the beginning of line (do not
 * use '/* .. */'), because 'm2p' only recognizes '//'.
 */

/* Piano */
AcouGrandPiano	= 'prog(1)'
  AcouPiano	= 'prog(1)'
  Piano1	= 'prog(1)'
BrightAcouPiano = 'prog(2)'
  BrightPiano	= 'prog(2)'
  Piano2	= 'prog(2)'
ElecGrandPiano	= 'prog(3)'
  Piano3	= 'prog(3)'
HonkyTonk	= 'prog(4)'
EPiano1		= 'prog(5)'
EPiano2		= 'prog(6)'
Harpsichord	= 'prog(7)'
Clav		= 'prog(8)'

/* Chromatic Percussion */
Celesta		= 'prog(9)'
Glockenspiel	= 'prog(10)'
  Glocken	= 'prog(10)'
MusicBox	= 'prog(11)'
Vibraphone	= 'prog(12)'
Marimba		= 'prog(13)'
Xylophone	= 'prog(14)'
TubularBells	= 'prog(15)'
  TubularBell	= 'prog(15)'
  TubeBell	= 'prog(15)'
Dulcimer	= 'prog(16)'
  Santur	= 'prog(16)'

/* Organ */
DrawbarOrgan	= 'prog(17)'
  Organ1	= 'prog(17)'
PercussiveOrgan = 'prog(18)'
  Organ2	= 'prog(18)'
RockOrgan	= 'prog(19)'
  Organ3	= 'prog(19)'
ChurchOrgan	= 'prog(20)'
  ChurchOrg1	= 'prog(20)'
ReedOrgan	= 'prog(21)'
Accordion	= 'prog(22)'
  AccordionFr	= 'prog(22)'
Harmonica	= 'prog(23)'
TangoAccordion	= 'prog(24)'
  Bandoneon	= 'prog(24)'

/* Guitar */
NylonAcouGuitar = 'prog(25)'
  NylonGt	= 'prog(25)'
SteelAcouGuitar = 'prog(26)'
  SteelGt	= 'prog(26)'
JazzElecGuitar	= 'prog(27)'
  JazzGt	= 'prog(27)'
CleanElecGuitar = 'prog(28)'
  CleanGt	= 'prog(28)'
MutedElecGuitar = 'prog(29)'
  MutedGt	= 'prog(29)'
OverdrivenGuitar= 'prog(30)'
  OvedriveGt	= 'prog(30)'
DistortionGuitar= 'prog(31)'
  DistortionGt	= 'prog(31)'
GuitarHarmonics = 'prog(32)'
  GtHarmonics	= 'prog(32)'

/* Bass */
AcouBass	= 'prog(33)'
  AcousticBs	= 'prog(33)'
FingerElecBass	= 'prog(34)'
  FingeredBs	= 'prog(34)'
PickElecBass	= 'prog(35)'
  PickedBs	= 'prog(35)'
FretlessBass	= 'prog(36)'
  FretlessBs	= 'prog(36)'
SlapBass1	= 'prog(37)'
SlapBass2	= 'prog(38)'
SynthBass1	= 'prog(39)'
SynthBass2	= 'prog(40)'

/* Strings */
Violin		= 'prog(41)'
Viola		= 'prog(42)'
Cello		= 'prog(43)'
Contrabass	= 'prog(44)'
TremoloStrings	= 'prog(45)'
  TremoloStr	= 'prog(45)'
PizzicatoStrings= 'prog(46)'
  PizzicatoStr	= 'prog(46)'
OrchestralStrings= 'prog(47)'
  Harp		= 'prog(47)'
Timpani		= 'prog(48)'

/* Ensamble */
StringEnsemble1 = 'prog(49)'
  Strings	= 'prog(49)'
StringEnsemble2 = 'prog(50)'
  SlowStrings	= 'prog(50)'
SynthStrings1	= 'prog(51)'
  SynStrings1	= 'prog(51)'
SynthStrings2	= 'prog(52)'
  SynStrings2	= 'prog(52)'
ChoirAahs	= 'prog(53)'
VoiceOohs	= 'prog(54)'
SynthVoice	= 'prog(55)'
  SynVox	= 'prog(55)'
OrchestraHit	= 'prog(56)'

/* Brass */
Trumpet		= 'prog(57)'
Trombone	= 'prog(58)'
Tuba		= 'prog(59)'
MutedTrumpet	= 'prog(60)'
FrenchHorn	= 'prog(61)'
BrassSection	= 'prog(62)'
  Brass1	= 'prog(62)'
SynthBrass1	= 'prog(63)'
SynthBrass2	= 'prog(64)'

/* Reed */
SopranoSax	= 'prog(65)'
AltoSax		= 'prog(66)'
TenorSax	= 'prog(67)'
BaritoneSax	= 'prog(68)'
Oboe		= 'prog(69)'
EnglishHorn	= 'prog(70)'
Bassoon		= 'prog(71)'
Clarinet	= 'prog(72)'

/* Pipe */
Piccolo		= 'prog(73)'
Flute		= 'prog(74)'
Recorder	= 'prog(75)'
PanFlute	= 'prog(76)'
BlownBottle	= 'prog(77)'
  BottleBlow	= 'prog(77)'
Shakuhachi	= 'prog(78)'
Whistle		= 'prog(79)'
Ocarina		= 'prog(80)'

/* Synth Lead */
Square		= 'prog(81)'
  SquareWave	= 'prog(81)'
Sawtooth	= 'prog(82)'
  SawWave	= 'prog(82)'
Calliope	= 'prog(83)'
  SynCalliope	= 'prog(83)'
Chiff		= 'prog(84)'
  ChifferLead	= 'prog(84)'
Charang		= 'prog(85)'
Voice		= 'prog(86)'
  SoloVox	= 'prog(86)'
Fifths		= 'prog(87)'
  FifthSawWave	= 'prog(87)'
BassAndLead	= 'prog(88)'

/* Synth Pad */
NewAge		= 'prog(89)'
  Fantasia	= 'prog(89)'
Warm		= 'prog(90)'
  WarmPad	= 'prog(90)'
Polysynth	= 'prog(91)'
Choir		= 'prog(92)'
  SpaceVoice	= 'prog(92)'
Bowed		= 'prog(93)'
  BowedGlass	= 'prog(93)'
Metallic	= 'prog(94)'
  MetalPad	= 'prog(94)'
Halo		= 'prog(95)'
  HaloPad	= 'prog(95)'
Sweep		= 'prog(96)'
  SweepPad	= 'prog(96)'

/* Synth Effects */
Rain		= 'prog(97)'
  IceRain	= 'prog(97)'
Soundtrack	= 'prog(98)'
Crystal		= 'prog(99)'
Atmosphere	= 'prog(100)'
Brightness	= 'prog(101)'
Goblins		= 'prog(102)'
  Goblin	= 'prog(102)'
Echoes		= 'prog(103)'
  EchoDrops	= 'prog(103)'
SciFi		= 'prog(104)'
  StarTheme	= 'prog(104)'

/* Ethnic */
Sitar		= 'prog(105)'
Banjo		= 'prog(106)'
Shamisen	= 'prog(107)'
Koto		= 'prog(108)'
Kalimba		= 'prog(109)'
BagPipe		= 'prog(110)'
Fiddle		= 'prog(111)'
Shanai		= 'prog(112)'

/* Percussive */
TinkleBell	= 'prog(113)'
Agogo		= 'prog(114)'
SteelDrums	= 'prog(115)'
Woodblock	= 'prog(116)'
TaikoDrum	= 'prog(117)'
  Taiko		= 'prog(117)'
MelodicTom	= 'prog(118)'
  MeloTom1	= 'prog(118)'
SynthDrum	= 'prog(119)'
ReverseCymbal	= 'prog(120)'
  ReverseCym	= 'prog(120)'

/* Sound Effects */
GuitarFretNoise = 'prog(121)'
  GtFretNoise	= 'prog(121)'
BreathNoise	= 'prog(122)'
Seashore	= 'prog(123)'
BirdTweet	= 'prog(124)'
  Bird		= 'prog(124)'
TelephoneRing	= 'prog(125)'
  Telephone1	= 'prog(125)'
Helicopter	= 'prog(126)'
Applause	= 'prog(127)'
Gunshot		= 'prog(128)'
  GunShot	= 'prog(128)'
