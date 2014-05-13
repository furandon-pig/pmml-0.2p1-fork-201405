/*
 * M2P: A SMF-to-PMML Translator
 *   
 * mplout.c: outputting routine for the measure-per-line mode
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

/* current state of PMML registers */
typedef struct reg_state {
    int   ch;
    int   v;
    int   nv;
    long  l;
    long  gt;  /* negative if gt=XX is not executed */ 
} RegState;

/* current output column number */
static int  column;

#define GATETIME(ep)  ((ep)->ev_partner->time - (ep)->time) 
#define Min(x,y)  ((x) < (y) ? (x) : (y))

static void	output_ctrl_section P((Event *, long, long, RegState *, Options *, int, int, int));
static void	output_note_section P((Event *, long, long, RegState *, Options *, int));
static void	output_rest P((long, RegState *, long, Options *, int));
static void	output_chord P((Event *, int, long, RegState *, long, Options *, int, Event *, int));
static void	output_note P((Event *, RegState *, long, Options *, int, int, Event *, int));
static int	get_most_freq_value P((long [], int, long *));
static void	sort_by_note_num P((Event **));
static void	output_string P((char *, int));

void
mplout(evlist, opt, no_marker)
Event  *evlist;	/* Calling mplout will destory this event list! */
Options  *opt;
int  no_marker;	/* true for track 2 or above in format-1 SMF */ 
{
    Event  *note_head, **note_tail;      /* list of normal note events */
    Event  *isnote_head, **isnote_tail;  /* list of isolated note events */
    Event  *pdnote_head, **pdnote_tail;  /* list of the next measure's 
					    note events */
    Event  *other_head, **other_tail;    /* list of other type of events */
    Event  *ep, *epnext;
    RegState  state;
    long  last_end_time, start_time, end_time, tmp_time, other_maxtime;
    int  indent = opt->measure_on ? 17 : 0;
    int  first_line = 1;

    state.ch = state.v = state.nv = MAXINT;
    state.l = MAXLONG;
    state.gt = -1;
    pdnote_head = NULL;
    last_end_time = 0;

    /* find the first note event and record its channel and note-off vel */
    for( ep = evlist; ep != NULL; ep = ep->next ) {
	if( ep->type == E_NoteOn ) {
	    state.ch = ep->ch + 1;
	    if( ep->ev_partner )  state.nv = ep->ev_partner->ev_veloc;
	    break;
	}
    }

    /* for each measure */
    for( ep = evlist; ep->type != E_TrkEnd || pdnote_head; ) {
	/* skip already-processed note-off events */
	while( ep->type == E_NoteOff && ep->ev_partner ) {
	    ep = ep->next;
	}

	/* calculate the time range of the measure where ep is contained */ 
	if( pdnote_head ) {
	    get_measure_range(last_end_time, &start_time, &end_time);
	} else {
	    get_measure_range(ep->time, &start_time, &end_time);
	}

	/* classfy events into four types until the measure end */
	if( pdnote_head ) {
	    note_head = pdnote_head; 
	    note_tail = pdnote_tail;
	} else {
	    note_tail = &note_head;
	}
	pdnote_tail = &pdnote_head;
	isnote_tail = &isnote_head;
	other_tail = &other_head;

	while( ep->time < end_time ) {
	    if( ep->type == E_TrkEnd ) {
		end_time = ep->time;
		break;
	    }

	    epnext = ep->next;
	    switch( ep->type ) {
	    case E_NoteOn:
		if( !ep->ev_partner ) {  /* incomplete note-on */
		    *isnote_tail = ep;
		    isnote_tail = &ep->next;
		    break;
		} 
		/* If this note continues to the next measure ... */
		if( ep->ev_partner->time > end_time + opt->max_ovs_len ) {
		    if( ep->time >= end_time - opt->max_ovs_len ) {
			/* If this note is considered as the next measure's 
			   note, save it to the pending note list. */
			*pdnote_tail = ep;
			pdnote_tail = &ep->next;
			break;
		    } else {
			/* Otherwise, extend end_time. */ 
			get_measure_range(ep->ev_partner->time, 
					  &tmp_time, &end_time);
			if( ep->ev_partner->time == tmp_time ) {
			    /* the note-off time is just on the boundary */
			    end_time = tmp_time;
			}
		    }
		}
		/* Now, insert it to the note list */
		*note_tail = ep;
		note_tail = &ep->next;
		break;
    
	    case E_NoteOff:
		if( !ep->ev_partner ) {  /* incomplete note-off */
		    *isnote_tail = ep;
		    isnote_tail = &ep->next;
		    break;
		}
		/* ignore it */
		break;

	    default:
		*other_tail = ep;
		other_tail = &ep->next;
		other_maxtime = ep->time;
	    }
	    ep = epnext;
	}
	*note_tail = NULL;
	*pdnote_tail = NULL;
	*isnote_tail = NULL;
	*other_tail = NULL;

	/* if no events (all events go to pdnote), shift start_time forward */
	if( !note_head && !isnote_head && !other_head ) {
	    start_time = end_time;
	}

	/* if there are blank measures before start_time, insert a rest */
	if( last_end_time < start_time ) {
	    if( opt->measure_on ) {
		fprintf(OUT, "/* %s */  ", get_measure(last_end_time));
	    }
	    fprintf(OUT, "r(%s)\n",
		    len_name(start_time - last_end_time, opt->use_len_name,1));
	}

	if( note_head || isnote_head || other_head ) {
	    /* To avoid an output like "ctrl_events ... r(w)\n r(8w)" */
	    if( !note_head && !pdnote_head && !isnote_head ) {
		if( other_maxtime == start_time )  end_time = start_time;
	    }

	    /* output the events */
	    if( opt->measure_on ) {
		fprintf(OUT, "/* %s */  ", get_measure(start_time));
	    }
	    column = indent;
	    if( first_line ) {
		char  buf[16];
		output_string("t=0", indent);
		if( state.ch != MAXINT && !opt->ignore_chan ) {
		    sprintf(buf, "ch=%d", state.ch);
		    output_string(buf, indent);
		}
		if( state.nv != MAXINT ) {
		    sprintf(buf, "nv=%d", state.nv);
		    output_string(buf, indent);
		}
		first_line = 0;
	    }
	    output_ctrl_section(other_head, start_time, end_time, &state, opt, 
				no_marker, isnote_head || note_head, indent);
	    if( other_head && isnote_head ) {
		fprintf(OUT, "%*s", indent, "");
		column = indent;
	    }
	    output_ctrl_section(isnote_head, start_time, end_time, &state, opt, 
				no_marker, !isnote_head || note_head, indent);
	    if( (other_head || isnote_head) && note_head ) {
		fprintf(OUT, "%*s", indent, "");
		column = indent;
	    }
	    /* output note events here */
	    output_note_section(note_head, start_time, end_time, &state, opt, 
				indent);
	}

	/* update last_end_time */
	last_end_time = end_time;
    }

    /* output the rest between last_end_time and TrkEnd event */
    if( last_end_time < ep->time ) {
	if( opt->measure_on ) {
	    fprintf(OUT, "/* %s */  ", get_measure(last_end_time));
	}
	fprintf(OUT, "r(%s)\n",
		len_name(ep->time - last_end_time, opt->use_len_name, 1));
    }
    
    /* output the end event */
    if( opt->measure_on ) {
	fprintf(OUT, "/* %s */  ", get_measure(ep->time));
    }
    fprintf(OUT, "end\n");
}

