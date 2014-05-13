/*
 * M2P: A SMF-to-PMML Translator
 *   
 * oututil.c: common routines for outputting
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

/*
 * Acknowledgments: I am grateful to Mr. Tomoaki Sasaki, and Mr. Yoshitaka
 * Endo for their help in developing this program.
 */

#include <stdio.h>
#include <math.h>
#include "m2p.h"

/* structure for time-signature information */
typedef struct timesig {
    struct timesig  *next;
    long  base_time;		/* aboslute time (in SMF ticks) */
    int  meas;			/* measure number from here */
    int  ticks_per_beat;	/* ticks per beat from here (For example,
				   if the time signature is 3/8 and 
				   SMF resolution is 480, this value is 240) */
    int  ticks_per_meas;	/* ticks per masure from here */
} TimeSig;

static TimeSig  *tslist = NULL;	/* top of the list of time-sig info */

/* structure for key-signature information */
typedef struct keysig {
    struct keysig  *next;
    long  time;
    short  sfmode;	/* upper byte: # of sharp(+) or flags(-)
			   lower byte: major(0) or minor(1) */
} KeySig;

static KeySig *g_kslist = NULL;	/* top of the list of "global" key-sig info
				   (key-sig information stored in Track 1) */
static KeySig *l_kslist = NULL; /* top of the list of "local" key-sig info 
				   (key-sig information in other tracks) */


/* table for guessing note names based on keys */
static char ptable[][3][12][3] =
{
    {
	/* for C-major, G-major, and F-major */
	{"C", "C#", "D", "Eb", "E", "F", 
	     "F#", "G", "Ab", "A", "Bb", "B"},
	/* for other major keys having sharps in key signature */
	{"C", "C#", "D", "D#", "E", "F",
	     "F#", "G", "G#", "A", "A#", "B"},
	/* for other major keys having flats in key signature */
	{"C", "Db", "D", "Eb", "E", "F",
	     "Gb", "G", "Ab", "A", "Bb", "B"}
    },
    {
	/* for A-minor, E-minor, and D-minor */
	{"C", "C#", "D", "Eb", "E", "F", 
	     "F#", "G", "G#", "A", "Bb", "B"},
	/* for other minor keys having sharps in key signature */
	{"C", "C#", "D", "D#", "E", "F",
	     "F#", "G", "G#", "A", "A#", "B"},
	/* for other minor keys having flats in key signature */
	{"C", "Db", "D", "Eb", "E", "F",
	     "Gb", "G", "Ab", "A", "Bb", "B"}
    }
};

/* GM instrument name for each program number */
static char  *gm_inst[128];

/* GM rhythmic instrument name for each note number */
static char  *gm_drums[128];


/*
 * register time-signature information to make get_measure() and 
 * get_measure_range() work
 */
void
init_measure(eseq, toffset)
EventSeq  *eseq;	/* list of events containing time signature */
long  toffset;		/* time offset */
{
    TimeSig  *ts, *newts;
    Event  *ep;

    /* If there is old information, destroy it. */
    {
	TimeSig  *tsnext;

	for( ts = tslist; ts != NULL; ts = tsnext ) {
	    tsnext = ts->next;
	    free(ts);
	} 
	tslist = NULL;
    }
    
    /* make the first TimeSig struct which represents default time signature */
    if( !(tslist = (TimeSig *)malloc(sizeof(TimeSig))) ) {
	err_nomem("init_measure");
    }
    tslist->base_time = toffset;
    tslist->meas = 1;
    tslist->ticks_per_beat = resolution;	/* The default is 4/4. */
    tslist->ticks_per_meas = resolution * 4;
    tslist->next = NULL;

    if( !eseq )  return;

    /* scan the event list and find TimeSig events */
    ts = tslist;
    for( ep = eseq->events; ep != NULL; ep = ep->next ) {
	if( ep->type != E_TimeSig )  continue;

	if( ep->time > toffset ) {   /* if time <= toffset, overwrite the
					first TimeSig structure */
	    if( !(newts = (TimeSig *)malloc(sizeof(TimeSig))) ) {
		err_nomem("init_measure");
	    }
	    newts->base_time = ep->time;
	    newts->meas = ts->meas +
		ceil((double)(ep->time - ts->base_time) / ts->ticks_per_meas);
	    ts->next = newts;
	    ts = newts;
	}

	ts->ticks_per_beat = resolution * ep->ev_data[2] / 24;
	ts->ticks_per_meas = 
	    (resolution * 4 * ep->ev_data[0]) >> ep->ev_data[1];
    }
    ts->next = NULL;
}

