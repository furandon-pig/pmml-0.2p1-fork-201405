/*
 * smf.c: standard MIDI file access routines
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
#include <errno.h>
#include <fcntl.h>
#define	__SMF_C__
#include "smf.h"

#define	ANY	0

extern	int	errno;

static	void	write_header P((MFILE *, int, int, int));
static	int  	read_mthd P((FILE *));
static	int  	read_mtrk P((FILE *));
static	int	buffer_putc P((int, MFILE *));
static	int	error_putc P((int, MFILE *));
static	int	direct_putc P((int, MFILE *));
static	void	buffer_flush P((MFILE *));


/*********************************************************************
 * Routines for writing MIDI files
 *********************************************************************/
/*
 * Open a MIDI file for write
 *  RETURN VALUE: 0 = error, non-zero = pointer to MFILE struct
 */
MFILE *
smf_wopen(path, format, ntrks, division)
char  *path;		/* path name of the file */
int   format;		/* single track (0), parallel tracks (1)
			   or sequential tracks (2) */
int   ntrks;		/* number of tracks */
int   division;		/* resolution of delta-time */
{
    FILE   *fp;
    MFILE  *mfp;

    if( strcmp(path, "-") == 0 ) {
	fp = stdout;
#ifdef DOS
	setmode(fileno(fp), O_BINARY);
#endif
    } else if( !(fp = fopen(path, "wb")) ) {
	return NULL;
    }

    if( !(mfp = smf_fpwopen(fp, format, ntrks, division)) ) {
	fclose(fp);
	return NULL;
    }
    
    return mfp;
}

/*
 * Create MFILE structure from already write-opened stream 
 *  RETURN VALUE: 0 = error, non-zero = pointer to MFILE struct 
 */
MFILE *
smf_fpwopen(fp, format, ntrks, division)
FILE  *fp;
int   format, ntrks, division;
{
    MFILE  *mfp;
    long   loc;
    
    mfp = (MFILE *) malloc( sizeof(MFILE) );
    if( !mfp ) {
	errno = ENOMEM;
	return NULL;
    }
    mfp->flags = MFLAG_RUNST;
    mfp->fp = fp;
    mfp->fn_putc = direct_putc;
    mfp->cur_track = 0;
    
    write_header(mfp, format, ntrks, division);
    
    /* check wether the file is seekable */
    if( !isatty(fileno(fp)) ) {
	fflush(fp);	/* for safety */
	loc = ftell(fp);
	if( fseek(fp, 0L, 0) == 0 ) {
	    mfp->flags |= MFLAG_SEEKOK;
	} 
	fseek(fp, loc, 0);
    }
    
    return  mfp;
}

/*
 * Close a MIDI file (common with reading routines)
 */
void
smf_close(mfp)
MFILE  *mfp;
{
    fclose(mfp->fp);
    free((char *) mfp);
}

/*
 * Change standard MIDI file header
 *  RETURN VALUE: 0 = normal end,  non-zero = error
 */
int
smf_chgheader(mfp, format, ntrks, division)
MFILE  *mfp;
int    format, ntrks, division;
{
    FILE  *fp = mfp->fp;
    long  loc;

    if( !(mfp->flags & MFLAG_SEEKOK) ) {
	errno = ESEEKFAIL_SMF;
	return MF_ERROR;
    }
    
    loc = ftell(fp);
    fseek(fp, 0L, 0);
    write_header(mfp, format, ntrks, division);
    fseek(fp, loc, 0);
    
    return 0;
}

/*
 * Put standard MIDI file header
 */
static void
write_header(mfp, format, ntrks, division)
MFILE *mfp;
int   format;		/* single track (0), parallel tracks (1)
			   or sequential tracks (2) */
int   ntrks;		/* number of tracks */
int   division;		/* resolution of delta-time */
{
    fprintf(mfp->fp, "MThd");
    smf_putint32(mfp, 6L);	/* length of header chunk */
    smf_putint16(mfp, format);
    smf_putint16(mfp, ntrks);
    smf_putint16(mfp, division);
}

/*
 * Put the begining of track
 */
