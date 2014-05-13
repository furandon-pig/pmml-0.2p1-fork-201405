/*
 * PMML: A Compiler for the Practical Music Macro Language
 *
 * smfout.c: terminating effector for standard MIDI file output
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
#include "../common/smf.h"

int	max_reso = 0;	/* maximum resolution among loaded files */

int	no_file_generated = 0;

/* per-instance data structure */
struct smfout_data {
    char  *fname;
    int   resolution;
    int   format;
    int   no_run_stat;
    int   pack_tracks;
    int   no_retrigger;
    char  *track_range;
    MFILE  *mfp;		/* MIDI file pointer */
    int   prev_tk;
    long  prev_time;
    long  end_time;
    int   no_genfile;
    float  *vmag;
    double  cur_tempo;
    int	     exist_begin, exist_end;
    SongPos  pos_begin, pos_end;
};

/*
 * macros
 */
/* convert rational time to SMF ticks (with flooring) */ 
#define ConvTime(tm, reso) \
    ((tm).intg * ((reso)*4) + ((tm).num * ((reso)*4)) / (tm).den)

/*
 * function prototypes
 */
static void  supply_tempo_map P((MTracks *, int, int));
static void  time_songpos P((Track *, SongPos *, Rational *));
static char* in_range_gettrkno P((char *, int, Track *, int *, int));
static void  warn_veloc P((Event *, int));
static void  warn_value P((Event *, PmmlInt));
static void  warn_tempo P((Event *, PmmlFloat));


/*
 * attach routine: analyzer parameter, set mask, etc.
 */
void
smfout_init(argc, argv, eip)
int  argc;
Object  *argv;
EffInst  *eip;
{
    struct smfout_data  *p;
    int  i;

    /* ARGUMENTS:
     *  argv[0]  file name
     *  argv[1]  resolution
     *  argv[2]  format 
     *  argv[3]  no_run_stat
     *  argv[4]  pack_tracks
     *  argv[5]  no_retrigger
     *  argv[6]  channel mask
     *  argv[7]  track range string
     *  argv[8]  beginning song position (optional)
     *  argv[9]  ending song position (optional)
     */

    if( !(p = (struct smfout_data *)
	  malloc(sizeof(struct smfout_data))) ) {
	err_nomem("smfout_init");
    }
    p->fname = attach_extension(argv[0].o_str, MFILE_EXT);
    p->resolution = argv[1].o_val;
    if( p->resolution < 0 ) {
	error(cur_srcpos, "smfout: Bad resolution value", 
	      "smfout: 分解能の値が間違っています");
    }
    p->format = argv[2].o_val;
    if( p->format < -1 || p->format > 2 ) {
	error(cur_srcpos, "smfout: Bad format value", 
	      "smfout: format の値が間違っています");
    }
    p->no_run_stat = argv[3].o_val;
    p->pack_tracks = argv[4].o_val;
    p->no_retrigger = argv[5].o_val;
    if( !(p->track_range = strdup(argv[7].o_str)) ) {
	err_nomem("smfout_init");
    }
    if( in_range(0, p->track_range, 1, 1, NULL) == -1 ) {
	error(cur_srcpos, "smfout: Bad track number specifier", 
	      "smfout: トラック番号指定が間違っています");
    }
    p->exist_begin = p->exist_end = 0;
    if( argc > 8 && * argv[8].o_str ) {
	p->exist_begin = 1;
	if( analy_songpos(argv[8].o_str, &p->pos_begin, 0) == -1 ) {
	    error(cur_srcpos, "smfout: Bad song position specifier", 
		  "smfout: 演奏位置指定が間違っています");
	}
    }
    if( argc > 9 && * argv[9].o_str ) {
	p->exist_end = 1;
	if( analy_songpos(argv[9].o_str, &p->pos_end, 1) == -1 ) {
	    error(cur_srcpos, "smfout: Bad song position specifier", 
		  "smfout: 演奏位置指定が間違っています");
	}
    }

    /* other initilizations */
    p->prev_tk = -1;
    p->no_genfile = 0;
    no_file_generated = 1;
    if( !(p->vmag = (float *) malloc(sizeof(float) * 16)) ) {
	err_nomem("smfout_init");
    }
    p->cur_tempo = 120.0;

    /* set event & channel masks */
    /* do not accept virtual control change except special ones */
    for( i = e_idx(E_VCtrl); i < e_idx(E_Meta); i++ ) {
	eip->ethru[i] = ~0L;
    }
    eip->ethru[e_idx(E_VCtrl)] = ~(e_mask(E_Bend) | e_mask(E_Kp) |
				   e_mask(E_Cpr) | e_mask(E_Prog) | 
				   e_mask(E_Vmag));
    eip->ethru[e_idx(E_Tempo)] &= ~(e_mask(E_Tempo) | e_mask(E_RTempo));

    eip->cthru = argv[6].o_val;
    if( p->format != 2 ) {
	eip->flags |= EF_MoveTimeEv;
    }
    eip->data = (long) p;
}