/*
 * calculate the "meas:beat:tick" string from absolute time 
 *   The returned string is static and should not be free'ed.
 */
char *
get_measure(time)
long  time;	/* absolute time in SMF ticks */
{
    TimeSig  *ts;
    static char  buf[32];
    int  meas, beat, ticks;
    long  meas_rem;

    if( time < tslist->base_time ) {	/* before the first measure */ 
	sprintf(buf, "%9ld", (time - tslist->base_time) * IRES / resolution);
    } else {
	for( ts = tslist; ts->next != NULL; ts = ts->next ) {
	    if( ts->next->base_time > time )  break;
	}
	meas = (time - ts->base_time) / ts->ticks_per_meas + ts->meas;
	meas_rem = (time - ts->base_time) % ts->ticks_per_meas;
	beat = meas_rem / ts->ticks_per_beat + 1;
	ticks = meas_rem % ts->ticks_per_beat;
	sprintf(buf, "%03d:%1d:%03d", meas, beat, ticks * IRES / resolution); 
    }
    return buf;
}

/*
 * find a measure satisfying measure_start_time <= time < measure_end_time
 * and returns the start and end times
 */
void
get_measure_range(time, starttime, endtime)
long  time;
long  *starttime, *endtime;  /* output */
{
    TimeSig  *ts;

    if( time < tslist->base_time ) {	/* before the first measure */ 
	*starttime = 0;
	*endtime = tslist->base_time;
    } else {
	for( ts = tslist; ts->next != NULL; ts = ts->next ) {
	    if( ts->next->base_time > time )  break;
	}
	*starttime = ts->base_time +
	    (time - ts->base_time) / ts->ticks_per_meas * ts->ticks_per_meas;
	*endtime = *starttime + ts->ticks_per_meas;
	if( ts->next != NULL && *endtime > ts->next->base_time ) {
	    *endtime = ts->next->base_time;
	}
    }
}

/*
 * register key-signature information to make get_key() work
 */
void
init_ksinfo(eseq, gl)
EventSeq  *eseq;	/* list of events containing time signature */
int  gl;		/* 0: global  or  1: local */
{
    KeySig  **kslist = (gl == KS_GLOBAL ? &g_kslist : &l_kslist);
    KeySig  *ks;
    Event  *ep;

    /* If there is old information, destroy it. */
    {
	KeySig  *ksnext;

	for( ks = *kslist; ks != NULL; ks = ksnext ) {
	    ksnext = ks->next;
	    free(ks);
	}
	*kslist = NULL;
    }
    
    /* make the first keySig struct which represents default key signature */
    if( gl == KS_GLOBAL ) {
	if( !(ks = (KeySig *)malloc(sizeof(KeySig))) ) {
	    err_nomem("init_ksinfo");
	}
	ks->time = 0;
	ks->sfmode = 0;  /* C-major */
	ks->next = NULL;
	*kslist = ks; 
	kslist = &ks->next;
    }

    if( !eseq )  return;

    /* scan the event list and find KeySig events */
    for( ep = eseq->events; ep != NULL; ep = ep->next ) {
	if( ep->type != E_KeySig )  continue;

	if( !(ks = (KeySig *)malloc(sizeof(KeySig))) ) {
	    err_nomem("init_ksinfo");
	}
	ks->time = ep->time;
	ks->sfmode = (ep->ev_data[0] << 8) | ep->ev_data[1];
	ks->next = NULL;
	*kslist = ks;
	kslist = &ks->next;
    }
}

/*
 * get the current key (sfmode) 
 *   The return value is supposed to be passed to "note_name".
 */
int
get_key(time)
long  time;		/* absolute time in SMF ticks */
{
    KeySig  *ks;

    if( l_kslist && time >= l_kslist->time ) {
	/* There is "local" key-sig information in this track. */
	ks = l_kslist;
    } else {
	ks = g_kslist;
    }

    for( ; ks->next != NULL; ks = ks->next ) {
	if( ks->next->time > time )  break;
    }
    return ks->sfmode;
}

