/*
 * Instrument names for GS
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
 * The instrument names defined here basically conincide with the names
 * shown in sound modules' manuals.  However, due to character-set 
 * restrictions in identifers, names are changed according to the 
 * following rules:
 *
 *   - Delimiters (spaces, '.', '+', and '-') are deleted.
 *   - When the second word of two words separated by the delimiters 
 *     begins with a lower-case letter, replace it to an upper-case letter. 
 *       (ex.) Honky-tonk -> HonkyTonk
 *     <Exception> If the second word is a single lower-case letter,
 *                 it is not changed to upper-case; instead; '_' is inserted 
 *                 between the two words.
 *       (ex.) Harpsi.w -> Harpsi_w
 *   - '&' is changed to 'And'
 *   - Names beginning with numbers are replaced with phonetic expressions.
 *       (ex.) 5thDist -> FifthDist
 *             12-str.Gt -> TwelveStrGt
 *             60's E. Piano -> SixtiesEPiano
 *             808Tom -> EightOEightTom
 *   - '-str.' in guitar names is omitted.
 *
 * If you find erros in instrument names or program numbers, please send 
 * an e-mail to 'nisim@u-aizu.ac.jp'.
 */
       					// Availability
					// 88: available only with SC-88
					// 55: available only with SC-55
/* Piano */
Piano1		= 'xprog(1, 0)'
Piano1w		= 'xprog(1, 8)'
Piano1d		= 'xprog(1, 16)'
Piano2		= 'xprog(2, 0)'
Piano2w		= 'xprog(2, 8)'
Piano3		= 'xprog(3, 0)'
EGRhodes1	= 'xprog(3, 1)'		// 88
EGRhodes2	= 'xprog(3, 2)'		// 88
Piano3w		= 'xprog(3, 8)'
HonkyTonk	= 'xprog(4, 0)'
OldUpright	= 'xprog(4, 8)'		// 88
HonkyTonk_w	= 'xprog(4, 8)'		// 55
EPiano1		= 'xprog(5, 0)'
DetunedEP1	= 'xprog(5, 8)'		// 55
StSoftEP	= 'xprog(5, 8)'		// 88
EPiano1v	= 'xprog(5, 16)'	// 55
FMSAEP		= 'xprog(5, 16)'	// 88
SixtiesEPiano	= 'xprog(5, 24)'
HardRhodes	= 'xprog(5, 25)'	// 88
MellowRhodes	= 'xprog(5, 26)'	// 88
EPiano2		= 'xprog(6, 0)'
DetunedEP2	= 'xprog(6, 8)'
EPiano2v	= 'xprog(6, 16)'	// 55
StFMEP		= 'xprog(6, 16)'	// 88
HardFMEP	= 'xprog(6, 24)'	// 88
Harpsichord	= 'xprog(7, 0)'
CoupledHps	= 'xprog(7, 8)'
Harpsi_w	= 'xprog(7, 16)'
Harpsi_o	= 'xprog(7, 24)'
Clav		= 'xprog(8, 0)'

/* Chromatic Percussion */
Celesta		= 'xprog(9, 0)'
Glockenspiel	= 'xprog(10, 0)'
MusicBox	= 'xprog(11, 0)'
Vibraphone	= 'xprog(12, 0)'
HardVibe	= 'xprog(12, 1)'	// 88
Vib_w		= 'xprog(12, 8)'
Marimba		= 'xprog(13, 0)'
Marimba_w	= 'xprog(13, 8)'
Barafon		= 'xprog(13, 16)'	// 88
Barafon2	= 'xprog(13, 17)'	// 88
Logdrum		= 'xprog(13, 24)'	// 88
Xylophone	= 'xprog(14, 0)'
TubularBell	= 'xprog(15, 0)'
ChurchBell	= 'xprog(15, 8)'
Carillon	= 'xprog(15, 9)'
Santur		= 'xprog(16, 0)'
Santur2		= 'xprog(16, 1)'	// 88
Cimbalom	= 'xprog(16, 8)'	// 88

