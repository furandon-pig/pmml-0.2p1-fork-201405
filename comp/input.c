/*
 * PMML: A Compiler for the Practical Music Macro Language
 *
 * input.c: input stream tokenizer with 'push-back' capability
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

#include <stdio.h>
#include <fcntl.h>
#include <ctype.h>
#include <errno.h>
#include "pmml.h"

/*
 * size of the input buffer where PMML sources are loaded
 */
#define	PMML_BUFSIZ	1024

/*
 * allocation block size of PB stack elements
 */
#define PBS_BLKSIZ	32	

/*
 * global variables 
 */
static	PBStack *pb_free_list;	/* list of free PB stack elements 
				   Caution: get_token() may retrun a token 
				   which is already free'ed, i.e., a token in
				   this free list. */

static	PBStack *active_tklist;	/* list of token-lists which is currently 
				   being referenced by some push-back stack */

Token  *cur_token;		/* the last token returned by get_token */ 
long   cur_srcpos;		/* current input location (for err msg.) */

static char *pmml_path;		/* search path for source file */

/*
 * macros
 */
#define	GetC(p)   (--(p)->cnt >= 0 ? *(p)->ptr++ : fill_ibuffer(p)) 
#define UnGetC(p) ((p)->cnt++, (p)->ptr--)

#define IsKanji(c)	((c) >= 0x80)
#ifdef DOS
#  define SJIS   1
#endif
#ifdef SJIS
#  define TwoByteChar(c)  ((c) >= 0x80 && (c) <= 0x9f || (c) >= 0xe0)
#else
#  define TwoByteChar(c)  0
#endif

#undef ID_PERCENT
#define ID_DOLLAR
#undef ID_DOT
#undef S_TO_D_COLON

#ifdef ID_DOT
#   define ISDOT(c)  ((c) == '.')
#else
#   define ISDOT(c)  (0)
#endif

#define BEL	'\007'

#define pb_alloc(newpb) \
   (pb_free_list ? \
    ((newpb) = pb_free_list, pb_free_list = pb_free_list->s.next) : \
    ((newpb) = _pb_alloc()) )

#define pb_free(p) { \
   (p)->s.next = pb_free_list; \
   pb_free_list = (p); }

#define pb_push(p) { \
   (p)->s.next = cur_thd->pb->top; \
   cur_thd->pb->top = (p); }

#define pb_pop()  { \
   PBStack *p = cur_thd->pb->top; \
   cur_thd->pb->top = p->s.next; \
   pb_free(p); }

/*
 * function prototypes
 */
static int	fill_ibuffer P((struct ibuffer *));
static void	remove_from_active_tklist P((PBStack *));
static PBStack *_pb_alloc P((void));
static Token *	read_token P((struct ibuffer *));


/*********************************************************************
 * Initilization
 *********************************************************************/
void
init_input(prep_path)
char  *prep_path;
{
    char  *p;

    if( !(p = getenv("PMMLPATH")) ) {
	p = PMML_PATH;
    }
    pmml_path = malloc(strlen(prep_path) + strlen(p) + 1);
    strcpy(pmml_path, prep_path);
    strcat(pmml_path, p);
}

/*
 * initialize PB stack and macro calling information
 *   At the bottom of stack, EOF token (used as a sentinel) is placed.
 *   After calling this routine, the contents of cur_token will be destroyed.
 */
void
init_pbstack(hp)
Thread  *hp;
{
    PBStack  *p;

    if( !(hp->pb = (PBinfo *) malloc(sizeof(PBinfo))) ) {
	err_nomem("init_pbstack");
    }
    hp->pb->calls = NULL;
    hp->pb->loop_count = 0;

    pb_alloc(p);
    p->type = T_EOF;
    p->t.src_pos = SRCPOS_EOF;
    p->s.next = NULL;
    hp->pb->top = hp->pb->bottom = p; 
}

/*
 * delete PB stack (assumes the push-back has only the EOF token)
 */
void
free_pbstack(hp)
Thread  *hp;
{
    pb_free(hp->pb->top);
    free(hp->pb);
}

/*********************************************************************
 * Routines for getting tokens from the input stream
 *********************************************************************/
/*
 * get a raw (non macro-expanded) token from the input stream
 *
 *   The return value (= cur_token) points to somewhere in the push-back 
 *   stack or somewhere in a token list in the name dictionary.  
 *   It is "read-only", and should not be modified.  
 *   Moreover, the returned structure must not be free'ed.
 *   These rules are also applied to the string contents (token->u.id.name 
 *   or token->u.str), i.e., these strings must NOT be free'ed on 
 *   the caller's side.
 *   However, when get_token returns the ARRAY pseudo token, its
 *   reference counter is already incremented; therefore, free_array must be 
 *   called for the returned array when the array becomes no longer needed.
 *
 *   The `next' member of the returned token is undefined.
 *
 *   get_token never returns a token whose type is T_WTOKENS or T_ISTREAM. 
 */