/*
 * output the lines for non-note events or isolated note events
 */
static void
output_ctrl_section(evlist, start_time, end_time, 
		    state, opt, no_marker, par_flag, indent) 
Event  *evlist;
long  start_time; /* start time of the current measure */
long  end_time;	  /* end time of the current measure */
RegState  *state; /* Current state of PMML registers (input/output) */
Options  *opt;
int  no_marker;	  /* true for track 2 or above in format-1 SMF */ 
int  par_flag;	  /* if true, let the total step time zero */
int  indent;      /* column number where the command string starts */
{
    long  last_time = start_time;
    int  braced = 0; 
    RegState  lstate;
    char  buf[32], *str;
    Event  *ep;

    for( ep = evlist; ep != NULL; ep = ep->next ) {
	/* output a rest if necessary */
	if( ep->time != last_time ) {
	    if( !braced && par_flag ) {
		output_string("{ ", indent);
		indent += 2;
		braced = 1;
		lstate = *state;
		state = &lstate;
	    }
	    sprintf(buf, "r(%s)",
		    len_name(ep->time - last_time, opt->use_len_name, 1));
	    output_string(buf, indent);
	}
 
	/* output "ch=XX" if necessary */
	if( (ep->type & ETYPE) < E_GCtrl || (ep->type & ETYPE) >= E_NoteOff ) {
	    if( ep->ch+1 != state->ch && !opt->ignore_chan ) {
		sprintf(buf, "ch=%d", ep->ch+1);
		output_string(buf, indent);
		state->ch = ep->ch+1;
	    }
	}

	/* output "nv=-1" if necessary */
	if( ep->type == E_NoteOff && ep->ev_veloc == -1 ) {
	    if( state->nv != -1 ) {
		output_string("nv=-1", indent);
		state->nv = -1;
	    }
	}

	/* output the command */
	str = pmml_code(ep, opt->sym_ctrl, no_marker, 1, opt->use_gm_name);
	if( !str ) {
	    output_string("", indent);  /* buffer flush */
	    if( column != indent ) {
		output_string("\n", indent);
		fprintf(OUT, "%*s", indent, "");
	    }
	    print_excl(ep, indent);
	    column = MAXCOL;  /* This urges new-line insertion. */
	} else {
	    output_string(str, indent);
	}

	last_time = ep->time;
    }

    /* output the rest at the end */
    if( !par_flag && last_time != end_time ) {
	sprintf(buf, "r(%s)",
		len_name(end_time - last_time, opt->use_len_name, 1));
	output_string(buf, indent);
    }

    /* close the brace */
    if( braced ) {
	output_string(" }&", indent);
    }

    /* new line */
    if( evlist )  output_string("\n", indent);
}

