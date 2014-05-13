/*
 * M2P: A SMF-to-PMML Translator
 *   
 * m2p.c: main routine
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

#include <stdio.h>
#include <ctype.h>
#include "m2p.h"

char	release[] = "0.2";
char	rcs_version[] = "$Revision: 0.13 $";

static void	output_track P((Track *, Options *, int, int));
static void	analyze_options P((int *, char ***, Options *, char *));
static void	decompose_string P((char *, int *, char ***));
static int	in_range P((int, char *, int));
static void	fprint_version P((FILE *));
static void	no_input_file P((char *));
static void	no_arg P((char *, char *));
static void	unknown_opt P((char *, int));
static void	too_many_args P((char *));
static void	bad_track P((char *, char *));
static void	bad_chan P((char *, char *));
static void	simple_help P((void));
static void	help P((char *));


main(argc, argv)
int  argc;
char  **argv;
{
    SMFInfo  smf;
    int  tk, ch;
    Track  *tp;
    Options  opt;
    char  *bname = basename(argv[0]);
    int  env_argc;
    char **env_argv;
    int  first_trk, trk_occupied;
    
    if( argc <= 1 )  simple_help();
    
    opt.mode = EPL;
    opt.measure_on = 1;
    opt.toffset = -1;	/* guess from SMF contents */
    opt.use_len_name = 1;
    opt.sep_note_ev = 0;
    opt.sym_ctrl = 1;
    opt.ignore_chan = 0;
    opt.split_mode = 1;
    opt.chord_on = 1;
    opt.cd_mindup = CD_MINDUP;
    opt.linefit_on = 0;
    opt.lf_maxtmstep = LF_MAXTMSTEP;
    opt.lf_tolerance = LF_TOLERANCE;
    opt.max_ovs_len = MAX_OVS_LEN;
    opt.inherit_v = 0;
    opt.track_list = "";
    opt.chan_list = "";
    opt.use_gm_name = 0;

    /* analyze options */
    decompose_string(getenv("M2POPT"), &env_argc, &env_argv);
    analyze_options(&env_argc, &env_argv, &opt, "\"M2POPT\"");
    argc--, argv++;
    analyze_options(&argc, &argv, &opt, bname);

    if( argc < 1 )  no_input_file(bname);
    else if( argc > 1 )  too_many_args(bname);

    /* read instrumental name file */
    if( opt.use_gm_name )  read_gm_files();

    /* read the Standard MIDI File */
    read_smf(*argv, &smf);

    /* Now that the SMF resolution is known, fix some option values */
    if( opt.toffset > 0 ) {
	opt.toffset = opt.toffset * smf.resolution / IRES;
    }
    opt.lf_maxtmstep = (long)opt.lf_maxtmstep * smf.resolution / IRES;
    opt.max_ovs_len = (long)opt.max_ovs_len * smf.resolution / IRES;
    
    /* output header part */
    if( smf.resolution != IRES || !(smf.format == 1 && smf.ntrks > 1) ) {
	fprintf(OUT, "%%{ ");
	if( !(smf.format == 1 && smf.ntrks > 1) ) {
	    fprintf(OUT, "$format = %d ", smf.format);
	}
	if( smf.resolution != IRES ) {
	    fprintf(OUT, "$resolution = %d ", smf.resolution);
	}
	fprintf(OUT, "}\n");
    }
    if( opt.use_gm_name ) {
	fprintf(OUT, "include(\"gm\")\n");
    }

    /* split up each track and do line fitting */
    for( tk = 1, tp = smf.trks; tk <= smf.ntrks; tk++, tp++ ) {
	/* split by MIDI channels */
	csplit(tp);

	/* Now, tp->eseq contains channel-independent events */
	if( tp->eseq ) {
	    /* split by event types */
	    esplit(&tp->eseq); 
	    
	    /* line fitting */
	    if( opt.linefit_on ) {
		line_fit(tp->eseq, opt.lf_maxtmstep, opt.lf_tolerance); 
	    }
	}

	/* tp->eseqch[ch] contains events for each channel */
	for( ch = 0; ch < 16; ch++ ) { 
	    if( tp->eseqch[ch] ) {
		/* split by event types */
		esplit(&tp->eseqch[ch]);

		/* make note-on and note-off events related.  Beware that
		   note events always reside in the first event sequence. */
		pair_notes(tp->eseqch[ch]->events);

		/* line fitting */
		if( opt.linefit_on ) {
		    line_fit(tp->eseqch[ch], 
			     opt.lf_maxtmstep, opt.lf_tolerance); 
		}
	    }
	}
    }

    /* output each track */
    first_trk = 1;
    for( tk = 1, tp = smf.trks; tk <= smf.ntrks; tk++, tp++ ) {
        /* initialize time-signature & key-signature information */
	if( tk == 1 || smf.format == 2 ) {
	    init_measure(tp->eseq, 
			 opt.toffset >= 0 ? opt.toffset :
			 (opt.measure_on ? guess_toffset(&smf, tk) : 0L));
	    init_ksinfo(tp->eseq, KS_GLOBAL); 
	} else {
	    init_ksinfo(tp->eseq, KS_LOCAL); 
	}

	/* skip unintended tracks */
	if( !in_range(tk, opt.track_list, 0) )  continue;

	/* delete event list of unintended channels */
	if( !in_range(0, opt.chan_list, 0) )  {
	    tp->eseq = NULL;   /* free? why should we do that? */
	}
	for( ch = 0; ch < 16; ch++ ) { 
	    if( !in_range(ch+1, opt.chan_list, 0) )  tp->eseqch[ch] = NULL;
	}

	/* merge sequences if "split by event type" is turned off. */
	if( opt.split_mode < 3 ) {
	    merge_evlist(tp->eseq);
	    for( ch = 0; ch < 16; ch++ ) { 
		merge_evlist(tp->eseqch[ch]);
	    }
	}

	/* merge sequences if "split by channel" is turned off. */
        if( opt.split_mode == 0 || opt.split_mode == 1 && smf.format != 0 ) {
	    cmerge(tp);
	}

	/* Now, output the track contents */
	trk_occupied = tp->eseq ? 1 : 0;
	for( ch = 0; ch < 16; ch++ ) {
	    if( tp->eseqch[ch] )  trk_occupied = 1;  
	}
	if( trk_occupied ) { 
	    if( first_trk ) {
		first_trk = 0;
	    } else {
		fprintf(OUT, "\n");
	    }
	    output_track(tp, &opt, tk, smf.format);
	}
    }

    exit(0);
}