void
smf_bgntrack(mfp)
MFILE  *mfp;
{
    FILE  *fp = mfp->fp;

    if( mfp->flags & MFLAG_SEEKOK ) {
	mfp->loc = ftell(fp);
    } else {
	mfp->loc = 0;
    }
    
    mfp->run_st = 0;
    mfp->cur_track++;
    fprintf(fp, "MTrk");
    
    if( mfp->flags & MFLAG_SEEKOK ) {
	smf_putint32(mfp, 0L);	/* track size: overwritten later */
    } else {
	mfp->fn_putc = buffer_putc;
	mfp->bp = NULL;
    }
}

/*
 * Put the end of track
 *  RETURN VALUE: 0 = normal end,  non-zero = error
 */
int
smf_endtrack(mfp, dtime)
MFILE  *mfp;
long   dtime;
{
    FILE  *fp = mfp->fp;
    long  bottom_of_track;

    smf_putvarlen(mfp, dtime);	/* put end-of-track event */
    smf_putc(0xff, mfp);
    smf_putc(0x2f, mfp);
    smf_putc(0x00, mfp);
    
    if( mfp->flags & MFLAG_SEEKOK ) {
	bottom_of_track = ftell(fp);
	if( fseek(fp, mfp->loc + 4, 0) )  return MF_ERROR;
	smf_putint32(mfp, bottom_of_track - mfp->loc - 8);
	if( fseek(fp, bottom_of_track, 0) )  return MF_ERROR;
    } else {
	if( mfp->fn_putc == error_putc ) {
	    return MF_ERROR;
	}
	mfp->fn_putc = direct_putc;
	smf_putint32(mfp, mfp->loc);	/* track size */
	buffer_flush(mfp);
    }
    
    return( ferror(fp) );
}

/*
 * Put an event (general form)
 */
void
smf_putevent(mfp, dtime, type, len, data)
MFILE *mfp;
long  dtime;		/* delta time */
int   type;		/* event type (MF_xxx) */
int   len;		/* length of data */
unsigned char  *data;	/* data (body of message, text, etc.) */
{
    unsigned char  *p;
    int  st;

    smf_putvarlen(mfp, dtime);
    p = data;
    switch( type ) {
    case MF_MIDI:
	st = data[0];
	len = midi_size(st);
	if( st == mfp->run_st && mfp->run_st < 0xf0 && 
	   (mfp->flags & MFLAG_RUNST) ) { 
	    /* omit status */
	    p++;
	    len--;
	} else {
	    mfp->run_st = st;
	}
	break;
    case MF_SYSEX:
	smf_putc(0xf0, mfp);
	smf_putvarlen(mfp, (long)len);
	mfp->run_st = 0;
	break;
    case MF_ARBIT:
	smf_putc(0xf7, mfp);
	smf_putvarlen(mfp, (long)len);
	mfp->run_st = 0;
	break;
    default:
	if( type < 0x100 ) { /* meta event */
	    smf_putc(0xff, mfp);
	    smf_putc(type, mfp);
	    smf_putvarlen(mfp, (long)len);
	    mfp->run_st = 0;
	    break;
	} else {
	    /* perhaps mistake */
	    break;
	}
    }
    /*
     * ouput data part
     */
    while( --len >= 0 ) {
	smf_putc(*p, mfp);
	p++;
    }
}

/*
 * Put a MIDI message event
 */
void
smf_midi(mfp, dtime, st, d1, d2)
MFILE *mfp;
long  dtime;		/* delta time */
int   st, d1, d2;	/* MIDI status, data */
{
    unsigned char  msg[3];
	
    msg[0] = st;
    msg[1] = d1;
    msg[2] = d2;
    smf_putevent(mfp, dtime, MF_MIDI, ANY, msg);
}

/*
 * Put a system exclusive message event
 */
void
smf_excl(mfp, dtime, len, data)
MFILE *mfp;
long  dtime;
int   len;		/* length of exclusive message */
unsigned char  *data;	/* exclusive message (THIS MUST BE TERMINATED
			   BY 0xf7 (END OF EXCLUSIVE) */
{
    smf_putevent(mfp, dtime, MF_SYSEX, len, data);
}

/*
 * Put an arbitrary MIDI message event
 */
void
smf_arbit(mfp, dtime, len, data)
MFILE *mfp;
long  dtime;
int   len;		/* length of message */
unsigned char  *data;	/* message */
{
    smf_putevent(mfp, dtime, MF_ARBIT, len, data);
}

/*
 * Put a sequence number event
 */