/* Organ */
Organ1		= 'xprog(17, 0)'
Organ101	= 'xprog(17, 1)'	// 88
DetunedOr1	= 'xprog(17, 8)'
Organ109	= 'xprog(17, 9)'	// 88
SixtiesOrgan1	= 'xprog(17, 16)'
SixtiesOrgan2	= 'xprog(17, 17)'	// 88
SixtiesOrgan3	= 'xprog(17, 18)'	// 88
CheeseOrgan	= 'xprog(17, 24)'	// 88
Organ4		= 'xprog(17, 32)'
EvenBar		= 'xprog(17, 33)'	// 88
OrganBass	= 'xprog(17, 40)'	// 88
Organ2		= 'xprog(18, 0)'
Organ201	= 'xprog(18, 1)'	// 88
DetunedOr2	= 'xprog(18, 8)'
Organ5		= 'xprog(18, 32)'
Organ3		= 'xprog(19, 0)'
RotaryOrg	= 'xprog(19, 8)'	// 88
RotaryOrgS	= 'xprog(19, 16)'	// 88
RotaryOrgF	= 'xprog(19, 24)'	// 88
ChurchOrg1	= 'xprog(20, 0)'
ChurchOrg2	= 'xprog(20, 8)'
ChurchOrg3	= 'xprog(20, 16)'
OrganFlute	= 'xprog(20, 24)'	// 88
TremFlute	= 'xprog(20, 32)'	// 88
ReedOrgan	= 'xprog(21, 0)'
AccordionFr	= 'xprog(22, 0)'
AccordionIt	= 'xprog(22, 8)'
Harmonica	= 'xprog(23, 0)'
Harmonica2	= 'xprog(23, 1)'	// 88
Bandoneon	= 'xprog(24, 0)'

/* Guitar */
NylonGt		= 'xprog(25, 0)'
Ukulele		= 'xprog(25, 8)'
NylonGt_o	= 'xprog(25, 16)'
VeloHarmnix	= 'xprog(25, 24)'	// 88
NylonGt2	= 'xprog(25, 32)'
LequintGt	= 'xprog(25, 40)'	// 88
SteelGt		= 'xprog(26, 0)'
TwelveStrGt	= 'xprog(26, 8)'
NylonSteel	= 'xprog(26, 9)'	// 88
Mandolin	= 'xprog(26, 16)'
SteelGt2	= 'xprog(26, 32)'	// 88
JazzGt		= 'xprog(27, 0)'
MellowGt	= 'xprog(27, 1)'	// 88
HawaianGt	= 'xprog(27, 8)'	// 55
PedalSteel	= 'xprog(27, 8)'	// 88
CleanGt		= 'xprog(28, 0)'
ChorusGt	= 'xprog(28, 8)'
MutedGt		= 'xprog(29, 0)'
MutedDisGt	= 'xprog(29, 1)'	// 88
FunkGt		= 'xprog(29, 8)'	// 55
FunkPop		= 'xprog(29, 8)'	// 88
FunkGt2		= 'xprog(29, 16)'
OvedriveGt	= 'xprog(30, 0)'
DistortionGt	= 'xprog(31, 0)'
DistGt2		= 'xprog(31, 1)'	// 88
DazedGuitar	= 'xprog(31, 2)'	// 88
FeedbackGt	= 'xprog(31, 8)'
FeedbackGt2	= 'xprog(31, 9)'	// 88
PowerGuitar	= 'xprog(31, 16)'	// 88
PowerGt2	= 'xprog(31, 17)'	// 88
FifthDist	= 'xprog(31, 18)'	// 88
RockRhythm	= 'xprog(31, 24)'	// 88
RockRhythm2	= 'xprog(31, 25)'	// 88
GtHarmonics	= 'xprog(32, 0)'
GtFeedback	= 'xprog(32, 8)'
AcGtHarmnx	= 'xprog(32, 16)'	// 88

