/*
 * PMML: A Compiler for the Practical Music Macro Language
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

/********************************************************************
 * Constants
 ********************************************************************/
/*
 * default search path for source file
 */
#ifndef PMML_PATH
#ifdef DOS
#  define PMML_PATH  ".;c:/pmml/lib;../lib"
#else
#  define PMML_PATH  ".;/usr/local/lib/pmml;../lib"
#endif
#endif

/*
 * default extention name of PMML source files
 */
#define SRC_EXT   ".pml"

/*
 * default output file name
 */
#define OUT_MID   "out.mid"

/*
 * default output resolution
 */
#define ORES	480

/*
 * resolution (ticks per quoter_note) determining the value of
 * 'u' rational constant and relative ticks in the -f or -t option.
 */
#define IRES    480

/*
 * standard denominator for rational constants
 */
#define LDEN	(IRES * 4 * 5)

/*
 * watermark of the push-back stack for warnning output
 */
#define PBS_WARN  10000

/*
 * mode for tp-register evaluation
 */
#define ADD_TP_IN_CONST

/********************************************************************
 * Definition of byte order
 ********************************************************************/
#ifndef PMML_BIG_ENDIAN
#  ifdef PMML_LITTLE_ENDIAN
#    define PMML_BIG_ENDIAN  0
#  else
#    if defined(mc68000) || defined(sparc) || defined(MIPSEB)
#      define PMML_BIG_ENDIAN  1
#    else
#      if defined(i386) || defined(MIPSEL) || defined(alpha)
#        define PMML_BIG_ENDIAN  0
#      else
         ... Either PMML_BIG_ENDIAN or PMML_LITTLE_ENDIAN must be defined.
#      endif
#    endif
#  endif
#endif

/********************************************************************
 * Definitions for making the code adaptable to either ANSI C or non-ANSI C 
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
 * Type definitions
 ********************************************************************/

typedef unsigned char  uchar;
typedef long   PmmlInt;
typedef double PmmlFloat;

#include "rational.h"

/* 
 * Object - the data type of the PMML
 *   Each macro is associated with an object.
 *   Each array element is an object.
 *   Each object occupies 8 bytes.
 */
/* Objects other than floating numbers are represented
   by 'NaN' of the IEEE floating point format. */
#define  NaN_Base   (unsigned) 0x7ff0
#define  O_INT	    (NaN_Base + T_NUMBER)	/* 32-bit integer */
#define  O_RATIONAL (NaN_Base + T_RATIONAL)	/* rational number */
#define  O_STRING   (NaN_Base + T_STRING)	/* string */
#define  O_ARRAY    (NaN_Base + T_ARRAY)	/* array */
/* Strong token lists are different from weak token lists in that
   a strong token list is not expanded (in other words, it remains to be 
   a token-list object) when it is taken as an expression operand. */ 
#define  O_STOKENS  (NaN_Base + T_STOKENS)	/* strong token list */
#define  O_WTOKENS  (NaN_Base + T_WTOKENS)	/* weak token list */
#define  O_NIL	    (NaN_Base + T_NIL)		/* means an empty message 
						   or invalid operand 
						   (for internal use only) */
#define  isfloat(_type)   ((_type) < O_INT || (_type) > O_NIL)
#define  isnumber(_type)  ((_type) <= O_RATIONAL || (_type) > O_NIL)
#define  isleftval(_type) ((_type) < O_ARRAY || (_type) > O_NIL)

typedef union object {
    struct {
#if PMML_BIG_ENDIAN
	unsigned short  type;	/* O_XXX */
	short  pad1;
	short  pad2;
	short  pad3;
#else
	short  pad1;
	short  pad2;
	short  pad3;
	unsigned short  type;	/* O_XXX */
#endif
    } _t;

    Rational  r;

    PmmlFloat  fpval;		/* for O_FLOAT */ 

    struct {
#if PMML_BIG_ENDIAN
	long   pad;
#endif
	union object_data {
	    PmmlInt  val;	/* for O_INT */
	    char   *str;	/* for O_STRING */
	    struct array  *ap;	/* for O_ARRAY */
	    struct token  *tp;	/* for O_STOKENS or O_WTOKENS */
	} _u;
    } _d;
	
#   define o_type  _t.type 
#   define o_val   _d._u.val
#   define o_str   _d._u.str
#   define o_ap    _d._u.ap
#   define o_tp    _d._u.tp

    /* used only in the 'arg' member of MacroCall */
    struct {
	int  defined;
	struct dic_ent  *dp;
    } id;
} Object;
		 
/*
 * Token - structure associated with a token
 */