Token *
get_token()
{
    register PBStack  *top;
    register Token  *tp;

    for(;;) {
	top = cur_thd->pb->top;
	switch( top->type ) {
	case T_ISTREAM:
	    if( !(tp = read_token(top->s.u.ip)) ) { /* EOF? */
		/* destory ibuffer */
		free(top->s.u.ip->base);
		freestr(top->s.u.ip->strbuf);
		if( top->s.u.ip->fd != -1 )  close(top->s.u.ip->fd);
		free(top->s.u.ip);
		pb_pop();
		continue;
	    }
	    cur_srcpos = tp->src_pos;
	    return (cur_token = tp);

	case T_WTOKENS:
	    if( !(tp = top->s.u.tp) ) {
		if( top->s.flags & TF_CALLEOM ) {
		    end_of_macro();
		}
		remove_from_active_tklist(top);
		if( top->s.flags & TF_FREE ) {
		    free_tklist(top->s.tklist);
		}
		pb_pop();
		continue;
	    }

	    if( top->s.flags & TF_RPLPOS ) {
		/* change src_pos in the original token to current position */
		tp->src_pos = top->t.src_pos;
	    }

	    if( tp->type == T_ARRAY ) {
		tp->u.obj.o_ap->ref++;
	    }

	    top->s.u.tp = tp->next;	/* advance to the next token */

	    cur_srcpos = tp->src_pos;
	    return (cur_token = tp);

	case T_EOF:
	    tp = &top->t;
	    cur_srcpos = tp->src_pos;
	    return (cur_token = tp);
    
	default: /* T_NUMBER, T_RATIONAL, T_STRING, T_ARRAY, T_STOKENS */
	    tp = &top->t;
	    pb_pop();

	    cur_srcpos = tp->src_pos;
	    return (cur_token = tp);
	}

	/* not reached */
    }
}

/*
 * Probe the next token in the input stream
 *   This is useful for look-aheading in the parser.
 *
 *   !!! After calling this routine, the contents of cur_token are destroyed.
 *
 *   The src_pos element of returned structure may not be correct.
 */
Token *
probe_next_token()
{
    register PBStack  *top;
    register Token  *tp;

    for(top = cur_thd->pb->top;; top = top->s.next ) {
	switch( top->type ) {
	case T_ISTREAM:
	    if( !(tp = read_token(top->s.u.ip)) ) { /* EOF? */
		continue;
	    }
	    top->s.u.ip->look_ahead = 1;
	    return tp;

	case T_WTOKENS:
	    if( !(tp = top->s.u.tp) ) {
		continue;
	    }
	    return tp;

	default: /* T_NUMBER, T_RATIONAL, T_STRING, T_ARRAY, T_STOKENS
		    or T_EOF */
	    return(&top->t);
	}
	/* not reached */
    }
}

/*********************************************************************
 * Routines for push-backing
 *
 * Push-back routines will dispose the token currently bound to 'cur_token'.
 * After calling these routines, the 'cur_token' will be undefined.
 *********************************************************************/
/*
 * push back a file to the input stream
 *   This is for the initialization and the processing of `include'.
 */
void
pushbk_file(fname, do_search_path)
char  *fname;
int  do_search_path;	/* if true, PMMLPATH is consulted */
{
    int	 fd, exist;
    char  *p, *path, *newfname;
    struct ibuffer  *ip;
    PBStack  *pbs;

    /* check if the file name is "-" */
    if( strcmp(fname, "-") == 0 ) {
	fd = 0;
	newfname = "stdin";
    } else {
	exist = -1;
	if( !do_search_path || *fname == '/' || 
	   *fname == '\\' || fname[1] == ':' /* MS-DOS drive name */ ) {
	    /* absolute path */

	    /* If the file name has no extension, append default extension */
	    if( !(newfname = attach_extension(fname, SRC_EXT)) ) {
		err_nomem("pushbk_file");
	    }
	    if( (exist = access(newfname, 0)) == -1 ) {
		/* If file does not exist, try the original file name */
		free(newfname);
		if( !(newfname = strdup(fname)) ) {
		    err_nomem("pushbk_file");
		}
		exist = access(newfname, 0);
	    }
	} else {
	    path = pmml_path;
	    while( (p = prepend_path(&path, fname)) != NULL ) {
		if( !(newfname = attach_extension(p, SRC_EXT)) ) {
		    err_nomem("pushbk_file");
		}
		if( (exist = access(newfname, 0)) == 0 ) {
		    free(p);
		    break;
		} else {
		    free(newfname);
		    newfname = p;
		    if( (exist = access(newfname, 0)) == 0 ) {
			break;
		    }
		    free(p);
		}
	    }
	}
	if( exist == -1 ) {
	    error(cur_srcpos, "%s: No such file", 
		  "%s: ファイルが見つかりません",
		  attach_extension(fname, SRC_EXT));
	}
	if( (fd = open(newfname, O_RDONLY)) == -1 ) {
	    error(cur_srcpos, "%s: Cannot open (%s)", 
		  "%s: オープンできません (%s)", newfname, sys_errlist[errno]);
	}
    }

    /* create ibuffer */
    ip = (struct ibuffer *) malloc(sizeof(struct ibuffer));
    if( !ip )  err_nomem("pushbk_file");
    ip->cnt = 0;
    ip->base = (uchar *) malloc(PMML_BUFSIZ + 1);
    if( !ip->base )  err_nomem("pushbk_file");
    ip->ptr = ip->base;
    ip->fd = fd;
    ip->src_pos = ((long) err_newfname(newfname) << 24) | 1;
    ip->eof = 0;
    ip->look_ahead = 0;
    initstr(ip->strbuf);

    /* push an ISTREAM element to the push-back stack */
    pb_alloc(pbs);
    pbs->type = T_ISTREAM;
    pbs->s.u.ip = ip;
    pb_push(pbs);
}