/* Bass */
AcousticBs	= 'xprog(33, 0)'
FingeredBs	= 'xprog(34, 0)'
FingeredBs2	= 'xprog(34, 1)'	// 88
JazzBass	= 'xprog(34, 2)'	// 88
PickedBs	= 'xprog(35, 0)'	// 55
PickedBass	= 'xprog(35, 0)'	// 88
MutePickBs	= 'xprog(35, 8)'	// 88
FretlessBs	= 'xprog(36, 0)'
FretlessBs2	= 'xprog(36, 1)'	// 88
FretlessBs3	= 'xprog(36, 2)'	// 88
FretlessBs4	= 'xprog(36, 3)'	// 88
SynFretless	= 'xprog(36, 4)'	// 88
MrSmooth	= 'xprog(36, 5)'	// 88
SlapBass1	= 'xprog(37, 0)'
ResoSlap	= 'xprog(37, 8)'	// 88
SlapBass2	= 'xprog(38, 0)'
SynthBass1	= 'xprog(39, 0)'
SynthBass101	= 'xprog(39, 1)'
SynthBass3	= 'xprog(39, 8)'	// 55
AcidBass	= 'xprog(39, 8)'	// 88
TB303Bass	= 'xprog(39, 9)'	// 88
TeknoBass	= 'xprog(39, 10)'	// 88
ResoSHBass	= 'xprog(39, 16)'	// 88
SynthBass2	= 'xprog(40, 0)'
SynthBass201	= 'xprog(40, 1)'	// 88
ModularBass	= 'xprog(40, 2)'	// 88
SeqBass		= 'xprog(40, 3)'	// 88
SynthBass4	= 'xprog(40, 8)'	// 55
BeefFMBass	= 'xprog(40, 8)'	// 88
XWireBass	= 'xprog(40, 9)'	// 88
RubberBass	= 'xprog(40, 16)'
SH101Bass1	= 'xprog(40, 17)'	// 88
SH101Bass2	= 'xprog(40, 18)'	// 88
SmoothBass	= 'xprog(40, 19)'	// 88

/* Strings */
Violin		= 'xprog(41, 0)'
SlowViolin	= 'xprog(41, 8)'
Viola		= 'xprog(42, 0)'
Cello		= 'xprog(43, 0)'
Contrabass	= 'xprog(44, 0)'
TremoloStr	= 'xprog(45, 0)'
SlowTremolo	= 'xprog(45, 8)'	// 88
SuspenseStr	= 'xprog(45, 9)'	// 88
PizzicatoStr	= 'xprog(46, 0)'
Harp		= 'xprog(47, 0)'
Timpani		= 'xprog(48, 0)'

/* Ensamble */
Strings		= 'xprog(49, 0)'
Strings2	= 'xprog(49, 1)'	// 88
Orchestra	= 'xprog(49, 8)'
Orchestra2	= 'xprog(49, 9)'	// 88
TremoloOrch	= 'xprog(49, 10)'	// 88
ChoirStr	= 'xprog(49, 11)'	// 88
StStrings	= 'xprog(49, 16)'	// 88
VeloStrings	= 'xprog(49, 24)'	// 88
SlowStrings	= 'xprog(50, 0)'
SlowStrings2	= 'xprog(50, 1)'	// 88
LegatoStr	= 'xprog(50, 8)'	// 88
WarmStrings	= 'xprog(50, 9)'	// 88
StSlowStr	= 'xprog(50, 10)'	// 88
SynStrings1	= 'xprog(51, 0)'
OBStrings	= 'xprog(51, 1)'	// 88
SynStrings3	= 'xprog(51, 8)'
SynStrings2	= 'xprog(52, 0)'
ChoirAahs	= 'xprog(53, 0)'
StChoir		= 'xprog(53, 8)'	// 88
MelloChoir	= 'xprog(53, 9)'	// 88
ChoirAahs2	= 'xprog(53, 32)'
VoiceOohs	= 'xprog(54, 0)'
SynVox		= 'xprog(55, 0)'
SynVoice	= 'xprog(55, 8)'	// 88
OrchestraHit	= 'xprog(56, 0)'
ImpactHit	= 'xprog(56, 8)'	// 88
PhillyHit	= 'xprog(56, 9)'	// 88
DoubleHit	= 'xprog(56, 10)'	// 88
LoFiRave	= 'xprog(56, 16)'	// 88

