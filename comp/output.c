/*
 * PMML: A Compiler for the Practical Music Macro Language
 *
 * output.c: effector & event buffer related routines
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
#include <ctype.h>
#include <math.h>
#include "pmml.h"

/* set of events that must be placed in Track 1 */
static  EventSet  is_time_event;

/* set of events to be registered in "marks" track */
static  EventSet  is_mark_event;

/* set of per-channel events */
static	EventSet  per_ch_event;

/* set of events that should be put before leading blank measures */
static  EventSet  is_t0_event;

/* event priority table */
static char  epri[E_END];
int	sort_by_priority = 1;

/*
 * table of built-in effectors
 */
struct bi_effs {
    char  *name;
    int   flags;	/* effector flags */
    char  *arg_spec;	/* argument specification for init action */
    void  (*init)();	/* init action */
    void  (*detach)();	/* "beginning of detach" action */
    void  (*action)();  /* event handling action */  
    void  (*wrap)();	/* "end of detach" action */
};

void  smfout_init P((int, Object *, EffInst *));
void  smfout_detach P((EffInst *));
void  smfout_action P((Event *, EffInst *));
void  smfout_wrap P((EffInst *));

static struct bi_effs  bi_effs_tab[] = {
    { "$smfout", EF_CollectMarker, "siiiiiis:ss",
	  smfout_init, smfout_detach, smfout_action, smfout_wrap },
};

/*
 * event priority:
 *  1(highest): note-off with its corresponding note-on event in the past
 *  2         : meta (except tempo)
 *  3         : control-change / exclusive
 *  4         : track-end / load-end
 *  5         : note-on / note-off with no note-on or simultaneous note-on
 *  6(lowest) : key-pressure / channel-pressure
 */
#define  PRI_NOTEOFF  1
#define  PRI_META     2
#define  PRI_CTRL     3
#define  PRI_END      4
#define  PRI_NOTEON   5
#define  PRI_AFTOUCH  6
 
/* function prototypes */
static void  init_bi_effs P((void));
static void  sim_all_notes_off P((Track *, Rational *, int, 
				  uchar (*)[16], Event *(*)[16]));
static void  adjust_time P((Track *, Rational *));
static Event *change_to_norm_ctrl P((Event *));
static void  flush_event P((Event **, Track *, Rational *, int));
static void  generate_line_ctrl P((Track *, Event **, Event *, 
				   Rational *, Rational *));
static void  create_new_point P((Event **, Rational *, PmmlFloat, Event *));
static void  generate_curve_ctrl P((Track *, Event **, Event *,
				    Rational *, Rational *));
static void  calc_slope P((float *, float *, int, double, double, float *));

#define EPRI(ep) \
    (((ep)->type != E_NoteOff) ? epri[(ep)->type & ETYPE] : \
     (((ep)->flags & IrregularNoteOff) ? PRI_NOTEON : PRI_NOTEOFF))

#define EV_LESS(ep1,ep2) \
    ((ev_less_tmp = rcomp(&(ep1)->time, &(ep2)->time)) < 0 || \
     (sort_by_priority && ev_less_tmp == 0 && (EPRI(ep1) < EPRI(ep2))))

#define abs(x)  ((x)>=0 ? (x) : -(x))


/*********************************************************************
 * Initializer
 *********************************************************************/
void
init_output()
{
    int  i;

    e_setinclusive(is_time_event, E_GCtrl, E_Meta);
    e_bitset(is_time_event, E_OrgTempo);
    e_bitset(is_time_event, E_Seqno);
    e_bitset(is_time_event, E_Smpte);
    e_bitset(is_time_event, E_TimeSig);

    e_bitset(is_mark_event, E_Marker);
    e_bitset(is_mark_event, E_TrkName);
    e_bitset(is_mark_event, E_TimeSig);

    e_setinclusive(per_ch_event, E_Ctrl, E_GCtrl);
    e_bitset(per_ch_event, E_NoteOff);
    e_bitset(per_ch_event, E_NoteOn);
    e_bitset(per_ch_event, E_Deleted);

    e_bitset(is_t0_event, E_Tempo);
    e_bitset(is_t0_event, E_OrgTempo);
    e_bitset(is_t0_event, E_RTempo);
    e_bitset(is_t0_event, E_Seqno);
    e_bitset(is_t0_event, E_TrkName);
    e_bitset(is_t0_event, E_Smpte);
    e_bitset(is_t0_event, E_TimeSig);
    e_bitset(is_t0_event, E_KeySig);

    /* set event priority */
    for( i = 0; i < E_Meta; i++ )  epri[i] = PRI_CTRL;
    for( i = E_Meta; i < E_NoteExcl; i++ )  epri[i] = PRI_META;
    epri[E_Excl] = epri[E_Arbit] = PRI_CTRL;
    epri[E_TrkEnd] = epri[E_LoadEnd] = PRI_END;
    epri[E_NoteOn] = PRI_NOTEON;
    epri[E_NoteOff] = epri[E_Deleted] = PRI_NOTEOFF;
    epri[E_Kp] = epri[E_Cpr] = PRI_AFTOUCH;

    init_bi_effs();
}

/*
 * install built-in effectors to the dictionary
 */
static void
init_bi_effs()
{
    int  i;
    struct bi_effs  *p;
    DicEnt  *dp;
    EffClass  *ecp;

    for( i = 0, p = bi_effs_tab;
	 i < (int)(sizeof(bi_effs_tab)/sizeof(struct bi_effs)); i++, p++ ) {

	if( !(ecp = (EffClass *) malloc(sizeof(EffClass))) ) {
	    err_nomem("init_bi_effs");
	}
	ecp->type = EC_BuiltIn;
	ecp->flags = p->flags;
	ecp->instances = NULL;
	ecp->arg_spec = p->arg_spec;
	ecp->u.b.init = p->init;
	ecp->u.b.detach = p->detach;
	ecp->u.b.action = p->action;
	ecp->u.b.wrap = p->wrap;

	if( !(dp = (DicEnt *) malloc(sizeof(DicEnt))) ) {
	    err_nomem("init_bi_effs");
	}
	dp->type = D_EffClass;
	if( !(dp->name = strdup(p->name)) ) {
	    err_nomem("init_bi_effs");
	}
	ecp->dp = dp;
	dp->hash = hash_func(p->name);
	dp->scope.c = root_thd;
	dp->active = 0;
	dp->dic_eclass = ecp;
	insert_dict(dp);
    }
}