/*
 * output the lines for note events
 */
static void
output_note_section(evlist, start_time, end_time, state, opt, indent) 
Event  *evlist;
long  start_time; /* start time of the current measure */
long  end_time;	  /* end time of the current measure */
RegState  *state; /* Current state of PMML registers (input/output) */
Options  *opt;
int  indent;      /* column number where the command string starts */
{
    static Event  *chord;  /* representation of a chord: chord[i] contains
			      list of notes belonging to the i-th thread */
    static Event  **chord_tail;
    static int  chord_nallocated = 0;
    int  nthread = 0;
    long  last_time = start_time;
    long  steptime, chord_start_time, chord_max_time;
    Event  *ep, *epnext;
    int  is_first = 1;

    /* allocate memory for 'chord': executed only at the first time */
    if( !chord_nallocated ) {
	if( !(chord = (Event *) malloc(sizeof(Event)) ) ||
	    !(chord_tail = (Event **) malloc(sizeof(Event *)) )) {
	    err_nomem("output_note_section");
	}
	chord_nallocated = 1;
	/* the first element of each list is a sentinel */
	chord[0].time = -1;
	chord[0].ev_partner = &chord[0];
    }

    /* do not inherit the previous line's l (and v) values */
    state->l = MAXLONG;	 
    if( !opt->inherit_v )  state->v = MAXINT;	 

    /* sort by note number */
    if( opt->chord_on )  sort_by_note_num(&evlist);

    /* for each event */
    for( ep = evlist; ep != NULL; ep = epnext ) {
	/* Does the note overlap with some note in 'chord'? */
	long  ovlp_time;
	int  ovlp_yes = 0, free_thd = -1, i, j;
	
	epnext = ep->next;
	ep->next = NULL;

	if( opt->chord_on ) {
	    for( i = 0; i < nthread; i++ ) {
		ovlp_time = chord_tail[i]->ev_partner->time - ep->time;
		if( ovlp_time <= 0 ) {
		    if( free_thd == -1 )  free_thd = i;
		} else if( ovlp_time * 100 >=
			  Min(GATETIME(chord_tail[i]), GATETIME(ep)) * 
			  opt->cd_mindup ) {
		    ovlp_yes = 1;
		}
	    }
	}

	if( ovlp_yes ) {
	    /* overlapped - insert the note to 'chord' */
	    if( free_thd >= 0 ) {  /* Is there an insertable thread? */
		chord_tail[free_thd]->next = ep;
		chord_tail[free_thd] = ep;
	    } else {
		/* increase the number of threads */
		if( ++nthread >= chord_nallocated ) {
		    if( !(chord = (Event *) 
			  realloc(chord, nthread * sizeof(Event)) ) ||
		       !(chord_tail = (Event **) 
			 realloc(chord_tail, nthread * sizeof(Event *)) )) {
			err_nomem("output_note_section");
		    }
		    chord_nallocated = nthread;
		    chord[nthread-1].time = -1;
		    chord[nthread-1].ev_partner = &chord[nthread-1];
		}
		/* insert the new note so that note numbers are ordered. */
		for( i = 0; 
		    i < nthread - 1 && chord_tail[i]->note > ep->note; i++);
		for( j = nthread - 2; j >= i; --j ) {
		    chord[j+1].next = chord[j].next;
		    chord_tail[j+1] = chord_tail[j];
		}
		chord[i].next = ep;
		chord_tail[i] = ep;
	    }
	    if( ep->ev_partner->time > chord_max_time ) {
		chord_max_time = ep->ev_partner->time;
	    }

	} else {
	    /* non-overlapped: output the 'chord' contents */

	    if( nthread ) {
		/* Probably, it is better to refine thread allcation here. */

		/* calculate steptime */
		steptime = ep->time - chord_start_time;
		steptime = adjust_steptime(chord_max_time - chord_start_time, 
					   steptime, state->l);
		
		/* output a rest (possibly negative length) if necessary */
		if( chord_start_time != last_time ) {
		    output_rest(chord_start_time - last_time, 
				state, steptime, opt, indent);
		}
		
		/* output chord */
		if( nthread == 1 && chord[0].next == chord_tail[0] ) {
		    output_note(chord[0].next, state, steptime, 
				opt, 0, indent, ep, is_first);
		} else {
		    output_chord(chord, nthread, chord_start_time,
				 state, steptime, opt, indent, ep, is_first);
		}
		is_first = 0;
		
		/* update last_time */
		last_time = chord_start_time + steptime; 
	    }
	    
	    /* insert the first note in each chord to 'chord' */
	    chord[0].next = ep;
	    chord_tail[0] = ep;
	    chord_start_time = ep->time;
	    chord_max_time = ep->ev_partner->time;
	    nthread = 1;
	}
    }

    /* output the last chord or note */
    if( nthread ) {
	/* calculate steptime */
	steptime = end_time - chord_start_time;
	steptime = adjust_steptime(chord_max_time - chord_start_time, 
				   steptime, state->l);
	
	/* output a rest (possibly negative length) if necessary */
	if( chord_start_time != last_time ) {
	    output_rest(chord_start_time - last_time, 
			state, steptime, opt, indent);
	}
	
	/* output chord */
	if( nthread == 1 && chord[0].next == chord_tail[0] ) {
	    output_note(chord[0].next, state, steptime, 
			opt, 0, indent, NULL, is_first);
	} else {
	    output_chord(chord, nthread, chord_start_time,
			 state, steptime, opt, indent, NULL, is_first);
	}
	
	/* update last_time */
	last_time = chord_start_time + steptime; 
    }
    
    /* output the rest at the end */
    if( evlist && last_time != end_time ) {
	output_rest(end_time - last_time, state, 0L, opt, indent);
    }

    /* new line */
    if( evlist )  output_string("\n", indent);
}