void
smf_seqno(mfp, dtime, seqno)
MFILE *mfp;
long  dtime;
int   seqno;		/* sequence number */
{
    unsigned char  d[2];

    d[0] = seqno >> 8;
    d[1] = seqno;
    smf_putevent(mfp, dtime, MF_SEQNO, 2, d);
}

/*
 * Put a text event
 */
void
smf_text(mfp, dtime, type, text)
MFILE *mfp;
long  dtime;
int   type;		/* type of text (MF_TEXT, MF_COPYRIGHT, etc.) */
char  *text;
{
    smf_putevent(mfp, dtime, type, strlen(text), (unsigned char *) text);
}

/*
 * Put a tempo change event
 */
void
smf_tempo(mfp, dtime, tempo)
MFILE *mfp;
long  dtime;
long  tempo;		/* tempo in microseconds per quarter-note */
{
    unsigned char  d[3];

    d[0] = tempo >> 16;
    d[1] = tempo >> 8;
    d[2] = tempo;
    smf_putevent(mfp, dtime, MF_TEMPO, 3, d);
}

/*
 * Put a SMPTE event
 */
void
smf_smpte(mfp, dtime, hr, mn, se, fr, ff)
MFILE *mfp;
long  dtime;
int   hr, mn, se, fr, ff;
{
    unsigned char  d[5];

    d[0] = hr;
    d[1] = mn;
    d[2] = se;
    d[3] = fr;
    d[4] = ff;
    smf_putevent(mfp, dtime, MF_SMPTE, 5, d);
}

/*
 * Put a time signature event
 */
void
smf_timesig(mfp, dtime, nn, dd, cc, bb)
MFILE *mfp;
long  dtime;
int   nn, dd, cc, bb;
{
    unsigned char  d[4];

    d[0] = nn;
    d[1] = dd;
    d[2] = cc;
    d[3] = bb;
    smf_putevent(mfp, dtime, MF_TIMESIG, 4, d);
}

/*
 * Put a key signature event
 */
void
smf_keysig(mfp, dtime, sf, mi)
MFILE *mfp;
long  dtime;
int   sf, mi;
{
    unsigned char  d[2];

    d[0] = sf;
    d[1] = mi;
    smf_putevent(mfp, dtime, MF_KEYSIG, 2, d);
}

/*
 * Toggle the running status rule
 */
void
smf_runstat(mfp, rs_on)
MFILE *mfp;
int   rs_on;
{
    if( rs_on ) {
	mfp->flags |= MFLAG_RUNST;
	mfp->run_st = 0;
    } else {
	mfp->flags &= ~MFLAG_RUNST;
    }
}

/*********************************************************************
 * Routines for reading MIDI files
 *********************************************************************/
/*
 * Open a MIDI file for read
 *  RETURN VALUE: 0 = error, non-zero = pointer to MFILE struct 
 */
MFILE *
smf_ropen(path, format, ntrks, division)
char  *path;		/* path name of the file */
int   *format;
int   *ntrks;
int   *division;
{
    FILE  *fp;
    MFILE *mfp;

    if( strcmp(path, "-") == 0 ) {
	fp = stdin;
#ifdef DOS
	setmode(fileno(fp), O_BINARY);
#endif
    } else if( !(fp = fopen(path, "rb")) ) {
	return NULL;
    }
    
    if( !(mfp = smf_fpropen(fp, format, ntrks, division)) ) {
	fclose(fp);
	return NULL;
    }
    
    return mfp;
}

/*
 * Create MFILE structure from already read-opened stream 
 *  RETURN VALUE: 0 = error, non-zero = pointer to MFILE struct 
 */
MFILE *
smf_fpropen(fp, format, ntrks, division)
FILE  *fp;
int   *format;
int   *ntrks;
int   *division;
{
    MFILE *mfp;
    long  size;
    
    mfp = (MFILE *) malloc( sizeof(MFILE) );
    if( !mfp ) {
	errno = ENOMEM;
	return NULL;
    }
    mfp->fp = fp;
    mfp->flags = 0;
    mfp->cur_track = 0;
    
    if( read_mthd(fp) != 0 ) {
	errno = ENOHEAD_SMF;
	free((char *)mfp);
	return NULL;
    }
    size = smf_getint32(mfp);
    *format = smf_getint16(mfp);
    *ntrks = smf_getint16(mfp);
    *division = smf_getint16(mfp);
    if( size < 6 || *format < 0 || *format > 2 
       || *division <= 0 || *ntrks < 0 ) {
	errno = EBADHEAD_SMF;
	free((char *)mfp);
	return NULL;
    }
    /* skip excess header */
    while( size > 6 ) {
	(void) getc(fp);
	size--;
    }
    
    mfp->loc = ftell(fp);
    return mfp;
}