/*
 * convert note number to note string
 *   The returned string should not be free'ed.
 */
char *
note_name(note, key, use_drums_name)
int  note;		/* MIDI note number */
int  key;		/* return value of "get_key" */
int  use_drums_name;	/* if true, use the GM rhythmic instrument name */
{
    int  oct, sf, idx;
    static char  buf[8];

    if( use_drums_name && gm_drums[note] != NULL ) {
	return gm_drums[note];
    }

    sf = (short)key >> 8;
    if( sf > 1 )  idx = 1;
    else if( sf < -1 )  idx = 2;
    else idx = 0;

    oct = note / 12 - 1;

    sprintf(buf, "%s%d", ptable[key & 1][idx][note % 12], oct); 

    return buf;
}


/*
 * convert time value to string
 *   The returned string is static and should not be free'ed.
 */
char *
len_name(len, use_len_name, l_equal) 
long  len;
int  use_len_name;	/* if ture, use symbolic time representation */
int  l_equal;		/* prefix 'l=' when the result is merely an integer */
{
    static char  ltable[] = {'w', 'h', 'q', 'i', 's', 'z'};
    static char  buf[32];
    char  *p;
    int  rank, blen, times;
    int  no_dots = 0;

    p = buf;
    for( times = 1;; times++ ) {  /* up to 2 times */
	if( len < 0 ) {
	    if( p == buf && l_equal ) {
		strcpy(p, "l=");
		p += 2;
	    }
	    *p++ = '-';
	    len = -len;
	} else if( p != buf ) {
	    *p++ = '+';
	}

	if( use_len_name ) {
	    blen = resolution << 2;
	    for( rank = 0; rank < 6; rank++ ) {
		if( len >= blen )  break;
		blen >>= 1;
	    }
	    /* the value of rank is:
                 0:     len >= w
		 1: w > len >= h
		 2: h > len >= q
		 3: q > len >= i
		 4: i > len >= s
		 5: s > len >= z
		 6: z > len        */
	    
	    /* First, try basic, dotted and double-dotted note names. */ 
	    if( rank < 6 ) {
		if( len == blen ) {
		    *p++ = ltable[rank];
		    *p = 0;
		    break;
		} else if( !no_dots ) {
		    if( (blen & 1) == 0 && len == blen + (blen >> 1) ) { 
			*p++ = ltable[rank];
			*p++ = '.';
			*p = 0;
			break;
		    } 
#ifdef USE_DOUBLE_DOTTED
		    else if( (blen & 3) == 0 && 
			    len == blen + (blen >> 1) + (blen >> 2) ) { 
			*p++ = ltable[rank];
			strcpy(p, "..");
			break;
		    }
#endif
		}
	    }
	    
	    /* Then, try triplet. */
	    if( times == 1 && rank >= 2 && len * 3 == (blen << 2) ) {
		*p++ = ltable[rank-2];
		strcpy(p, "/3");
		break;
	    }

	    /* Try the '<interger>w' form if longer than '2w' */
	    if( len >= (resolution << 3) ) {
		int  n = len / (resolution << 2);
		len %= (resolution << 2);
		if( len >= 1.75 * (resolution << 2) ) {
		    n++;
		    len -= (resolution << 2);
		}
		sprintf(p, "%dw", n);
		if( len == 0 ) {
		    break;
		} else if( times == 1 ) {
		    /* try to find an 'X+Y' or 'X-Y' (X>Y) form */
		    p += strlen(p);
		    continue;
		}
	    } 

	    /* try to find an 'X+Y' or 'X-Y' (X>Y) form if the length is 
	       is nearly equal to basic note length or if the length is
	       longer than whole note */
	    if( rank < 6 && times == 1 && 
	       (rank == 0 || len <= 1.25 * blen || len >= 1.75 * blen) ) { 
		if( len >= 1.75 * blen ) {
		    rank--;
		    blen <<= 1;
		} 
		if( rank == -1 ) {
		    strcpy(p, "2w");
		    p += 2;
		} else {
		    *p++ = ltable[rank];
		    *p = 0;
		    no_dots = 1;
		}
		len -= blen;
		continue;
	    }
	}

	/* give up using symbolic names */ 
	if( (len * IRES) % resolution == 0 ) {
	    if( p == buf && l_equal ) {
		strcpy(p, "l=");
		p += 2;
	    }
	    sprintf(p, "%ld", (len * IRES) / resolution);
	} else if( (len * IRES * 4) % resolution == 0 ) {
	    /* If SMF resolution is not a divisor of 480, 
	       there might be a case when 'u' can not be used.
	       In this case, still use 'XX.25u', 'XX.5u', or 'XX.75u' 
	       if possible, or a fraction style otherwise. */ 
	    if( p == buf && l_equal ) {
		strcpy(p, "l=");
		p += 2;
	    }
	    sprintf(p, "%.9g", (float)(len * IRES) / resolution);
	} else {
	    sprintf(p, "%ldq/%d", len, resolution);
	}
	break;
    }

    return buf;
}