/*
 * output a rest between notes
 */
static void
output_rest(len, state, next_steptime, opt, indent) 
long  len;
RegState  *state;
long  next_steptime;
Options *opt;
int  indent;
{
    char  buf[32];

    if( len >= 0 ) {
	if( len == state->l ) {
	    sprintf(buf, "r");
	} else if( next_steptime == len ) {
	    sprintf(buf, "%s r", len_name(len, opt->use_len_name, 1));
	    state->l = len;
	} else {
	    sprintf(buf, "r(%s)", len_name(len, opt->use_len_name, 1));
	}
    } else {
	sprintf(buf, "t-=%s", len_name(-len, opt->use_len_name, 0));
    }
    output_string(buf, indent);
}

/*
 * output a chord
 */
static void
output_chord(chord, nthread, chord_start_time, 
	     state, steptime, opt, indent, next_note, is_first)
Event  *chord;
int  nthread;
long  chord_start_time;
RegState  *state;
long  steptime;
Options  *opt;
int  indent;
Event  *next_note;
int  is_first;		/* true if the chord lies at the top of measure */
{
    RegState  cstate;	/* register state inside '[ .. ]' */ 
    RegState  lstate;	/* register state inside '{ .. }' */
    long  *values;	/* buffer for calculating most-frequent values */
    long  chord_ch, chord_v, chord_nv, chord_gt;
    int  i;
    Event  *ep;
    long  last_time;
    char  buf[32];

    /* calculate the ch, v, nv, and gt values of the chord as a whole
       by taking the most-frequent value among the 1st notes of all threads */
    if( !(values = (long *)malloc(sizeof(long) * nthread)) ) {
	err_nomem("output_chord");
    }
    for( i = 0; i < nthread; i++ ) {
	values[i] = GATETIME(chord[i].next);
    }
    get_most_freq_value(values, nthread, &chord_gt);
    for( i = 0; i < nthread; i++ ) {
	values[i] = chord[i].next->ch + 1;
    }
    get_most_freq_value(values, nthread, &chord_ch);
    for( i = 0; i < nthread; i++ ) {
	values[i] = chord[i].next->ev_veloc;
    }
    get_most_freq_value(values, nthread, &chord_v);
    for( i = 0; i < nthread; i++ ) {
	values[i] = chord[i].next->ev_partner->ev_veloc;
    }
    get_most_freq_value(values, nthread, &chord_nv);
    free(values);

    /* output "ch=XX" if necessary */
    if( chord_ch >= 0 && chord_ch != state->ch &&
       (next_note && next_note->ch + 1 == chord_ch ||
	!next_note && is_first) && !opt->ignore_chan ) {
	sprintf(buf, "ch=%d", chord_ch);
	output_string(buf, indent);
	state->ch = chord_ch;
    }
	
    /* output "v=XX" if necessary */
    if( chord_v >= 0 && chord_v != state->v && 
       (next_note && next_note->ev_veloc == chord_v ||
	!next_note && is_first) ) {
	sprintf(buf, "v=%d", chord_v);
	output_string(buf, indent);
	state->v = chord_v;
    }
	
    /* output "nv=XX" if necessary */
    if( chord_nv >= 0 && chord_nv != state->nv &&
       (next_note && next_note->ev_partner->ev_veloc == chord_nv ||
	!next_note && is_first) ) {
	sprintf(buf, "nv=%d", chord_nv);
	output_string(buf, indent);
	state->nv = chord_nv;
    }

    /* output length if necessary */
    if( steptime != state->l ) {
	output_string(len_name(steptime, opt->use_len_name, 1), indent);
	state->l = steptime;
    }

    /* output '[' and make a new state */
    output_string("[", indent);
    cstate = *state;
    
    /* output "gt=XX" if necessary */
    for( i = 0; i < nthread; i++ ) {
	/* If some thread has more than one notes, or if some note does not
	   begin from the start, do not use "gt=XX". */
	if( chord[i].next->next || chord[i].next->time != chord_start_time ) {
	    chord_gt = -1;
	    break;
	}
    }
    if( chord_gt >= 0 && 
       (cstate.gt >= 0 ? cstate.gt : steptime) != chord_gt ) {
	sprintf(buf, "gt=%s", len_name(chord_gt, opt->use_len_name, 0));
	output_string(buf, indent);
	cstate.gt = chord_gt;
    }
    
    /* do the same for ch, v and nv */
    if( chord_ch >= 0 && chord_ch != cstate.ch && !opt->ignore_chan ) {
	sprintf(buf, "ch=%d", chord_ch);
	output_string(buf, indent);
	cstate.ch = chord_ch;
    }
    if( chord_v >= 0 && chord_v != cstate.v ) {
	sprintf(buf, "v=%d", chord_v);
	output_string(buf, indent);
	cstate.v = chord_v;
    }
    if( chord_nv >= 0 && chord_nv != cstate.nv ) {
	sprintf(buf, "nv=%d", chord_nv);
	output_string(buf, indent);
	cstate.nv = chord_nv;
    }
    
    /* output each thread */
    for( i = nthread; --i >= 0; ) {
	ep = chord[i].next;
	/* if the number of notes in the thread > 1, or if rests are needed
	   before/after note, use "{ .. }" */
	if( ep->next == NULL ) {
	    if( ep->time == chord_start_time &&
	       adjust_steptime(GATETIME(ep), steptime, 0L) == steptime ) {
		/* without "{ .. } "*/
		output_note(ep, &cstate, steptime, opt, 1, indent, NULL, 0);
		continue;
	    }
	}
	/* with "{ .. }" */
	output_string("{", indent);
	lstate = cstate;

	/* print each note in { .. } */
	last_time = chord_start_time;
	for( ; ep != NULL; ep = ep->next ) {
	    long  st;

	    /* calculate the steptime of the note */
	    st = (ep->next ? 
		  ep->next->time : chord_start_time + steptime) - ep->time;
	    st = adjust_steptime(GATETIME(ep), st, lstate.l);

	    /* output a rest if necessary */
	    if( ep->time != last_time ) {
		output_rest(ep->time - last_time, &lstate, st, opt, indent);
	    }
	    
	    /* output the note and update last_time */
	    output_note(ep, &lstate, st, opt, 0, indent, ep->next, 0);
	    last_time = ep->time + st;
	}

	/* output a rest at the end of { .. }, and close '}' */
	if( last_time < chord_start_time + steptime ) {
	    output_rest(chord_start_time + steptime - last_time,
			&lstate, 0L, opt, indent);
	}
	output_string("}", indent);
    }

    /* output ']' */
    output_string("]", indent);
}