/*
 * Read the header string "MThd" with skipping initial garbage
 *  RETURN VALUE: 0 = normal end, non-zero = error
 */
static int
read_mthd(fp)
FILE  *fp;
{
    /* We skip initial garbage using a finite state automaton. */
 state0:
    switch( getc(fp) ) {
    case EOF:	goto err;
    case 'M':	goto state1;
    default:	goto state0;
    }
 state1:
    switch( getc(fp) ) {
    case EOF:	goto err;
    case 'M':	goto state1;
    case 'T':	goto state2;
    default:	goto state0;
    }
 state2:
    switch( getc(fp) ) {
    case EOF:	goto err;
    case 'M':	goto state1;
    case 'h':	goto state3;
    default:	goto state0;
    }
 state3:
    switch( getc(fp) ) {
    case EOF:	goto err;
    case 'M':	goto state1;
    case 'd':	return 0;	/* ok */
    default:	goto state0;
    }
 err:
    errno = EBADEOF_SMF;
    return 1;
}

/*
 * Read the header string "MTrk" with skipping initial garbage
 *  RETURN VALUE: 0 = normal end, non-zero = error
 */
static int
read_mtrk(fp)
FILE  *fp;
{
    /* We skip initial garbage using a finite state automaton. */
 state0:
    switch( getc(fp) ) {
    case EOF:	goto err;
    case 'M':	goto state1;
    default:	goto state0;
    }
 state1:
    switch( getc(fp) ) {
    case EOF:	goto err;
    case 'M':	goto state1;
    case 'T':	goto state2;
    default:	goto state0;
    }
 state2:
    switch( getc(fp) ) {
    case EOF:	goto err;
    case 'M':	goto state1;
    case 'r':	goto state3;
    default:	goto state0;
    }
 state3:
    switch( getc(fp) ) {
    case EOF:	goto err;
    case 'M':	goto state1;
    case 'k':	return 0;	/* ok */
    default:	goto state0;
    }
 err:
    errno = EBADEOF_SMF;
    return 1;
}

/*
 * Start reading a track
 *  RETURN VALUE: negative value = error
 *		  otherwise = track size
 */
long
smf_nexttrack(mfp)
MFILE  *mfp;
{
    long  size;
    FILE  *fp = mfp->fp;
    
    if( read_mtrk(fp) )  return -1L;
    size = smf_getint32(mfp);
    mfp->run_st = 0;
    mfp->cur_track++;
    return  size;
}

/*
 * Set file pointer to the beginning of specified track
 *  RETURN VALUE: negative value = error
 *		  otherwise = track size
 */
long
smf_gototrack(mfp, track)
MFILE  *mfp;
int  track;	/* track number (base is 1) */
{
    long  size;
    FILE  *fp = mfp->fp;
    
    if( track <= mfp->cur_track ) {
	/* rewind needed */
	if( isatty(fileno(fp)) || fseek(fp, mfp->loc, 0) == -1 ) {
	    errno = ESEEKFAIL_SMF;
	    return -1L;
	}
	mfp->cur_track = 0;
    }
    for(;;) {
	if( (size = smf_nexttrack(mfp)) < 0 )  return size;
	if( mfp->cur_track == track )  break;
	/* skip track contents */
	while( --size >= 0 ) {
	    if( getc(fp) == -1 ) {
		errno = EBADEOF_SMF;
		return -1L;
	    }
	}
    }
    return  size;
}

/*
 * Get an event
 *  RETURN VALUE: 0 = normal end
 *		  -1 = end of track
 *		  1 = error (Bad MIDI file format)
 *		  2 = warnning (A long exclusive message has been truncated)
 */