/*
 * push back an object to the input stream
 */
void
pushbk_object(obj)
Object  *obj;
{
    register PBStack  *p;

    pb_alloc(p);
    p->t.src_pos = cur_srcpos;
    switch( obj->o_type ) {
    case O_WTOKENS:
	p->type = T_WTOKENS;
	p->s.u.tp = p->s.tklist = obj->o_tp;
	p->s.flags = TF_RPLPOS;
	p->s.atl_next = active_tklist;
	active_tklist = p;
	break;
    case O_INT:
    case O_RATIONAL:
    case O_STRING:
    case O_STOKENS:
	p->type = obj->o_type - NaN_Base;
	p->t.u.obj = *obj;	/* structure copy */
	break;
    case O_ARRAY:
	p->type = T_ARRAY;
	p->t.u.obj = *obj;	/* structure copy */
	obj->o_ap->ref++;
	break;
    default:  /* float */
	p->type = T_NUMBER;
	p->t.u.obj = *obj;	/* structure copy */
	break;
    }
    pb_push(p);
}

/*
 * push back a token list to the input stream
 *   This is also for the processing of macros.
 */
void
pushbk_tklist(tp, flags)
Token  *tp;
int  flags;		/* TF_RPLPOS, TF_CALLEOM, and/or TF_FREE */
{
    register PBStack  *p;

    pb_alloc(p);
    p->type = T_WTOKENS;
    p->s.u.tp = tp;
    p->s.tklist = tp;
    p->t.src_pos = cur_srcpos;
    p->s.flags = flags;

    if( !(flags & TF_FREE) ) {
	p->s.atl_next = active_tklist;
	active_tklist = p;
    }
    pb_push(p);
}

/*
 * insert a token list as well as BOT and EOT tokens 
 * at the bottom of PB stack of a particular thread
 */
void
push_tklist_at_bottom(tp, flags, thd, bot_tok)
Token  *tp;
int  flags;		/* TF_RPLPOS, TF_CALLEOM, and/or TF_FREE */
Thread  *thd;
Token  *bot_tok;
{
    register PBStack  *eot, *bot, *tkl, *eof;

    /* replace EOF element at the bottom to BOT */
    bot = thd->pb->bottom;
    bot->t = *bot_tok;	  /* structure copy */

    /* tkl is a PB-stack element for the token list */
    pb_alloc(tkl);
    tkl->type = T_WTOKENS;
    tkl->s.u.tp = tp;
    tkl->s.tklist = tp;
    tkl->t.src_pos = cur_srcpos;
    tkl->s.flags = flags;

    if( !(flags & TF_FREE) ) {
	tkl->s.atl_next = active_tklist;
	active_tklist = tkl;
    }

    /* make EOT element */
    pb_alloc(eot);
    eot->type = T_EOT;
    eot->t.src_pos = cur_srcpos;
    
    /* remake EOF element */
    pb_alloc(eof);
    eof->type = T_EOF;
    eof->t.src_pos = SRCPOS_EOF;

    /* now, connect all the PB elements */
    bot->s.next = tkl;
    tkl->s.next = eot;
    eot->s.next = eof;
    eof->s.next = NULL;
    thd->pb->bottom = eof;
}

/*
 * push back an integer to the input stream
 */
void
pushbk_int(val)
PmmlInt  val;
{
    register PBStack  *p;

    pb_alloc(p);
    p->type = T_NUMBER;
    p->t.u.obj.o_type = O_INT;
    p->t.u.obj.o_val = val;
    p->t.src_pos = cur_srcpos;
    pb_push(p);
}

/*
 * push back a floating-point number to the input stream
 */
void
pushbk_float(val)
PmmlFloat  val;
{
    register PBStack  *p;

    pb_alloc(p);
    p->type = T_NUMBER;
    p->t.u.obj.fpval = val;
    p->t.src_pos = cur_srcpos;
    pb_push(p);
}

/*
 * push back a string to the input stream
 */
void
pushbk_string(str)
char  *str;
{
    register PBStack  *p;

    pb_alloc(p);
    p->type = T_STRING;
    p->t.u.obj.o_type = O_STRING;
    p->t.u.obj.o_str = str;
    p->t.src_pos = cur_srcpos;
    pb_push(p);
}

/*
 * push back a compiled array to the input stream
 */
void
pushbk_array(ap)
Array  *ap;
{
    register PBStack  *p;

    pb_alloc(p);
    p->type = T_ARRAY;
    p->t.u.obj.o_type = O_ARRAY;
    p->t.u.obj.o_ap = ap;
    p->t.src_pos = cur_srcpos;
    pb_push(p);
}

/*
 * combination of pushbk_array and get_token
 */
void
push_and_get_array(ap)
Array  *ap;
{
    register PBStack  *p;

    pb_alloc(p);
    p->type = T_ARRAY;
    p->t.u.obj.o_type = O_ARRAY;
    p->t.u.obj.o_ap = ap;
    p->t.src_pos = cur_srcpos;
    cur_token = &p->t;
    pb_free(p);
}

/*
 * push back a token (only for `end of loop' tokens) to the input stream 
 *   If the argument is NULL, the EOL token read by get_token just 
 *   before is pushed back.
 *   Do not use this routine for tokens other than 'end of loop' tokens;
 *   if it should be used, data associated with the token are never free'ed.
 */