typedef struct token {
    /* header part */
    int  type;			/* token type */
    struct token  *next;
    long  src_pos;		/* Position in the source file (for err msg.)
				   Upper 8 bits identify the source file name.
				   Lower 24 bits represent the line number. */

    /* data part */
    union tok_data {
	Object  obj;		/* for T_NUMBER, T_RATIONAL, T_STRING, 
				   T_ARRAY, T_STOKENS, T_SHIFT, and T_ACCENT */

	int  oct;		/* for T_OCTAVE */

	struct tok_id {		/* for T_ID */
	    int  hash; 		/* hash function value */
	    char  *name;
	} id;

	struct tok_pitch {	/* for T_PITCH */
	    char  flags;	/* bit 0: with accidentals(1) or w/o them(0)
				   bit 1: octave is absolute(1) or relative(0) 
				   bit 2: rest(1) or note(0) */
	    char  note_num;	/* C(0), C#(1), ..., B(11) */
	    short octave;
	} pitch;

#       define ACCIDENTAL 0x01
#       define ABS_OCT    0x02
#       define RESTFLAG   0x04
	
	struct tok_efor {		/* for T_EFOR */
	    struct token  *body;	/* loop body */
	    struct dic_ent *loopvar;	/* loop variable */
	    PmmlInt  limit;	   	/* loop count limit */
	    PmmlInt  step;		/* loop count step */
	} efor;

	struct tok_eforeach {		/* for T_EFOREACH */
	    struct token  *body;   	/* loop body */
	    struct dic_ent *loopvar;	/* loop variable */
	    struct array  *ap;	   	/* value list */
	    int   count;           	/* loop count */
	} eforeach;
	
	struct tok_ewhile {		/* for T_EWHILE */
	    struct token  *body;   	/* loop body */
	    struct token  *cond;   	/* loop exit condition */
	} ewhile;
	
	struct tok_erepeat {		/* for T_EREPEAT */
	    struct token  *body;   	/* loop body */
	    int  count;	    		/* loop count */
	} erepeat;

	struct tok_bot {		/* for T_BOT */
	    Rational  stime;		/* time when 'THREAD {' is executed */
	    struct macro_call  *calls;	/* pb->calls when ...ditto */ 
	    int  avoid_sync;		/* true if ended with '}+' */
	} bot;

	struct tok_ewait {		/* for T_EWAIT */
	    struct channel  *chnl;
	    struct wait_status  *wp;		
	    struct dic_ent  *dp;	/* destination of message */
	} ewait; 

	struct tok_ealt {		/* for T_EALT */
	    struct altguard  *guards;	/* list of guards */
	} ealt;
	
	struct dic_ent  *dp;		/* for T_THREAD, T_EFFCLASS 
					   and T_LOCALID */
    } u;

} Token;

#define  TOKEN_SIZE \
    (sizeof(Token) - sizeof(union tok_data) + sizeof(Object))

/* token type */
#define T_EOF		0	/* pseudo token: end of file */
#define T_NUMBER	1	/* 32-bit integer or FP number */
#define T_RATIONAL	2	/* rational number */
#define T_STRING	3	/* string */
#define T_ARRAY		4	/* array */
#define T_STOKENS	5	/* strong token list */
#define T_WTOKENS	6	/* weak token list: This is not a type of a
				   token; It is a type of push-back stack 
				   element. */
#define T_NIL		7	/* This is not a type of a token. See Object */
/* first 8 tokens can not be moved elsewhere */
#define T_ID		8	/* identifier */ 
#define T_LOCALID	9	/* pseudo token: local ID in "q"-argument */
#define T_ISTREAM	10	/* input stream: This is not a type of a token;
				   It is a type of push-back stack element */
/* The following three tokens must be contiguous because we use the equation
   `type >= T_THREAD && type <= T_EFFINST' to identify these tokens. */
#define T_THREAD	11	/* pseudo token: thread name */
#define T_EFFCLASS	12	/* pseudo token: effector class name */
#define T_EFFINST	13	/* pseudo token: effector instance name */
#define T_PITCH		14	/* pitch constant */   
#define T_EFOR		15	/* pseudo token: end of `for' loop */
#define T_EFOREACH	16	/* pseudo token: end of `foreach' loop */
#define T_EWHILE	17	/* pseudo token: end of `while' loop */
#define T_EREPEAT	18	/* pseudo token: end of `repeat' loop */
#define T_OCTAVE	19	/* ^, ^^, ^^^, ... or _, __, ___, ... */ 
#define T_TSHIFT	20	/* >., >>, >>., ... or <., <<, <<., ... */
#define T_ACCENT	21	/* +., ++, ++., ... or -., --, --., ... */
#define T_EOB		22	/* pseudo token: end of sequential block */
#define T_EOINIT	23	/* pseudo token: end of init block */
#define T_BOT		24	/* pseudo token: beginning of named thread */
#define T_EOT		25	/* pseudo token: end of named thread */
#define T_EOP		26	/* pseudo token: end of parallel block */
#define T_ESYNC		27	/* pseudo token: completion of global sync */
#define T_EWAIT		28	/* pseudo token: completion of wait */
#define T_EALT		29	/* pseudo token: end of alt */
/* Codes 33-64 must be reserved for single-character tokens. */
#define T_GTEQ		65	/* >= */    
#define T_LTEQ		66	/* <= */    
#define T_LOGAND	67	/* && */    
#define T_LOGOR		68	/* || */    
#define T_EQ		69	/* == */    
#define T_NEQ		70	/* != */    
#define T_PLUSEQ	71	/* += */    
#define T_MINUSEQ	72	/* -= */    
#define T_MULTEQ	73	/* *= */    
#define T_DIVEQ		74	/* /= */    
#define T_REMEQ		75	/* %= */    
#define T_SHLEQ		76	/* shl= */    
#define T_SHREQ		77	/* shr= */    
#define T_ANDEQ		78	/* &= */    
#define T_XOREQ		79	/* xor= */    
#define T_OREQ		80	/* |= */    
#define T_DCOLON	81	/* :: */    
#define T_UMINUS	82	/* - (unary operator) */
#define T_BOTTOM	83	/* pseudo token: bottom of operator stack */
#define T_ASUB		84	/* obsolete */
#define T_INITBLK	85	/* %{ */
#define T_TRIEQ		86	/* === */
#define T_DDOT		87	/* .. */
#define T_TDOT		88	/* ... */
/* Be careful with fprint_token() (in token.c) when you add new tokens */

