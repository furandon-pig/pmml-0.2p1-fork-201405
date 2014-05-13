/*
 * M2P: A SMF-to-PMML Translator
 *   
 * eplout.c: outputting routine for the event-per-line mode
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
 * Acknowledgments: I am grateful to Mr. Tomoaki Sasaki and Mr. Atsushi 
 * Marui for their help in developing this program.
 */

#include <stdio.h>
#include "m2p.h"

void
eplout(evlist, opt, no_marker)
Event  *evlist;
Options  *opt;
int  no_marker;	/* must be true for track 2 or above in format-1 SMF */ 
{
    Event  *ep, *epnext, *epprev = NULL;
    char  *str;
    int  col;

    /* rewind time */
    if( opt->mode != EPL_A ) {
	fprintf(OUT, "t=0\n");
    }

    /* output the first rest (EPL only) */
    if( opt->mode == EPL ) {
	if( evlist->time > 0 ) {
	    if( opt->measure_on ) {
		fprintf(OUT, "/* %s */  ", get_measure(0L));
	    }
	    fprintf(OUT, "%sr(%s)\n", opt->ignore_chan ? "" : "       ",
		    len_name(evlist->time, opt->use_len_name, 1));
	}
    }

    for( ep = evlist; ; epprev = ep, ep = epnext ) {
	/* skip note-off events */
	epnext = ep->next;
	if( !opt->sep_note_ev ) {
	    while( epnext && epnext->type==E_NoteOff && epnext->ev_partner ) { 
		epnext = epnext->next;
	    }
	}
	
	col = 0;
	/* output measure number */
	if( opt->measure_on ) {
	    if( epprev && epprev->time == ep->time ) {
		fprintf(OUT, "                 ");
	    } else {
		fprintf(OUT, "/* %s */  ", get_measure(ep->time));
	    }
	    col += 17;
	}

	/* output time (EPL_A or EPL_Y only) */
	if( opt->mode == EPL_A ) {
	    fprintf(OUT, "t=%-10s ", len_name(ep->time, opt->use_len_name, 0));
	    col += 13;
	} else if( opt->mode == EPL_Y ) {
	    long  rtime = ep->time - (epprev ? epprev->time : 0);
	    if( rtime ) {
		fprintf(OUT, "t+=%-9s ",len_name(rtime, opt->use_len_name, 0));
	    } else {
		fprintf(OUT, "             ");
	    }
	    col += 13;
	}

	/* output channel number */
	if( !opt->ignore_chan ) {
	    if( (ep->type & ETYPE) < E_GCtrl || 
	       (ep->type & ETYPE) >= E_NoteOff ) {
		fprintf(OUT, "ch=%-2d  ", ep->ch + 1);
	    } else {
		fprintf(OUT, "       ");
	    }
	    col += 7;
	}

	/* output event contents */
	if( !opt->sep_note_ev && ep->type == E_NoteOn && ep->ev_partner ) {
	    /* in case of note event */
	    fprintf(OUT, "%-3s(",
		    note_name(ep->note, get_key(ep->time), 
			      opt->use_gm_name && ep->ch == RHYTHM_CH)); 
	    if( opt->mode == EPL ) {
		fprintf(OUT, "%s ", len_name(epnext->time - ep->time, 
					     opt->use_len_name, 1));
	    }
	    fprintf(OUT, "gt=%s v=%d", 
		    len_name(ep->ev_partner->time - ep->time, 
			     opt->use_len_name, 0),
		    ep->ev_veloc);
	    if( ep->ev_partner->ev_veloc >= 0 ) { 
		fprintf(OUT, " nv=%d", ep->ev_partner->ev_veloc);
	    }
	    fprintf(OUT, opt->mode == EPL_Y ? ")&\n" : ")\n");
	} else {
	    /* in case of non-note event */ 
	    if( (str = pmml_code(ep, opt->sym_ctrl, 
				 no_marker, 0, opt->use_gm_name)) ) {
		fprintf(OUT, "%s", str);
	    } else {
		print_excl(ep, col);
	    }

	    /* If there is a timelag between the current event and next event,
	       insert a rest (EPL only) */
	    if( opt->mode == EPL && 
	       ep->type != E_TrkEnd && epnext->time != ep->time ) {
		long  len = epnext->time - ep->time;
		/* If the length of the rest >= whole rest, insert a newline */
		if( len >= resolution * 4 || str == NULL ) {
		    fprintf(OUT, "\n%*sr(%s)", col, "", 
			    len_name(len, opt->use_len_name, 1));
		} else {
		    fprintf(OUT, " r(%s)",len_name(len, opt->use_len_name, 1));
		}
	    }
	    fprintf(OUT, "\n");
	}

	if( ep->type == E_TrkEnd )  break;
    }
}