/*
 * beginning-of-detach: open the file and output SMF header
 */
void
smfout_detach(eip)
EffInst  *eip;
{
    struct smfout_data  *p = (struct smfout_data *) eip->data;
    int  nt, i, eflags;
    Rational  len, t1, t2, len2;

    /* determine resolution */
    if( p->resolution == 0 ) {
	p->resolution = max_reso ? max_reso : ORES;
    }

    /* decide the final format */
    if( p->format == -1 ) {
	p->format = eip->ebuf.ntrks > 1 ? 1 : 0;
    }

    /* check if there are no tracks */
    if( eip->ebuf.ntrks == 0 ) {
	p->no_genfile = 1;
	warn(0L, "%s: No events - no output file generated",
	     "%s: イベントが１つも無いため出力ファイルは生成されません", 
	     p->fname);
	return;
    }

    /* sort marker track */
    sort_event(&eip->ebuf.marks);

    /* select intended tracks */
    for( i = 0; i < eip->ebuf.ntrks; i++ ) {
	if( !in_range(i+1, p->track_range, 
		      1, p->format < 2, &eip->ebuf.marks) ) {
	    destroy_track(&eip->ebuf.trks[i]);
	}
    }

    /* post process */
    eflags = EF_ExpandCtrl | EF_CutAfterEnd;
    if( !p->no_retrigger )  eflags |= EF_Retrigger;
    if( p->exist_begin || p->exist_end ) {
	/* with time range */
	eflags |= EF_TimeRange;
	if( p->exist_begin ) {
	    time_songpos(&eip->ebuf.marks, &p->pos_begin, &t1);
	} else  t1 = r_zero;
	if( p->exist_end ) {
	    time_songpos(&eip->ebuf.marks, &p->pos_end, &t2);
	} else  t2 = r_max;
	if( rlesseq(&t2, &t1) ) {
	    error(0L, "%s: begin-time should be earlier than end-time",
		  "%s: 演奏終了時刻が開始時刻以前になっています", p->fname);
	}
	post_process(&eip->ebuf, eflags, &t1, &t2, &len);
	rsub(&eip->thd->parent->reg.t, &t1, &len2);
    } else {
	/* without time range */
	post_process(&eip->ebuf, eflags, NULL, NULL, &len);
	len2 = eip->thd->parent->reg.t;
    }

    /* Now, "len" is the song length calulcated based on event times, 
       and "len2" is the song length obtained by detaching time.
       Let the final song length be max(len, len2) */
    if( rgreater(&len2, &len) )  len = len2;
    if( len.intg < 0 ) {
	warn(0L, "%s: begin-time is later than the song end",
	      "%s: 演奏開始時刻が曲の終り以降になっています", p->fname);
	len = r_zero;
    }
    p->end_time = ConvTime(len, p->resolution);

    /* re-sort all tracks */
    for( i = 0; i < eip->ebuf.ntrks; i++ ) {
	sort_event(&eip->ebuf.trks[i]);
    }

    /* merge all tracks if format is 0 */
    if( p->format == 0 && eip->ebuf.ntrks > 1 ) {
	merge_all_tracks(&eip->ebuf);
	/* retriger process again */
	if( !p->no_retrigger ) {
	    post_process(&eip->ebuf, EF_Retrigger, NULL, NULL, NULL);
	    sort_event(&eip->ebuf.trks[0]);
	}
    }
    
    /* check that there is at least one event */
    for( i = 0, nt = 0; i < eip->ebuf.ntrks; i++ ) {
	if( eip->ebuf.trks[i].eseq )  nt++;
    }
    if( nt == 0 ) {
	p->no_genfile = 1;
	warn(0L, "%s: No events - no output file generated",
	     "%s: イベントが１つも無いため出力ファイルは生成されません", 
	     p->fname);
	return;
    }

    /* supply tempo map */
    supply_tempo_map(&eip->ebuf, p->format, p->pack_tracks);

    /* determine the final number of tracks */
    if( p->pack_tracks ) {
	/* count the number of non-empty tracks */
	for( i = 0, nt = 0; i < eip->ebuf.ntrks; i++ ) {
	    if( eip->ebuf.trks[i].eseq )  nt++;
	}
    } else {
	nt = eip->ebuf.ntrks;
    }
    
    /* file open */
    if( !(p->mfp = smf_wopen(p->fname, p->format, nt, p->resolution)) ) {
	mferror(strcmp(p->fname, "-") == 0 ? "stdout" : p->fname);
    }

    if( p->no_run_stat ) {
	smf_runstat(p->mfp, 0);
    }

    no_file_generated = 0;
}