/* Codes 91-127 must be reserved for single-character tokens. */
/* Keyword tokens are defined in keyword.h. */
#include "keyword.h"

/*
 * dynamically-growing string 
 */
#include "../common/util.h"

/*
 * structure for input buffer
 *  (We don't use the stdio library for the input of PMML sources,
 *   because of the lack of the capability for initializing buffer contents,
 *   and also because of the unefficient implementation of ungetc.)
 */
struct ibuffer {
    int   cnt;			/* number of characters in the buffer */
    uchar *ptr;			/* pointer to the next character */
    uchar *base;		/* buffer start address */
    int   fd;			/* file descriptor */
    long  src_pos;		/* line number (for error message) 
				   Upper 8 bits are the file name ID. */
    int   eof;			/* true if (1) the file already reachs 
				   its end, or (2) buffer is created 
				   by `evalstr' */
    int   look_ahead;		/* true if the next token is already read */
    Token  tkbuf;     		/* buffer for the next token */
    String  strbuf;		/* string buffer for the next token */
};

/*
 * PBStack - type of 'push-back' stack associated with each thread
 */
typedef union pbstack {
    int	 type;
    Token  t;

    /* The following structure must be consistent with Token structure */
    struct {
	int  type;	
	union pbstack  *next;
	long  src_pos;

	union {
	    Token  *tp;		    /* if type == T_WTOKENS, 
				       pointer to current token */
	    struct ibuffer  *ip;    /* if type == T_ISTREAM */
	} u;

	Token  *tklist;		    /* if type == T_WTOKENS, points the top 
				       of token list */
	union pbstack  *atl_next;   /* if type == T_WTOKENS, next entry in the
				       list of active token-lists */ 

	int  flags;		/* TF_RPLPOS, TF_CALLEOM, TF_FREE */
    } s;
} PBStack;

/* for 'flags' of PBStack */
#define TF_RPLPOS	0x01	/* replace token's src_pos to the current
				   src_pos when the token is taken */
#define TF_CALLEOM	0x02	/* call end_of_macro routine when the 
				   the token list is exhausted */
#define TF_FREE		0x04	/* call free_tklist() when the token list is
				   is exhausted */

/* 
 * Array - structure associated with an array
 */
typedef struct array {
    int	 size;		/* number of elements */
    int	 offset;	/* subscript of elms[] giving the 1st element */
    int  alloc_size;	/* (number of elements to which memory is
			   allocated) minus 1. When alloc_size is -1, 
			   actually-allocated size is equal to `size' */
    unsigned short ref;	/* reference counter (how many this array is
			   referenced by macros or parent arrays). */
    char  ng_flag;	/* non-growable flag (for '$*')  */
    Object  *elms;	/* ptr to array contents */
} Array;

#define array_ref(ap, i) \
    ((ap)->elms[((i)+(ap)->offset) & (ap)->alloc_size])
    /* if alloc_size == -1 then the and'ing makes no effect */ 

/*
 * Thread - the PMML thread
 */
#define REST_NOTE   ((PmmlInt)0x80000000)

typedef unsigned long  EEBV;	/* type of an enabled effectors bit vector */

typedef struct pbinfo {
    PBStack  *top, *bottom;	/* push-back stack */
    struct macro_call *calls;   /* list of currently expanding macros */
    int  loop_count;		/* current nest level of loops */
} PBinfo;

