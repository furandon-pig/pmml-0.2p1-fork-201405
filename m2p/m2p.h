/*
 * M2P: A SMF-to-PMML Translator
 */

/*
 *  Copyright (C) 1998   Satoshi Nishimura
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/********************************************************************
 * Constants
 ********************************************************************/
/*
 * PMML's resolution (ticks per quoter_note)
 */
#define IRES    480

/*
 * maximum time step for continuous control change (in 1/480 of a quarter note)
 */
#define LF_MAXTMSTEP   240	/* eighth note */

/*
 * error tolerance for line fitting (used in making continuous control change)
 */
#define LF_TOLERANCE   1.0

/*
 * maximum length of overshoot part of measure (in 1/480 of a quarter note)
 *   This value affects measure partitioning in measure-oriented output mode.
 */
#define MAX_OVS_LEN	240	/* eighth note */

/* 
 * minimum percentage of duplicated segment of notes within a chord 
 */
#define CD_MINDUP   30

/*
 * column length
 */
#define MAXCOL   79	

/*
 * output stream
 */
#define OUT   stdout

/*
 * default search path for the files describing GM instrument names
 */
#ifndef PMML_PATH
#ifdef DOS
#  define PMML_PATH  ".;c:/pmml/lib;../lib"
#else
#  define PMML_PATH  ".;/usr/local/lib/pmml;../lib"
#endif
#endif

/*
 * name of the file describing GM instrument names
 */
#define GM_INST   "gm_inst.pml"

/*
 * name of the file describing GM rhythmic instrument names
 */
#define GM_DRUMS  "gm_drums.pml"

/*
 * maximum length of instrument names
 */
#define MAX_INST_LEN   24

/*
 * MIDI channel for rhythmic part - 1
 */
#define RHYTHM_CH    (10 - 1)

/********************************************************************
 * Type definitions
 ********************************************************************/

typedef unsigned char  uchar;

/*
 * Event: per-event structure
 */
/* event type */
#define E_Ctrl	    0x000 	/* 0x00-0x7f: control change */
#define E_Mod       0x001	/* modulation wheel */
#define E_Breath    0x002	/* breath control */
#define E_Foot      0x004	/* foot controller */
#define E_Pmtime    0x005	/* portamento time */
#define E_Vol       0x007	/* volume */
#define E_Pan       0x00a	/* pan pot */
#define E_Expr      0x00b	/* expression control */
#define E_DataEntL  0x006	/* data entry LSB */
#define E_DataEntH  0x026	/* data entry MSB */
#define E_ModeMsg   0x060
#define E_RPNL	    0x064	/* registered parameter number LSB */
#define E_RPNH	    0x065	/* registered parameter number MSB */
#define E_NRPNL	    0x066	/* non-registered parameter number LSB */
#define E_NRPNH	    0x067	/* non-registered parameter number MSB */
#define E_ResetAllCtrl 0x079	/* reset all controllers */
#define E_AllNotesOff  0x07b	/* all notes off */

#define E_VCtrl     0x080	/* 0x80-0xff: extended control change */
#define E_Bend      0x080
#define E_Kp        0x081
#define E_Cpr       0x082
#define E_Prog      0x083
#define E_GCtrl	    0x0c0	/* (0xc0-0xff): control for all channels */
#define E_Tempo     0x0c0

#define E_Meta      0x100	/* 0x100-0x17f: meta event (except tempo) */
#define E_Seqno	    0x100
#define E_Text      0x101
#define E_Copyright 0x102
#define E_TrkName   0x103
#define E_InstName  0x104
#define E_Lyric     0x105
#define E_Marker    0x106
#define E_Cue       0x107
#define E_TextEnd   0x10f
#define E_TrkEnd    0x12f
#define E_OrgTempo  0x151	/* never used */
#define E_Smpte     0x154
#define E_TimeSig   0x158
#define E_KeySig    0x159

#define E_NoteExcl  0x190	/* 0x190-0x193: note on/off and exclusive */
#define	E_Excl      0x190
#define	E_Arbit     0x191
#define E_NoteOff   0x192
#define	E_NoteOn    0x193
#define E_END	    0x194	/* Not an event type; for range checking */
#define ETYPE	    0xfff

#define	E_To       0x1000	/* continuous control change flag (line) */
/* currently the following two symbols are not used */
#define	E_CTo      0x2000	/* continuous control change flag (curve) */
#define	E_Pt       0x3000	/* continuous control change flag 
				   (intermediate point) */
#define CTYPE_SHIFT  12
#define CTYPE	   0xf000

typedef struct event {
    struct event  *next;
    long   time;		/* time in SMF ticks */
    long   seqno;		/* sequential number: for the sake of keeping 
				   the original order of events */
    unsigned short  type;	/* event type */
    uchar  ch;			/* channel number - 1 */
    uchar  note;		/* note number for note-on/off & after-touch */
    union {
	struct {
	    int  veloc;              /* velocity for note-on/off */     
	    struct event *partner;   /* corresponding note-off/on event */
	} _n;

	float  val;		/* control value */

	struct {
	    float  val;		/* control value for continuous ctrl chg 
				   (must coincide with the above 'val') */
	    int  tmstep; 	/* time step (in SMF ticks) */
	} _c;

	struct {
	    int  len;		/* length of exclusive or meta message */
	    uchar  *data;	/* exclusive or meta data (NULL if len = 0) */
	} _m;
    } u;

#   define ev_veloc    u._n.veloc
#   define ev_partner  u._n.partner
#   define ev_tmstep   u._c.tmstep
#   define ev_len      u._m.len
#   define ev_data     u._m.data

} Event;