/*
 * convert an event to its PMML code string
 *   The returned string should not be free'ed.
 *   If retrun value is NULL, it means the event is either exclusive,
 *   or non-standard meta events.  Use 'print_excl' for such an event.  
 */
#ifdef DOS
#  define SJIS   1
#endif
#ifdef SJIS
#  define TwoByteChar(c)  ((unsigned char)(c) >= 0x80 \
			   && (unsigned char)(c) <= 0x9f || \
			   (unsigned char)(c) >= 0xe0)
#else
#  define TwoByteChar(c)  0
#endif

#define  MIN_BUFSIZE  100
 
char *
pmml_code(ep, sym_ctrl, no_marker, pack, use_gm_name)
Event  *ep;
int  sym_ctrl;	/* if ture, use a controller name for ctrl change */
int  no_marker; /* should be true for track 2 or above in Fomart-1 SMF */
int  pack;	/* if true, spaces after commas are eliminated */
int  use_gm_name;  /* if true, use GM instrument name */
{
    static char  *buf = NULL;
    static int   alloc_size;

    static char  *ctrl_name[] = { NULL, "mod", "breath", NULL, 
				      "foot", "pmtime", NULL, "vol",
				      NULL, NULL, "pan", "expr" };
    static char  *switch_name[] = { "ped", "pm", NULL, "sped" };
    static char  *vctrl_name[] = { "bend", "kp", "cpr", "prog" };
    static char  *text_name[] = { "comment", "copyright", "trackname",
				      "instname", "lyric", "marker", "cue" };
    char  *cname = NULL;
    int  etype = ep->type & ETYPE;

    if( !buf ) {
	if( !(buf = malloc(MIN_BUFSIZE) ) )  err_nomem("pmml_code");
	alloc_size = MIN_BUFSIZE;
    }

    if( etype < E_Meta ) {
	/* (extended) control change */
	if( !(ep->type & E_To) && sym_ctrl && etype >= 0x40 &&
	   etype < 0x40 + (int)(sizeof(switch_name)/sizeof(char *)) &&
	   (ep->u.val == 0 || ep->u.val == 127) ) {
	    /* ped, pedoff, etc. */
	    sprintf(buf, (int)ep->u.val == 0 ? "%soff" : "%s", 
		    switch_name[etype - 0x40]); 
	} else if( etype == E_Prog && use_gm_name && ep->ch != RHYTHM_CH ) {
	    strcpy(buf, gm_inst[(int)ep->u.val - 1]);
	} else {
	    if( etype == E_Tempo ) {
		cname = "tempo";
	    } else if( etype >= E_VCtrl ) {
		cname = vctrl_name[etype - E_VCtrl];
	    } else if( sym_ctrl 
		      && etype < (int)(sizeof(ctrl_name)/sizeof(char *)) ) {
		cname = ctrl_name[etype];
	    }
	    if( !(ep->type & E_To) ) {
		if( cname ) {
		    sprintf(buf, "%s(%.8g)", cname, ep->u.val);
		} else {
		    sprintf(buf, pack ? "ctrl(%d,%.8g)" : "ctrl(%d, %.8g)", 
			    etype, ep->u.val);
		}
	    } else {
		/* continuous control change */
		if( cname ) {
		    sprintf(buf, "%s_to(%.8g)", cname, ep->u.val);
		} else {
		    sprintf(buf, pack ? "ctrl_to(%d,%.8g,%s,0)" : 
			    "ctrl_to(%d, %.8g, %s, 0)", etype, 
			    ep->u.val, len_name((long)ep->ev_tmstep, 0, 0));
		}
	    }
	    if( etype == E_Kp ) {
		/* for key-pressure event, specify note number */
		sprintf(buf + strlen(buf), 
			"(n=%s)", note_name(ep->note, get_key(ep->time), 0));
	    }
	}

    } else if( etype >= E_Text && etype <= E_TextEnd ) {
	/* text event */
	char  *p, *s;
	int  i;

	if( ep->ev_len * 2 + MIN_BUFSIZE > alloc_size ) { /* "*2" is for '\' */
	    if( !(buf = realloc(buf, ep->ev_len * 2 + MIN_BUFSIZE)) ) {
		err_nomem("pmml_code");
	    }
	}
	if( etype < E_Text + (int)(sizeof(text_name)/sizeof(char *)) ) {
	    cname = text_name[etype - E_Text];
	}
	if( no_marker && (etype == 2 || etype == 6) ) {
	    cname = NULL;  /* To avoid that the text event goes to Track 1,
			      do not use copyright(..) or marker(..). */
	}
	if( cname ) {
	    sprintf(buf, "%s(\"", cname); 
	} else {
	    sprintf(buf, pack ? "text(%d,\"" : "text(%d, \"", 
		    etype - E_Text + 1); 
	}

	/* copy the text string with handling special characters */
	p = buf + strlen(buf);
	for( i = ep->ev_len, s = (char *)ep->ev_data; --i >= 0; s++ ) {
	    if( *s == '\0' ) {
		/* If the string contains the '\0' character, use "meta()", 
		   because the interpreter can not handle it as a string */
		return NULL;
	    }
	    if( TwoByteChar(*s) && i >= 1 ) {
		*p++ = *s++;
		--i;
	    } else if( *s == '"' || *s == '\n' || *s == '\\' ) {
		*p++ = '\\';
	    }
	    *p++ = *s;
	}
	strcpy(p, "\")");

    } else switch( etype ) {
    case E_NoteOn:
	sprintf(buf, pack ? "note_on(%s,%d)" : "note_on(%s, %d)", 
		note_name(ep->note, get_key(ep->time), 0), ep->ev_veloc);
	break;
    case E_NoteOff:
	if( ep->ev_veloc < 0 ) {
	    sprintf(buf, "note_off(%s)", 
		note_name(ep->note, get_key(ep->time), 0));
	} else {
	    sprintf(buf, pack ? "note_off(%s,%d)" : "note_off(%s, %d)", 
		note_name(ep->note, get_key(ep->time), 0), ep->ev_veloc);
	}
	break;
    case E_Seqno:
	sprintf(buf, "seqno(%d)", (ep->ev_data[0] << 8) + ep->ev_data[1]);
	break;
    case E_Smpte:
	sprintf(buf, 
		pack ? "smpte(%d,%d,%d,%d,%d)" : "smpte(%d, %d, %d, %d, %d)",
		ep->ev_data[0], ep->ev_data[1], ep->ev_data[2],
		ep->ev_data[3], ep->ev_data[4]);
	break;
    case E_TimeSig:
	{
	    int  p3 = (ep->ev_data[2] << ep->ev_data[1]) / 96;
	    if( ep->ev_data[3] != 8 ) {
		sprintf(buf, pack ? "timesig(%d,%d,%d,%d)" : 
			"timesig(%d, %d, %d, %d)",
			ep->ev_data[0], 1 << ep->ev_data[1], p3, 
			ep->ev_data[3]);
	    } else if( p3 != 1 ) {
		sprintf(buf, 
			pack ? "timesig(%d,%d,%d)" : "timesig(%d, %d, %d)",
			ep->ev_data[0], 1 << ep->ev_data[1], p3); 
	    } else {
		sprintf(buf, pack ? "timesig(%d,%d)" : "timesig(%d, %d)",
			ep->ev_data[0], 1 << ep->ev_data[1]); 
	    }
	}
	break;
    case E_KeySig:
	{
	    int  sf = ep->ev_data[0];
	    /* safe sign extension */
	    if( sf & 0x80 )  sf = (short)(sf | 0xff00); 
	    sprintf(buf, pack ? "keysig(%d,%d)" : "keysig(%d, %d)", 
		    sf, ep->ev_data[1]);
	}
	break;
    case E_TrkEnd:
	sprintf(buf, "end");
	break;
    default:
	/* exclusive or other type of meta event */
	return NULL;
    }

    return buf;
}