typedef struct thread {
    struct registers {
	Rational  t;	/* absolute time */
	Rational  tb;	/* base time by which relative time is measured */
	Rational  dt;	/* delta time */
	Rational  sh;	/* dt_reg increment value on ">>" command */
	Rational  l;	/* step time */
	Rational  dofs;	/* duration offset (real name is "do") */

	PmmlInt  tk;	/* SMF track number (1-) */
	PmmlInt  ch;	/* MIDI channel number (1-16) */
	PmmlInt  n;	/* note number (0-127, middle C = 60)
			   REST_NOTE (0x80000000) for rests */
	PmmlInt  tp;	/* transpose value */
	PmmlInt  o;	/* octave */
	PmmlInt  key;	/* current key (number of sharps(+) or flats(-)) */
	PmmlInt  v;	/* velocity (0-127) */
	PmmlInt  nv;	/* note-off velocity (0-127);
			   negative when note-off velocity is not used */
	PmmlInt  ac;	/* v_reg increment value on "++" command */
	PmmlInt  dp;	/* duration percentage in step time */
    } reg;
	
    struct eff_inst  *effs;	/* list of attached effectors */
    EEBV  eebv;			/* enabled effectors bit vector */
    int	  pri;			/* priority used in dictionary search */
    Rational  maxtime;		/* maximum time among child threads or
				   among commands in a chord block */
    int	  flags;		/* thread flags */
    struct thread  *parent;	/* parent thread  */
    struct dic_ent  *macros;	/* list of macros, effector classes, and effec-
				   tor instances defined in this thread */
    struct dic_ent  *dp;	/* if this is a named thread 
				   or it is an effector thread,
				   dp contains the ptr to DicEnt struct;
				   in case of an unnamed thread, dp is NULL */
    struct thread  *rtl_next;	/* next entry in the ready thread list */
    int  nchild;		/* number of child threads (effector threads
				   are not included) */
    PBinfo   *pb;		/* information about push-back stack and
				   macro calling. */
} Thread;

#define NRREG	6	/* no. of rational-number registers */
#define NIREG	10	/* no. of integer registers */
#define IsIntReg(x)  ((x) >= NRREG)

#define	R_t	0
#define R_tb	1
#define R_dt	2
#define R_sh	3
#define R_l	4
#define R_do	5
#define R_tk	6
#define R_ch	7
#define R_n	8
#define R_tp	9
#define R_o	10
#define R_key	11
#define R_v	12
#define R_nv	13
#define R_ac	14
#define R_dp	15

typedef struct thread_regs {
    Rational  rreg[NRREG];
    PmmlInt   ireg[NIREG];
} ThreadRegs;

/* thread flags */
#define PFLAG   0x01		/* if true, thread for '[' ']' (chord) */
#define CFLAG   0x02		/* becomes 1 after '}+' */
#define MFLAG   0x04		/* if true, thread for modifier */ 
#define SFLAG   0x08		/* if true, no '&' after '}' or ']' */
#define EFLAG   0x10		/* if true, effector thread */
#define EXITING 0x20		/* if true, a thread is to be deleted but
				   waiting for the completion of its childs */
#define WAITEX  0x40		/* true if wait/alt has been executed */

/*
 * Channel: a way for inter-thread communication/synchronization 
 */
/* structure for each call of signal() */
typedef struct message {
    struct message  *next;
    Rational time;			/* the time when signal() is called */
    long  src_pos;			/* source code position of signal() */
    int   received;			/* true if this msg is once received 
					   (for warning output) */
    Object  message;			/* message contents (if object type
					   is T_NIL, it means a `signal') */
} Message;

/* structure per thread per channel */
typedef struct wait_status {
    struct wait_status  *next;
    enum {
	WS_Idle,
	WS_Executing,		/* thread is executing wait/alt */
	WS_WaitBlocked,		/* thread is being blocked by wait() */
	WS_AltBlocked		/* thread is being blocked by alt */
    } status;
    Thread  *wait_thd;		/* thread (subject of wait/alt history) */
    Thread  *sleep_thd;		/* thread to be awaken 
				   (valid only if status = WS_*Blocked) */
    Message *lastmsg;		/* message read by the last wait/alt call  */
    long  src_pos;		/* source code position of wait/alt call
				   (undefined if status == WS_Idle) */
} WaitStatus;

/* structure per communication channel */
typedef struct channel {
    struct channel  *next;
    struct dic_ent  *dp;	/* ptr to channel name entry */
    Message  *mtop;	  /* accumulated messages to this comm. channel */
    WaitStatus  *wtop;	  /* threads that have ever issued wait/alt calls 
			     to this communication channel */
} Channel;

/* structure per guard of alt */
typedef struct altguard {
    struct altguard  *next;
    Channel  *chnl;		/* waiting channel */
    WaitStatus  *wp;
    Token  *body;		/* action */
    struct dic_ent  *dp;	/* destination of message */
} AltGuard;

/*
 * Event: per-event structure
 */
/* event type */
/* check event_free() when you change the definition of event type numbers */
#define E_Ctrl	    0x000 	/* 0x00-0x7f: control change */
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
#define E_Vmag      0x084
#define E_GCtrl	    0x0c0	/* (0xc0-0xff): control for all channels */
#define E_Tempo     0x0c0
#define E_RTempo    0x0c1

#define E_Meta      0x100	/* 0x100-0x17f: meta event (except tempo) */
#define E_Seqno	    0x100
#define E_Text      0x101
#define E_TrkName   0x103
#define E_Lyric     0x105
#define E_Marker    0x106
#define E_Cue       0x107
#define E_TextEnd   0x10f
#define E_TrkEnd    0x12f
#define E_OrgTempo  0x151
#define E_Smpte     0x154
#define E_TimeSig   0x158
#define E_KeySig    0x159

