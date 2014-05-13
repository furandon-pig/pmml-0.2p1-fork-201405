/*
 * PMML: A Compiler for the Practical Music Macro Language
 *
 * load.c: standard MIDI file loader
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
#include "pmml.h"
#include "../common/smf.h"

#define	MAX_LOAD	16	/* max number of MIDI files for input */

/* structure per opening MIDI file */
typedef struct load_slot {
    char  *fname;		/* file name (NULL means unused slot) */
    int	  ntrk;			/* no. of tracks */ 
    int	  format;		/* format */
    int	  division;		/* resolution */
    MFILE *mfp;
} LoadSlot;

static LoadSlot  ldslots[MAX_LOAD];

static LoadSlot *open_midi_file P((char *));
static void 	read_track P((LoadSlot *, Rational *, Rational *, int,
			      Rational *));
static 		ld_warn_note P((LoadSlot *, int));
static 		ld_warn_meta P((LoadSlot *, int));

extern int  max_reso;	/* maximum resolution among loaded files */

/*
 * load a standard MIDI file from the time indicated by t-register 
 *  (subsidiary of parse_load)
 */
void
load_midi_file(fname, end_time)
char  *fname;
Rational  *end_time;	/* (Output) End time of loaded song (max length 
			   of all the tracks) */
{
    LoadSlot  *slot;
    int  i;
    Rational  tmp;

    slot = open_midi_file(fname);

    *end_time = r_zero;

    for( i = 0; i < slot->ntrk; i++ ) {
	if( smf_nexttrack(slot->mfp) < 0 ) {
	    mferror(strcmp(slot->fname, "-") == 0 ? "stdin" : slot->fname); 
	}
	read_track(slot, &cur_thd->reg.t, &cur_thd->reg.dt, i, &tmp);
	if( rgreater(&tmp, end_time) )  *end_time = tmp;
    }

    /* free the slot */
    smf_close(slot->mfp);
    free(slot->fname);
    slot->fname = NULL;
}

/*
 * load a track in a standard MIDI file from the time indicated by t-register 
 *  (subsidiary of parse_loadtrk)
 *
 */
void
load_single_track(fname, track, dsttrk, end_time)
char  *fname;
int  track;		/* track number (base 1) */
int  dsttrk;		/* destination track number (base 0) */
Rational  *end_time;	/* (Output) End time of loaded track */
{
    LoadSlot  *slot;

    slot = open_midi_file(fname);
    if( track < 1 || track > slot->ntrk ) {
	error(cur_srcpos, "%s: Track %d does not exist",
	      "%s: '%d'番のトラックは存在しません",
	      strcmp(slot->fname, "-") == 0 ? "stdin" : slot->fname, track);
    }

    if( smf_gototrack(slot->mfp, track) < 0 ) {
	mferror(strcmp(slot->fname, "-") == 0 ? "stdin" : slot->fname); 
    }
    read_track(slot, &cur_thd->reg.t, &cur_thd->reg.dt, dsttrk, end_time);
}

/*
 * search the load slots, and if not found, open a MIDI file 
 */
static LoadSlot *
open_midi_file(fname)
char  *fname;
{
    int  i;
    char  *fn;
    LoadSlot  *newslot = NULL;

    if( !(fn = attach_extension(fname, MFILE_EXT)) ) {
	err_nomem("open_midi_file");
    }

    /* check if the file is already opened */
    for( i = 0; i < MAX_LOAD; i++ ) {
	if( ldslots[i].fname ) {
	    if( strcmp(ldslots[i].fname, fn) == 0 ) {
		free(fn);
		return &ldslots[i];
	    }
	} else {
	    if( !newslot )  newslot = &ldslots[i];
	}
    }
    
    /* new file */
    if( !newslot ) {
	error(cur_srcpos, "Too many MIDI files opened",
	      "オープン中のMIDIファイルが多すぎます");
    }

    newslot->fname = fn;
    newslot->mfp = smf_ropen(fn, &newslot->format, 
			     &newslot->ntrk, &newslot->division);
    if( ! newslot->mfp ) {
	mferror(strcmp(fn, "-") == 0 ? "stdin" : fn); 
    }

    return newslot;
}

/*
 * close a MIDI file and free its slot
 */
void
close_midi_file(fname)
char  *fname;
{
    int  i;
    char  *fn;

    if( !(fn = attach_extension(fname, MFILE_EXT)) ) {
	err_nomem("close_midi_file");
    }

    for( i = 0; i < MAX_LOAD; i++ ) {
	if( ldslots[i].fname ) {
	    if( strcmp(ldslots[i].fname, fn) == 0 ) {
		free(fn);
		/* free the slot */
		smf_close(ldslots[i].mfp);
		free(ldslots[i].fname);
		ldslots[i].fname = NULL;
		break;
	    }
	}
    }
    /* if no slots are found, do nothing */
}

/*
 * get MIDI file information
 */
int
get_ntrk(fname)
char  *fname;
{
    return( open_midi_file(fname)->ntrk );
}

int
get_format(fname)
char  *fname;
{
    return( open_midi_file(fname)->format );
}

int
get_resolution(fname)
char  *fname;
{
    return( open_midi_file(fname)->division );
}


/*
 * read events in a track 
 */
#define check_note(n)  ((n) < 0 ? ld_warn_note(slot, n), 0 : \
			(n) >= 128 ? ld_warn_note(slot, n), 127 : (n))

