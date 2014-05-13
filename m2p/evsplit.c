/*
 * M2P: A SMF-to-PMML Translator
 *   
 * evsplit.c: event list splitting & merging routines
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
 * Acknowledgments: I am grateful to Mr. Yoshitaka Endo for his help
 * in developing this program.
 */

#include <stdio.h>
#include "m2p.h"

static Event *	copy_trkend P((Event *));

/*
 * split an event list by MIDI channels
 *   - the input list is trk->eseq 
 *   - the output lists are
 *         trk->eseq : list of channel-independent events 
 *         trk->epseqch[0..16] : list of events for each channel
 */
void
csplit(trk)
Track  *trk;
{
    Event  *head[17], **tail[17];  /* [0] is for ch-indep events, and
				      [1..17] are for each channel */
    Event  *ep;
    int  idx, i;

    if( !trk->eseq || trk->eseq->events->type == E_TrkEnd )  return;

    /* initialize "tail" array */
    for( i = 0; i < 17; i++ ) {
	tail[i] = &head[i];
    }

    /* distribute each event */
    for( ep = trk->eseq->events; ep->type != E_TrkEnd; ep = ep->next ) {
	if( ep->type < E_GCtrl || ep->type >= E_NoteOff ) {
	    /* per-channel events */
	    idx = ep->ch + 1;
	} else {
	    idx = 0;
	}
	*tail[idx] = ep;
	tail[idx] = &ep->next;
    }

    /* complete ch-indep event list */
    *tail[0] = NULL;
    if( ! head[0] ) {
	free(trk->eseq);
	trk->eseq = NULL;
    } else {
	*tail[0] = copy_trkend(ep);
	trk->eseq->events = head[0];
    }

    /* complete per-channel event list */
    for( i = 1; i < 17; i++ ) {
	*tail[i] = NULL;
	if( ! head[i] ) {
	    trk->eseqch[i-1] = NULL;
	} else {
	    *tail[i] = copy_trkend(ep);
	    if( !(trk->eseqch[i-1] = (EventSeq *)malloc(sizeof(EventSeq)) )) {
		err_nomem("csplit");
	    }
	    trk->eseqch[i-1]->events = head[i];
	    trk->eseqch[i-1]->next = NULL;
	}
    }

    free(ep);	/* free the original TrkEnd event */
}

/*
 * split an event list by event types
 *   - the input list must be a single list
 *        S -> E -> E -> E -> E -> ...
 *   - the output lists are like the following:
 *        S -> E -> E -> .. [note-on & note-off]
 *        |               
 *        S -> E -> ...     [meta event except tempo]
 *        |
 *        :                   
 *     Lists are omitted if they are empty. 
 *     The "linefittable" member of EventSeq is set by this function.
 *
 *   The order of event types are:
 *      0. note-on & note-off
 *      1. meta event except tempo
 *    * 2. tempo
 *      3. excl & arbit
 *      4. prog change, ctrl(0), ctrl(32) (bank change)
 *      5. ctrl(6), ctrl(38), ctrl(96-101) (RPC) 
 *      6. ctrl(102-127) (mode change)
 *    * 7. pitch bend
 *    * 8. key pressure
 *    * 9. channel pressure
 *    *10. ctrl(1), ctrl(33)
 *    *11. ctrl(2), ctrl(34)
 *             : (lack ctrl(6))
 *    *39. ctrl(31), ctrl(63)
 *    *40. ctrl(64)
 *             :
 *    *71. ctrl(95)
 *  '*' means the possiblity of using continuous control change.
 */
static  uchar  evctg[E_END] = {  /* category code for each event type */
    /* ctrl 0-31 */
    4, 10, 11, 12, 13, 14, 5, 15, 16, 17, 18, 19, 20, 21, 22, 23,
    24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39,
    /* ctrl 32-63 */
    4, 10, 11, 12, 13, 14, 5, 15, 16, 17, 18, 19, 20, 21, 22, 23,
    24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39,
    /* ctrl 64-95 */
    40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55,
    56, 57, 58, 59, 60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71,
    /* ctrl 96-127 */
    5, 5, 5, 5, 5, 5, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
    6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
    /* ctrl 128-191 */
    7, 8, 9, 4, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    /* ctrl 192-255 */
    2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    /* meta 0-127 */
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    /* reserverd */
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    /* note, excl */
    3, 3, 0, 0,
};
#define	NECTGS	   72		/* number of event categories */
#define LINEFITTABLE(ctg)   (((ctg)==2) || ((ctg)>=7))

void
esplit(eseqp)
EventSeq  **eseqp;
{
    Event  *head[NECTGS], **tail[NECTGS]; 
    char   hasLSBctrl[NECTGS];
    Event  *ep;
    EventSeq  *sp, **stail;
    int  idx, i;

    if( (*eseqp)->events->type == E_TrkEnd )  return;

    /* initialize "tail" and "hasLSBctrl" array */
    for( i = 0; i < NECTGS; i++ ) {
	tail[i] = &head[i];
	hasLSBctrl[i] = 0;
    }

    /* distribute each event */
    for( ep = (*eseqp)->events; ep->type != E_TrkEnd; ep = ep->next ) {
	idx = evctg[ep->type];
	*tail[idx] = ep;
	tail[idx] = &ep->next;
	if( ep->type >= 32 && ep->type <= 63 ) {
	    hasLSBctrl[idx] = 1;
	}
    }

    /* complete event lists */
    free(*eseqp);
    stail = eseqp;
    for( i = 0; i < NECTGS; i++ ) {
	*tail[i] = NULL;
	if( head[i] ) {
	    *tail[i] = copy_trkend(ep);
	    if( !(sp = (EventSeq *)malloc(sizeof(EventSeq)) )) {
		err_nomem("esplit");
	    }
	    sp->events = head[i];
	    /* If there are "LSB" control changes, abandon line fitting */
	    sp->linefittable = LINEFITTABLE(i) && !hasLSBctrl[i];
	    *stail = sp;
	    stail = &sp->next;
	}
    }
    *stail = NULL;

    free(ep);	/* free the original TrkEnd event */
}

