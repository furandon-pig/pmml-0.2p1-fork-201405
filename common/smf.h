/*
 * smf.h: header for standard MIDI file access routines
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

#ifndef __SMF_H__
#define	__SMF_H__

/*
 * Event type definitions 
 */
#define MF_MIDI		0x100		/* MIDI message event */
#define MF_SYSEX	0x101		/* system exclusive message event */
#define MF_ARBIT	0x102		/* arbitrary message event */
#define MF_SEQNO	0x00		/* meta events */
#define MF_TEXT		0x01
#define MF_COPYRIGHT 	0x02
#define MF_TRKNAME	0x03
#define MF_INSTNAME	0x04
#define MF_LYRIC	0x05
#define MF_MARKER	0x06
#define MF_CUE		0x07
#define MF_EOT		0x2f
#define MF_TEMPO	0x51
#define MF_SMPTE	0x54
#define MF_TIMESIG	0x58
#define MF_KEYSIG	0x59

/*
 * Error return values
 */
#define	MF_ERROR	1
#define	MF_WARNNING	2

/* 
 * file name extension of standard MIDI files
 */
#ifndef MFILE_EXT
#define MFILE_EXT	".mid"
#endif

/*
 * structure for the output buffer
 */
#define	SMF_BLKSIZE  1000

struct smf_bufblk {
    struct smf_bufblk  *link;
    int	 nbytes;
    unsigned char  buf[SMF_BLKSIZE];
};

/*
 * FILE structure specialized for a MIDI file
 */
typedef struct {
    FILE  *fp;
    int	  run_st;	/* MIDI running status */
    long  loc;		/* In non-buffered write mode:
			     file offset at the top of the current track
			   In buffered write mode:
			     the number of bytes in a track
			   In read mode:
			     file offset at the top of the first track */
    int  flags;		/* Bit0: true if the running status rule is applied
			   Bit1: true if the file is lseek'able */
    int	 (*fn_putc)();	/* function that outputs a byte to the file */
    int  cur_track;	/* current track number (base is 1) */
    struct smf_bufblk  *bp;	/* pointer to the output buffer */
} MFILE;

#define	MFLAG_RUNST	0x01
#define	MFLAG_SEEKOK	0x02

/*
 * Error status code 
 */
#define	EBADEOF_SMF	512	/* Bad MIDI file format (Unexpected EOF) */
#define	EBADHEAD_SMF	513	/* Bad MIDI file format (Bad Header) */
#define	ENOHEAD_SMF	514	/* Bad MIDI file format (No Header) */
#define	EBADSTAT_SMF	515	/* Bad MIDI file format (No MIDI status) */
#define	EBADMETA_SMF	516	/* Bad MIDI file format 
				   (Too short tempo or seqno event) */
#define ESEEKFAIL_SMF 	517	/* Can not seek (used by smf_chgheader) */

/*
 * macros
 */
#define	smf_putc(c,mfp)	(*(mfp)->fn_putc)((c),(mfp))
#define smf_getc(mfp)	getc((mfp)->fp)

/*
 * function prototypes
 */
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

MFILE	*smf_wopen P((char *, int, int, int));
MFILE	*smf_fpwopen P((FILE *, int, int, int));
void	smf_close P((MFILE *));
int	smf_chgheader P((MFILE *, int, int, int));
void	smf_bgntrack P((MFILE *));
int	smf_endtrack P((MFILE *, long));
void	smf_putevent P((MFILE *, long, int, int, unsigned char *));
#ifdef __SMF_C__
void	smf_midi P((MFILE *, long, int, int, int));
#else
void	smf_midi P((MFILE *, long, int, ...));
#endif
void	smf_excl P((MFILE *, long, int, unsigned char *));
void	smf_arbit P((MFILE *, long, int, unsigned char *));
void	smf_seqno P((MFILE *, long, int));
void	smf_text P((MFILE *, long, int, char *));
void	smf_tempo P((MFILE *, long, long));
void	smf_smpte P((MFILE *, long, int, int, int, int, int));
void	smf_timesig P((MFILE *, long, int, int, int, int));
void	smf_keysig P((MFILE *, long, int, int));
void	smf_runstat P((MFILE *, int));

MFILE	*smf_ropen P((char *, int *, int *, int *));
MFILE	*smf_fpropen P((FILE *, int *, int *, int *));
long	smf_nexttrack P((MFILE *));
long	smf_gototrack P((MFILE *, int));
int	smf_getevent P((MFILE *, long *, int *, int *, int, unsigned char *));

void	smf_perror P((char *));

void	smf_putvarlen P((MFILE *, long));
void	smf_putint16 P((MFILE *, int));
void	smf_putint24 P((MFILE *, long));
void	smf_putint32 P((MFILE *, long));

long	smf_getvarlen P((MFILE *));
int	smf_getint16 P((MFILE *));
long	smf_getint24 P((MFILE *));
long	smf_getint32 P((MFILE *));

int	midi_size P((int));

#endif	/* __SMF_H__ */