/*
 * output a note
 */
static void
output_note(ep, state, steptime, opt, use_modifier, 
	    indent, next_note, is_first)
Event  *ep;
RegState  *state;
long  steptime;
Options  *opt;
int  use_modifier;	/* If true, 'C4(gt=.. v=..)' form is always used. */
int  indent;
Event  *next_note;
int  is_first;		/* true if the note is the one in each measure */
{
    char  buf[64], *p;

    if( !use_modifier ) { 
	/* output "ch=XX" if necessary */
	if( ep->ch+1 != state->ch && 
	   (next_note && next_note->ch == ep->ch ||
	    !next_note && is_first) && !opt->ignore_chan ) {
	    sprintf(buf, "ch=%d", ep->ch+1);
	    output_string(buf, indent);
	    state->ch = ep->ch+1;
	}
	
	/* output "v=XX" if necessary */
	if( ep->ev_veloc != state->v && 
	   (next_note && next_note->ev_veloc == ep->ev_veloc ||
	    !next_note && is_first) ) {
	    sprintf(buf, "v=%d", ep->ev_veloc);
	    output_string(buf, indent);
	    state->v = ep->ev_veloc;
	}
	
	/* output "nv=XX" if necessary */
	if( ep->ev_partner->ev_veloc != state->nv &&
	   (next_note && 
	    next_note->ev_partner->ev_veloc == ep->ev_partner->ev_veloc ||
	    !next_note && is_first) ) {
	    sprintf(buf, "nv=%d", ep->ev_partner->ev_veloc);
	    output_string(buf, indent);
	    state->nv = ep->ev_partner->ev_veloc;
	}
	
	/* output length if necessary */
	if( steptime != state->l ) {
	    output_string(len_name(steptime, opt->use_len_name, 1), indent);
	    state->l = steptime;
	}
    }

    sprintf(buf, "%s(", note_name(ep->note, get_key(ep->time),
				  opt->use_gm_name && ep->ch == RHYTHM_CH));
    p = buf + strlen(buf);
    
    /* output length if necessary */
    if( steptime != state->l ) {
	sprintf(p, "%s", len_name(steptime, opt->use_len_name, 1));
	output_string(p = buf, indent);
    }
	
    /* output gate time if necessary */
    if( (state->gt >= 0 ? state->gt : steptime) != GATETIME(ep) ) {
	sprintf(p, "gt=%s", len_name(GATETIME(ep), opt->use_len_name, 0));
	output_string(p = buf, indent);
    }
	
    /* output "ch=XX" if necessary */
    if( ep->ch+1 != state->ch && !opt->ignore_chan ) {
	sprintf(p, "ch=%d", ep->ch+1);
	output_string(p = buf, indent);
    }
    
    /* output "v=XX" if necessary */
    if( ep->ev_veloc != state->v ) {
	sprintf(p, "v=%d", ep->ev_veloc);
	output_string(p = buf, indent);
    }
	
    /* output "nv=XX" if necessary */
    if( ep->ev_partner->ev_veloc != state->nv ) {
	sprintf(p, "nv=%d", ep->ev_partner->ev_veloc);
	output_string(p = buf, indent);
    }
    
    /* output the remaining part */
    if( p == buf ) {
	output_string(")", indent);
    } else {
	p[-1] = 0;  /* remove '(' */
	output_string(buf, indent);
    }
}