void
print_excl(ep, indent)
Event  *ep;
int  indent;		/* column number where the command string starts */
{
    static char  buf[16];
    int  len;
    int  i;
    int  col;

    len = ep->ev_len;
    if( ep->type == E_Excl ) {
	/* If the array has no terminating F7, or if it contains 
	   data with MSB = 1, use excl2 */
	for( i = 0; i < len; i++ ) {
	    if( ep->ev_data[i] & 0x80 )  break;
	}
	if( i == len - 1 && ep->ev_data[i] == 0xf7 ) {
	    strcpy(buf, "excl(#(");
	    len--;
	} else {
	    strcpy(buf, "excl2(#(");
	}
    } else if( ep->type == E_Arbit ) {
	strcpy(buf, "arbit(#(");
    } else if( ep->type >= E_Meta && ep->type < E_NoteExcl ) {
	sprintf(buf, "meta(%d, #(", ep->type - E_Meta); 
    } else {
	return;
    }

    indent += strlen(buf);
    fprintf(OUT, buf);

    col = indent;
    for( i = 0; i < len; i++ ) {
	fprintf(OUT, "0x%02x", ep->ev_data[i]);
	col += 5;
	if( i != len - 1 ) {
	    /* if( i % ndata_per_line == ndata_per_line - 1 ) { */
	    if( col + 6 >= MAXCOL ) {
		fprintf(OUT, ",\n%*s", indent, "");
		col = indent;
	    } else {
		fprintf(OUT, ",");
	    }
	}
    }

    fprintf(OUT, "))");
}