/*
 * handler for each event
 */
void
smfout_action(ep, eip)
Event  *ep;
EffInst  *eip;
{
    struct smfout_data  *p = (struct smfout_data *) eip->data;
    int  track = p->format != 0 ? ep->track : 0;
    int  ch = ep->ch; 
    unsigned char  msg[3];
    long  t, dtime;
    PmmlInt  val;
    PmmlFloat  fpval;
    int  veloc;
    int  etype;

    if( track != p->prev_tk ) {
	if( p->prev_tk != -1 ) {
	    smf_endtrack(p->mfp, 
			 p->format < 2 ? p->end_time - p->prev_time : 0L);
	}
	if( !p->pack_tracks ) {
	    /* insert empty tracks */
	    while( p->prev_tk != track - 1 ) {
		smf_bgntrack(p->mfp);
		smf_endtrack(p->mfp, p->format < 2 ? p->end_time : 0L);
		p->prev_tk++;
	    }
	}
	smf_bgntrack(p->mfp);
	p->prev_tk = track;
	p->prev_time = 0;
	{ int i;  for( i = 0; i < 16; i++ )  p->vmag[i] = 1.0; }
    }

    t = ConvTime(ep->time, p->resolution);
    dtime = t - p->prev_time ;

    etype = ep->type & ETYPE;
    if( etype < E_Meta ) {  /* control change (must not be a continuous CC) */
	if( !isnumber(ep->u.obj.o_type) ) {
	    error(0L, "Inappropriate type of control value (time=%s func_no=%d)",
		  "コントロール値の型が不適当です (time=%s func_no=%d)",
		  rstring(&ep->time), etype);
	}
	switch( etype ) {
	case E_Bend:
	    conv_to_int(&ep->u.obj, &val); 
	    val += 8192;
	    if( val < 0 ) {
		warn_value(ep, val); val = 0;
	    } else if( val >= 16384 ) {
		warn_value(ep, val); val = 16383;
	    }
	    msg[0] = 0xe0 | ch;
	    msg[1] = val & 0x7f;
	    msg[2] = (val >> 7) & 0x7f;
	    smf_putevent(p->mfp, dtime, MF_MIDI, 3, msg);
	    p->prev_time = t;
	    break;

	case E_Kp:
	    conv_to_int(&ep->u.obj, &val); 
	    if( val < 0 ) {
		warn_value(ep, val); val = 0;
	    } else if( val >= 128 ) {
		warn_value(ep, val); val = 127;
	    }
	    msg[0] = 0xa0 | ch;
	    msg[1] = ep->note;
	    msg[2] = val;
	    smf_putevent(p->mfp, dtime, MF_MIDI, 3, msg);
	    p->prev_time = t;
	    break;
	    
	case E_Cpr:
	    conv_to_int(&ep->u.obj, &val); 
	    if( val < 0 ) {
		warn_value(ep, val); val = 0;
	    } else if( val >= 128 ) {
		warn_value(ep, val); val = 127;
	    }
	    msg[0] = 0xd0 | ch;
	    msg[1] = val;
	    smf_putevent(p->mfp, dtime, MF_MIDI, 2, msg);
	    p->prev_time = t;
	    break;
	    
	case E_Prog:
	    conv_to_int(&ep->u.obj, &val); 
	    if( val < 1 ) {
		warn_value(ep, val); val = 1;
	    } else if( val >= 129 ) {
		warn_value(ep, val); val = 128;
	    }
	    msg[0] = 0xc0 | ch;
	    msg[1] = val - 1;
	    smf_putevent(p->mfp, dtime, MF_MIDI, 2, msg);
	    p->prev_time = t;
	    break;
	    
	case E_Vmag:
	    conv_to_float(&ep->u.obj, &fpval); 
	    p->vmag[ch] = fpval;
	    break;
	    
	case E_Tempo:
	case E_RTempo:
	    conv_to_float(&ep->u.obj, &fpval); 
	    if( etype == E_Tempo ) { 
		p->cur_tempo = fpval;
	    } else {
		fpval = p->cur_tempo * fpval;
	    }
	    if( fpval <= 0 ) {
		warn_tempo(ep, fpval); fpval = 120.0;
	    }
	    val = floor(6e+7 / fpval + .5);
	    smf_tempo(p->mfp, dtime, val);
	    p->prev_time = t;
	    break;

	default:  /* real control change */
	    conv_to_int(&ep->u.obj, &val); 
	    if( val < 0 ) {
		warn_value(ep, val); val = 0;
	    } else if( val >= 128 ) {
		warn_value(ep, val); val = 127;
	    }
	    msg[0] = 0xb0 | ch;
	    msg[1] = etype & 0x7f;
	    msg[2] = val;
	    smf_putevent(p->mfp, dtime, MF_MIDI, 3, msg);
	    p->prev_time = t;
	    break;
	}
    } else if( etype < E_NoteExcl ) {  /* meta event */
	if( etype < E_IntMeta && etype != E_TrkEnd ) {
	    smf_putevent(p->mfp, dtime, 
			 etype - E_Meta, ep->ev_len, ep->ev_data);
	    p->prev_time = t;
	}
    } else {
	switch( etype ) {
	case E_NoteOn:
	case E_NoteOff:
	    veloc = ep->ev_veloc;
	    if( etype == E_NoteOn ) {
		veloc = floor(veloc * p->vmag[ch] + .5);
		if( veloc <= 0 ) {
		    warn_veloc(ep, veloc);
		    /* delete the note event */
		    if( ep->ev_partner )  ep->ev_partner->type = E_Deleted;
		    break;
		} else if( veloc >= 128 ) {
		    warn_veloc(ep, veloc); veloc = 127;
		}
		msg[0] = 0x90 | ch;
		msg[1] = ep->note;
		msg[2] = veloc;
	    } else if( veloc < 0 ) {	/* no note-off velocity? */
		msg[0] = 0x90 | ch;
		msg[1] = ep->note;
		msg[2] = 0;
	    } else {
		if( veloc >= 128 ) {
		    warn_veloc(ep, veloc); veloc = 127;
		}
		msg[0] = 0x80 | ch;
		msg[1] = ep->note;
		msg[2] = veloc;
	    }
	    smf_putevent(p->mfp, dtime, MF_MIDI, 3, msg);
	    p->prev_time = t;
	    break;

	case E_Excl:
	    smf_putevent(p->mfp, dtime, MF_SYSEX, ep->ev_len, ep->ev_data);
	    p->prev_time = t;
	    break;
	case E_Arbit:
	    smf_putevent(p->mfp, dtime, MF_ARBIT, ep->ev_len, ep->ev_data);
	    p->prev_time = t;
	    break;
	case E_Deleted:
	    /* ignore it */
	    break;
	}
    }
    
    event_free(ep);
}