static void
read_track(slot, btime, dt, dsttrk, end_time)
LoadSlot  *slot;
Rational  *btime;	/* time from which the track is loaded */
Rational  *dt;		/* time modifier which is added to every events */
int   dsttrk;		/* destination track number (base 0) */
Rational  *end_time;	/* (Output) End time of loaded track */
{
    long  dtime, atime;
    Rational  timeofs;	/* btime + dt */
    int   type;
    int   len;
    uchar  msg[3];
    Event  *ep;
    int	  tp = cur_thd->reg.tp;

    if( slot->division >= 8192 /* max reso of Rational / 4 */ ) {
	error(cur_srcpos, "%s: Too high resolution", "%s: 分解能が高すぎます",
	      strcmp(slot->fname, "-") == 0 ? "stdin" : slot->fname);
    }

    /* update max_reso */
    if( slot->division > max_reso )  max_reso = slot->division;

    radd(btime, dt, &timeofs);

    atime = 0;
    do {
	if( smf_getevent(slot->mfp, &dtime, 
			 &type, &len, 0, msg) == MF_ERROR ) { 
	    mferror(strcmp(slot->fname, "-") == 0 ? "stdin" : slot->fname); 
	}
	
	atime += dtime;

	event_alloc(ep);
	rset(atime, slot->division * 4, &ep->time);
	radd(&ep->time, &timeofs, &ep->time);
	if( ep->time.intg < 0 ) {
	    warn_time(&ep->time);
	    ep->time = r_zero;
	}
	ep->track = dsttrk;
	ep->next = NULL;

	if( type == MF_MIDI ) {
	    ep->ch = msg[0] & 0xf;
	    switch( msg[0] & 0xf0 ) {
	    case 0x80:	/* note off */
		ep->type = E_NoteOff;
		ep->flags = IrregularNoteOff;
		ep->note = check_note((int)msg[1] + tp);
		ep->ev_partner = NULL;
		ep->ev_veloc = msg[2];
		break;
	    case 0x90:	/* note on */
		ep->note = check_note((int)msg[1] + tp);
		ep->ev_partner = NULL;
		if( msg[2] == 0 ) {
		    ep->type = E_NoteOff;
		    ep->flags = IrregularNoteOff;
		    ep->ev_veloc = -1;
		} else {
		    ep->type = E_NoteOn;
		    ep->ev_veloc = msg[2];
		}
		break;
	    case 0xa0:	/* key pressure */
		ep->type = E_Kp;
		ep->note = check_note((int)msg[1] + tp);
		ep->u.obj.o_type = O_INT;
		ep->u.obj.o_val = msg[2];
		break;
	    case 0xb0:	/* control change */
		ep->type = E_Ctrl + msg[1];
		ep->u.obj.o_type = O_INT;
		ep->u.obj.o_val = msg[2];
		break;
	    case 0xc0:	/* program change */
		ep->type = E_Prog;
		ep->u.obj.o_type = O_INT;
		ep->u.obj.o_val = msg[1] + 1;
		break;
	    case 0xd0:	/* channel pressure */
		ep->type = E_Cpr;
		ep->u.obj.o_type = O_INT;
		ep->u.obj.o_val = msg[1];
		break;
	    case 0xe0:	/* pitch bend */
		ep->type = E_Bend;
		ep->u.obj.o_type = O_INT;
		ep->u.obj.o_val = msg[1] + (msg[2] << 7) - 8192;
		break;
	    }
	} else {
	    ep->ev_len = len;
	    /* read data part */
	    if( !len )  ep->ev_data = NULL;
	    else {
		if( !(ep->ev_data = (uchar *) malloc(len)) ) {
		    err_nomem("read_track");
		}
		fread(ep->ev_data, 1, len, slot->mfp->fp);
	    }

	    switch(type) {
	    case MF_SYSEX:
		ep->type = E_Excl;
		break;
	    case MF_ARBIT:
		ep->type = E_Arbit;
		break;
	    case MF_TEMPO:
		/* change it to virtual control change */
		{
		    long  tempo = ((long)ep->ev_data[0] << 16) +
			          ((long)ep->ev_data[1] << 8) + ep->ev_data[2];

		    ep->type = E_Tempo;
		    free(ep->ev_data);
		    ep->u.obj.fpval = 6e+7 / tempo;
		}
		ep->ch = 0;
		break;
	    case MF_EOT:
		/* To avoid the unexpected termination of song, 
		   change it to a dummy event */
		ep->type = E_LoadEnd;
		break;
	    default:  /* meta event */
		if( type >= 128 ) {
		    /* illegal meta event */
		    ld_warn_meta(slot, type);
		    if( ep->ev_data )  free(ep->ev_data);
		    free(ep);
		    continue;
		}
		ep->type = E_Meta + type;
		break;
	    }
	}

	output_event(ep);

    } while( type != MF_EOT );

    rset(atime, slot->division * 4, end_time);
    radd(end_time, btime, end_time);
}

static
ld_warn_note(slot, note)
LoadSlot *slot;
int  note;
{
    warn(cur_srcpos, "%s: Note number out of range (note=%d)",
	 "%s: ノート番号が範囲外です (note=%d)", 
	 strcmp(slot->fname, "-") == 0 ? "stdin" : slot->fname, note);
}

static
ld_warn_meta(slot, mtype)
LoadSlot *slot;
int  mtype;
{
    warn(cur_srcpos, "%s: Ignored illegal meta event (meta_type=%d)",
	 "%s: 無効なメタイベントを無視します (meta_type=%d)", 
	 strcmp(slot->fname, "-") == 0 ? "stdin" : slot->fname, mtype);
}