#define E_IntMeta   0x180	/* 0x180-0x18f: 
				   special meta event (internal use) */   
#define E_LoadEnd   0x180	/* dummy event to insert rest at song end */

#define E_NoteExcl  0x190	/* 0x190-0x193: note on/off and exclusive */
#define	E_Excl      0x190
#define	E_Arbit     0x191
#define E_NoteOff   0x192
#define	E_NoteOn    0x193
#define E_Deleted   0x194	/* deleted event (previously it was NoteOff) */
#define E_END	    0x195	/* Not an event type; for range checking */

/* The following two are not event types but used as flags in the 'scope' 
   element of EffRule struct. */
#define E_NotePair  0x198	/* "case(note)" action flag */
#define E_RawMeta   0x199	/* "case(meta)" action flag */

#define ETYPE	    0xfff

#define	E_To       0x1000	/* continuous control change flag (line) */
#define	E_CTo      0x2000	/* continuous control change flag (curve) */
#define	E_Pt       0x3000	/* continuous control change flag 
				   (intermediate point) */
#define CTYPE_SHIFT  12
#define CTYPE	   0xf000

#define IrregularNoteOff  1	/* see the 'flags' member of Event below */

typedef struct event {
    struct event  *next;
    Rational  time;
    EEBV  eebv;			/* enabled effectors bit vector */
    unsigned short  type;	/* event type */
    unsigned short  track;	/* track number - 1 */
    uchar  ch;			/* channel number - 1 */
    uchar  note;		/* note number for note-on/off & after-touch */
    uchar  flags;		/* bit 0: For a NoteOff event, this bit is
				     zero if the corresponding NoteOn event
				     exists in the past (not at the same time).
				     This bit determines event priority.
				     For other types of events, the value of 
				     this bit is undefined.
				   bits 1-7: unused */
    union {
	struct {
	    PmmlInt  veloc;          /* velocity for note-on/off */     
	    struct event *partner;   /* corresponding note-off/on event */
	} _n;

	Object  obj;		/* control value */

	struct cont_event  *cp;	/* continuous control-chage info */ 

	struct {
	    int  len;		/* length of exclusive or meta message */
	    uchar  *data;	/* exclusive or meta data (NULL if len = 0) */
	} _m;
    } u;

#   define ev_veloc    u._n.veloc
#   define ev_partner  u._n.partner
#   define ev_len      u._m.len
#   define ev_data     u._m.data

} Event;

/* external structure for continuous control-change event */
typedef struct cont_event {
    PmmlFloat  cval;		/* control value */
    Rational  tmstep;		/* time step */
    float  thres;		/* minimum value change in output */
    float  slope1, slope2;	/* slope values at the beginning and 
				   ending points */ 
} ContEvent;

#define  MAXTRACK   65536

#define	 LASTVAL    ((PmmlInt)0x80000000)

/* 
 * Track: event buffer organized as a list of time-ordered event sequences
 */
typedef struct event_seq {
    struct event_seq  *next;	/* ptr to the next event sequence */
    Event  *events;		/* time-ordered sequence of events */
} EventSeq;
    
typedef struct track {
    EventSeq *eseq;		/* ptr to the list of event sequences */
    Event    *last_ep;		/* last inserted event */
    EventSeq *last_sp;		/* last created event sequence */
} Track;

/*
 * MTracks: multiple tracks
 */
typedef struct mtracks {
    int  ntrks;		/* number of tracks */
    Track  *trks;	/* array of tracks */
    Track  marks;	/* track for copied marker/trackname/timesig events */
} MTracks;
    
/*
 * EventSet: set of event types (bit vector)
 */
#define EVSET_SIZE  13 		/* 8 words for control change events,
				   4 words for meta events, and
				   1 word for other events */
typedef unsigned long  EventSet[EVSET_SIZE];	

#define e_idx(type)    (((type) >> 5) & 0xf)
#define e_mask(type)   (1L << ((type) & 0x1f))
#define e_isset(evset, type)    ((evset)[e_idx(type)] & e_mask(type))
#define e_bitset(evset, type)   ((evset)[e_idx(type)] |= e_mask(type))
#define e_bitclr(evset, type)   ((evset)[e_idx(type)] &= ~e_mask(type))
#define e_allclr(evset)   	memset(evset, 0, sizeof(EventSet))
#define e_allset(evset)   	memset(evset, 0xff, sizeof(EventSet))
#define e_copy(d_evs, s_evs)    memcpy(d_evs, s_evs, sizeof(EventSet))
#define e_or(d_evs, s_evs) \
    { int i; for( i = 0; i < EVSET_SIZE; i++ ) (d_evs)[i] |= (s_evs)[i]; }
#define e_andnot(d_evs, s_evs) \
    { int i; for( i = 0; i < EVSET_SIZE; i++ ) (d_evs)[i] &= ~(s_evs)[i]; }