/*
 * guess time offset from SMF contents
 *    Must be called after splitting
 */
long
guess_toffset(smf, trk_no)
SMFInfo  *smf;
int  trk_no;	/* track number beginning from 1 */
{
    long  noteontime;	/* time of the earliest note-on event */
    long  tsigtime;	/* time of the TimeSig event after noteontime */
    int  ticks_per_meas, max_boffset;
    long  toffset;
    int  tk, ch, i;
    float  *hist, max_freq, sec_freq, total;
    Track  *tp;
    Event  *ep;
    
    /* First, calculate the time of the earliest note-on event. */
    noteontime = MAXLONG;
    for( tk = (smf->format==2 ? trk_no-1 : 0);
	tk < (smf->format==2 ? trk_no : smf->ntrks); tk++ ) {
	tp = &smf->trks[tk];
	for( ch = 0; ch < 16; ch++ ) {
	    if( tp->eseqch[ch] && 
	       tp->eseqch[ch]->events->type == E_NoteOn &&
	       tp->eseqch[ch]->events->time < noteontime ) {
		noteontime = tp->eseqch[ch]->events->time;
	    }
	}
    }
    /* fprintf(stderr, "Earliest NoteON time = %d\n", noteontime); */

    /* If there are no note events, maybe 0 is the best solution. */
    if( noteontime == MAXLONG )  return 0L;

    /* Find the earliest TimeSig event after (or at) noteontime,
       and at the same time, calculate ticks per measure at noteontime. */
    tsigtime = MAXLONG;
    ticks_per_meas = resolution * 4;	/* The default is 4/4. */
    tp = &smf->trks[smf->format==2 ? trk_no-1 : 0];
    for( ep = tp->eseq->events; ep != NULL; ep = ep->next ) {
	if( ep->type == E_TimeSig ) {
	    if( ep->time > noteontime || 
	       ep->time == noteontime && noteontime > 0 ) {
		tsigtime = ep->time;
		break;
	    } else {
		ticks_per_meas = 
		    (resolution * 4 * ep->ev_data[0]) >> ep->ev_data[1];  
	    } 
	}
    }

    /* If there is a TimeSig event after noteontime, it probably exists at
       a measure boundary.  Therefore, we can guess toffset from the time
       of the TimeSig event. */
    if( tsigtime != MAXLONG ) {
	/* Beware that the following '/' performs integer division. */
	toffset = tsigtime - 
	    (tsigtime - noteontime) / ticks_per_meas * ticks_per_meas; 
    } else {
    
	/* Now, we estimate the strongest beat position by taking the histogram
	   of the note-on time (modulo ticks_per_meas) for all the notes. */
	if( !(hist = (float *)malloc(sizeof(float) * ticks_per_meas)) ) {
	    err_nomem("guess_toffset");
	}
	memset(hist, 0, sizeof(float) * ticks_per_meas);
	total = 0;
	for( tk = (smf->format==2 ? trk_no-1 : 0);
	    tk < (smf->format==2 ? trk_no : smf->ntrks); tk++ ) {
	    tp = &smf->trks[tk];
	    for( ch = 0; ch < 16; ch++ ) {
		if( tp->eseqch[ch] ) {
		    for( ep = tp->eseqch[ch]->events; ep; ep = ep->next ) {
			if( ep->type == E_NoteOn ) {
			    /* The following formula was decided empirically */
			    float  w = (140 - ep->note) / 140.0;
			    float  weight = w * w * ep->ev_veloc;
			    hist[ep->time % ticks_per_meas] += weight;
			    total += weight;
			}
		    }
		}
	    }
	}
	
	/* When collected information is too few, 
	   maybe 0 is the best solution. */
	if( total < 500.0 )  return 0L;
	
	/* add 7% (This value was determined empirically) bonus on time 0 */
	hist[0] += total * 0.07;
	/* add 3% bonus on noteontime */
	hist[noteontime % ticks_per_meas] += total * 0.03;
	
	/* calculate the most and second most frequent times */
	max_freq = sec_freq = -1;
	for( i = 0; i < ticks_per_meas; i++ ) {
	    if( hist[i] > max_freq ) {
		max_boffset = i;
		sec_freq = max_freq;
		max_freq = hist[i];
	    } else if( hist[i] > sec_freq ) {
		sec_freq = hist[i];
	    }
	}
	/* Now, max_boffset is the beat position (modulo ticks_per_meas) */
	
	/* Find the earliest beat position after (or at) noteontime */
	toffset = (int)ceil((float)(noteontime - max_boffset) / ticks_per_meas)
	    * ticks_per_meas + max_boffset;
    }
	
    /* If the Auftakt part is longer than 2/3 of measure length,
       move toffset 1 measure earlier */
    if( toffset >= ticks_per_meas &&
       (toffset - noteontime) > ticks_per_meas * 2 / 3 ) {
	toffset -= ticks_per_meas;
    }

    /* fprintf(stderr, "Gussed toffset = %d (margin=%.1f%%)\n",
	    toffset, (max_freq - sec_freq) / max_freq * 100); */

    return toffset;
}