int
smf_getevent(mfp, dtime, type, len, maxlen, data)
MFILE *mfp;
long  *dtime;
int   *type;
int   *len;
int   maxlen;		/* if maxlen = 0, excl/meta messages are not stored */
unsigned char  *data;
{
    int  c;
    int  n, i;
    unsigned char  *p;
    FILE *fp = mfp->fp;
    int  ret = 0;
    
    *dtime = smf_getvarlen(mfp);
    
    if( (c = getc(fp)) == EOF ) {
	errno = EBADEOF_SMF;
	return MF_ERROR;
    }

    switch( c ) {
    case 0xf0:	/* exclusive */
	*type = MF_SYSEX;
	*len = n = smf_getvarlen(mfp);
	goto read_data;

    case 0xf7:  /* arbitrary message */
	*type = MF_ARBIT;
	*len = n = smf_getvarlen(mfp);
	goto read_data;
	
    case 0xff:	/* meta event */
	if( (c = getc(fp)) == EOF ) {
	    errno = EBADEOF_SMF;
	    return MF_ERROR;
	}
	*type = c;
	*len = n = smf_getvarlen(mfp);
	
	if( (c == MF_SEQNO && n < 2) || (c == MF_TEMPO && n < 3) ) {
	    errno = EBADMETA_SMF;
	    return MF_ERROR;
	}
	if( c == MF_EOT ) {
	    ret = -1;
	}

    read_data:
	if( !maxlen )  break;
	if( n > maxlen ) {
	    ret = MF_WARNNING;
	    *len = maxlen;
	}
	p = data;
	for( i = 0; i < n; i++ ) {
	    c = getc(fp);
	    if( i < maxlen )  *p++ = c;
	}
	/* put the string end mark (for text event) */
	if( i < maxlen )  *p = 0;
	break;

    default:  /* song message event */
	*type = MF_MIDI;
	p = data;
	if( c >= 0x80 ) {  /* It's a status! */
	    if( c < 0xf0 )  mfp->run_st = c;
	    n = (*len = midi_size(c)) - 1;
	} else {	   /* no status */
	    if( mfp->run_st == 0 ) {
		errno = EBADSTAT_SMF;
		return MF_ERROR;
	    }
	    n = (*len = midi_size(mfp->run_st)) - 2;
	    *p++ = mfp->run_st;
	}
	*p++ = c;
	while( --n >= 0 )  *p++ = getc(fp);
    }
    
    return ret;
}

/*********************************************************************
 * Error Message Output
 *********************************************************************/
/*
 * perror() routine specialized for this module
 */
void
smf_perror(msg)
char  *msg;
{
    char  *reason;
    
    switch( errno ) {
    case EBADEOF_SMF:
    case EBADHEAD_SMF:
    case ENOHEAD_SMF:
    case EBADSTAT_SMF:
    case EBADMETA_SMF:
	switch( errno ) {
	case EBADEOF_SMF:
	    reason = "Unexpected EOF";
	    break;
	case EBADHEAD_SMF:
	    reason = "Bad header";
	    break;
	case ENOHEAD_SMF:
	    reason = "No header";
	    break;
	case EBADSTAT_SMF:
	    reason = "No MIDI status";
	    break;
	case EBADMETA_SMF:	
	    reason = "Too short tempo or seqno event";
	    break;
	default:  /* never reached */
	    reason = "";
	}
	fprintf(stderr, "%s: Bad MIDI file format (%s)\n", msg, reason);
	break;
    case ESEEKFAIL_SMF:
	fprintf(stderr, "%s: Can not seek\n", msg);
	break;
    default:
	perror(msg);
	break;
    }
}

/*********************************************************************
 * Low Level Access Routines
 *********************************************************************/
/*
 * Put a variable length quantity
 */
void
smf_putvarlen(mfp, value)
MFILE *mfp;
long  value;
{
    long  buffer;
    
    buffer = value & 0x7f;
    while((value >>= 7) > 0) {
	buffer <<= 8;
	buffer |= 0x80;
	buffer += (value & 0x7f);
    }
    for(;;) {
	smf_putc((int)buffer, mfp);
	if( buffer & 0x80 ) {
	    buffer >>= 8;
	} else {
	    break;
	}
    }
}

/*
 * Put a 16-bit integer number
 */
void
smf_putint16(mfp, value)
MFILE *mfp;
int   value;
{
    smf_putc((int)(value >> 8), mfp);
    smf_putc(value, mfp);
}

/*
 * Put a 24-bit integer number
 */
void
smf_putint24(mfp, value)
MFILE *mfp;
long  value;
{
    smf_putc((int)(value >> 16), mfp);
    smf_putc((int)(value >> 8), mfp);
    smf_putc((int)value, mfp);
}