/*
 * output one track
 */
static void
output_track(tp, opt, tk, format)
Track  *tp;
Options  *opt;
int  tk;
int  format;
{
    EventSeq  *sp;
    int  ch, cnt;
    void (*outfunc)() = opt->mode == MPL ? mplout : eplout;
    
    fprintf(OUT, "/* === Track %d === */\n", tk);
    if( format != 0 || tp->eseq ) {
	fprintf(OUT, "tk=%d\n", tk);
    }
    
    cnt = 0;
    for( sp = tp->eseq; sp; sp = sp->next ) {
	if( cnt++ )  fprintf(OUT, "\n");
	(*outfunc)(sp->events, opt, format == 1 && tk > 1);
    }
    for( ch = 0; ch < 16; ch++ ) { 
	for( sp = tp->eseqch[ch]; sp; sp = sp->next ) {
	    if( cnt++ )  fprintf(OUT, "\n");
	    if( format == 0 && sp == tp->eseqch[ch] ) {
		fprintf(OUT, "tk=%d\n", ch+2);
	    }
	    (*outfunc)(sp->events, opt, format == 1 && tk > 1);
	}
    }
}

/*
 * decompose a string into tokens (delimiter is the space charater) 
 */
static void
decompose_string(str, argc_p, argv_p)
char  *str;
int  *argc_p;	  /* output */
char  ***argv_p;  /* output */
{
    char  *p;
    int  cnt;

    if( !str ) {
	*argc_p = 0;
	return;
    }

    if( !(p = malloc(strlen(str)+2)) )  err_nomem("decompose_string");
    *p++ = '\0';  /* sentinel */
    strcpy(p, str);
    
    for(cnt = 0; *p != '\0'; cnt++) {
	while( isspace(*p) )  *p++ = '\0';
	while( *p != '\0' && !isspace(*p) )  p++;
    }
    if( (*argc_p = cnt) > 0 ) {
	if( !(*argv_p = (char **)malloc(sizeof(char *) * cnt)) ) {
	    err_nomem("decompose_string");
	}
	while( --cnt >= 0 ) {
	    while( *--p != '\0' );
	    (*argv_p)[cnt] = p+1;
	}
    }
}