/* Brass */
Trumpet		= 'xprog(57, 0)'
Trumpet2	= 'xprog(57, 1)'	// 88
FlugelHorn	= 'xprog(57, 8)'	// 88
BrightTp	= 'xprog(57, 24)'	// 88
WarmTp		= 'xprog(57, 25)'	// 88
Trombone	= 'xprog(58, 0)'
Trombone2	= 'xprog(58, 1)'
Tuba		= 'xprog(59, 0)'
Tuba2		= 'xprog(59, 1)'	// 88
MutedTrumpet	= 'xprog(60, 0)'
FrenchHorn	= 'xprog(61, 0)'	// 55
FrenchHorns	= 'xprog(61, 0)'	// 88
FrHorn		= 'xprog(61, 1)'	// 55
FrHorn2		= 'xprog(61, 1)'	// 88
FrHornSolo	= 'xprog(61, 8)'	// 88
HornOrch	= 'xprog(61, 16)'	// 88
Brass1		= 'xprog(62, 0)'
Brass2		= 'xprog(62, 8)'
BrassFall	= 'xprog(62, 16)'	// 88
SynthBrass1	= 'xprog(63, 0)'
PolyBrass	= 'xprog(63, 1)'	// 88
SynthBrass3	= 'xprog(63, 8)'
QuackBrass	= 'xprog(63, 9)'	// 88
AnalogBrass1	= 'xprog(63, 16)'	// 55
OctaveBras	= 'xprog(63, 16)'	// 88
SynthBrass2	= 'xprog(64, 0)'
SoftBrass	= 'xprog(64, 1)'	// 88
SynthBrass4	= 'xprog(64, 8)'
AnalogBrass2	= 'xprog(64, 16)'	// 55
VeloBrass1	= 'xprog(64, 16)'	// 88
VeloBrass2	= 'xprog(64, 17)'	// 88

/* Reed */
SopranoSax	= 'xprog(65, 0)'
AltoSax		= 'xprog(66, 0)'
HyperAlto	= 'xprog(66, 8)'	// 88
TenorSax	= 'xprog(67, 0)'
BreathyTenor	= 'xprog(67, 8)'	// 88
BaritoneSax	= 'xprog(68, 0)'
Oboe		= 'xprog(69, 0)'
EnglishHorn	= 'xprog(70, 0)'
Bassoon		= 'xprog(71, 0)'
Clarinet	= 'xprog(72, 0)'
BsClarinet	= 'xprog(72, 8)'	// 88

/* Pipe */
Piccolo		= 'xprog(73, 0)'
Flute		= 'xprog(74, 0)'
Recorder	= 'xprog(75, 0)'
PanFlute	= 'xprog(76, 0)'
Kawala		= 'xprog(76, 8)'	// 88
BottleBlow	= 'xprog(77, 0)'
Shakuhachi	= 'xprog(78, 0)'
Whistle		= 'xprog(79, 0)'
Ocarina		= 'xprog(80, 0)'

/* Synth Lead */
SquareWave	= 'xprog(81, 0)'
Square		= 'xprog(81, 1)'
HollowMini	= 'xprog(81, 2)'	// 88
MellowFM	= 'xprog(81, 3)'	// 88
CCSolo		= 'xprog(81, 4)'	// 88
Shmoog		= 'xprog(81, 5)'	// 88
LMSquare	= 'xprog(81, 6)'	// 88
SinWave		= 'xprog(81, 8)'
SawWave		= 'xprog(82, 0)'
Saw		= 'xprog(82, 1)'
PulseSaw	= 'xprog(82, 2)'	// 88
FelineGR	= 'xprog(82, 3)'	// 88
BigLead		= 'xprog(82, 4)'	// 88
VeloLead	= 'xprog(82, 5)'	// 88
GR300		= 'xprog(82, 6)'	// 88
LASaw		= 'xprog(82, 7)'	// 88
DoctorSolo	= 'xprog(82, 8)'
WaspySynth	= 'xprog(82, 16)'	// 88
SynCalliope	= 'xprog(83, 0)'
VentSynth	= 'xprog(83, 1)'	// 88
PurePanLead	= 'xprog(83, 2)'	// 88
ChifferLead	= 'xprog(84, 0)'
Charang		= 'xprog(85, 0)'
DistLead	= 'xprog(85, 8)'	// 88
SoloVox		= 'xprog(86, 0)'
FifthSawWave	= 'xprog(87, 0)'
BigFives	= 'xprog(87, 1)'	// 88
BassAndLead	= 'xprog(88, 0)'
BigAndRaw	= 'xprog(88, 1)'	// 88
FatAndPerky	= 'xprog(88, 2)'	// 88