void
pushbk_token(tp)
Token  *tp;
{
    register PBStack  *p;

    pb_alloc(p);	/* This must be the most recently freed one. */
    if( tp ) {
	p->t = *tp;	  /* structure copy */
    }
    pb_push(p);
}

/*
 * push a symbol
 */
void
pushbk_symbol(type)
int  type;
{
    register PBStack  *p;

    pb_alloc(p);
    p->type = type;
    p->t.src_pos = cur_srcpos;
    pb_push(p);
}

/*
 * push back T_THREAD or T_EFFCLASS
 */
void
pushbk_special_id(type, dp)
int  type;
DicEnt  *dp;
{
    register PBStack  *p;

    pb_alloc(p);
    p->type = type;
    p->t.u.dp = dp;
    p->t.src_pos = cur_srcpos;
    pb_push(p);
}

/*
 * push back a string to be re-evaluated to the input stream
 *  This is for the processing of `evalstr'.
 *  The string in the argument is free'ed IN FUTURE by the get_token.
 *  Therefore, the caller must duplicate the string before calling.
 */
void
pushbk_evalstr(str)
char  *str;
{
    struct ibuffer  *ip;
    PBStack  *p;

    /* create ibuffer */
    ip = (struct ibuffer *) malloc(sizeof(struct ibuffer));
    if( !ip )  err_nomem("pushbk_evalstr");
    ip->cnt = strlen(str);
    ip->base = (uchar *) str;
    ip->ptr = ip->base;
    ip->fd = -1;
    ip->src_pos = cur_srcpos;
    ip->eof = 1;
    ip->look_ahead = 0;
    initstr(ip->strbuf);

    /* push an ISTREAM element to the push-back stack */
    pb_alloc(p);
    p->type = T_ISTREAM;
    p->s.u.ip = ip;
    pb_push(p);
}

/*********************************************************************
 * Other supporting routines
 *********************************************************************/
/*
 * check if a token list is referenced from PB stack or not
 *   If the token list is referenced, set the TF_FREE flag.
 *   This routine is used for avoiding core dump 
 *   when a macro being expanded deletes itself.
 */
int
check_active_tklist(tp)
Token  *tp;
{
    PBStack  *p;

    for( p = active_tklist; p; p = p->s.atl_next ) { 
	if( p->s.tklist == tp ) {
	    p->s.flags |= TF_FREE;
	    return 1;
	}
    }
    return 0;
}

/*
 * remove an element from active token list
 */
static void
remove_from_active_tklist(pbs)
PBStack  *pbs;
{
    PBStack  **pp = &active_tklist;

    for( pp = &active_tklist; *pp; pp = &(*pp)->s.atl_next ) { 
	if( *pp == pbs ) {
	    *pp = pbs->s.atl_next;
	    break;
	}
    }
}

/*
 * fill the input buffer and return the first character
 */
static int
fill_ibuffer(ip)
struct ibuffer  *ip;
{
    int	 n;
   
    if( ip->eof )  return EOF;

    /* to enable 2-character push back, save the last character */
    if( ip->ptr > ip->base )  ip->base[0] = ip->ptr[-1];

    n = read(ip->fd, ip->base + 1, PMML_BUFSIZ);
    if( n == -1 ) {	/* maybe I/O error */
	error(ip->src_pos, "Source file read error (%s)", 
	      "ソースファイル読み取りエラー (%s)", sys_errlist[errno]);
    } else if( !n ) {	/* EOF reached */
	ip->eof = 1;
	return EOF;
    }
    ip->cnt = n;
    ip->ptr = ip->base + 1;
    return GetC(ip);	/* return the first charactor */
}

/*
 * allocate memory for the next block of PB stack elemetns
 */
static PBStack *
_pb_alloc()
{
    static int  already_warnned = 0;
    static int  pb_nallocated = 0;
    char  *blk;
    PBStack  *p;
    int  i;
    
    if( !(blk = malloc(sizeof(PBStack) * PBS_BLKSIZ)) ) {
	err_nomem("pb_alloc");
    }
    for( i = 0; i < PBS_BLKSIZ; i++ ) {
	p = (PBStack *) (blk + i * sizeof(PBStack));
	p->s.next = pb_free_list;
	pb_free_list = p;
    }

    pb_nallocated += PBS_BLKSIZ;
    if( !already_warnned && pb_nallocated > PBS_WARN ) {
	warn(cur_srcpos, "Macro expansion level exceeds %d",
	     "マクロ展開レベルが %d を越えました", PBS_WARN);
	already_warnned = 1;
    }

    p = pb_free_list;
    pb_free_list = pb_free_list->s.next;
    return p;
}

/*********************************************************************
 * Lexical analyzer
 *********************************************************************/

#define	AFTER_OCT	1
#define AFTER_INT	2
#define AFTER_FLOAT	3

/*
 * get a token from the input stream (a DFA-based lexical analyzer)
 *  The results are stored in *tp.
 *  Returns NULL if the stream reaches the end.
 */
static Token *
read_token(ip)
struct ibuffer  *ip;
{
    register int  c;
    register int  tmp;
    Token  *tp;
    int  state = 0;
    PmmlInt  val;
    PmmlFloat  fpval;
    double  atof();
    int	 flags, octave;
    int  dotcnt;
    struct keyword  *kw;
    long src_pos1;
    static int  pitch_table[7] = { 9, 11, 0, 2, 4, 5, 7 };