/*
 * analyze option strings
 */
static void
analyze_options(argc_p, argv_p, opt, bname)
int  *argc_p;	  /* input/output */
char  ***argv_p;  /* input/output */
Options  *opt;    /* output */
char  *bname;
{
    int  argc = *argc_p;
    char  **argv = *argv_p;
    int  ochar;
    char  *p;

    while( argc > 0 && 
	  ((ochar = *(p = *argv)) == '-' || ochar == '+') && p[1] ) {
	while( *++p ) {
	    switch( *p ) {
	    case 'h':
		help(bname);
		exit(2);
	    case 'a': opt->mode = EPL_A; break;
	    case 'e': opt->mode = EPL; break;
	    case 'm': 
		opt->mode = MPL;
		opt->linefit_on = 1;
		break;
	    case 'y': opt->mode = EPL_Y; break;
	    case 'd': opt->sep_note_ev = (ochar == '-');  break; 
	    case 'g': opt->chord_on = !(ochar == '-');  break;
	    case 'i': opt->ignore_chan = (ochar == '-'); break;
	    case 'l': opt->linefit_on = (ochar == '-'); break;
	    case 'n': opt->measure_on = !(ochar == '-'); break;
	    case 'u': opt->use_len_name = !(ochar == '-'); break;
	    case 'v': opt->inherit_v = (ochar == '-'); break;
	    case 'G': opt->use_gm_name = (ochar == '-'); break;
		
	    case 's':
		if( p[1] >= '0' && p[1] <= '3' ) {
		    opt->split_mode = *++p - '0';
		} else  no_arg(bname, "-s");
		break;

	    case 'o':
		if( p[1] ) {
		    if( !isdigit(p[1]) )  no_arg(bname, "-o");
		    opt->toffset = atoi(p+1);
		} else {
		    ++argv;
		    if( --argc < 1 || !isdigit(**argv) )  no_arg(bname, "-o");
		    opt->toffset = atoi(*argv);
		}
		goto end_loop;

	    case 'c':
		if( p[1] ) {
		    opt->chan_list = strdup(p+1);
		} else {
		    if( --argc < 1 )  no_arg(bname, "-c");
		    opt->chan_list = strdup(*++argv);
		}
		if( !opt->chan_list )  err_nomem("main");
		if( in_range(0, opt->chan_list, 0) == -1 ) {
		    bad_chan(bname, opt->chan_list); 
		}
		goto end_loop;

	    case 'T':
		if( p[1] ) {
		    opt->track_list = strdup(p+1);
		} else {
		    if( --argc < 1 )  no_arg(bname, "-T");
		    opt->track_list = strdup(*++argv);
		}
		if( !opt->track_list )  err_nomem("main");
		if( in_range(0, opt->track_list, 0) == -1 ) {
		    bad_track(bname, opt->track_list); 
		}
		goto end_loop;

	    default:
		unknown_opt(bname, *p);
	    }
	}
    end_loop:
	argc--, argv++;
    }

    *argc_p = argc;
    *argv_p = argv;
}

/*
 * determine if a number is in the range or not
 *   - returns -1 if error
 */