/*
 * sort events so that the highest note comes first among isotemporal notes
 */
static void
sort_by_note_num(evlist_ptr)
Event  **evlist_ptr;
{
    Event  **cur_time_top, *ep, **epp, **epp2;

    /* a variation of the direct-insertion sorting algorithm */
    cur_time_top = epp = evlist_ptr;
    while( (ep = *epp) != NULL ) {
	if( (*cur_time_top)->time != ep->time ) {
	    cur_time_top = epp;
	    epp = &ep->next;
	} else {
	    for( epp2 = cur_time_top; ; epp2 = &(*epp2)->next ){
		if( epp2 == epp ) {
		    epp = &ep->next;
		    break;
		} else if( (*epp2)->note <= ep->note ) {
		    *epp = ep->next;
		    ep->next = *epp2;
		    *epp2 = ep;
		    break;
		}
	    }
	}
    }
}

/*
 * calculate the most frequently appearing value in an array
 *   Return value is the frequency.  *val_rtn is set to the most 
 *   frequently appearing value (-1 if frequency <= 1 ).
 */
static int  longcompare(i,j)
long  *i, *j;
{
    return(*i > *j ? 1 : *i == *j ? 0 : -1);
}

static int
get_most_freq_value(values, n, val_rtn)
long  values[];  /* array of values: This is modified by this function. */
int  n;  /* number of elements */
long  *val_rtn;	 /* output */
{
    int  maxfrq = 0, frq = 1;
    int  i;
    
    qsort(values, n, sizeof(long), longcompare);
    for( i = 0; i < n - 1; i++ ) {
	if( values[i] == values[i+1] ) {
	    frq++;
	} else {
	    frq = 1;
	}
	if( frq > maxfrq ) {
	    maxfrq = frq;
	    *val_rtn = values[i];
	}
    }

    if( maxfrq <= 1 )  *val_rtn = -1;

    return maxfrq;
}