/*
 * Put a 32-bit integer number
 */
void
smf_putint32(mfp, value)
MFILE *mfp;
long  value;
{
    smf_putc((int)(value >> 24), mfp);
    smf_putc((int)(value >> 16), mfp);
    smf_putc((int)(value >> 8), mfp);
    smf_putc((int)value, mfp);
}

/*
 * Get a variable length quantity
 */
long
smf_getvarlen(mfp)
MFILE  *mfp;
{
    long  value;
    int   c;
    FILE  *fp = mfp->fp;
    
    if( (value = getc(fp)) == EOF ) return 0L;
    if( value & 0x80 ) {
	value &= 0x7f;
	do {
	    if( (c = getc(fp)) == EOF ) return 0L;
	    value = (value << 7) + (c & 0x7f);
	} while( c & 0x80 );
    }
    return value;
}

/*
 * Get a 16-bit (signed) integer number (high byte first)
 */
int
smf_getint16(mfp)
MFILE  *mfp;
{
    int   value;
    FILE  *fp = mfp->fp;
    
    value = (/*signed*/ char) getc(fp);
    value = (value << 8) | (getc(fp) & 0xff);
    return value;
}

/*
 * Get a 24-bit integer number
 */
long
smf_getint24(mfp)
MFILE  *mfp;
{
    long  value;
    FILE  *fp = mfp->fp;
    
    value = (/*signed*/ char) getc(fp);
    value = (value << 8) | (getc(fp) & 0xff);
    value = (value << 8) | (getc(fp) & 0xff);
    return value;
}

/*
 * Get a 32-bit integer number
 */
long
smf_getint32(mfp)
MFILE  *mfp;
{
    long  value;
    FILE  *fp = mfp->fp;
    
    value = (/*signed*/ char) getc(fp);
    value = (value << 8) | (getc(fp) & 0xff);
    value = (value << 8) | (getc(fp) & 0xff);
    value = (value << 8) | (getc(fp) & 0xff);
    return value;
}

/*********************************************************************
 * Routines for handling the output buffer
 *  (The output buffer is used when the output file is
 *   not seek'able)
 *********************************************************************/
/*
 * Put 1 char to the output buffer
 */
static int
buffer_putc(c, mfp)
int  c;
MFILE  *mfp;
{
    struct smf_bufblk  *newbp;
    
    if( mfp->bp && mfp->bp->nbytes < SMF_BLKSIZE ) {
	mfp->bp->buf[mfp->bp->nbytes++] = c;
    } else {
	newbp = (struct smf_bufblk *) malloc(sizeof(struct smf_bufblk));
	if( !newbp ) {
	    mfp->fn_putc = error_putc;
	    return MF_ERROR;
	}
	
	newbp->nbytes = 1;
	newbp->buf[0] = c;
	if( !mfp->bp ) {
	    newbp->link = newbp;
	} else {
	    newbp->link = mfp->bp->link;
	    mfp->bp->link = newbp;
	}
	mfp->bp = newbp;
    }
    
    mfp->loc++;
    return 0;
}

/*
 * dummy putc after memory is exhausted
 */
static int
error_putc(c, mfp)
int  c;
MFILE  *mfp;
{
    return 0;
}

/*
 * Put 1 char without using the output buffer
 */
static int
direct_putc(c, mfp)
int  c;
MFILE  *mfp;
{
    return( putc(c, mfp->fp) );
}

/*
 * Flush the contents of the buffer to a file
 */
static void
buffer_flush(mfp)
MFILE  *mfp;
{
    struct smf_bufblk  *cur_bp, *next_bp;
    unsigned char  *p;
    
    if( !mfp->bp ) return;
    for( cur_bp = mfp->bp->link; ; cur_bp = next_bp ) {
	for( p = cur_bp->buf; --cur_bp->nbytes >= 0; p++ ) {
	    putc(*p, mfp->fp);
	}
	next_bp = cur_bp->link;
	free( (char *)cur_bp );
	if( cur_bp == mfp->bp )  break;
    }
    mfp->bp = NULL;
}

/*********************************************************************
 * Utilities
 *********************************************************************/
/*
 * midi_size returns the size of the MIDI message whose status byte is s.
 */