    if( ip->look_ahead ) {
	ip->look_ahead = 0;
	return &ip->tkbuf;
    }

    tp = &ip->tkbuf;
    resetstr(ip->strbuf);
    c = GetC(ip);

 again:
    switch(c) {
    case 'r': case 'R': 
	flags = RESTFLAG;
	goto note_rest;

    case 'a': case 'b': case 'c': case 'd': case 'e': case 'f': case 'g':
	tp->u.pitch.note_num = pitch_table[c - 'a'];
	flags = 0;
	goto note_rest;

    case 'A': case 'B': case 'C': case 'D': case 'E': case 'F': case 'G':
	tp->u.pitch.note_num = pitch_table[c - 'A'];
	flags = 0;

      note_rest:
	if( state != AFTER_OCT ) {
	    octave = 0;
	}
	if( state >= AFTER_INT ) {
	    error(ip->src_pos, "'%c' is not a valid length name", 
		  "'%c' は長さ記号ではありません", c);
	}
	addchar(ip->strbuf, c);
	while( (c = GetC(ip)) == 'b' ) {
	    addchar(ip->strbuf, c);
	    if( tp->u.pitch.note_num == 0 ) {
		tp->u.pitch.note_num = 11, octave--;
	    } else {
		tp->u.pitch.note_num--;
	    }
	    flags |= ACCIDENTAL;
	}
	for( tmp = 0; isdigit(c); c = GetC(ip) ) {
	    tmp = tmp * 10 + c - '0';
	    addchar(ip->strbuf, c);
	    flags |= ABS_OCT;
	}
	if( isalpha(c) || c == '_' || IsKanji(c) || ISDOT(c) ) {
	    if( state == AFTER_OCT ) {
		addchar(ip->strbuf, c);  addchar(ip->strbuf, 0);
		error(ip->src_pos, "Bad pitch specifier '%s'", 
		      "音高指定が間違っています '%s'", ip->strbuf.buf);
	    }
	    goto identifier;
	}
	/* Now, there is no possiblity that the token is an identifier. */
	if( flags & ABS_OCT ) {
	    octave += tmp;
	} else {
	    for(;;) {
		if( c == '#' ) {
		    if( tp->u.pitch.note_num == 11 ) {
			tp->u.pitch.note_num = 0, octave++;
		    } else {
			tp->u.pitch.note_num++;
		    }
		} else if( c == 'b' ) {
		    if( tp->u.pitch.note_num == 0 ) {
			tp->u.pitch.note_num = 11, octave--;
		    } else {
			tp->u.pitch.note_num--;
		    }
		} else if( c != '%' ) {
		    break;
		} 
		flags |= ACCIDENTAL;
		c = GetC(ip);
	    }
	    if( c == '-' ) {	/* minus octave */
		if( c = GetC(ip), isdigit(c) ) {
		    flags |= ABS_OCT;
		    for( tmp = 0; isdigit(c); c = GetC(ip) ) {
			tmp = tmp * 10 + (c - '0');
		    }
		    octave -= tmp;
		} else {
		    UnGetC(ip);
		}
	    } else {
		for( tmp = 0; isdigit(c); c = GetC(ip) ) {
		    tmp = tmp * 10 + (c - '0');
		    flags |= ABS_OCT;
		}
		octave += tmp;
	    }
	}
	tp->type = T_PITCH;
	tp->u.pitch.flags = flags;
	tp->u.pitch.octave = octave;
	UnGetC(ip);
	break;

    case 'w': case 'W':  tmp = LDEN;	goto rational_const;
    case 'h': case 'H':  tmp = LDEN/2;  goto rational_const;
    case 'i': case 'I':  tmp = LDEN/8;  goto rational_const;
    case 's': case 'S':  tmp = LDEN/16; goto rational_const;
    case 'z': case 'Z':  tmp = LDEN/32; goto rational_const;
    case 'u': case 'U':  tmp = LDEN/(IRES * 4);  goto rational_const;
    case 'q': case 'Q':  tmp = LDEN/4;  
      rational_const:
	if( state == AFTER_OCT ) {
	    error(ip->src_pos, "Identifer can not be prefixed by '_' or '^'",
		  "名前を '_' または '^' で始めることはできません");
	}
	addchar(ip->strbuf, c);

	for( dotcnt = 0;; ) { 
	    if( (c = GetC(ip)) == '.' ) {
		addchar(ip->strbuf, c);
		dotcnt++;
	    } else if( 
#ifndef ID_DOT
		      dotcnt == 0 &&
#endif
		      (isalnum(c) || c == '_' || IsKanji(c)) ) {
		if( state >= AFTER_INT ) {
		    addchar(ip->strbuf, c);  addchar(ip->strbuf, 0);
		    error(ip->src_pos, "Bad rational constant '%s'", 
			  "有理数定数が間違っています '%s'", ip->strbuf.buf);
		} 
		goto identifier;
	    } else break;
	}

	tp->type = T_RATIONAL;
	tp->u.obj.o_type = O_RATIONAL;
	tp->u.obj.r.intg = 0;
	tp->u.obj.r.num = (tmp << 1) - (tmp >> dotcnt);
	tp->u.obj.r.den = LDEN;
	while( tp->u.obj.r.num >= tp->u.obj.r.den ) {
	    tp->u.obj.r.num -= tp->u.obj.r.den;
	    tp->u.obj.r.intg++;
	}
	if( state == AFTER_INT ) {
	    rimult(&tp->u.obj.r, val, &tp->u.obj.r); 
	} else if( state == AFTER_FLOAT ) {
	    Rational  rtmp;
	    float_to_rational(fpval * (IRES * 4), &rtmp);
	    rmult(&tp->u.obj.r, &rtmp, &tp->u.obj.r); 
	}

	UnGetC(ip);
	break;

    case 'j': case 'k': case 'l': case 'm': case 'n': case 'o':
    case 'p': case 't': case 'v': case 'x': case 'y':
    case 'J': case 'K': case 'L': case 'M': case 'N': case 'O':
    case 'P': case 'T': case 'V': case 'X': case 'Y':
	if( state >= AFTER_INT ) {
	    error(ip->src_pos, "'%c' is not a valid length name", 
		  "'%c' は長さ記号ではありません", c);
	} else if( state == AFTER_OCT ) {
	    error(ip->src_pos, "Identifer can not be prefixed by '_' or '^'",
		  "名前を '_' または '^' で始めることはできません");
	} 
	goto identifier;

    case '.':
	c = GetC(ip);
	if( isdigit(c) ) {
	    UnGetC(ip);
	    c = '.';
	    goto number_frac;
	} else if( c == '.' ) {
	    if( (c = GetC(ip)) == '.' )  tp->type = T_TDOT;
	    else { UnGetC(ip); tp->type = T_DDOT; }
	} else {		
	    UnGetC(ip);
#ifdef ID_DOT
	    c = '.';
	    goto identifier;
#else
	    tp->type = '.';
#endif
	}
	break;

    case '0': 
	addchar(ip->strbuf, c);
	c = GetC(ip);
	if( isdigit(c) ) {
	    goto number_int;
	}
	val = 0;
	if( c == '.' )  goto number_frac;
	else if( c == 'x' || c == 'X' ) {
	    while( c = GetC(ip), isxdigit(c) ) {
		c -= isdigit(c) ? '0' : islower(c) ? 'a'-10 : 'A'-10;
		val = (val << 4) + c;
	    }
	} else if( c == 'e' || c == 'E' )  goto number_exponent;
	goto number_int_end;
	
    case '1': case '2': case '3': case '4': case '5': case '6':
    case '7': case '8': case '9':
      number_int:
	addchar(ip->strbuf, c);
	val = c - '0';
	while( c = GetC(ip), isdigit(c) ) {
	    addchar(ip->strbuf, c);
	    val = val * 10 + c - '0';
	}
	if( c == '.' )  goto number_frac;
	else if( c == 'e' || c == 'E' )  goto number_exponent;

      number_int_end:
	if( isalpha(c) ) {	/* Is it a rational constant? */
	    state = AFTER_INT;
	    goto again;
	}
	tp->type = T_NUMBER;
	tp->u.obj.o_type = O_INT;
	tp->u.obj.o_val = val;
	UnGetC(ip);
	break;

      number_frac:
	addchar(ip->strbuf, c);
	while( c = GetC(ip), isdigit(c) ) {
	    addchar(ip->strbuf, c);
	}
	if( c != 'e' && c != 'E' )  goto number_float_end;

      number_exponent:
	addchar(ip->strbuf, c);
	c = GetC(ip);
	if( c == '+' || c == '-' ) {
	    addchar(ip->strbuf, c);
	    c = GetC(ip);
	}
	while( isdigit(c) ) {
	    addchar(ip->strbuf, c);
	    c = GetC(ip);
	}

      number_float_end:
	addchar(ip->strbuf, 0);
	fpval = atof(ip->strbuf.buf);
	if( isalpha(c) ) {	/* Is it a rational constatnt? */
	    state = AFTER_FLOAT;
	    goto again;
	}
	tp->type = T_NUMBER;
	tp->u.obj.fpval = fpval;
	UnGetC(ip);
	break;

#ifdef ID_PERCENT
    case '%':
	if( (c = GetC(ip)) == '{' )  tp->type = T_INITBLK;
	else  UnGetC(ip);
	while( (c = GetC(ip)) == '%' ) {
	    addchar(ip->strbuf, c);
	}
	UnGetC(ip);
	c = '%';
	goto identifier;
#else
    case '%':
	if( (c = GetC(ip)) == '{' )  tp->type = T_INITBLK;
	else if( c == '=' )  tp->type = T_REMEQ;
	else { UnGetC(ip); tp->type = '%'; }
	break;
#endif

    case '_':
    case '^':
	octave = 0;
	do {
	    c == '_' ? --octave : ++octave;
	    addchar(ip->strbuf, c);
	    c = GetC(ip);
	} while( c == '_' || c == '^' );
	tp->type = T_OCTAVE;
	tp->u.oct = octave;
	if( isalpha(c) ) {	/* Is it a pitch constant? */
	    state = AFTER_OCT;
	    goto again;
	}
	UnGetC(ip);
	break;

    case '+':
	if( (c = GetC(ip)) == '=' ) tp->type = T_PLUSEQ;
	else if( c == '+' || c == '.' ) {
	    for( tmp = 0; c == '+'; tmp += 2 )  c = GetC(ip);
	    if( c == '.' )  tmp++;
	    else UnGetC(ip);
	    tp->type = T_ACCENT;
	    rset(tmp, 2, &tp->u.obj.r);
	} else {
	    UnGetC(ip);
	    tp->type = '+';
	}
	break;

    case '-':
	if( (c = GetC(ip)) == '=' ) tp->type = T_MINUSEQ;
	else if( c == '-' || c == '.' ) {
	    for( tmp = 0; c == '-'; tmp += 2 )  c = GetC(ip);
	    if( c == '.' )  tmp++;
	    else UnGetC(ip);
	    tp->type = T_ACCENT;
	    rset(-tmp, 2, &tp->u.obj.r);
	} else {
	    UnGetC(ip);
	    tp->type = '-';
	}
	break;

    case '>':
	if( (c = GetC(ip)) == '=' ) tp->type = T_GTEQ;
	else if( c == '>' || c == '.' ) {
	    for( tmp = 0; c == '>'; tmp += 2 )  c = GetC(ip);
	    if( c == '.' )  tmp++;
	    else UnGetC(ip);
	    tp->type = T_TSHIFT;
	    rset(tmp, 2, &tp->u.obj.r);
	} else {
	    UnGetC(ip);
	    tp->type = '>';
	}
	break;

    case '<':
	if( (c = GetC(ip)) == '=' ) tp->type = T_LTEQ;
	else if( c == '<' || c == '.' ) {
	    for( tmp = 0; c == '<'; tmp += 2 )  c = GetC(ip);
	    if( c == '.' )  tmp++;
	    else UnGetC(ip);
	    tp->type = T_TSHIFT;
	    rset(-tmp, 2, &tp->u.obj.r);
	} else {
	    UnGetC(ip);
	    tp->type = '<';
	}
	break;

    case '"':
	while( (c = GetC(ip)) != '"' ) {
	    if( c == '\n' || c == EOF ) {
		error(ip->src_pos, "Newline in string constant",
		      "文字列定数の中に改行が含まれています");
	    } else if( c == '\\' ) {
		c = GetC(ip); 
		if( c >= '0' && c <= '7' ) {
		    tmp = c - '0';
		    if( (c = GetC(ip)) >= '0' && c <= '7' ) { 
			tmp = (tmp << 3) + c - '0';
			if( (c = GetC(ip)) >= '0' && c <= '7' ) { 
			    tmp = (tmp << 3) + c - '0';
			} else UnGetC(ip);
		    } else UnGetC(ip);
		    c = tmp;
		} else switch(c) {
		case 'n':  c = '\n';  break;
		case 't':  c = '\t';  break;
		case 'v':  c = '\v';  break;
		case 'b':  c = '\b';  break;
		case 'r':  c = '\r';  break;
		case 'f':  c = '\f';  break;
		case 'a':  c = BEL;   break;
		case 'x':
		    c = GetC(ip);
		    if( !isxdigit(c) ) {
			UnGetC(ip);
			continue;
		    }
		    tmp = c - (isdigit(c) ? '0' : 
			       islower(c) ? 'a'-10 : 'A'-10);
		    if( c = GetC(ip), isxdigit(c) ) {
			tmp = (tmp << 4) + c - (isdigit(c) ? '0' : 
						islower(c) ? 'a'-10 : 'A'-10);
		    } else UnGetC(ip);
		    c = tmp;
		    break;
		case '\n':
		    if( !ip->eof )  ip->src_pos++;
		    break;
		default: 
		    break;
		}
	    } else if( TwoByteChar(c) ) {
		addchar(ip->strbuf, c);
		c = GetC(ip);
	    }
	    addchar(ip->strbuf, c);
	}
	addchar(ip->strbuf, 0);
	tp->type = T_STRING;
	tp->u.obj.o_type = O_STRING;
	tp->u.obj.o_str = ip->strbuf.buf;
	break;

    case '&':
	if( (c = GetC(ip)) == '=' )  tp->type = T_ANDEQ;
	else if( c == '&' )  tp->type = T_LOGAND;
	else { UnGetC(ip); tp->type = '&'; }
	break;

    case '|':
	if( (c = GetC(ip)) == '=' )  tp->type = T_OREQ;
	else if( c == '|' )  tp->type = T_LOGOR;
	else { UnGetC(ip); tp->type = '|'; }
	break;

    case '=':
	if( (c = GetC(ip)) == '=' ) {
	    if( (c = GetC(ip)) == '=' )  tp->type = T_TRIEQ;
	    else { UnGetC(ip); tp->type = T_EQ; }
	} else { UnGetC(ip); tp->type = '='; }
	break;

    case '!':
	if( (c = GetC(ip)) == '=' )  tp->type = T_NEQ;
	else { UnGetC(ip); tp->type = '!'; }
	break;

    case '*':
	if( (c = GetC(ip)) == '=' )  tp->type = T_MULTEQ;
	else if( c == '/' ) {
	    error(ip->src_pos, "'*/': Corresponding '/*' is not found",
		  " '*/' に対応する '/*' がありません");
	} else { UnGetC(ip); tp->type = '*'; }
	break;

    case '/':
	switch( c = GetC(ip) ) {
	case '/':	/* C++ comment */
	    while( (c = GetC(ip)) != '\n' && c != EOF );
	    goto again;
	case '*':	/* C comment (but nestable) */
	    tmp = 0; 	/* nest level */
	    src_pos1 = ip->src_pos;
	    do {
		if( (c = GetC(ip)) == '/' ) {
		    if( (c = GetC(ip)) == '*' )  tmp++;
		} else if( c == '*' ) {
		    do {
			if( (c = GetC(ip)) == '/' && tmp-- == 0 ) {
			    c = GetC(ip);
			    goto again;
			}
		    } while( c == '*' );
		}
		if( c == '\n' ) {
		    if( !ip->eof )  ip->src_pos++;
		}
	    } while( c != EOF );
	    error(src_pos1, "'/*': Closing '*/' is not found",
		  " '/*' に対応する '*/' がありません");
	    goto again;
	case '=':
	    tp->type = T_DIVEQ;
	    break;
	default:
	    UnGetC(ip);
	    tp->type = '/';
	}
	break;

    case ':':
#ifdef S_TO_D_COLON
	if( (c = GetC(ip)) == ':' )  tp->type = T_DCOLON;
	else { UnGetC(ip); tp->type = T_DCOLON; }
#else
	if( (c = GetC(ip)) == ':' )  tp->type = T_DCOLON;
	else { UnGetC(ip); tp->type = ':'; }
#endif
	break;

#if 0
    case '[':
	/* 
	 * If the leading character is a non-space character,
	 * '[' is treated as the array subscript operator; 
	 * otherwise, it is viewed as the beggining of a parallel block.
	 */
	if( ip->ptr != ip->base + 1 && !isspace(ip->ptr[-2]) ) {
	    tp->type = T_ASUB;
	} else {
	    tp->type = '[';
	}
	break;
#endif

#ifdef ID_DOLLAR
    case '$':
	c = GetC(ip);
	if( isalpha(c) || c == '_' || IsKanji(c) || ISDOT(c) ) {
	    addchar(ip->strbuf, '$');
	    goto identifier;
	}
	UnGetC(ip); 
	tp->type = '$';
	break;
#endif

    case ' ':
    case '\t':
	c = GetC(ip);
	goto again;

    case '\n':
	if( !ip->eof )  ip->src_pos++;
	c = GetC(ip);
	goto again;

    case EOF:
	return NULL;

    default:
	if( IsKanji(c) ) {
	    goto identifier;
	} else if( iscntrl(c) ) {
	    error(ip->src_pos, 
		  "Illegal character in the input file (char_code=0x%02x)",
		  "入力ファイル中に不適切な文字が含まれています(コード=0x%02x)", c);
	}
	tp->type = c;
	break;
    }