/*
 * output a string with possible insertion of a space or new line
 */
static void
output_string(str, indent)
char  *str;	/* "\n" for flush & newline, "" for flush request */
int  indent;
{
    static char  buf[MAXCOL + 1];
    static int   buf_n = 0;	/* number of chars in the buffer */
    int  prev_char;
    int  no_space;
    
    prev_char = buf_n > 0 ? buf[buf_n - 1] : 0;
    no_space = (prev_char == '{' || prev_char == '[' || 
		prev_char == '(' || prev_char == ' ' ||
		str[0] == '}' || str[0] == ']' || 
		str[0] == ')' || str[0] == ' ');
    
    if( buf_n && (*str == '\n' || *str == 0 || !no_space) ) {
	/* flush the buffer */
	if( column <= indent ) {  /* beginning of the line */
	    fprintf(OUT, "%s", buf);
	    column += buf_n;
	} else if( column + buf_n < MAXCOL ) {
	    fprintf(OUT, " %s", buf);
	    column += buf_n + 1;
	} else {
	    fprintf(OUT, "\n%*s%s", indent, "", buf); 
	    column = indent + buf_n;
	}
	buf_n = 0;
    }

    if( *str == '\n' ) {
	putc('\n', OUT);
	column = 0;
    } else {
	strcpy(&buf[buf_n], str);
	buf_n += strlen(str);
    }
}