/* Synth Pad */
Fantasia	= 'xprog(89, 0)'
Fantasia2	= 'xprog(89, 1)'	// 88
WarmPad		= 'xprog(90, 0)'
ThickPad	= 'xprog(90, 1)'	// 88
HornPad		= 'xprog(90, 2)'	// 88
RotaryStrng	= 'xprog(90, 3)'	// 88
SoftPad		= 'xprog(90, 4)'	// 88
Polysynth	= 'xprog(91, 0)'
EightiesPolySyn	= 'xprog(91, 1)'	// 88
SpaceVoice	= 'xprog(92, 0)'
HeavenII	= 'xprog(92, 1)'	// 88
BowedGlass	= 'xprog(93, 0)'
MetalPad	= 'xprog(94, 0)'
TinePad		= 'xprog(94, 1)'	// 88
PannerPad	= 'xprog(94, 2)'	// 88
HaloPad		= 'xprog(95, 0)'
SweepPad	= 'xprog(96, 0)'
PolarPad	= 'xprog(96, 1)'	// 88
Converge	= 'xprog(96, 8)'	// 88
Shwimmer	= 'xprog(96, 9)'	// 88
CelestialPd	= 'xprog(96, 10)'	// 88

/* Synth Effects */
IceRain		= 'xprog(97, 0)'
HarmoRain	= 'xprog(97, 1)'	// 88
AfricanWood	= 'xprog(97, 2)'	// 88
ClaviPad	= 'xprog(97, 8)'	// 88
Soundtrack	= 'xprog(98, 0)'
Ancestral	= 'xprog(98, 1)'	// 88
Prologue	= 'xprog(98, 2)'	// 88
Rave		= 'xprog(98, 8)'	// 88
Crystal		= 'xprog(99, 0)'
SynMallet	= 'xprog(99, 1)'
SoftCrystal	= 'xprog(99, 2)'	// 88
RoundGlock	= 'xprog(99, 3)'	// 88
LoudGlock	= 'xprog(99, 4)'	// 88
GlockenChime	= 'xprog(99, 5)'	// 88
ClearBells	= 'xprog(99, 6)'	// 88
ChristmasBel	= 'xprog(99, 7)'	// 88
VibraBells	= 'xprog(99, 8)'	// 88
DigiBells	= 'xprog(99, 9)'	// 88
ChoralBells	= 'xprog(99, 16)'	// 88
AirBells	= 'xprog(99, 17)'	// 88
BellHarp	= 'xprog(99, 18)'	// 88
Gamelimba	= 'xprog(99, 19)'	// 88
Atmosphere	= 'xprog(100, 0)'
WarmAtmos	= 'xprog(100, 1)'	// 88
NylonHarp	= 'xprog(100, 2)'	// 88
Harpvox		= 'xprog(100, 3)'	// 88
HollowReleas	= 'xprog(100, 4)'	// 88
NylonRhodes	= 'xprog(100, 5)'	// 88
AmbientPad	= 'xprog(100, 6)'	// 88
Brightness	= 'xprog(101, 0)'
Goblin		= 'xprog(102, 0)'
Goblinson	= 'xprog(102, 1)'	// 88
FiftiesSciFi	= 'xprog(102, 2)'	// 88
EchoDrops	= 'xprog(103, 0)'
EchoBell	= 'xprog(103, 1)'
EchoPan		= 'xprog(103, 2)'
EchoPan2	= 'xprog(103, 3)'	// 88
BigPanner	= 'xprog(103, 4)'	// 88
ResoPanner	= 'xprog(103, 5)'	// 88
WaterPiano	= 'xprog(103, 6)'	// 88
StarTheme	= 'xprog(104, 0)'
StarTheme2	= 'xprog(104, 1)'	// 88

/* Ethnic */
Sitar		= 'xprog(105, 0)'
Sitar2		= 'xprog(105, 1)'
DetuneSitar	= 'xprog(105, 2)'	// 88
Tambra		= 'xprog(105, 8)'	// 88
Tamboura	= 'xprog(105, 16)'	// 88
Banjo		= 'xprog(106, 0)'
MutedBanjo	= 'xprog(106, 1)'	// 88
Rabab		= 'xprog(106, 8)'	// 88
Gopichant	= 'xprog(106, 16)'	// 88
Oud		= 'xprog(106, 24)'	// 88
Shamisen	= 'xprog(107, 0)'
Tsugaru		= 'xprog(107, 1)'	// 88
Koto		= 'xprog(108, 0)'
TaishoKoto	= 'xprog(108, 8)'
Kanoon		= 'xprog(108, 16)'	// 88
Kalimba		= 'xprog(109, 0)'
BagPipe		= 'xprog(110, 0)'	// 55
Bagpipe		= 'xprog(110, 0)'	// 88
Fiddle		= 'xprog(111, 0)'
Shanai		= 'xprog(112, 0)'
Shanai2		= 'xprog(112, 1)'	// 88
Pungi		= 'xprog(112, 8)'	// 88
Hichiriki	= 'xprog(112, 16)'	// 88