    tp->src_pos = ip->src_pos;
    return tp;

 identifier:
    do {
	addchar(ip->strbuf, c);
	if( TwoByteChar(c) ) {
	    c = GetC(ip);
	    addchar(ip->strbuf, c);
	}
	c = GetC(ip);
    } while( isalnum(c) || c == '_' || IsKanji(c) || ISDOT(c) );
    addchar(ip->strbuf, 0);
#ifdef ID_DOT
    {   char *p;
	for( p = ip->strbuf.buf; *p; p++ ) {
	    if( *p == '.' )  *p = '_';
	}
    }
#endif
    if( (kw = in_word_set(ip->strbuf.buf, ip->strbuf.len - 1)) ) {
	if( tkinfo[(tp->type = kw->val)].flags & F_HAVEEQ && c == '=' ) { 
	    tp->type = tkinfo[tp->type].regno;
	} else UnGetC(ip);
    } else {
	tp->type = T_ID;
	tp->u.id.name = ip->strbuf.buf;
	tp->u.id.hash = hash_func(ip->strbuf.buf);
	UnGetC(ip);
    }
    tp->src_pos = ip->src_pos;
    return tp;
}

/*********************************************************************
 * Debugging aids
 *********************************************************************/
#ifdef DEBUG
dump_pbstack()
{
    PBStack  *p;
    int  i = 0;

    printf("### push-back stack contents ###\n");
    for( p = cur_thd->pb->top; p; p = p->s.next ) {
	printf("[%2d] ", i++);
	switch( p->type ) {
	case T_WTOKENS:
	    fprint_tklist(stdout, p->s.u.tp, O_WTOKENS, -1, -1);
	    printf(" %s%s%s", 
		   p->s.flags & TF_RPLPOS ? "/RPLPOS" : "",
		   p->s.flags & TF_CALLEOM ? "/CALLEOM" : "",
		   p->s.flags & TF_FREE ? "/FREE" : "");
	    break;
	case T_ISTREAM:
	    printf("<ISTREAM> fd=%d file=", p->s.u.ip->fd);
	    fprint_fname(stdout, p->s.u.ip->src_pos);
	    printf(": column=%d look_ahead=%d",
		   p->s.u.ip->cnt, p->s.u.ip->look_ahead);
	    break;
	default:
	    fprint_token(stdout, &p->t, -1, -1);
	}
	printf("\n");
    }
}
#endif

#ifdef INPUT_SELF_TEST
/* To do self test, compile input.c with error.c, token.c, util.c, array.c, 
   rational.c and keyword.c */
int  japan = 0;
void fprint_calls(fp) FILE *fp; {}
void end_of_macro() {}
struct tok_table  tkinfo[256];
int hash_func(p) char *p; { return 0; }
Thread  *cur_thd;

main()
{
    Token  *tp;

    init_input();
    cur_thd = (Thread *) malloc(sizeof(Thread));
    init_pbstack(cur_thd);
    pushbk_file("-", 0);

    while( (tp = get_token())->type != T_EOF ) {
	printf("%d: ", tp->type);
	fprint_token(stdout, tp, -1, -1);
	printf("\n");
    }
}
#endif