int
midi_size(s)
int  s;
{
    static int  table[8] = {3, 3, 3, 3, 2, 2, 3, 0};
    int	 len;
    
    if( (len = table[(s >> 4) & 7]) ) {
	return len;
    } else switch(s) {
    case 0xf0:	
	return 0;	/* exclusive */
    case 0xf1:
    case 0xf3:
	return 2;
    case 0xf2:
	return 3;
    default:
	return 1;
    }
}

/*********************************************************************
 * Self Tester (not perfect)
 *********************************************************************/
#ifdef SELFTEST
#define	T(step)	printf("Step %d -- ok\n", step);
main()
{
	MFILE	*mfp;
	int	fmt, ntrks, div;
	int	i;
	FILE	*fp;
	int	c, sum;
	long	dtime;
	int	type, len;
	unsigned char  data[80];

	if( !(mfp = smf_wopen("ttt.mid", 1, 2, 120) ) )
		goto err;
T(1)	for( i = 0; i < 2; i++ ) {
		smf_bgntrack(mfp);
		if( i==1 ) smf_runstat(mfp, 0);
T(2)		smf_tempo(mfp, 0L, 1234L);
T(3)		smf_midi(mfp, 10L, 0x90, 12, 60);
T(4)		smf_midi(mfp, 10L, 0x90, 12, 0);
T(5)		smf_text(mfp, 0L, MF_TEXT, "doushite");
		if( smf_endtrack(mfp, 20L) )
			goto err;
T(6)	}
T(7)	smf_close(mfp);

	if( !(fp = fopen("ttt.mid", "rb")) )  goto err;	
	sum = 0;
	while( (c = getc(fp)) != EOF )  sum += c;
	fclose(fp);
	if( sum != 5975 )  goto err;

T(8)	if( !(mfp = smf_ropen("ttt.mid", &fmt, &ntrks, &div) ) )
		goto err;
T(9)	if( fmt != 1 || ntrks != 2 || div != 120 )
		goto err;
	for( i = 0; i < 2; i++ ) {
		if( smf_nexttrack(mfp) < 0 )
			goto err;
T(10)		if( smf_getevent(mfp, &dtime, &type, &len, 80, data) != 0 )
			goto err;
		if( dtime != 0 || type != MF_TEMPO || len != 3 ||
		    *(long*)data != 1234 )
			goto err;
T(11)		if( smf_getevent(mfp, &dtime, &type, &len, 80, data) != 0 )
			goto err;
		if( dtime != 10 || type != MF_MIDI || len != 3 )
			goto err;
		if( data[0] != 0x90 || data[1] != 12 || data[2] != 60 )
			goto err;
T(12)		if( smf_getevent(mfp, &dtime, &type, &len, 80, data) != 0 )
			goto err;
		if( dtime != 10 || type != MF_MIDI || len != 3 )
			goto err;
		if( data[0] != 0x90 || data[1] != 12 || data[2] != 0 )
			goto err;
T(13)		if( smf_getevent(mfp, &dtime, &type, &len, 80, data) != 0 )
			goto err;
		if( dtime != 0 || type != MF_TEXT || len != 8 )
			goto err;
		if( strcmp(data, "doushite") != 0 ) 
			goto err;
T(14)		if( smf_getevent(mfp, &dtime, &type, &len, 80, data) != -1 )
			goto err;
		if( dtime != 20 || type != MF_EOT )
			goto err;
	}		
T(15)	smf_close(mfp);

#ifndef DOS
	fp = popen("cat > ttt2.mid", "wb");
	if( !(mfp = smf_fpwopen(fp, 1, 2, 120) ) )
		goto err;
T(16)	for( i = 0; i < 2; i++ ) {
		smf_bgntrack(mfp);
		if( i==1 ) smf_runstat(mfp, 0);
		smf_tempo(mfp, 0L, 1234L);
		smf_midi(mfp, 10L, 0x90, 12, 60);
		smf_midi(mfp, 10L, 0x90, 12, 0);
		smf_text(mfp, 0L, MF_TEXT, "doushite");
		if( smf_endtrack(mfp, 20L) )
			goto err;
	}
	smf_close(mfp);
	if( system("sleep 1; cmp -s ttt.mid ttt2.mid") )  goto err;
#endif

	puts("self test passed.");
	exit(0);

err:
	puts("self test failed.");
	exit(1);
}
#endif