/*
 * end-of-detach: close the file and free memory
 */
void
smfout_wrap(eip)
EffInst  *eip;
{
    struct smfout_data  *p = (struct smfout_data *) eip->data;
    
    if( !p->no_genfile ) {
	if( !p->pack_tracks ) {
	    /* insert empty tracks */
	    while( p->prev_tk != eip->ebuf.ntrks - 1 ) {
		smf_endtrack(p->mfp, 
			     p->format < 2 ? p->end_time - p->prev_time : 0L);
		smf_bgntrack(p->mfp);
		p->prev_tk++;
		p->prev_time = 0;
	    }
	}
	if( smf_endtrack(p->mfp,
			 p->format < 2 ? p->end_time - p->prev_time : 0L) ) {
	    mferror(strcmp(p->fname, "-") == 0 ? "stdout" : p->fname);
	}
	smf_close(p->mfp);
    }

    free(p->fname);
    free(p->track_range);
    free(p->vmag);
    free(p);
}

/* 
 * supply tempo map
 *   If the track lacks initial tempo and timesig, supply them. 
 */
static void
supply_tempo_map(ebuf, format, pack_tracks)
MTracks  *ebuf;
int  format;
int  pack_tracks;
{
    Track  *bp;
    EventSeq  *sp;
    Event  *ep;
    int  tempo_found, timesig_found;
    int  tk;

    for( tk = 0; tk < ebuf->ntrks; tk++ ) {
	bp = &ebuf->trks[tk];

	tempo_found = timesig_found = 0;

	if( bp->eseq ) {
	    for( ep = bp->eseq->events; ep != NULL; ep = ep->next ) {
		if( rgtzero(&ep->time) )  break;
		if( ep->type == E_Tempo || ep->type == E_OrgTempo ) {
		    tempo_found = 1;
		} else if( ep->type == E_TimeSig ) {
		    timesig_found = 1;
		}
	    }
	} else {
	    if( format == 2 && pack_tracks ) {
		continue;
	    }
	    /* create a new event sequence */
	    if( !(sp = (EventSeq *) malloc(sizeof(EventSeq))) ) {
		err_nomem("supply_tempo_map");
	    }
	    sp->events = NULL;
	    sp->next = NULL;
	    bp->eseq = sp;
	}

	if( !tempo_found ) {
	    event_alloc(ep);
	    ep->type = E_Tempo;
	    ep->u.obj.fpval = 120.0;	/* default tempo = 120 */
	    ep->time = r_zero;
	    ep->track = tk;
	    ep->next = bp->eseq->events;
	    bp->eseq->events = ep;
	}

	if( !timesig_found ) {
	    event_alloc(ep);
	    ep->type = E_TimeSig;
	    if( !(ep->ev_data = (uchar *) malloc(4)) ) {
		err_nomem("supply_tempo_map");
	    }
	    ep->ev_data[0] = 4;		/* default timesig = 4/4 */
	    ep->ev_data[1] = 2;
	    ep->ev_data[2] = 24;
	    ep->ev_data[3] = 8;
	    ep->ev_len = 4;
	    ep->time = r_zero;
	    ep->track = tk;
	    ep->next = bp->eseq->events;
	    bp->eseq->events = ep;
	}

	if( format < 2 )  break;
    }
}

