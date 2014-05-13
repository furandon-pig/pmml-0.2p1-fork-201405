/*
 * M2P: A SMF-to-PMML Translator
 *   
 * readsmf.c: read a standard MIDI file
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
 * Acknowledgments: I am grateful to Mr. Tomoaki Sasaki for his help
 * in developing this program.
 */

#include <stdio.h>
#include <math.h>
#include "m2p.h"
#include "../common/smf.h"

int	resolution;   /* SMF resolution (ticks per quarter note) */

static float	conv_tempo P((long));

void 
read_smf(filename, buf)
char  *filename;
SMFInfo  *buf;
{
    MFILE  *mfp;
    Track  *tp;
    Event  *ep, *head, **tail;
    char  *fname, *err_fname;
    int  tk, format, ntrks, reso;
    long  dtime, atime, seqno;
    int  type, len;
    uchar  msg[3], *data;
    
    if(!(fname=attach_extension(filename, MFILE_EXT))) { 
	err_nomem("read_smf");
    }
    
    err_fname=(!strcmp(fname, "-")) ? "stdin" : fname;
    
    /* open the file and read the header part */
    if(!(mfp=smf_ropen(fname, &format, &ntrks, &reso))) {
	smf_perror(err_fname);
	exit(1);
    }
    
    buf->ntrks = ntrks;
    buf->format = format;
    buf->resolution = resolution = reso;

    /* allocate memory for an array of tracks */
    tp = buf->trks = (Track *) malloc(sizeof(Track) * ntrks);
    if(!tp)  err_nomem("read_smf");
    memset(tp, 0, sizeof(Track) * ntrks); /* initialize all pointers to NULL */
    
    for( tk = 1; tk <= ntrks; tk++, tp++ ) {   /* for each track */
	if( smf_nexttrack(mfp) < 0 ) {
	    smf_perror(err_fname);
	    exit(1);
	}
	
	tail = &head;
	seqno = 0;  /* set the event sequential number to zero */
	atime = 0;
	
	do {
	    /* get the next event */
	    if( smf_getevent(mfp, &dtime, &type, &len, 0, msg) == MF_ERROR ) {
		smf_perror(err_fname);
		exit(1);
	    }
	    
	    atime += dtime; /* add the delta time to the absolute time */
	    
	    /* allocate memory for a new event */
	    if( !(ep = (Event *)malloc(sizeof(Event))) ) {
		err_nomem("read_smf");
	    }
	    
	    ep->time = atime;
	    ep->seqno = ++seqno;
	    
	    if( type == MF_MIDI ) {
		ep->ch = (msg[0] & 0x0f);
		switch(msg[0] & 0xf0) {
		case 0x80: /* NoteOff */
		    ep->note = msg[1]; 
		    ep->ev_partner = NULL;
		    ep->type = E_NoteOff;
		    ep->ev_veloc = msg[2];
		    break;
		case 0x90: /* NoteOn */
		    ep->note = msg[1];
		    ep->ev_partner = NULL;
		    if( msg[2] ) { /* make it to NoteOFF if velocity is 0 */
			ep->type = E_NoteOn;
			ep->ev_veloc = msg[2];
		    } else {
			ep->type = E_NoteOff;
			ep->ev_veloc = -1;
		    }
		    break;
		case 0xa0: /* Key Pressure */
		    ep->type = E_Kp;
		    ep->note = msg[1];
		    ep->u.val = msg[2];
		    break;
		case 0xb0: /* Control Change */
		    ep->type = E_Ctrl + msg[1];
		    ep->u.val = msg[2];
		    break;
		case 0xc0: /* Program Change */
		    ep->type = E_Prog;
		    ep->u.val = msg[1] + 1;
		    break;
		case 0xd0: /* Channel Pressure */
		    ep->type = E_Cpr;
		    ep->u.val = msg[1];
		    break;
		case 0xe0: /* Pitch Bend */
		    ep->type = E_Bend;
		    ep->u.val = msg[1] + (msg[2] << 7) - 8192;
		    break;
		}
	    } else {
		/* exclusive or meta event */
		if(!len) {
		    data=NULL;
		} else {
		    if(!(data=(uchar *)malloc(len))) {
			err_nomem("read_smf");
		    }
		    fread(data, 1, len, mfp->fp);
		}
		ep->ev_len = len;
		ep->ev_data = data;
		
		switch(type) {
		case MF_SYSEX: /* MIDI System Exclusive Message */
		    ep->type = E_Excl;
		    break;
		case MF_ARBIT: /* Arbitrary MIDI Message */
		    ep->type = E_Arbit;
		    break;
		case MF_TEMPO: /* Change it to No.192 control change */
		    {
			long  tempo = ((long)ep->ev_data[0]<<16) + 
			    ((long)ep->ev_data[1]<<8) + ep->ev_data[2];
			ep->type = E_Tempo;
			ep->u.val = conv_tempo(tempo);
			free(data);
		    }
		    break;
		default: /* other Meta Events */
		    if( type >= 128 ) {
			/* ignore it */
			if( data ) free(data);
			free(ep);
			continue;
		    }
		    ep->type = E_Meta + type;
		    break;
		}
	    }
	    
	    /* add the event to the event list */
	    *tail = ep;
	    tail = &ep->next;

	} while( type != MF_EOT );
	
	*tail = NULL;  /* end of event list */

	if( head ) {  /* Is there at least one event? */
	    if( !(tp->eseq = (EventSeq *)malloc(sizeof(EventSeq))) ) {
		err_nomem("read_smf");
	    }
	    tp->eseq->events = head;
	    tp->eseq->next = NULL;	/* only one event sequence */
	}
    }
    
    smf_close(mfp); 
    free(fname);
}

/*
 * convert a SMF tempo value to "beats per minute".
 */
static float
conv_tempo(smftempo)
long  smftempo;
{
    float  ftempo, ftempo_better, sc;
    int  i;

    if( smftempo == 0 )  smftempo = 1;

    ftempo = 6e7 / smftempo;

    /* improve the tempo value so as to minimize the number of digits when
       it is converted to a decimal form */
    sc = 1.0;
    for( i = 0; i < 4; i++ ) {
	ftempo_better = floor(ftempo * sc + .5) / sc;
	if( smftempo == floor(6e7 / ftempo_better + .5) ) {
	    return ftempo_better;
	}
	sc *= 10.0;
    }
    return ftempo;
}