#define e_comp(d_evs) \
    { int i; for( i = 0; i < EVSET_SIZE; i++ ) (d_evs)[i] = ~(d_evs)[i]; }

/*
 * ChanSet: set of channel numbers (bit vector)
 */
typedef unsigned short  ChanSet;

/*
 * EffClass - effector class (data structure for each effector definition)
 */
typedef struct eff_rule {
    struct eff_rule  *next;	/* ptr to next rule */
    EventSet  scope;		/* scope of this rule */
    Token  *action;		/* action token list */
} EffRule;

typedef struct eff_class {
    enum {
	EC_Normal,		/* normal effector */
	EC_BuiltIn 		/* built-in effector */
    } type;

    struct dic_ent  *dp;	/* ptr to the dictionary entry */
    int  flags;			/* effector flags */
    struct eff_inst *instances;	/* list of instances */
    char   *arg_spec;		/* argument specification for init action */

    union {
	struct {		/* for Normal effectors */
	    Token  *init;	/* init action */
	    Token  *detach;	/* "beginning of detach" action */
	    Token  *wrap;	/* "end of detach" action */
	    EffRule  *rules;	/* list of rules */
	    EventSet  ethru;	/* default event through bits */
	} n;
	struct {		/* for BuiltIn effectors */
	    void   (*init)();	/* init action */
	    void   (*detach)();	/* "beginning of detach" action */
	    void   (*action)(); /* event handling action */  
	    void   (*wrap)();	/* "end of detach" action */
	} b;
    } u;
} EffClass;

/* effector flags */
/* If you change these values, recheck the start-up files. */
#define EF_MergeTracks   0x01   /* merge tracks after sorting */
#define EF_ExpandCtrl    0x02	/* expand continuous ctrl change */
#define EF_Retrigger	 0x0100	/* retrigger notes upon note collision */
#define	EF_TimeRange	 0x0400	/* narrow the time range of song */
#define EF_CollectMarker 0x0800	/* collect marker/trackname/timesig events */
#define EF_CutAfterEnd   0x1000	/* cut off after the Track-End event */
#define EF_MoveTimeEv	 0x2000	/* move TEMPO, SEQNO, SMPTE, TIMEISG, and
				   No. >=192 ctrl-chg events to track 1 */

/*
 * EffInst - effector instance (associated with each effector attachment)
 */
typedef struct eff_inst {
    struct eff_inst  *next;	/* list of succeeding effectors */
    struct eff_inst  *cnext;	/* next effector instanciated by same class */
    EffClass  *class;		/* effector class */
    Thread    *thd;		/* effector thread */
    struct dic_ent  *dp;	/* ptr to the dictionary entry */
    int	      flags;		/* effector flags */
    EventSet  ethru;		/* event through bits */
    ChanSet   cthru;		/* channel through bits */
    long      data;		/* reserved for built-in effectors */
    MTracks   ebuf;		/* event buffer for this effector */
    struct dic_ent  *val, *etype, *ctype;	/* special effector-
						   instance macros */
} EffInst;


/*
 * DicEnt - entry for the name dictionary
 */
typedef struct dic_ent {
    enum dicent_type {
	D_LocalMacro,		/* macro-local macro name */
	D_ThreadMacro,		/* thread-local macro name (incl. global) */
	D_BuiltIn,		/* built-in macro name */
	D_ThreadName,		/* thread name */
	D_EffClass,		/* effector class name */
	D_EffInst,		/* effector instance name */
	D_Channel		/* communication channel name */
    } type;

    char  *name;		/* name string */
    int   hash;			/* hash value */
    int   active;		/* is currently defining, or being used 
				   as a loop variable */
    struct dic_ent  *hnext;	/* next entry having same hash value */
    struct dic_ent  *dnext;	/* next entry belonging to same thread */

    union {
	struct macro_call *m;	/* macro defining this name (D_LocalMacro) */
	Thread  *c;		/* thread defining this name (others) */
    } scope;

    union {
	struct {
	    char  *arg_spec;	/* argument specification string 
				 *    NULL means a variable macro
				 *    DEF_ARG_SPEC means "q*"
				 */
	    Object  obj;	/* macro contents (for D_*Macro, D_BuiltIn) */
	} _macro;

	struct {
	    char  *arg_spec;
	    void  (*handler)();	/* handler function (D_BuiltIn) */
	} _builtin;
	    
	Thread  *thd;		/* thread (for D_ThreadName) */
	EffInst  *einst;	/* effector instance (for D_EffInst) */
	EffClass  *eclass;	/* effector class (for D_EffClass) */
	Channel  *chnl;		/* communication channel (for D_Channel) */
    } data;

#   define dic_obj  	data._macro.obj
#   define dic_arg_spec data._macro.arg_spec
#   define dic_handler	data._builtin.handler
#   define dic_thd	data.thd
#   define dic_einst	data.einst
#   define dic_eclass	data.eclass
#   define dic_chnl	data.chnl

} DicEnt;