/*
 * merge all tracks to make Format-0 file
 */
void
merge_all_tracks(ebuf)
MTracks  *ebuf;
{
    int  i;
    EventSeq  **spp;

    /* assuming that all the tracks are already sorted, i.e.,
       each track has only one event sequence */

    spp = &ebuf->trks[0].eseq;
    for( i = 0; i < ebuf->ntrks; i++ ) {
	if( ebuf->trks[i].eseq ) {
	    *spp = ebuf->trks[i].eseq;
	    spp = &(*spp)->next;
	}
    }

    sort_event(&ebuf->trks[0]);
    ebuf->ntrks = 1;
}

/*
 * determine if a number is in the range or not
 *   - returns -1 if error
 */
int
in_range(num, range, trk1ch0, enable_one, trknames)
int   num; 
char  *range;
int   trk1ch0;      /* 1: track number  0: channel number */ 
int   enable_one;   /* If true, track 1 is implicitly enabled. */
Track *trknames;    /* track for trackname (and other) events (possibly NULL)*/
{
    int  f, t;
    int  state = 2;	/* 0:explicitly disabled
			   1:disabled
			   2:enabled
			   3:explicitly enabled  */

    while( *range ) {
	while( isspace(*range) || *range == ',' )  range++;
	if( !*range )  break;
	if( !(range = in_range_gettrkno(range, trk1ch0, 
					trknames, &f, num==1)) ) {
	    return -1;
	}
	while( isspace(*range) )  range++;
	if( *range == '-' ) {
	    range++;
	    while( isspace(*range) )  range++;
	    if( !(range = in_range_gettrkno(range, trk1ch0, 
					    trknames, &t, num==1))) {
		return -1;
	    }
	    if( f < 0 ) {
		t = t < 0 ? -t : t;
		if( num >= -f && num <= t )  state = 0;
	    } else {
		if( state == 2 && !(num >= f && num <= t) &&
		   !(enable_one && num == 1) ) {
		    state = 1;
		} else if( state > 0 && (num >= f && num <= t) ) {
		    state = 3;
		}
	    }
	} else {
	    if( f < 0 ) {
		if( -f == num )  state = 0;
	    } else {
		if( state == 2 && f != num && !(enable_one && num == 1) ) {
		    state = 1;
		} else if( state > 0 && f == num ) {
		    state = 3;
		}
	    }
	}
    }
    return (state >= 2);
}