static int
in_range(num, range, enable_one)
int   num; 
char  *range;
int   enable_one;	/* flag for implicit enabling of track 1 */
{
    int  f, f_neg, t;
    int  state = 2;	/* 0:explicitly disabled
			   1:disabled
			   2:enabled
			   3:explicitly enabled  */

    while( *range ) {
	while( isspace(*range) || *range == ',' )  range++;
	if( !*range )  break;
	if( *range == '-' )  f_neg = 1, range++;  else  f_neg = 0;
	if( !isdigit(*range) )  return -1;
	f = atoi(range);
	if( *range == '-' )  range++;
	while( isdigit(*range) )  range++;
	while( isspace(*range) )  range++;
	if( *range == '-' ) {
	    range++;
	    while( isspace(*range) )  range++;
	    if( !isdigit(*range) && *range != '-' )  return -1;
	    t = atoi(range);
	    if( *range == '-' )  range++;
	    while( isdigit(*range) )  range++;

	    if( f_neg ) {
		t = t < 0 ? -t : t;
		if( num >= f && num <= t )  state = 0;
	    } else {
		if( state == 2 && !(num >= f && num <= t) &&
		   !(enable_one && num == 1) ) {
		    state = 1;
		} else if( state > 0 && (num >= f && num <= t) ) {
		    state = 3;
		}
	    }
	} else {
	    if( f_neg ) {
		if( f == num )  state = 0;
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

/*
 * message output routines
 */
static void
fprint_version(fp)
FILE  *fp;
{
    char  *p;

    p = malloc(sizeof(rcs_version));
    *p = 0;
    sscanf(rcs_version, "$Revision: %s", p);
    fprintf(fp, "SMF-to-PMML Translator  Release %s (m2p version %s)\n", 
	    release, p);
}

static void
simple_help()
{
    fprint_version(stderr);
    fprintf(stderr, "Type 'm2p -h' for help.\n");
    exit(2);
}

static void
no_input_file(bname)
char  *bname;
{
    fprintf(stderr, "%s: No input file specified\n", bname);
    exit(1);
}

static void
no_arg(bname, optstr)
char  *bname;
char  *optstr;
{
    fprintf(stderr, "%s: Argument to '%s' is missing\n", bname, optstr);
    exit(1);
}

static void
unknown_opt(bname, opt)
char  *bname; 
int  opt;
{
    fprintf(stderr, "%s: Unknown option '-%c'\n", bname, opt);
    exit(1);
}

static void
too_many_args(bname)
char  *bname; 
{
    fprintf(stderr, "%s: Too many command-line arguments\n", bname);
    exit(1);
}

static void
bad_track(bname, track_list)
char  *bname; 
char  *track_list;
{
    fprintf(stderr, "%s: Bad track number list '-T %s'\n", bname, track_list);
    exit(1);
}

static void
bad_chan(bname, chan_list)
char  *bname; 
char  *chan_list;
{
    fprintf(stderr, "%s: Bad channel number list '-c %s'\n", bname, chan_list);
    exit(1);
}

static void
help(bname)
char  *bname;
{
    static char *help[] = {
	"Options:",
	"  -a       absolute-time event-per-line mode",
	"  -c ch,ch-ch,...",
	"           output only the specified MIDI channel(s)",
        "  -d       show note-on and note-off events separately in event-per-line mode",
	"  -e       relative-time event-per-line mode (default)",
	"  -g       do not group notes into chords in meausre-oriented mode",
	"  -h       show help",
	"  -i       ignore MIDI channel numbers",
	"  -l       use ctrl_to() if possible", 
	"  -m       measure-per-line mode (This includes '-l'.)",
	"  -o time  set the length of preamble part of music (in 1/480 of quarter note)",
	"  -n       do not show measure numbers",
	"  -s0      do not split tracks",
	"  -s1      split tracks by MIDI channel if the input is Format-0 (default)",
	"  -s2      split tracks by MIDI channel",
	"  -s3      split tracks by MIDI channel and event type",
	"  -u       do not use symbolic time values",
	"  -v       inherit velocity value across measures in measure-per-line mode",
	"  -y       yet another relative-time event-per-line mode",
	"  -G       use General MIDI instrument names",
	"  -T track,track-track,...",
	"           output only the specified track(s)",
	"",
	"Options are also taken from the environment variable \"M2POPT\".",
	"To reset an option to its default setting, use \"+\" instead of \"-\".", 
	"",
	"Examples:",
	"  m2p file       Event-per-line mode with displaying measure numbers",
	"  m2p -m file    Measure-per-line mode with chord grouping",
	"  m2p -anu file  Output in the simplest form",
	"  m2p -ds0 file  Output with strictly preserving the original order of events",
	NULL
    };
    char  **p;

    fprint_version(stdout);
    printf("Copyright (C) 1998  Satoshi Nishimura\n");
    printf("This program comes with ABSOLUTELY NO WARRANTY.\n\n");

    printf("usage: %s [options] midifile\n", bname);
    for( p = help; *p != NULL; p++ )  puts(*p);
}

/*
 * no-memory error
 */
void
err_nomem(str)
char  *str;
{
    fprintf(stderr, "Sorry. Can not continue discompilation due to the lack of memory space (%s)\n", str);
    exit(1);
}