#define DEF_ARG_SPEC	((char *)1)


/*
 * structure created for each macro call
 */
#define ARG_INITSIZ  6	   /* initial size of arugment object array */
#define ARG_GROWSIZ  16    /* growing unit size of arugment object array */

typedef struct macro_call {
    struct macro_call *next;	/* link to upper level macro */
    DicEnt  *macro;		/* macro itself */
    DicEnt  *lmacros;		/* list of local macros */
    long  src_pos;		/* calling position */
    int   nargs;		/* number of arguments */
    int   maxargs;		/* maximum possible number of arguments */
    int   ref;			/* reference counter: more than 1 if thread
				   is switched during macro expansion */
    Array   *arg_array;		/* The '$*' array */
    Object  arg[ARG_INITSIZ];	/* argement contents */
} MacroCall;

#define MAC_BASE_SIZE	(sizeof(MacroCall) - sizeof(Object [ARG_INITSIZ]))

/*
 * keyword
 */
struct keyword {
    char  *name;
    int	  val;
};

/*
 * token information table
 */
struct tok_table {
    short  flags;		/* flags */
    char   prec;		/* operator precedence, etc. */
    char   regno;		/* register number etc. */
    int   (*handler)();
};

#define F_B 	  0x0001	/* binary operator */
#define F_REG	  0x0002	/* is a register name ? */
#define F_PU	  0x0004	/* operators preceding a unary operator */
#define F_T 	  0x0008	/* ternary operator */
#define F_U	  0x0010	/* unary operator */
#define F_OPD	  0x0020	/* operand */
#define	F_HAVEEQ  0x0040	/* takes following '=' ? */
#define F_M	  0x0080	/* macros, macro directives, array operators */
#define F_AG	  0x0100	/* argument reference operator */
#define F_EV	  0x0200	/* eval or evalstr */
#define F_TK	  0x0400	/* strong token list */
#define F_EOL	  0x0800	/* `end of loop' tokens */
#define F_CA      0x1000	/* compound assignment operator */
#define F_C	  0x2000	/* command */
#define F_L	  0x4000	/* operators which take '{.. }' */

/*
 * song position (for '-f' or '-t' options)
 */
typedef struct song_pos {
    char  *marker;	/* marker string (No.6 text event) */
    int  bar;		/* bar (measure): if marker is NULL this means
			   the absolute bar count; otherwise this means
			   bar count relative to the marker. */
    int  tick;		/* ticks (quoter note = IRES ticks) */
} SongPos; 


/********************************************************************
 * Global variables
 ********************************************************************/

extern	int  japan;		/* Japanese error message */

extern	struct tok_table  tkinfo[];	/* token information table */

extern	Token   *cur_token;
extern  long    cur_srcpos;  	/* current input location (for err msg.) */

extern	Thread  *cur_thd;	/* current thread */
extern  Thread  *root_thd;	/* root (global) thread */

extern	int  err_maxelms, err_maxtklen, err_maxargs;	/* for err msg. */

/********************************************************************
 * System Library Functions
 ********************************************************************/

char	*malloc();
char	*calloc();
char	*realloc();
char	*strdup();
char	*strchr();
char	*getenv();

extern char *sys_errlist[];
extern int  errno;

/********************************************************************
 * User Function Prototypes and Macros
 ********************************************************************/
/* input.c */
void	init_input P((char *));
void	init_pbstack P((Thread *));
void	free_pbstack P((Thread *));
Token *	get_token P((void));
Token * probe_next_token P((void));
void	pushbk_file P((char *, int));
void	pushbk_object P((Object *));
void	pushbk_tklist P((Token *, int));
void	push_tklist_at_bottom P((Token *, int, Thread *, Token *));
void	pushbk_int P((PmmlInt));
void	pushbk_float P((PmmlFloat));
void	pushbk_string P((char *));
void	pushbk_array P((Array *));
void	push_and_get_array P((Array *));
void	pushbk_token P((Token *));
void	pushbk_symbol P((int));
void	pushbk_special_id P((int, DicEnt *));
void	pushbk_evalstr P((char *));
int	check_active_tklist P((Token *));

/* keyword.c */
struct keyword *  in_word_set P((char *, int));

/* macro.c */
void	end_of_macro P((void));
void	delete_mc P((MacroCall *));
void	fprint_calls P((FILE *));
void	fprint_args P((FILE *, MacroCall *, int));
#define  BREAK_LOOP     0x01
#define  EFF_ACTION     0x02

/* parseut.c */
MacroCall *scan_args P((char *, char *));
int     valid_arg_spec P((char *));
Token *	scan_tklist P((int, int, char *, int, int));
#define  SKIP     1
#define  COLLECT  0 
int	get_id P((int, int, DicEnt **));
#define  DEF_ON_UNDEF  1