static char *
in_range_gettrkno(range, trk1ch0, trknames, track_rtn, warn_out)
char  *range;
int   trk1ch0;
Track *trknames;
int   *track_rtn;
int   warn_out;
{
    int  minus = 0;
    char *label;
    int  lblen;
    Event  *ep;
    int  found;
    char  delim;

    if( *range == '-' ) { range++;  minus = 1; }
    if( isdigit(*range) ) {
	*track_rtn = atoi(range);
	while( isdigit(*range) )  range++;
    } else if( trk1ch0 && ((delim = *range) == '/' || delim == ':') ) {
	label = ++range;
	lblen = 0;
	while( *range && *range != delim ) {
	    range++; lblen++;
	}
	if( *range == delim )  range++;
	if( trknames ) {
	    found = 0;
	    if( trknames->eseq ) {
		for( ep = trknames->eseq->events; ep; ep = ep->next ) {
		    if( ep->type == E_TrkName &&
		       ep->ev_len == lblen && 
		       strncmp(ep->ev_data, label, lblen) == 0 ) {
			if( found++ == 0 ) 
			    *track_rtn = (ep->track + 1);
		    }
		} 
	    }
	    if( !found ) {
		error(0L, "\"%.*s\": No such track name",
		      "\"%.*s\": トラック名が存在しません", lblen, label);
	    } else if( warn_out && found > 1 ) {
		warn(0L, 
		     "\"%.*s\": More than one identical track name is found",
		     "\"%.*s\": 同じトラック名が２つ以上あります", 
		     lblen, label);
	    }
	}
    } else {
	return NULL;
    }

    if( minus )  *track_rtn = - *track_rtn;
    return range;
}

/*
 * analyze song position string
 *   - returns -1 if error
 */