/*********************************************************************
 * Event buffer related routines
 *********************************************************************/
/*
 * insert an event to event buffer
 */
void
insert_event(bp, ep)
Track  *bp;
Event  *ep;
{
    EventSeq  *sp;
    long  ev_less_tmp;

    if( ! bp->eseq || EV_LESS(ep, bp->last_ep) ) {
	/* create a new event sequence */
	if( !(sp = (EventSeq *) malloc(sizeof(EventSeq))) ) {
	    err_nomem("insert_event");
	}
	sp->events = ep;

	if( ! bp->eseq ) {
	    bp->eseq = sp;
	} else {
	    bp->last_sp->next = sp;
	}
	sp->next = NULL;
	bp->last_sp = sp;
    } else {
	/* append the event to the event sequence */
	bp->last_ep->next = ep;
    }
   
    ep->next = NULL;
    bp->last_ep = ep;
}

/*
 * insert a time-ordered event list to event buffer
 */
void
insert_evlist(bp, ep)
Track  *bp;
Event  *ep;
{
    EventSeq  *sp;
    long  ev_less_tmp;

    if( !ep )  return;

    if( ! bp->eseq || EV_LESS(ep, bp->last_ep) ) {
	/* create a new event sequence */
	if( !(sp = (EventSeq *) malloc(sizeof(EventSeq))) ) {
	    err_nomem("insert_evlist");
	}
	sp->events = ep;

	if( ! bp->eseq ) {
	    bp->eseq = sp;
	} else {
	    bp->last_sp->next = sp;
	}
	sp->next = NULL;
	bp->last_sp = sp;
    } else {
	/* append the event to the event sequence */
	bp->last_ep->next = ep;
    }
   
    while( ep->next != NULL ) {
	ep = ep->next;
    }
    bp->last_ep = ep;
}

/*
 * make a clone of an event
 */
Event *
copy_event(ep)
Event  *ep;
{
    Event  *ecopy;

    event_alloc(ecopy);
    memcpy(ecopy, ep, sizeof(Event));
    if( ep->type & CTYPE ) {
	if( !(ecopy->u.cp = (ContEvent *) malloc(sizeof(ContEvent))) ) {
	    err_nomem("copy_event");
	}
	memcpy(ecopy->u.cp, ep->u.cp, sizeof(ContEvent));
    } else if( ep->type < E_Meta ) {
	copy_object(&ecopy->u.obj, &ep->u.obj);
    } else if( ep->type < E_NoteOff ) {
	if( !(ecopy->ev_data = (uchar *) malloc(ep->ev_len)) ) {
	    err_nomem("copy_event");
	} 
	memcpy(ecopy->ev_data, ep->ev_data, ep->ev_len);
    }
    return ecopy;
}

/*
 * sort event by time 
 *   Events are sorted by the merge sort algorithm. This usually
 *   takes O(NlogN) time where N is the number of events. However,
 *   since the events in this application are already sorted
 *   partially, the time complexity of this algorithm is O(MSlogS)
 *   where S is the number of pre-sorted event sequences and M is
 *   the avarage number of events in each event sequence (i.e. N=M*S).
 */
