/*
 * M2P: A SMF-to-PMML Translator
 *   
 * linefit.c: aggregate control-change events lying on lines into 
 *            continuous control-change events
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
#include "m2p.h"

/* histogram data structure for finding the most frequent value of
   time intervals, which is later used as 'tmstep' parameter of ctrl_to. */
#define  HIST_INIT_SIZE   32
#define  HIST_GROW_SIZE   16
typedef struct histogram {
    int  ndata;
    int  nalloc;
    struct histent  *data;
} Histogram;

typedef struct histent {
    int  tmstep;
    int  frq;
} HistEnt;

#define  HUGE    1e10

static void	line_fit_a_sequence P((EventSeq *, int, float));
static int	recg_line P((Event *, Event **, int, float, Histogram *));
static void	init_histogram P((Histogram *));
static void	insert_histogram P((Histogram *, int));
static int	get_tmstep P((Histogram *));
static void	reinit_histogram P((Histogram *));
static void	destroy_histogram P((Histogram *));

/*
 * aggregate control-change events lying on lines into continuous 
 * control-change events
 */
void
line_fit(eseq, maxtmstep, tolerance)
EventSeq  *eseq;	/* list of event sequences */
int  maxtmstep;		/* maximum time interval of adjacent control-change
			   events in a single continuous control-change */
float  tolerance;	/* error tolerance for line fitting: It is guaranteed
			   that no control values are apart from the line
			   by more than this torelance. */
{
    EventSeq  *sp, *spn, *spn1;

    for( sp = eseq; sp != NULL; sp = sp->next ) {
	if( sp->linefittable ) {
	    if( sp->events->type == E_Kp ) {
		/* For key-pressure events, divide the sequence into 
		   subsequences based on note numbers, 
		   call line_fit_a_sequence for each subsequence,
		   and then merge the subsequences. */
		spn = nsplit(sp->events);
		for( spn1 = spn; spn1 != NULL; spn1 = spn1->next ) {
		    line_fit_a_sequence(spn1, maxtmstep, tolerance); 
		}
		merge_evlist(spn);
		sp->events = spn->events;
		free(spn);
	    } else {
		line_fit_a_sequence(sp, maxtmstep, tolerance); 
	    }
	}
    } 
}

/*
 * per-sequence line fitting routine
 */
static void
line_fit_a_sequence(eseq, maxtmstep, tolerance)
EventSeq  *eseq;	/* a single sequence of events */
int  maxtmstep;
float  tolerance;	
{
    Event  *ep1, *ep2;
    Histogram  hist;

    init_histogram(&hist);
    
    for( ep1 = eseq->events; ep1 != NULL; ep1 = ep2 ) {
	if( recg_line(ep1, &ep2, maxtmstep, tolerance, &hist) > 1 ) {
	    /* line fitting has succeeded */
	    /* delete events between ep1 and ep2 */
	    { 
		Event  *ep, *epnext;
		for( ep = ep1->next; ep != ep2; ep = epnext ) { 
		    epnext = ep->next;
		    free(ep);
		}
	    }
	    ep1->next = ep2;

	    /* change the last event to a continous one */
	    ep2->type |= E_To;

	    /* set tmstep calulated from the histogram */
	    ep2->ev_tmstep = get_tmstep(&hist);

	    /* clear (reinitialize) the histogram */
	    reinit_histogram(&hist);
	}
    }

    destroy_histogram(&hist);
}

/*
 * try to fit control-change events beginning from ep1 to a line
 *   When fitting is successful, ep2 becomes the event at the end of line,
 *   and the return value becomes greater than 1. 
 *   Otherwise, ep2 becomes ep1->next, and the return value is 0 or 1.
 */
static int 
recg_line(ep1, ep2, maxtmstep, tolerance, hist)
Event *ep1;
Event **ep2;	/* output */
int  maxtmstep;
float  tolerance;
Histogram  *hist;
{
    Event  *ep, *epnext;
    int  count = 0;	/* the number of events processed */
    int  tmstep, tdiff;
    double  slope, s, uplimit = HUGE, lowlimit = -HUGE;   
    
    ep = ep1;
    while( (epnext = ep->next) && epnext->type != E_TrkEnd ) {
	/* if the time is identical, do not recognize as a line */ 
	if( epnext->time == ep1->time )  break;

	/* calculate the slope of the line */
	tdiff = epnext->time - ep1->time;
	slope = (epnext->u.val - ep1->u.val) / tdiff;

	/* time interval from the previous event */
	tmstep = epnext->time - ep->time;

	/* If the slope is out of the range [lowlimit, uplimit], or 
	   tmstep exceeds the given maximum, terminate the line */
	if(slope > uplimit || slope < lowlimit || tmstep > maxtmstep) {
	    break;
	}

	/* update the lower limit of the slope */
	s = (epnext->u.val - ep1->u.val - tolerance) / tdiff;
	if( s > lowlimit )  lowlimit = s;

	/* update the upper limit of the slope */
	s = (epnext->u.val - ep1->u.val + tolerance) / tdiff;
	if( s < uplimit )  uplimit = s;

	insert_histogram(hist, tmstep);
    
	ep = epnext;
	count++;
    }

    if( count ) {
	/* one step backward */
	*ep2 = ep;
    } else {
	*ep2 = epnext;
    }
    return count;
}

/*
 * initialize histogram
 */
static void
init_histogram(hist)
Histogram  *hist;
{
    hist->ndata = 0;
    hist->nalloc = HIST_INIT_SIZE;
    if( !(hist->data = (HistEnt *)malloc(sizeof(HistEnt) * HIST_INIT_SIZE))) {
	err_nomem("init_histogram");
    }
}

/*
 * insert a data value to the histogram
 */
static void
insert_histogram(hist, tmstep)
Histogram  *hist;
int  tmstep;
{
    int  i;
    HistEnt  *p;

    for( i = 0, p = hist->data; i < hist->ndata; i++, p++ ) {
	if( p->tmstep == tmstep ) {
	    p->frq++;
	    return;
	} 
    }
    /* create a new entry */
    if( hist->ndata == hist->nalloc ) {
	hist->nalloc += HIST_GROW_SIZE;
	if( !(hist->data = 
	      (HistEnt *)realloc(hist->data, sizeof(HistEnt) * hist->nalloc))){
	    err_nomem("insert_histogram");
	}
    }
    hist->data[hist->ndata].tmstep = tmstep;
    hist->data[hist->ndata].frq = 1;
    hist->ndata++;
}

/*
 * calculate the final tmstep as the most frequently value of tmstep
 */
static int
get_tmstep(hist)
Histogram  *hist;
{
    int  maxfreq = -1, tmstep, mintmstep = MAXINT;
    int  i;
    HistEnt  *p;

    for( i = 0, p = hist->data; i < hist->ndata; i++, p++ ) {
	if( p->frq > maxfreq )  {
	    tmstep = p->tmstep;
	    maxfreq = p->frq;
	}
	if( p->tmstep > 0 && p->tmstep < mintmstep ) {
	    mintmstep = p->tmstep;
	}
    }

    /* if the most frequently value is not found, use the non-zero minimum */
    if( maxfreq == 1 ) {
	tmstep = mintmstep;
    }
   
    return tmstep;
}

/*
 * clear a histogram to the empty state
 */
static void
reinit_histogram(hist)
Histogram  *hist;
{
    hist->ndata = 0;
}

/*
 * destroy a histogram
 */
static void
destroy_histogram(hist)
Histogram  *hist;
{
    free(hist->data);
}