int
analy_songpos(str, pos_rtn, to_flag)
char  *str;
SongPos  *pos_rtn;
int  to_flag;	/* true for '-t' option */
{
    SongPos  pos;
    char  *p;
    int   n;
    int   neg = 0;
    char  delim;

    pos.marker = NULL;
    pos.bar = 0;
    pos.tick = 0;
	
    if( (delim = *str) == '/' || delim == ':' ) {
	for( p = ++str; *p && *p != delim; p++ );
	n = p - str;
	if( !(pos.marker = malloc(n + 1)) ) {
	    err_nomem("analy_songpos");
	}
	strncpy(pos.marker, str, n);
	pos.marker[n] = 0;
	
	str = p;
	if( *str == delim )  str++;
	if( *str == 0 )  goto finish;
	if( *str == '+' )  str++;
	else if( *str == '-' )  neg = 1, str++;
    }

    if( !(isdigit(*str) || *str == '-') )  return -1;

    pos.bar = atoi(str);
    if( *str == '-' )  str++;
    while( isdigit(*str) )  str++;

    if( *str == ':' ) {
	str++;
	pos.tick = atoi(str);
	while( isdigit(*str) )  str++;
    } else {
	/* if used with "-t" option, and if neither marker or tick is
	   specified,  increment the bar count */
	if( to_flag && pos.marker == NULL ) {
	    pos.bar++;
	}
    }
	
    if( *str != 0 )  return -1;
	    
    if( neg ) {
	pos.bar = - pos.bar;
	pos.tick = - pos.tick;
    }

 finish:
    if( pos_rtn ) {
	*pos_rtn = pos;
    } else {
	if( pos.marker )  free(pos.marker);
    }
    return 0;
}

/*
 * calculate time from SongPos structure
 */
static void
time_songpos(bp, pos, time_rtn)
Track  *bp;	/* *sorted* track including TimeSig & Marker events */
SongPos  *pos;
Rational  *time_rtn;  /* Output */
{
    Event  *ep, *mkr;
    Event  *tsig;	/* next TimeSignature event */
    int  marker_found = 0;
    int  len;
    Rational  t;
    Rational  barlen;	/* length of a bar */
    int   n;
    Rational  tmp, tmp2;

    /* first, calculate the bar number */
    if( pos->marker ) {
	len = strlen(pos->marker);
	t = r_zero;
	n = 0;
	barlen = r_one;
	if( bp->eseq ) {
	    for( ep = bp->eseq->events; ep != NULL; ep = ep->next ) {
		if( ep->type == E_Marker && len == ep->ev_len &&
		   strncmp(ep->ev_data, pos->marker, len) == 0 ) {
		    if( marker_found++ == 0 ) {
			mkr = ep;
		    }
		} else if( ep->type == E_TimeSig ) {
		    if( !marker_found || requal(&mkr->time, &ep->time) ) {
			/* n += ceil((ep->time - t) / barlen); */
			rsub(&ep->time, &t, &tmp);
			rdiv(&tmp, &barlen, &tmp);
			n += tmp.intg;
			if( tmp.num > 0 ) n++;

			t = ep->time;
			rset(ep->ev_data[0], 1 << ep->ev_data[1], &barlen);
		    }
		}
	    }
	}
	if( !marker_found ) {
	    error(0L, "\"%s\": No such marker",
		  "\"%s\": マーカーが存在しません", pos->marker);
	} else if( marker_found > 1 ) {
	    warn(0L, "\"%s\": More than one equi-named marker is found",
		  "\"%s\": 同じ名前のマーカーが２つ以上あります", pos->marker);
	}

	/* If there is no relative bar count, return now. */
	if( pos->bar == 0 ) {
	    /* return( mkr->time + pos->tick/1920 ); */
	    rset(pos->tick, IRES * 4, &tmp);
	    radd(&mkr->time, &tmp, time_rtn);
	    return;
	} 

	/* n += floor((mkr->time - t) / barlen); */
	rsub(&mkr->time, &t, &tmp);
	rdiv(&tmp, &barlen, &tmp);
	n += tmp.intg;

	/* Now, n is (the bar number - 1) where the marker is included. */

	/* If the marker event is not just on the bar boundary,
	   and if the relative bar count is negative, increment n. */
	if( pos->bar < 0 ) {
	    /* if( (mkr->time - t) % barlen != 0 ) */
	    rsub(&mkr->time, &t, &tmp);
	    rmod(&tmp, &barlen, &tmp);
	    if( !rzerop(&tmp) ) {
		n++;
	    }
	}
	n += pos->bar;
    } else {
	n = pos->bar - 1;
    }

    /* calculate the beggining time of the (n+1)-th bar */
    t = r_zero;
    barlen = r_one;
    if( bp->eseq == NULL )  tsig = NULL;
    else {
	for( tsig = bp->eseq->events; tsig != NULL; tsig = tsig->next ) {
	    if( tsig->type == E_TimeSig ) {
		rset(tsig->ev_data[0], 1 << tsig->ev_data[1], &barlen);
		break;
	    }
	}
    }
    while( tsig ) {
	/* if( t + n * barlen <= tsig->time )  break; */
	rimult(&barlen, n, &tmp);
	radd(&t, &tmp, &tmp);
	if( rlesseq(&tmp, &tsig->time) )  break; 

	/* n -= ceil((tsig->time - t) / barlen); */
	rsub(&tsig->time, &t, &tmp);
	rdiv(&tmp, &barlen, &tmp);
	n -= tmp.intg;
	if( tmp.num > 0 ) n--;

	t = tsig->time;
	rset(tsig->ev_data[0], 1 << tsig->ev_data[1], &barlen);
	for( tsig = tsig->next; tsig != NULL; tsig = tsig->next ) {
	    if( tsig->type == E_TimeSig )  break;
	}
    }

    /* return( t + n * barlen + pos->tick/1920 ); */
    rimult(&barlen, n, &tmp);
    rset(pos->tick, IRES * 4, &tmp2);
    radd(&tmp, &tmp2, &tmp);
    radd(&t, &tmp, time_rtn);
}