/* token.c */
Token * copy_token P((Token *));
Token * new_token P((int, long));
Token * copy_tklist P((Token *));
int	token_equal P((Token *, Token *));
void	fprint_token P((FILE *, Token *, int, int));
void	free_tklist P((Token *));
void	fprint_tklist P((FILE *, Token *, int, int, int));
void    copy_object P((Object *, Object *));
void	free_object P((Object *));
int	istrue_object P((Object *));
void	_fprint_object P((FILE *, Object *, int, int, int));
#define fprint_object(fp, op, me, mt)     _fprint_object(fp, op, me, mt, 1) 
#define fprint_object_nq(fp, op, me, mt)  _fprint_object(fp, op, me, mt, 0) 
int	compare_object P((Object *, Object *));
int	conv_to_int P((Object *, PmmlInt *));
int	conv_to_rational P((Object *, Rational *));
int	conv_to_float P((Object *, PmmlFloat *));

/* expr.c */
void	init_expression P((void));
Object *_get_expression();
#define get_expression() _get_expression(0)
int	pitch_to_val P((Token *));

/* array.c */
Array * create_array P((int, Object *));
Array * dup_array P((Array *));
Array * create_reparray P((int, Object *));
Array * create_ngarray P((int, Object *));
void	ngarray_to_array P((Array *));
Array * create_array_from_uchar P((int, uchar *));
void    destroy_array P((Array *));
#define free_array(ap)  { if( --(ap)->ref == 0 ) destroy_array(ap); }
void    append_array P((Array *, Object *));
void    insert_array P((Array *, Object *));
void	shift_array P((Array *, int));
void	fprint_array P((FILE *, Array *, int, int));

/* error.c */
int	err_newfname P((char *));
void	fprint_fname P((FILE *, long));
#ifdef __ERROR_C__
void	error();
void	terror();
void	warn();
#else
void	error P((long, char *, char *, ...));
void	terror P((long, Token *, char *, char *, ...));
void	warn P((long, char *, char *, ...));
#endif
void	parse_error P((Token *));
void	mferror P((char *));
void	err_nomem P((char *));
#define  SRCPOS_CMDLN  (long)0xffffffff
#define  SRCPOS_EOF    (long)0xfffffffe

/* dict.c */
DicEnt *define_macro P((char *, int, Thread *, Object *, char *));
void	redefine_macro P((DicEnt *, Object *, char *));
void	insert_dict P((DicEnt *));
DicEnt *search_dict P((int, char *, int, Thread *, MacroCall *));
void	delete_dict P((DicEnt *));
void	unlink_dnext_link P((DicEnt *));
int	hash_func P((char *));
void	print_def P((DicEnt *, int));
void	dump_dict P((int, Thread *));
#define SD_ANY    0
#define SD_MINE	  1
#define SD_LOCAL  2
#define SD_THREAD 3

/* builtin.c */
void	init_builtin P((void));
void	pmml_sprintf P((String *, char *, char *, int, Object *));

/* parse.c */
void	init_parse P((void));
void	parse_initblk P((void));
void	parse_main P((void));
Token * parse_cmds P((int));
#define ALL_CMDS   (F_C|F_M|F_AG|F_EV|F_TK)
#define MACRO_CMDS (F_M|F_AG|F_EV|F_TK)
#define ARGS_ONLY  (F_AG|F_EV|F_TK)
void	end_parse P((void));
void	destroy_channel P((Channel *));
void	delete_effclass P((EffClass *));
void	print_effclass P((EffClass *));
void	print_effinst P((EffInst *));
void	delete_thread P((Thread *, int));
void	warn_time P((Rational *));

/* output.c */
void	init_output P((void));
void	insert_event P((Track *, Event *));
void	insert_evlist P((Track *, Event *));
Event * copy_event P((Event *));
void	sort_event P((Track *));
void	increase_ntrks P((MTracks *, int));
void	destroy_track P((Track *));
void	output_event P((Event *));
void	e_setinclusive P((EventSet, int, int));
void	post_process P((MTracks *, int, Rational *, Rational *, Rational *));
void	pair_notes P((MTracks *));

#define event_alloc(ep) { \
    if( !((ep) = (Event *) malloc(sizeof(Event))) ) \
	err_nomem("event_alloc"); \
}

/* macro for free'ing an event. */
#define event_free(ep)	{ \
    if( (ep)->type & CTYPE ) free((ep)->u.cp); \
    else if( (ep)->type < E_Meta )  free_object(&(ep)->u.obj); \
    else if( (ep)->type < E_NoteOff ) { \
        if( (ep)->ev_data )  free((ep)->ev_data); \
    } else if( (ep)->type <= E_NoteOn ) { \
	if( (ep)->ev_partner ) (ep)->ev_partner->ev_partner = NULL; \
    } \
    free(ep); \
}

/* smfout.c */
void	merge_all_tracks P((MTracks *));
int	in_range P((int, char *, int, int, Track *));
int     analy_songpos P((char *, SongPos *, int));

/* load.c */
void	load_midi_file P((char *, Rational *));
void	load_single_track P((char *, int, int, Rational *));
void	close_midi_file P((char *));
int	get_ntrk P((char *));
int	get_format P((char *));
int	get_resolution P((char *));