/* Percussive */
TinkleBell	= 'xprog(113, 0)'
Bonang		= 'xprog(113, 8)'	// 88
Gender		= 'xprog(113, 9)'	// 88
GamelanGong	= 'xprog(113, 10)'	// 88
StGamelan	= 'xprog(113, 11)'	// 88
RAMACymbal	= 'xprog(113, 16)'	// 88
Agogo		= 'xprog(114, 0)'
Atarigane	= 'xprog(114, 8)'	// 88
SteelDrums	= 'xprog(115, 0)'
Woodblock	= 'xprog(116, 0)'
Castanets	= 'xprog(116, 8)'
Taiko		= 'xprog(117, 0)'
ConcertBD	= 'xprog(117, 8)'
MeloTom1	= 'xprog(118, 0)'
RealTom		= 'xprog(118, 1)'	// 88
MeloTom2	= 'xprog(118, 8)'
RockTom		= 'xprog(118, 9)'	// 88
SynthDrum	= 'xprog(119, 0)'
EightOEightTom	= 'xprog(119, 8)'
ElecPerc	= 'xprog(119, 9)'
ReverseCym	= 'xprog(120, 0)'
ReverseCym2	= 'xprog(120, 1)'	// 88
RevSnare1	= 'xprog(120, 8)'	// 88
RevSnare2	= 'xprog(120, 9)'	// 88
RevKick1	= 'xprog(120, 16)'	// 88
RevConBD	= 'xprog(120, 17)'	// 88
RevTom1		= 'xprog(120, 24)'	// 88
RevTom2		= 'xprog(120, 25)'	// 88

/* Sound Effects */
GtFretNoise	= 'xprog(121, 0)'
GtCutNoise	= 'xprog(121, 1)'
StringSlap	= 'xprog(121, 2)'
GtCutNoise2	= 'xprog(121, 3)'	// 88
DistCutNoiz	= 'xprog(121, 4)'	// 88
BassSlide	= 'xprog(121, 5)'	// 88
PickScrape	= 'xprog(121, 6)'	// 88
BreathNoise	= 'xprog(122, 0)'
FlKeyClick	= 'xprog(122, 1)'
Seashore	= 'xprog(123, 0)'
Rain		= 'xprog(123, 1)'
Thunder		= 'xprog(123, 2)'
Wind		= 'xprog(123, 3)'
Stream		= 'xprog(123, 4)'
Bubble		= 'xprog(123, 5)'
Bird		= 'xprog(124, 0)'
Dog		= 'xprog(124, 1)'
HorseGallop	= 'xprog(124, 2)'
Bird2		= 'xprog(124, 3)'
Kitty		= 'xprog(124, 4)'	// 88
Growl		= 'xprog(124, 5)'	// 88
Telephone1	= 'xprog(125, 0)'
Telephone2	= 'xprog(125, 1)'
DoorCreaking	= 'xprog(125, 2)'
Door		= 'xprog(125, 3)'
Scratch		= 'xprog(125, 4)'
Windchime	= 'xprog(125, 5)'	// 55
WindChimes	= 'xprog(125, 5)'	// 88
Scratch2	= 'xprog(125, 7)'	// 88
Helicopter	= 'xprog(126, 0)'
CarEngine	= 'xprog(126, 1)'
CarStop		= 'xprog(126, 2)'
CarPass		= 'xprog(126, 3)'
CarCrash	= 'xprog(126, 4)'
Siren		= 'xprog(126, 5)'
Train		= 'xprog(126, 6)'
Jetplane	= 'xprog(126, 7)'
Starship	= 'xprog(126, 8)'
BurstNoise	= 'xprog(126, 9)'
Applause	= 'xprog(127, 0)'
Laughing	= 'xprog(127, 1)'
Screaming	= 'xprog(127, 2)'
Punch		= 'xprog(127, 3)'
HeartBeat	= 'xprog(127, 4)'
Footsteps	= 'xprog(127, 5)'
Applause2	= 'xprog(127, 6)'	// 88
GunShot		= 'xprog(128, 0)'
MachineGun	= 'xprog(128, 1)'
Lasergun	= 'xprog(128, 2)'
Explosion	= 'xprog(128, 3)'