/*
 * warnning routines
 */
static void
warn_veloc(ep, veloc)
Event  *ep;
int  veloc;
{
    warn(0L, "Velocity out of range (time=%s type=%s n=%d v=%d)",
	 "ベロシティーが範囲外です (time=%s type=%s n=%d v=%d)",
	 rstring(&ep->time), ep->type == E_NoteOn ? "NOTE_ON" : "NOTE_OFF",
	 ep->note, veloc);
} 

static void
warn_value(ep, val)
Event  *ep;
PmmlInt  val;
{
    switch( ep->type ) {
    case E_Bend:
	warn(0L, 
	     "Pitch bend value out of range (time=%s val=%ld)",
	     "ピッチベンドの値が範囲外です (time=%s val=%ld)",
	     rstring(&ep->time), val); 
	break;
    case E_Kp:
	warn(0L, 
	     "Key pressure value out of range (time=%s n=%d val=%ld)",
	     "キープレッシャーの値が範囲外です (time=%s n=%d val=%ld)",
	     rstring(&ep->time), ep->note, val); 
	break;
    case E_Cpr:
	warn(0L, 
	     "Channel pressure value out of range (time=%s val=%ld)",
	     "チャネルプレッシャーの値が範囲外です (time=%s val=%ld)",
	     rstring(&ep->time), val); 
	break;
    case E_Prog:
	warn(0L, 
	     "Program number out of range (time=%s val=%ld)",
	     "プログラム番号が範囲外です (time=%s val=%ld)",
	     rstring(&ep->time), val); 
	break;
    default:
	warn(0L, 
	     "Control value out of range (time=%s func_no=%d val=%ld)",
	     "コントロール値が範囲外です (time=%s func_no=%d val=%ld)",
	     rstring(&ep->time), ep->type, val);
	break;
    }
}

static void
warn_tempo(ep, fpval)
Event  *ep;
PmmlFloat  fpval;
{
    warn(0L, "Bad tempo value (time=%s val=%f)",
	 "テンポの値が間違っています (time=%s val=%f)",
	 rstring(&ep->time), fpval);
}