/*
 * adjust step time so that it is not so longer than gate time
 */
long
adjust_steptime(gatetime, steptime, last_steptime)
long  gatetime, steptime;
long  last_steptime;  /* preferred steptime: 0 if no preference */
{
    int  len_table[][2] = {
	/* {possible step time, minimum gate time (not-inclusive)} */
	{ 1920, 1440 },  /* w */       /* { a, b }  */
	{ 1440,  960 },  /* h. */      /* { c, d }  b <= c */
	{  960,  480 },  /* h */
	{  480,  120 },  /* q */
	{  240,   60 },  /* i */
	{  120,   30 },  /* s */
	{   60,    0 },  /* z */
	{0,0}
    };
    int  len_table3[][2] = {
	/* special table for triplets */
	{  640,  320 },  /* w/3 */
	{  320,  100 },  /* h/3 */
	{  160,   40 },  /* q/3 */
	{   80,   20 },  /* i/3 */
	{   40,    0 },  /* s/3 */
	{0,0}
    };
    int  newsteptime;
    int  (*table)[2];
    int  trlen;
    int  i, k;

    if( steptime <= gatetime ) {
	return steptime;
    }
    if( gatetime == 0 ) {
	return 0L;
    } else if( gatetime > resolution * 4 ) {
	return gatetime;
    }

    /*
     * Policies:
     *   1. The closer gatetime/steptime is to 1.0, the better.
     *      But do not excced 1.0.
     *   2. The resulting steptime should be a "standard" length. 
     *   3. gatetime/steptime should be greater than a lower limit
     *      which is defined for each standard length.
     *   4. If there are multiple candidates, put a preference on 
     *      the same step time as the previous note.
     */

    /* decide which table is used */
    table = len_table;
    if( resolution % 12 == 0 ) {   /* Can s/3 be excatly expressed? */ 
	for( trlen = resolution*4/3; trlen >= resolution/12; trlen >>= 1 ) {
	    if( steptime == trlen ) {
		/* It's a triplet note. */
		table = len_table3;
		break;
	    }
	}
    }

    /* find the first candidate */
    for( i = 0; table[i][0] != 0; i++ ) {
	newsteptime = table[i][0] * resolution / IRES;
	if( steptime >= newsteptime &&
	    gatetime <= newsteptime &&
	    gatetime > table[i][1] * resolution / IRES ) {
	    break;
	}
    }

    if( table[i][0] == 0 ) {
	/* no candidate - there are two cases:
	   1. very short note (steptime < z) 
	   2. notes like steptime=w-10u and gatetime=h.+10u */
	return steptime;
    }
    
    /* Now, try to find the prefered candidate. */
    for( k = i; table[k][0] != 0; k++ ) {
	newsteptime = table[k][0] * resolution / IRES;
	if( gatetime <= newsteptime ) {
	    if( newsteptime == last_steptime ) {
		/* It's the prefered one. */
		return  newsteptime;
	    }
	}
    }

    /* There is no prefered one in the candidates. */
    return(table[i][0] * resolution / IRES);
}