/*
 * duplicate Track-End event
 */
static Event *
copy_trkend(ep)
Event  *ep;
{
    Event  *ep1;

    if( !(ep1 = (Event *)malloc(sizeof(Event))) ) {
	err_nomem("copy_trkend");
    }
    memcpy(ep1, ep, sizeof(Event));
    ep1->next = NULL;
    return ep1;
}

/*
 * split a list of key-pressure events by note numbers
 *   - the input is a single list;
 *        E -> E -> E -> E -> ...
 *   - the output lists are like the following:
 *        S -> E -> E -> .. lowest note
 *        |                 
 *        S -> E -> ...     2nd lowest note 
 *        : 
 *   Return value is the top of the output lists.
 */
EventSeq *
nsplit(evlist)
Event  *evlist;
{
    Event  *head[128], **tail[128]; 
    Event  *ep;
    EventSeq  *sp, *shead, **stail;
    int  idx, i;

    /* initialize "tail" array */
    for( i = 0; i < 128; i++ ) {
	tail[i] = &head[i];
    }

    /* distribute each event */
    for( ep = evlist; ep != NULL; ep = ep->next ) {
	idx = ep->note;
	*tail[idx] = ep;
	tail[idx] = &ep->next;
    }

    /* complete each list */
    stail = &shead;
    for( i = 0; i < 128; i++ ) {
	*tail[i] = NULL;
	if( head[i] ) {
	    if( !(sp = (EventSeq *)malloc(sizeof(EventSeq)) )) {
		err_nomem("nsplit");
	    }
	    sp->events = head[i];
	    *stail = sp;
	    stail = &sp->next;
	}
    }
    *stail = NULL;

    return shead;
}


/*
 * merge event sequences into one
 */
void
merge_evlist(eseq)
EventSeq  *eseq;
{
    Event  *a, *b;
    Event  **link;
    EventSeq  *sp, *save;

    if( !eseq )  return;

    while( eseq->next ) { /* repeat until only 1 sequence exists */
	for( sp = eseq; sp && sp->next; ) {
	    /* pick two sequences and merge them to one */
	    a = sp->events;
	    b = sp->next->events;
	    link = &(sp->events);
	    save = sp->next;
	    sp->next = save->next;
	    free(save);
	    sp = sp->next;

	    for(;;) {
		if( !a ) {
		    *link = b;
		    break;
		} else if( !b ) {
		    *link = a;
		    break;
		} else if( b->time < a->time || 
			  (b->time == a->time && b->seqno < a->seqno) ) {
		    *link = b;
		    link = &(b->next);
		    b = b->next;
		} else {
		    *link = a;
		    link = &(a->next);
		    a = a->next;
		}
	    }
	}
    }
}

/*
 * merge event sequences split by csplit
 *   This routine assumes that each of trk->eseq and trk->eseqch[] has only
 *   one event sequence. 
 */
void
cmerge(trk)
Track  *trk;
{
    EventSeq  *head, *sp;
    int  i;

    /* connect EventSeq's */
    head = NULL;
    for( i = 16; --i >= -1; ) {  
	sp = i >= 0 ? trk->eseqch[i] : trk->eseq;
	if( sp ) {
	    sp->next = head;
	    head = sp;
	}
    }

    /* merge the sequences */
    merge_evlist(head);

    /* set Track members */
    trk->eseq = head;
    for( i = 0; i < 16; i++ )  trk->eseqch[i] = NULL;
}

/*
 * make a couple of isolated note-on & note-off events
 *   This routine assumes that the event list contains time-ordered events 
 *   of a single MIDI channel.
 */
void
pair_notes(evlist)
Event  *evlist;
{
    int  key;
    Event  *ep, **t, *ep1;
    Event  *note_tbl[128];	/* list of pending note-on events (for 
				   contructing list, ev_partner member of 
				   note_on event is used as pointer to 
				   the next element. */

    /* clear the note table */
    memset(note_tbl, 0, 128 * sizeof(Event *));

    /* look at each event */
    for( ep = evlist; ep != NULL; ep = ep->next ) {
	if( ep->type == E_NoteOn && !ep->ev_partner ) {
	    /* put it to the top of note-on events list */
	    t = &note_tbl[ep->note];
	    ep->ev_partner = *t;
	    *t = ep;
	} else if( ep->type == E_NoteOff && !ep->ev_partner ) {
	    t = &note_tbl[ep->note];
	    if( *t ) { 
		/* get the top of note-on events list 
		   and make it be the partner */
		ep->ev_partner = *t;
		*t = (*t)->ev_partner;
		ep->ev_partner->ev_partner = ep;
	    } else {
		/* It's really an isolated note-off event. Leave it. */
	    }
	}
    }

    /* Now, note-on event list has really isolated note-on events. */
    for( key = 0; key < 128; key++ ) {
	ep = note_tbl[key];
	while( ep ) {
	    ep1 = ep->ev_partner;
	    ep->ev_partner = NULL;
	    ep = ep1;
	}
    }
}