void
sort_event(bp)
Track  *bp;
{
    Event  *a, *b;
    Event  **link;
    EventSeq  *sp, *save;
    long  ev_less_tmp;

    if( ! bp->eseq ) {
	return;
    }

    while( bp->eseq->next ) { /* repeat until only 1 sequence exists */
	for( sp = bp->eseq; sp && sp->next; ) {
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
		} else if( EV_LESS(b, a) ) {
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
 * free the memory used by a track
 */
void
destroy_track(bp)
Track  *bp;
{
    EventSeq  *sp, *sp_next;
    Event  *ep, *ep_next;

    for( sp = bp->eseq; sp != NULL; sp = sp_next ) {
	for( ep = sp->events; ep != NULL; ep = ep_next ) {
	    ep_next = ep->next;
	    event_free(ep);
	}
	sp_next = sp->next;
	free(sp);
    }
    bp->eseq = NULL;
}

/*
 * increase the number of tracks
 */ 
void
increase_ntrks(ebuf, new_ntrks)
MTracks  *ebuf;
int  new_ntrks;
{
    if( ebuf->ntrks == 0 ) {
	ebuf->trks = (Track *) malloc(sizeof(Track) * new_ntrks);
    } else {
	ebuf->trks = (Track *) realloc(ebuf->trks, sizeof(Track) * new_ntrks);
    }
    if( !ebuf->trks ) {
	err_nomem("increase_ntrks");
    }
    memset(&ebuf->trks[ebuf->ntrks], 0, 
	   (new_ntrks - ebuf->ntrks) * sizeof(Track));
    ebuf->ntrks = new_ntrks;
}


/*
 * output an event (or event list) to appropriate event buffer 
 * 
 *   In case of event list, all the events in the list must have the same 
 *   track number, channel number, and event type and be time-orderd.
 *   The list must not contain meta events.
 *
 *   Make sure that the 'next' element of the last event is NULL!
 */
void
output_event(ep)
Event  *ep;
{
    int  track;
    ChanSet  cmask = 0;
    int  idx;
    unsigned long  mask;
    EEBV  eebv;
    EffInst  *eip;
    Event  *ep1;
    
    track = ep->track;
    idx = e_idx(ep->type);
    mask = e_mask(ep->type);
    if( per_ch_event[idx] & mask ) {
	cmask = (1 << ep->ch);
    }
    eebv = cur_thd->eebv;

    for( eip = cur_thd->effs; eip != NULL; eip = eip->next ) {
	if((eebv & 1) && !(eip->ethru[idx] & mask) && !(eip->cthru & cmask)) {

	    for( ep1 = ep; ep1 != NULL; ep1 = ep1->next ) {
		ep1->eebv = eebv >> 1;
	    }

	    if( (eip->flags & EF_MoveTimeEv) &&
	       (is_time_event[idx] & mask) ) {
		ep->track = track = 0;
	    }
	    if( track >= eip->ebuf.ntrks ) {
		increase_ntrks(&eip->ebuf, track + 1);
	    }
	    insert_evlist(&eip->ebuf.trks[track], ep);

	    if( (eip->flags & EF_CollectMarker) &&
	       (is_mark_event[idx] & mask) ) {
		/* copy the meta event and insert it to "marks" track */
		insert_evlist(&eip->ebuf.marks, copy_event(ep));
	    }
		    
	    return;
	}
	eebv >>= 1;
    }

    /* No destination!  Some warnnings? */
    for( ; ep != NULL; ep = ep1 ) {
	ep1 = ep->next;
	event_free(ep);
    }
}

/*
 * set EventSet bits where  n1 <= etype < n2
 */
void
e_setinclusive(evset, n1, n2)
EventSet  evset;
int  n1, n2;
{
    int  i, i0;
    long  d, n1m;
    
    if( n1 >= n2 ) return;

    i0 = n1 >> 5;
    d = evset[i0];
    for( i = i0; i < n2 >> 5; i++ ) {
	evset[i] |= ~0L;
    }
    if( n2 & 0x1f ) {
	evset[i] |= (1L << (n2 & 0x1f)) - 1;
    }
    n1m = (1L << (n1 & 0x1f)) - 1;
    evset[i0] = (evset[i0] & ~n1m) | (d & n1m);
}

/*********************************************************************
 * Post processing routines
 *********************************************************************/
/*
 * apply ExpandCtrl, TimeRange, Retrigger, CutAfterEnd operations
 * to an event buffer. 
 *
 * Limitations
 *   - TimeRange can not be applied without ExpandCtrl
 *   - If more than 256 note_on events are plied to the same note,
 *     the Retrigger operation does not work correctly (note_off events are
 *     generated earlier than expected).
 *
 * Caution
 *   - Be sure to check begin_time < end_time before calling
 */ 
void
post_process(ebuf, oprs, begin_time, end_time, max_time)
MTracks  *ebuf;
int  oprs;		/* list of operations (bit vector) */
Rational *begin_time;	/* begin time of playing (if negative, blank bars 
			   are inserted at the beginning of song.) */
Rational *end_time;	/* end time of playing */
Rational *max_time;	/* (Output) the time length of song (negative 
			   if begin_time is later than the song end).
			   This is equal to the maximum time among 
			   all the events measured from begin_time.
			   This pointer may be NULL. */ 
{
    int  tk;
    Track  *bp, tmp;
    Event  *ep, *ep1, *epnext, **t;
    int  key, ch, etype, i;
    /*
     * `output' indicates the output status: 0 if time < begin_time
     *		                             1 if time == begin_time
     *                                       2 if time > begin_time 
     *		                             3 if time >= end_time 
     * The reason why state 1 is needed is that pending notes must be 
     * outputed just AFTER the time exceeds begin_time (otherwise, pending 
     * notes ending at begin_time will be undesirably retriggerd), while 
     * meta events should be outputed from begin_time inclusive. 
     */
    int  output;  
    Rational  cur_time;
    Rational  term_time;	/* actual ending time 
				   ( < end_time if "end" cmd exists ) */
    uchar  (*nnotes)[16];	/* no. of playing notes per channel per key */
    Event  *(*note_tbl)[16];	/* the previous event for note-on */
    Event  *(*ctrl_tbl)[16];	/* list of previous events for ctrl 0-95 */
    Event  *(*vctrl_tbl)[16];	/* list of previous events for ctrl 128-191 */
    Event  **gctrl_tbl;		/* list of pervious events for ctrl 192-255 */
    Event  *(*at_tbl)[16];	/* list of pervious events for after touch */
    Event  *timesig, *keysig;
    uchar  *n;

    nnotes    = (uchar (*)[16])  malloc(128 * 16 * sizeof(uchar));
    note_tbl  = (Event *(*)[16]) malloc(128 * 16 * sizeof(Event *));
    ctrl_tbl  = (Event *(*)[16]) malloc((E_ModeMsg - E_Ctrl) 
					* 16 * sizeof(Event *));
    vctrl_tbl = (Event *(*)[16]) malloc((E_GCtrl - E_VCtrl) 
					* 16 * sizeof(Event *));
    gctrl_tbl = (Event **)       malloc((E_Meta - E_GCtrl)
					* 64 * sizeof(Event *));
    at_tbl    = (Event *(*)[16]) malloc(128 * 16 * sizeof(Event *));
    if( !nnotes || !note_tbl || !ctrl_tbl || 
       !vctrl_tbl || !gctrl_tbl || !at_tbl ) {
	err_nomem("post_process");
    }
    
    if( !(oprs & EF_TimeRange) ) {
	begin_time = &r_zero;
	end_time = &r_max;
    }
    if( max_time )  *max_time = r_zero;

    for( tk = 0; tk < ebuf->ntrks; tk++ ) {
	bp = &ebuf->trks[tk];
	if( !bp->eseq ) continue;

	/* clear all the tables */
	memset(nnotes,    0, 128 * 16 * sizeof(uchar));
	memset(note_tbl,  0, 128 * 16 * sizeof(Event *));
	memset(ctrl_tbl,  0, (E_ModeMsg - E_Ctrl) * 16 * sizeof(Event *));
	memset(vctrl_tbl, 0, (E_GCtrl - E_VCtrl) * 16 * sizeof(Event *));
	memset(gctrl_tbl, 0, (E_Meta - E_GCtrl) * sizeof(Event *));
	memset(at_tbl,    0, 128 * 16 * sizeof(Event *));
	timesig = keysig = NULL;

	/* initialize temporary track buffer */
	memset(&tmp, 0, sizeof(Track));

	/* sort events */
	sort_event(bp);

	/* scan all the events in the current track */
	output = (oprs & EF_TimeRange) ? 0 : 2;
	term_time = *end_time;
	for( ep = bp->eseq->events; ep != NULL; ep = epnext ) {
	    epnext = ep->next;
	    cur_time = ep->time;

	    /* check the playing time range */
	    if( output == 0 && requal(&cur_time, begin_time) ) {
		output = 1;
	    }
	    if( output < 2  && rgreater(&cur_time, begin_time) ) {
		/* playing from halfway -- trun on all the playing notes */
		for( ch = 0; ch < 16; ch++ ) {
		    for( key = 0; key < 128; key++ ) {
			ep1 = note_tbl[key][ch];
			if( nnotes[key][ch] ) {
			    insert_event(&tmp, ep1);
			} else {
			    if( ep1 ) event_free(ep1);
			}
		    }
		}
		/* output pending meta events */
		if( timesig )  insert_event(&tmp, timesig);
		if( keysig )  insert_event(&tmp, keysig);
		output = 2;
	    }
	    if( output == 2 && rgreatereq(&cur_time, end_time) ) {
		/* turn-off all the playing notes */
		for( ch = 0; ch < 16; ch++ ) {
		    sim_all_notes_off(&tmp, &term_time, ch, nnotes, note_tbl);
		}
		output = 3;
	    }
	    if( output <= 2 && 
	       ep->type == E_TrkEnd && (oprs & EF_CutAfterEnd) ) {
		term_time = cur_time;
		if( output == 2 ) {
		    /* turn-off all the playing notes */
		    for( ch = 0; ch < 16; ch++ ) {
			sim_all_notes_off(&tmp,&term_time,ch,nnotes,note_tbl);
		    }
		}
		insert_event(&tmp, ep);
		output = 3;

		continue;
	    }
	   
	    /* process per event type */
	    switch( (etype = ep->type & ETYPE) ) {
	    case E_NoteOn:
		if( output == 3 ) {
		    event_free(ep);
		    break;
		}
		n = &nnotes[ep->note][ep->ch];
		t = &note_tbl[ep->note][ep->ch];
		if( output == 2 ) {
		    if( (oprs & EF_Retrigger) && *n > 0 ) {
			/* retrigger the note (insert note-off) */
			event_alloc(ep1);
			ep1->type = E_NoteOff;
			ep1->time = cur_time;
			ep1->eebv = (*t)->eebv;
			ep1->track = ep->track;
			ep1->ch = ep->ch;
			ep1->note = ep->note;
			ep1->ev_veloc = -1;
			ep1->ev_partner = *t;
			(*t)->ev_partner = ep1;
			ep1->flags = requal(&cur_time, &(*t)->time) ? 
			    IrregularNoteOff : 0;
			insert_event(&tmp, ep1);
		    }
		    insert_event(&tmp, ep);
		} else {
		    if( *t != NULL )  event_free(*t);
		}
		if( *n < 255 )  (*n)++;
		*t = ep;
		break;

	    case E_NoteOff:
		n = &nnotes[ep->note][ep->ch];
		t = &note_tbl[ep->note][ep->ch];
		if( output == 2 ) {
		    if( oprs & EF_Retrigger ) {
			if( *n == 1 ) {
			    ep->ev_partner = *t;
			    (*t)->ev_partner = ep;
			    ep->flags = requal(&cur_time, &(*t)->time) ? 
				IrregularNoteOff : 0;
			    insert_event(&tmp, ep);
			} else {
			    ep->ev_partner = NULL;
			    event_free(ep);
			}
		    } else {
			insert_event(&tmp, ep);
		    }
		} else {
		    event_free(ep);
		}
		if( *n > 0 )  --(*n);
		break;

	    case E_AllNotesOff:
		ch = ep->ch;
		if( output == 2 ) {
		    if( oprs & EF_Retrigger ) {
			sim_all_notes_off(&tmp, &cur_time, ch, 
					  nnotes, note_tbl);
			event_free(ep);
		    } else {
			insert_event(&tmp, ep);
		    }
		} else {
		    event_free(ep);
		}
		for( key = 0; key < 128; key++ ) {
		    nnotes[key][ch] = 0;
		}
		break;

	    case E_ResetAllCtrl:
		ch = ep->ch;
		for( i = 0; i < E_ModeMsg; i++ ) {
		    if( ctrl_tbl[i][ch] ) {
			flush_event(&ctrl_tbl[i][ch], &tmp, begin_time, 0);
		    }
		}
		if( output != 3 )  insert_event(&tmp, ep);
		else  event_free(ep);
		break;

	    case E_RPNL:
	    case E_RPNH:
	    case E_NRPNL:
	    case E_NRPNH:
		ch = ep->ch;
		if( ctrl_tbl[E_DataEntL][ch] ) {
		    flush_event(&ctrl_tbl[E_DataEntL][ch], &tmp, begin_time,0);
		}
		if( ctrl_tbl[E_DataEntH][ch] ) {
		    flush_event(&ctrl_tbl[E_DataEntH][ch], &tmp, begin_time,0);
		}
		if( output != 3 )  insert_event(&tmp, ep);
		else  event_free(ep);
		break;

	    case E_Text:
		/* Basically, text events are outputed only in the time range.
		   However, if the event time is 0, it is outputed anyway,
		   because such an event is normally used as a song title. */
		if( output == 1 || output == 2 || rzerop(&cur_time) ) {
		    insert_event(&tmp, ep);
		} else {
		    event_free(ep);
		}
		break;

	    case E_Lyric:
	    case E_Marker:
	    case E_Cue:
		if( output == 1 || output == 2 ) {
		    insert_event(&tmp, ep);
		} else {
		    event_free(ep);
		}
		break;

	    case E_TimeSig:
		if( output == 3 ) {
		    event_free(ep);
		} else {
		    if( output == 2 ) {
			insert_event(&tmp, ep);
		    } else {
			if( timesig != NULL )  event_free(timesig);
		    }
		    timesig = ep;
		}
		break;

	    case E_KeySig:
		if( output == 3 ) {
		    event_free(ep);
		} else {
		    if( output == 2 ) {
			insert_event(&tmp, ep);
		    } else {
			if( keysig != NULL )  event_free(keysig);
		    }
		    keysig = ep;
		}
		break;

	    default:
		if( (oprs & EF_ExpandCtrl) 
		   && (etype < E_ModeMsg || (etype >= E_VCtrl && 
					     etype < E_Meta)) ) {
		    if( etype >= E_GCtrl ) {
			t = &gctrl_tbl[etype - E_GCtrl];
		    } else if( etype == E_Kp ) {
			t = &at_tbl[ep->note][ep->ch];
		    } else if( etype >= E_VCtrl ) {
			t = &vctrl_tbl[etype - E_VCtrl][ep->ch];
		    } else {
			t = &ctrl_tbl[etype][ep->ch];
		    }
		    if( (ep->type & CTYPE) == 0 && 
		       ep->u.obj.o_type == O_INT &&
		       ep->u.obj.o_val == LASTVAL ) {
			for( ep1 = *t; ep1 && (ep1->type & CTYPE) == E_Pt; 
			    ep1 = ep1->next );
			if( !ep1 ) {
			    warn(0L, "LASTVAL is used without previous value (time=%s, func_no=%d)",
				 "1つ前のイベントが無いのにLASTVALが使われています (time=%s, func_no=%d)",
				 rstring(&ep->time), ep->type & ETYPE);
			    ep->u.obj.o_val = 0;
			} else {
			    copy_object(&ep->u.obj, &ep1->u.obj);
			}
		    }

		    if( output == 0 ) {
			if( (ep->type & CTYPE) == E_Pt ) { 
			    /* insert to the top of the list */
			    ep->next = *t;
			    *t = ep;
			} else {
			    /* free all the events in the list */
			    for( ; *t; *t = ep1 ) {
				ep1 = (*t)->next;
				event_free(*t);
			    }
			    /* replace the list with the event */
			    change_to_norm_ctrl(ep);
			    ep->next = NULL;
			    *t = ep; 
			}
		    } else {  /* output == 1, 2, or 3 */
			if( (ep->type & CTYPE) == 0 ) {
			    /* normal ctrl change */
			    flush_event(t, &tmp, begin_time, output == 1);
			    if( output < 3 ) {
				insert_event(&tmp, ep);
				*t = ep;
			    }
			} else if( (ep->type & CTYPE) == E_Pt ) {
			    if( output < 3 || *t != NULL ) {
				/* insert to the top of the list */
				ep->next = *t;
				*t = ep;
			    }
			} else {
			    if( output < 3 || *t != NULL ) {
				if( (ep->type & CTYPE) == E_To ) { 
				    generate_line_ctrl(&tmp, t, ep, 
						       begin_time, &term_time);
				} else { /* type == E_CTo */
				    generate_curve_ctrl(&tmp, t, ep, 
							begin_time,&term_time);
				}
				if( rless(&(*t)->time, begin_time) ) {
				    event_free(*t);
				}
				if( output < 3 )  *t = ep;
				else  *t = NULL;
			    } 
			}
			/* The free of ctrl events after end_time is omitted */
		    }
		} else {
		    /* non-special meta event & control change 96-127 */
		    if( output != 3 ) {
			insert_event(&tmp, ep);
		    } else {
			event_free(ep);
		    }
		}
	    }
	}
	
	/* turn-off all the playing notes */
	if( (oprs & EF_Retrigger) && output == 2 ) {
	    for( ch = 0; ch < 16; ch++ ) {
		sim_all_notes_off(&tmp, &cur_time, ch, nnotes, note_tbl);
	    }
	}

	/* output all the pending control changes before begin_time */
	for( i = 0; i < E_ModeMsg; i++ ) {
	    for( ch = 0; ch < 16; ch++ ) {
		if( ctrl_tbl[i][ch] ) 
		    flush_event(&ctrl_tbl[i][ch], &tmp, begin_time, 0);
	    }
	}
	for( i = 0; i < 128; i++ ) {
	    for( ch = 0; ch < 16; ch++ ) {
		if( at_tbl[i][ch] ) 
		    flush_event(&at_tbl[i][ch], &tmp, begin_time, 0);
	    }
	}
	for( i = 0; i < (E_GCtrl - E_VCtrl); i++ ) {
	    for( ch = 0; ch < 16; ch++ ) {
		if( vctrl_tbl[i][ch] )
		    flush_event(&vctrl_tbl[i][ch], &tmp, begin_time, 0);
	    }
	}
	for( i = 0; i < (E_Meta - E_GCtrl); i++ ) {
	    if( gctrl_tbl[i] ) 
		flush_event(&gctrl_tbl[i], &tmp, begin_time, 0);
	}

	/* output pending meta events */
	if( output < 2 ) {
	    if( timesig )  insert_event(&tmp, timesig);
	    if( keysig )  insert_event(&tmp, keysig);
	}

	/* shift event time */
	if( oprs & EF_TimeRange )  adjust_time(&tmp, begin_time);

	/* replace the original track buffer by the temporary track buffer */
	free(bp->eseq);
	bp->eseq = tmp.eseq;

	/* update max_time */
	if( output < 3 )  term_time = cur_time;
	if( max_time ) {
	    if( rgreater(&term_time, max_time) )  *max_time = term_time;
	}
    }

    free(nnotes);
    free(note_tbl);
    free(ctrl_tbl);
    free(vctrl_tbl);
    free(gctrl_tbl);
    free(at_tbl);
    
    if( max_time )  rsub(max_time, begin_time, max_time);
}

/*
 * simulate all-notes-off by note-off events
 */
static void
sim_all_notes_off(dst, cur_time, ch, nnotes, note_tbl)
Track  *dst;
Rational  *cur_time;
int  ch;
uchar  (*nnotes)[16];
Event  *(*note_tbl)[16];
{
    int  key;
    Event  *ep, *e_on;

    for( key = 0; key < 128; key++ ) {
	if( nnotes[key][ch] > 0 ) {
	    e_on = note_tbl[key][ch];
	    event_alloc(ep);
	    ep->time = *cur_time;
	    ep->eebv = e_on->eebv;
	    ep->track = e_on->track;
	    ep->ch = e_on->ch;
	    ep->note = e_on->note;
	    ep->type = E_NoteOff;
	    ep->ev_veloc = -1;
	    ep->ev_partner = e_on;
	    e_on->ev_partner = ep;
	    ep->flags = requal(cur_time, &e_on->time) ? IrregularNoteOff : 0;
	    insert_event(dst, ep);
	}
    }
}

/*
 * shift event time by begin_time
 */
static void
adjust_time(bp, begin_time)
Track  *bp;
Rational  *begin_time;
{
    Track  tmp;	    /* temporary track buffer: Since the order of events
		       may change, event sequence must be reconstructed. */
    EventSeq  *sp, *sp_next;
    Event  *ep, *ep_next;

    memset(&tmp, 0, sizeof(Track));

    for( sp = bp->eseq; sp != NULL; sp = sp_next ) {
	sp_next = sp->next;
	for( ep = sp->events; ep != NULL; ep = ep_next ) {
	    ep_next = ep->next;
	    /* If begin_time is negative, some meta events at time 0 
	       should not be time-shifted. */
	    if( !(begin_time->intg < 0 && rzerop(&ep->time) && 
		  e_isset(is_t0_event, ep->type)) ) {
		rsub(&ep->time, begin_time, &ep->time);
		if( ep->time.intg < 0 )  ep->time = r_zero;
	    }
	    insert_event(&tmp, ep);
	}
	free(sp);
    }

    /* replace the original track buffer by the temporary track buffer */
    bp->eseq = tmp.eseq;
}

/*
 * change a control change event to non-continuous one
 *   In the expansion of continuous events, an event on a grid point is
 *   changed to non-continuous one, rather than freeing it and reallocating 
 *   a new non-continuous event.
 */
static Event *
change_to_norm_ctrl(ep)
Event  *ep;
{
    ContEvent  *cp;

    if( ep->type & CTYPE ) {
	ep->type &= ~CTYPE;
	cp = ep->u.cp;
	ep->u.obj.fpval = cp->cval; 
	free(cp);
    }
    return ep;
}

/*
 * output a pending control-change event if its time is before begin_time
 */
static void
flush_event(t, bp, begin_time, no_output)
Event  **t;		/* list of previous events */
Track  *bp;		/* destination track */
Rational  *begin_time; 
int  no_output;
{
    Event  *ep1;

    /* warn if there are "pt" events in the list */
    while( *t && ((*t)->type & CTYPE) == E_Pt ) {
	warn(0L, "Floating ctrl_pt (time=%s, func_no=%d)", 
	     "宙に浮いた ctrl_pt があります (time=%s, func_no=%d)", 
	     rstring(&(*t)->time), (*t)->type & ETYPE);
	ep1 = (*t)->next;
	event_free(*t);
	*t = ep1;
    }
    /* output event before begin_time if it exists */
    if( *t && rless(&(*t)->time, begin_time) ) {
	if( no_output ) {
	    event_free(*t);
	} else {
	    insert_event(bp, *t);
	}
    }
    *t = NULL;
}

/*
 * generate control change events according to connected line segments
 *   - lastpt is the "ctrl_to" event 
 *   - ptlist is the list of "ctrl_pt" and "ctrl" events in the reverse time
 *     order
 *   - the "ctrl_pt" events in the ptlist are deleted in this function
 *   - lastpt is changed to a normal control change
 */
static void
generate_line_ctrl(bp, ptlist, lastpt, begin_time, end_time)
Track  *bp;
Event  **ptlist, *lastpt;
Rational *begin_time, *end_time;
{
    Event  *evlist, *ep1, *ep2;
    Rational  step;
    float  th;
    PmmlFloat  val1, val2;
    float  v, ta, tb, tx;
    Rational  t, *t1, *t2;
    
    step = lastpt->u.cp->tmstep;
    th = lastpt->u.cp->thres;
    evlist = NULL;

    /* generate events backward */
    for(ep2 = lastpt; (ep2->type & CTYPE) != 0; ep2 = ep1) {
	/* output the last or "pt" events */
	if( (ep2->type & CTYPE) == E_Pt ) {
	    /* delete from ptlist */
	    *ptlist = ep2->next;
	}
	change_to_norm_ctrl(ep2);
	if( rgreatereq(&ep2->time,begin_time) && rless(&ep2->time,end_time) ) {
	    ep2->next = evlist;
	    evlist = ep2;
	    /* The free of out-ranged events is omitted */
	}
	
	/* get the begging point to ep1 */
	if( (ep1 = *ptlist) == NULL ) {
	    error(0L, "ctrl_to without beginning point (time=%s, func_no=%d)",
		  "ctrl_to に開始点がありません (time=%s, func_no=%d)",
		  rstring(&lastpt->time), lastpt->type & ETYPE);
	}

	/* determine val1 and val2 */
	if( (ep1->type & CTYPE) == E_Pt ) {
	    val1 = ep1->u.cp->cval;
	} else {  /* CTYPE == 0 */
	    if( !isnumber(ep1->u.obj.o_type) ) {
		error(0L, "ctrl_to: Inappropriate type of control value at the beginning point (time=%s, func_no=%d)",
		      "ctrl_to: 開始点のコントロール値の型が不適当です (time=%s, func_no=%d)",
		      rstring(&ep1->time), ep1->type);
	    }
	    conv_to_float(&ep1->u.obj, &val1);
	}
	val2 = ep2->u.obj.fpval;

	/* generate events between (ep1->time,val1) and (ep2->time,val2) */
	t1 = &ep1->time;
	if( rless(t1, begin_time) )  t1 = begin_time;
	t2 = &ep2->time;
	if( rgreater(t2, end_time) )  t2 = end_time;
	if( rgreatereq(t2, t1) ) {
	    ta = rational_to_float(&ep1->time);
	    tb = rational_to_float(&ep2->time) - ta; 

	    rsub(t2, &step, &t);  /* t = t2 - step */
	    while( rgreater(&t, t1) ) {
		tx = rational_to_float(&t);
		v = val1 + (tx - ta) / tb * (val2 - val1);
		if( !evlist || abs(evlist->u.obj.fpval - v) >= th ) {
		    create_new_point(&evlist, &t, v, lastpt);
		}
		rsub(&t, &step, &t);  /* t -= step */
	    }
	    if( t1 != &ep1->time ) {  /* t1 must be begin_time */
		tx = rational_to_float(t1);
		create_new_point(&evlist, &ep1->time,
				 val1 + (tx - ta) / tb * (val2 - val1), 
				 lastpt);
	    }
	}
    }

    insert_evlist(bp, evlist);
}

static void
create_new_point(evlistp, time, val, ref_ev)
Event  **evlistp;
Rational  *time;
PmmlFloat  val;
Event  *ref_ev;
{
    Event  *ep;

    event_alloc(ep);
    ep->time = *time;
    ep->eebv = ref_ev->eebv;
    ep->type = ref_ev->type;
    ep->track = ref_ev->track;
    ep->ch = ref_ev->ch;
    ep->note = ref_ev->note;
    ep->u.obj.fpval = val;
    ep->next = *evlistp;
    *evlistp = ep;
}

/*
 * generate control change events according to a piecewise polynominal curve
 * which keeps 1st order continuity
 *   - lastpt is the "ctrl_cto" event 
 *   - ptlist is the list of "ctrl_pt" and "ctrl" events in the reverse time
 *     order
 *   - the "ctrl_pt" events in the ptlist are deleted in this function
 *   - lastpt is changed to a normal control change
 */
static void
generate_curve_ctrl(bp, ptlist, lastpt, begin_time, end_time)
Track  *bp;
Event  **ptlist, *lastpt;
Rational  *begin_time, *end_time;
{
    Event  *evlist, *ep, *firstpt;
    Rational  step;
    float  th, slope1, slope2;
    int  ndata, i;
    float  *x, *y, *s;
    Rational  t, *t1, *t2;
    PmmlFloat  val1;
    
    step = lastpt->u.cp->tmstep;
    th = lastpt->u.cp->thres;
    slope1 = lastpt->u.cp->slope1;
    slope2 = lastpt->u.cp->slope2;
    
    /* conut number of data */
    ndata = 2;
    for( ep = *ptlist; ep && (ep->type & CTYPE) == E_Pt; ep = ep->next ) {
	ndata++;
    }
    if( !ep ) {
	error(0L, 
	      "ctrl_cto without beginning point (time=%s, func_no=%d)",
	      "ctrl_cto に開始点がありません (time=%s, func_no=%d)",
	      rstring(&lastpt->time), lastpt->type & ETYPE);
    }

    /* allocate memory */
    x = (float *) malloc( ndata * sizeof(float) );
    y = (float *) malloc( ndata * sizeof(float) );
    s = (float *) malloc( ndata * sizeof(float) );
    if( !x || !y || !s ) {
	err_nomem("generate_curve_ctrl");
    }
    
    /* obatin data values */
    i = ndata - 1;
    change_to_norm_ctrl(lastpt);
    x[i] = rational_to_float(&lastpt->time);
    y[i] = lastpt->u.obj.fpval;
    while( ((*ptlist)->type & CTYPE) == E_Pt ) {
	x[--i] = rational_to_float(&(*ptlist)->time);
	y[i] = (*ptlist)->u.cp->cval;
	ep = (*ptlist)->next;
	event_free(*ptlist);
	*ptlist = ep;
    }
    firstpt = *ptlist;
    if( !isnumber(firstpt->u.obj.o_type) ) {
	error(0L, "ctrl_cto: Inappropriate type of control value at the beginning point (time=%s, func_no=%d)",
	      "ctrl_cto: 開始点のコントロール値の型が不適当です (time=%s, func_no=%d)",
	      rstring(&firstpt->time), firstpt->type);
    }
    x[0] = rational_to_float(&firstpt->time);
    conv_to_float(&firstpt->u.obj, &val1);
    y[0] = val1;

    /* check zero x-width */
    for(i = 0; i < ndata - 1; i++ ) {
	if( x[i] == x[i+1] ) {
	    error(0L, "ctrl_cto: More than one control point exists at the same time (time=%s, func_no=%d)",
		  "ctrl_cto: 同一時刻に複数の制御点があります (time=%s, func_no=%d)",
		  rstring(&lastpt->time), lastpt->type & ETYPE);
	}
    }
	    
    /* calculate slope at each point */
    calc_slope(x, y, ndata, (y[1] - y[0]) / (x[1] - x[0]) * slope1,
	       (y[ndata-1] - y[ndata-2]) / (x[ndata-1] - x[ndata-2]) * slope2,
	       s);

    /* generate events at the end point */
    if( rless(&lastpt->time, end_time) ) {
	evlist = lastpt;
	lastpt->next = NULL;
    } else {
	evlist = NULL;
    }

    /* generate events with cubic interpolation */
    t1 = &firstpt->time;
    if( rless(t1, begin_time) )  t1 = begin_time;
    t2 = &lastpt->time;
    if( rgreater(t2, end_time) )  t2 = end_time;
    if( rgreatereq(t2, t1) ) {
	float  h, m, p2, p3, a, b;
	float  tx, v;

	i = ndata - 1;
	rsub(t2, &step, &t);  /* t = t2 - step */
	while( rgreater(&t, t1) ) {
	    tx = rational_to_float(&t);
	    if( tx < x[i] ) {
		while( tx < x[i] ) --i;
		h = x[i+1] - x[i];
		m = (y[i+1] - y[i]) / h;
		p2 = 3 * m - 2 * s[i] - s[i+1];
		p3 = s[i+1] + s[i] - 2 * m;
	    }
	    a = tx - x[i];
	    b = a / h;
	    v = y[i] + a * (s[i] + b * (p2 + b * p3));
	    if( !evlist || a == 0.0 || abs(evlist->u.obj.fpval - v) >= th ) {
		create_new_point(&evlist, &t, v, lastpt);
	    }
	    rsub(&t, &step, &t);  /* t -= step */
	}

	if( t1 != &firstpt->time ) {  /* t1 must be begin_time */
	    tx = rational_to_float(t1);
	    for( i = ndata - 1; tx < x[i]; --i );
	    h = x[i+1] - x[i];
	    m = (y[i+1] - y[i]) / h;
	    p2 = 3 * m - 2 * s[i] - s[i+1];
	    p3 = s[i+1] + s[i] - 2 * m;
	    a = tx - x[i];
	    b = a / h;
	    v = y[i] + a * (s[i] + b * (p2 + b * p3));
	    create_new_point(&evlist, &firstpt->time, v, lastpt);
	}
    }

    insert_evlist(bp, evlist);

    /* free arrays */
    free(x);
    free(y);
    free(s);
}

/*
 * calculate slope at each point using Akima's local polynominal 
 * interpolation method
 *   See:  Akima, Hiroshi, "A new method of interpolation and smooth curve
 *   fitting based on local procedures", J. ACM, Vol. 17, pp. 589-602, 1970.
 */
static void
calc_slope(x, y, n, left_t, right_t,  t)
float x[];		/* data points (x-coordinates) */
float y[];         	/* data points (y-coordinates) */
int n;			/* number of data points */
double left_t, right_t;	/* slope values at the beginning and end */
float t[];		/* result slope */
{
    float *m;
    int  i;
    float r, l;

    m = (float *) malloc(sizeof(float) * (n+3));
    if( !m )  err_nomem("calc_slope");

    for(i = 0; i <= n-2; i++ ) {
	m[i+2] = (y[i+1] - y[i]) / (x[i+1] - x[i]);
    }
    m[0] = m[1] = left_t;
    m[n+1] = m[n+2] = right_t;

    for( i = 0; i < n; i++ ) {
	r = abs(m[i+3] - m[i+2]);
	l = abs(m[i+1] - m[i]);
	if( r + l <= 1e-30 ) {
	    t[i] = (m[i+1] + m[i+2])/2;
	} else {
	    t[i] = (r * m[i+1] + l * m[i+2]) / (r + l);
	}
    }

    free(m);
}

/*
 * make isolated note-on & note-off events into pairs
 *   - after calling this routine, events are guaranteed to be sorted.
 */
void
pair_notes(ebuf)
MTracks  *ebuf;
{
    int  tk, ch, key;
    Track  *bp;
    Event  *ep, **t, *ep1;
    Event  *(*note_tbl)[16];	/* list of pending note-on events (for 
				   contructing list, ev_partner member of 
				   note_on event is used as pointer to 
				   the next element. */
    long  npend;		/* number of pending notes */
    Event  **ctop;		/* the position where non-irregular note-off
				   events should be inserted */

    note_tbl  = (Event *(*)[16]) malloc(128 * 16 * sizeof(Event *));
    if( !note_tbl )  err_nomem("pair_notes");

    for( tk = 0; tk < ebuf->ntrks; tk++ ) {
	bp = &ebuf->trks[tk];
	if( !bp->eseq ) continue;

	/* clear the note table */
	memset(note_tbl, 0, 128 * 16 * sizeof(Event *));

	/* sort events */
	sort_event(bp);

	/* look at each event */
	ep = bp->eseq->events; 
	ctop = &bp->eseq->events;
	while( ep != NULL ) {
	    if( ep->type == E_NoteOn && !ep->ev_partner ) {
		/* put it to the top of note-on events list */
		t = &note_tbl[ep->note][ep->ch];
		ep->ev_partner = *t;
		*t = ep;
		npend++;
	    } else if( ep->type == E_NoteOff && !ep->ev_partner ) {
		t = &note_tbl[ep->note][ep->ch];
		if( *t ) { 
		    /* get the top of note-on events list 
		       and make it be the partner */
		    ep->ev_partner = *t;
		    *t = (*t)->ev_partner;
		    ep->ev_partner->ev_partner = ep;
		    npend--;
		    if( !requal(&ep->time, &ep->ev_partner->time) ) {
			Event  *ep2;

			ep->flags &= ~IrregularNoteOff;
			/* Now that the event priority has been changed, 
			   we have to move the note-off event before
			   control-change events. */
			if( sort_by_priority && *ctop != ep ) {
			    for( ep2 = *ctop; ep2->next != ep; ep2=ep2->next );
			    ep2->next = ep->next;
			    ep->next = *ctop;
			    *ctop = ep;
			    ep = ep2;
			}
		    }
		} else {
		    /* It's really an isolated note-off event. Leave it. */
		}
	    }

	    if( npend ) { /* For performance reasons, we skip the update of
			     ctop if there are no isolated note-on events. */
		if( ((*ctop)->type == E_NoteOff && 
		     !((*ctop)->flags & IrregularNoteOff)) ||
		   (ep->next && !requal(&(*ctop)->time, &ep->next->time)) ) {
		    ctop = &ep->next;
		}
	    }

	    ep = ep->next;
	}

	/* Now, note-on event list has really isolated note-on events. */
	for( key = 0; key < 128; key++ ) {
	    for( ch = 0; ch < 16; ch++ ) {
		ep = note_tbl[key][ch];
		while( ep ) {
		    ep1 = ep->ev_partner;
		    ep->ev_partner = NULL;
		    ep = ep1;
		}
	    }
	}
    }

    free(note_tbl);
}

/*********************************************************************
 * Debuggin aids
 *********************************************************************/
#ifdef DEBUG
dump_track(bp)
Track  *bp;
{
    EventSeq  *sp;
    Event  *ep;
    int  etype;

    for( sp = bp->eseq; sp != NULL; sp = sp->next ) {
	for( ep = sp->events; ep != NULL; ep = ep->next ) {
	    printf("%5d+%5d/%5d: tk=%-2d ch=%-2d  ", 
		   ep->time.intg, ep->time.num, ep->time.den, 
		   ep->track+1, ep->ch+1);
	    etype = ep->type & ETYPE;
	    switch( etype ) {
	    case E_NoteOn:
		if( ep->ev_partner ) {
		    printf("Note_on   n=%d v=%d partner.time=%s\n",
			   ep->note, ep->ev_veloc, 
			   rstring(&ep->ev_partner->time));
		} else { 
		    printf("Note_on   n=%d v=%d partner.time=none\n",
			   ep->note, ep->ev_veloc);
		}
		break;
	    case E_NoteOff:
		if( ep->ev_partner ) {
		    printf("Note_off  n=%d v=%d partner.time=%s\n",
			   ep->note, ep->ev_veloc, 
			   rstring(&ep->ev_partner->time));
		} else {
		    printf("Note_off  n=%d v=%d partner.time=none\n",
		       ep->note, ep->ev_veloc);
		}
		break;
	    case E_Excl:
		printf("Exclusive len=%d\n", ep->ev_len);
		break;
	    case E_Arbit:
		printf("Arbitrary len=%d\n", ep->ev_len);
		break;
	    case E_Deleted:
		printf("Deleted\n");
		break;
	    default:
		if( etype < E_Meta ) {
		    switch( ep->type & CTYPE ) {
		    case E_To:
			printf("Control_to  fno=%d n=%d val=%.7g tmstep=%s thres=%.4g\n",
			       etype, ep->note, ep->u.cp->cval, 
			       rstring(&ep->u.cp->tmstep), ep->u.cp->thres);
			break;
		    case E_CTo:
			printf("Control_cto  fno=%d n=%d val=%.7g tmstep=%s thres=%.4g s1=%g s2=%g\n",
			       etype, ep->note, ep->u.cp->cval, 
			       rstring(&ep->u.cp->tmstep), ep->u.cp->thres, 
			       ep->u.cp->slope1, ep->u.cp->slope2);
			break;
		    case E_Pt:
			printf("Control_pt  fno=%d n=%d val=%.7g\n",
			       etype, ep->note, ep->u.cp->cval);
			break;
		    default:
			printf("Control  fno=%d n=%d val=", etype, ep->note);
			fprint_object(stdout, &ep->u.obj, -1, -1);
			printf("\n");
		    }
		} else {
		    int  i;
		    printf("Meta  fno=%d len=%d :", etype, ep->ev_len);
		    for( i = 0; i < ep->ev_len; i++ ) {
			printf(" %02x", ep->ev_data[i]);
		    }
		    printf("\n");
		}
	    }
		
	}
	printf("\n");
    }
}
#endif