/*
 * Read the files describing GM instrument names into gm_inst and gm_drums
 */
#define  LINEBUFSIZ   128

void
read_gm_files()
{
    char  *pmml_path, *path, *fname;
    char  linebuf[LINEBUFSIZ], namebuf[LINEBUFSIZ];
    FILE  *fp;
    int  n, len;

    if( !(pmml_path = getenv("PMMLPATH")) ) {
	pmml_path = PMML_PATH;
    }

    /* read gm_inst.pml */
    path = pmml_path;
    while( (fname = prepend_path(&path, GM_INST)) != NULL ) {
	if( (fp = fopen(fname, "r")) != NULL ) {
	    break;
	}
    }
    if( !fname ) {
	fprintf(stderr, "%s: Can not read instrument name file\n", GM_INST);
	exit(1);
    }

    while( fgets(linebuf, LINEBUFSIZ-1, fp) != NULL ) {
	/* skip if the line begins with `//' */
	if( linebuf[0] == '/' && linebuf[1] == '/' )  continue;
	if( sscanf(linebuf, "%s = ' prog ( %d", namebuf, &n) == 2 ) {
	    if( (len = strlen(namebuf)) > MAX_INST_LEN ) {
		fprintf(stderr,
			"Warning: %s: Too long instrument name `%s'\n",
			fname, namebuf);
	    } else if( n < 1 || n > 128 ) {
		fprintf(stderr,
			"Warning: %s: Program number %d is out of range\n",
			fname, n);
	    } else {
		if( gm_inst[n-1] == NULL ) {  /* take the first one! */
		    if( !(gm_inst[n-1] = malloc(len + 1)) ) {
			err_nomem("read_gm_files");
		    }
		    strcpy(gm_inst[n-1], namebuf);
		}
	    }
	}
    }

    free(fname);
    fclose(fp);

    /* read gm_drums.pml */
    path = pmml_path;
    while( (fname = prepend_path(&path, GM_DRUMS)) != NULL ) {
	if( (fp = fopen(fname, "r")) != NULL ) {
	    break;
	}
    }
    if( !fname ) {
	fprintf(stderr, "%s: Can not read rhythmic instrument name file\n", 
		GM_DRUMS);
	exit(1);
    }

    while( fgets(linebuf, LINEBUFSIZ-1, fp) != NULL ) {
	/* skip if the line begins with `//' */
	if( linebuf[0] == '/' && linebuf[1] == '/' )  continue;
	if( sscanf(linebuf, "%s = ' note ( n = %d", namebuf, &n) == 2 ) {
	    if( (len = strlen(namebuf)) > MAX_INST_LEN ) {
		fprintf(stderr,
			"Warning: %s: Too long instrument name `%s'\n",
			fname, namebuf);
	    } else if( n < 0 || n > 127 ) {
		fprintf(stderr,
			"Warning: %s: Note number %d is out of range\n",
			fname, n);
	    } else {
		if( gm_drums[n] == NULL ) {  /* take the first one! */
		    if( !(gm_drums[n] = malloc(len + 1)) ) {
			err_nomem("read_gm_files");
		    }
		    strcpy(gm_drums[n], namebuf);
		}
	    }
	}
    }
    
    free(fname);
    fclose(fp);
}