/* 
 * Track: event buffer organized as a list of time-ordered event sequences
 */
typedef struct event_seq {
    struct event_seq  *next;	/* ptr to the next event sequence */
    Event  *events;		/* time-ordered sequence of events */
    int  linefittable;		/* true if continuous control change can be
				   applied to the events in this sequence */
} EventSeq;
    
typedef struct track {
    EventSeq *eseq;		/* ptr to the list of event sequences 
				   (This keeps all kinds of events before
				    channnel-spliting or after channel-merging;
				    otherwise, it keeps only 
				    channel-independent events) */
    EventSeq *eseqch[16];	/* event sequences per channel */
} Track;

/*
 * SMFInfo: information contained in a standard MIDI file
 */
typedef struct smfinfo {
    int  ntrks;		/* number of tracks */
    Track  *trks;	/* array of tracks */
    int  format;
    int  resolution;
} SMFInfo;
    
/*
 * Options: user's preference information 
 */
typedef struct options {
    int  mode;		/* EPL, EPL_A, EPL_Y, or MPL */
    int  measure_on;	/* display measure numbers */
    long  toffset;	/* length of the preamble section in SMF ticks */
    int  use_len_name;	/* use symbolic time representation if possible */
    int  sep_note_ev;	/* separate note-on and note-off events */
    int  sym_ctrl;	/* symbolic output of ctrl-chgs (mod, vol, etc.) */ 
    int  ignore_chan;	/* ignore MIDI channel numbers */
    int  split_mode;	/* 0: no split  1: split by MIDI channel  
			   2: split by MIDI channel and event type */
    int  chord_on;	/* make chords (MPL mode only) */
    int  cd_mindup;	/* minimum percentage of duplicated segment of notes 
			   within a chord */
    int  linefit_on;	/* make continuous control-change events */
    int  lf_maxtmstep;	/* maximum time step for line fitting (in SMF ticks)*/
    float lf_tolerance;	/* error tolerance for line fitting */
    int  max_ovs_len;	/* maximum length of "overshoot" part of measure */ 
    int  inherit_v;	/* inherit "v" value from the previous measure */
    char *track_list;	/* list of active tracks (argument of -T option) */
    char *chan_list;	/* list of active channels (argument of -c option) */
    int  use_gm_name;	/* use General MIDI instrument names */
} Options;

#define EPL	0	/* Event-per-line mode (relative time) */
#define EPL_A	1	/* Event-per-line mode (absolute time) */
#define EPL_Y	2	/* Event-per-line mode (another form of relative tm) */
#define MPL	3	/* Measure-oriented (measure-per-line) mode */


/********************************************************************
 * Value definitions
 ********************************************************************/

#ifndef MAXINT
#define  MAXINT   (~(1 << ((sizeof(int) * 8) - 1)))
#endif
#ifndef MAXLONG
#define  MAXLONG   (~(1 << ((sizeof(long) * 8) - 1)))
#endif

/********************************************************************
 * Global variables
 ********************************************************************/

extern	int	resolution;   /* SMF resolution (ticks per quarter note) */

/********************************************************************
 * System Library Function Prototypes
 ********************************************************************/
#if defined(__STDC__) && !defined(PROTOTYPE)
#   define PROTOTYPE 1 
#endif
#if !defined(P) 
#   ifdef PROTOTYPE
#      define P(arg)  arg
#   else
#      define P(arg)  ()
#   endif
#endif

/********************************************************************
 * User Function Prototypes and Macros
 ********************************************************************/
/* m2p.c */
void	err_nomem P((char *));

/* readsmf.c */
void	read_smf P((char *, SMFInfo *));

/* evlist.c */
void	csplit P((Track *));
void	esplit P((EventSeq **));
EventSeq *nsplit P((Event *));
void	merge_evlist P((EventSeq *));
void	cmerge P((Track *));
void	pair_notes P((Event *));

/* oututil.c */
void	init_measure P((EventSeq *, long));
char *	get_measure P((long));
void  	get_measure_range P((long, long *, long *));
#define  KS_GLOBAL  0
#define  KS_LOCAL   1
void	init_ksinfo P((EventSeq *, int));
int	get_key P((long));
char *	note_name P((int, int, int));
char *	len_name P((long, int, int));
char *	pmml_code P((Event *, int, int, int, int));
void	print_excl P((Event *, int));
long	guess_toffset P((SMFInfo *, int));
long	adjust_steptime P((long, long, long));
void	read_gm_files P((void));

/* eplout.c */
void	eplout P((Event *, Options *, int));

/* mplout.c */
void	mplout P((Event *, Options *, int));

#include "../common/util.h"
