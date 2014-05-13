/*
 * PMML: A Compiler for the Practical Music Macro Language
 *
 * pmml.c: main routine
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

char	release[] = "0.2";
char	rcs_version[] = "$Revision: 0.67 $";
int	japan = 0;
int	trace_level = 0;
extern  int  sort_by_priority;
extern  int  no_warning;
extern	int  no_file_generated;

static	void  fprint_version P((FILE *));
static  void  simple_help P((void));
static  void  no_input_file P((char *));
static  void  no_arg P((char *, char *));
static  void  bad_track P((char *, char *));
static  void  bad_chan P((char *, char *));
static  void  bad_reso P((char *));
static  void  bad_songpos P((char *, char *, char *));
static  void  unknown_opt P((char *, int));
static  void  too_many_args P((char *));
static	void  help P((char *));


main(argc, argv)
int  argc;
char  **argv;
{
    static Object  outfile, zero, one, format, resolution;
    static Object  track_list, ch_mask, pos_begin, pos_end;
    char  *chan_list = NULL;
    char  *startup_file = NULL;
    String  src_text, prep_path;
    int	  nflag =0, dflag = 0, Rflag = 0, qflag = 0;
    int   file_no_need = 0;
    char  *p, *bname;
    int   ch, r;

    japan = (p = getenv("LANG")) && strncmp(p, "ja", 2) == 0;
    bname = basename(argv[0]);

    outfile.o_type = O_STRING;
    zero.o_type = O_INT;
    zero.o_val = 0;
    one.o_type = O_INT;
    one.o_val = 1;
    format.o_type = O_INT;
    format.o_val = -1;
    resolution.o_type = O_INT;
    resolution.o_val = -1;
    track_list.o_type = O_STRING;
    ch_mask.o_type = O_INT;
    pos_begin.o_type = O_STRING;
    pos_end.o_type = O_STRING;

    initstr(src_text);
    initstr(prep_path);

    if( argc <= 1 )  simple_help();

    /* analyze options */
    for( argc--,argv++; argc > 0 && *(p = *argv)=='-' && p[1]; argc--,argv++) {
	while( *++p ) {
	    switch( *p ) {
	    case 'h':
		help(bname);
		exit(2);
	    case 'o':
		if( p[1] ) {
		    outfile.o_str = strdup(p+1);
		} else {
		    if( --argc < 1 )  no_arg(bname, "-o");
		    outfile.o_str = strdup(*++argv);
		}
		if( !outfile.o_str )  err_nomem("main");
		goto end_loop;
	    case 'i':
		if( p[1] ) {
		    startup_file = strdup(p+1);
		} else {
		    if( --argc < 1 )  no_arg(bname, "-i");
		    startup_file = strdup(*++argv);
		}
		if( !startup_file )  err_nomem("main");
		goto end_loop;
	    case 'I':
		if( p[1] ) {
		    addstr(prep_path, p+1);
		} else {
		    if( --argc < 1 )  no_arg(bname, "-I");
		    addstr(prep_path, *++argv);
		}
		addchar(prep_path, ';');
		goto end_loop;
	    case 'q':
		qflag = 1;
		break;
	    case 'e':
		if( p[1] ) {
		    addstr(src_text, p+1);
		} else {
		    if( --argc < 1 )  no_arg(bname, "-e");
		    addstr(src_text, *++argv);
		}
		addchar(src_text, ' ');
		file_no_need = 1;
		goto end_loop;
	    case 'l':
		addstr(src_text, "load(\"");
		if( p[1] ) {
		    addstr(src_text, p+1);
		} else {
		    if( --argc < 1 )  no_arg(bname, "-l");
		    addstr(src_text, *++argv);
		}
		addstr(src_text, "\") ");
		file_no_need = 1;
		goto end_loop;
	    case 'F':
		addstr(src_text, "include(\"");
		if( p[1] ) {
		    addstr(src_text, p+1);
		} else {
		    if( --argc < 1 )  no_arg(bname, "-F");
		    addstr(src_text, *++argv);
		}
		addstr(src_text, "\") ");
		file_no_need = 1;
		goto end_loop;
	    case 'p':
		sort_by_priority = 0;
		break;
	    case 'w':
		no_warning = 1;
		break;
	    case 'T':
		if( p[1] ) {
		    track_list.o_str = strdup(p+1);
		} else {
		    if( --argc < 1 )  no_arg(bname, "-T");
		    track_list.o_str = strdup(*++argv);
		}
		if( !track_list.o_str )  err_nomem("main");
		if( in_range(0, track_list.o_str, 1, 1, NULL) == -1 ) {
		    bad_track(bname, track_list.o_str); 
		}
		goto end_loop;
	    case 'c':
		if( p[1] ) {
		    chan_list = strdup(p+1);
		} else {
		    if( --argc < 1 )  no_arg(bname, "-c");
		    chan_list = strdup(*++argv);
		}
		if( !chan_list )  err_nomem("main");
		if( in_range(0, chan_list, 0, 0, NULL) == -1 ) {
		    bad_chan(bname, chan_list); 
		}
		goto end_loop;
	    case 'f':
		if( p[1] ) {
		    pos_begin.o_str = strdup(p+1);
		} else {
		    if( --argc < 1 )  no_arg(bname, "-f");
		    pos_begin.o_str = strdup(*++argv);
		}
		if( !pos_begin.o_str )  err_nomem("main");
		if( analy_songpos(pos_begin.o_str, NULL, 0) == -1 ) {
		    bad_songpos(bname, "-f", pos_begin.o_str); 
		}
		goto end_loop;
	    case 't':
		if( p[1] ) {
		    pos_end.o_str = strdup(p+1);
		} else {
		    if( --argc < 1 )  no_arg(bname, "-t");
		    pos_end.o_str = strdup(*++argv);
		}
		if( !pos_end.o_str )  err_nomem("main");
		if( analy_songpos(pos_end.o_str, NULL, 1) == -1 ) {
		    bad_songpos(bname, "-t", pos_end.o_str); 
		}
		goto end_loop;
	    case 'r':
		if( p[1] ) {
		    r = atoi(p+1);
		} else {
		    if( --argc < 1 )  no_arg(bname, "-r");
		    r = atoi(*++argv);
		}
		if( r < 0 )  bad_reso(bname);
		resolution.o_val = r;
		goto end_loop;
	    case '0':  format.o_val = 0;  break;
	    case '1':  format.o_val = 1;  break;
	    case '2':  format.o_val = 2;  break;
	    case 'n':  nflag = 1; break;
	    case 'd':  dflag = 1; break;
	    case 'R':  Rflag = 1; break;
	    default:
		unknown_opt(bname, *p);
	    }
	}
    end_loop:;
    }
    
    if( !file_no_need && argc < 1 )  no_input_file(bname);
    if( argc > 1 )  too_many_args(bname);

    /* call initialization routines */
    addchar(prep_path, 0);
    init_input(getstr(prep_path));
    init_expression();
    init_parse();
    init_builtin();	/* must be called after init_parse() */
    init_output();

    /* set input file and parse initialization block if it exists */
    if( argc >= 1 ) {
	pushbk_file(*argv, 0);
	parse_initblk();
    }

    /* set pre-defined macros */
    if( !outfile.o_str ) {
	if( argc < 1 || strcmp(*argv, "-") == 0 ) {
	    outfile.o_str = strdup(OUT_MID);
	} else {
	    outfile.o_str = replace_extension(*argv, MFILE_EXT);
	}
	if( !outfile.o_str )  err_nomem("main");
    }
    define_macro("$outfile", hash_func("$outfile"), 
		 root_thd, &outfile, NULL);

    if( format.o_val >= 0 ) {
	define_macro("$format", hash_func("$format"),
		     root_thd, &format, NULL);
    }
    if( nflag ) {
	define_macro("$no_retrigger", hash_func("$no_retrigger"),
		     root_thd, &one, NULL);
    }
    if( dflag ) {
	define_macro("$pack_tracks", hash_func("$pack_tracks"),
		     root_thd, &one, NULL);
    }
    if( Rflag ) {
	define_macro("$no_run_stat", hash_func("$no_run_stat"),
		     root_thd, &one, NULL);
    }
    if( track_list.o_str ) {
	define_macro("$track_list", hash_func("$track_list"),
		     root_thd, &track_list, NULL);
    }
    if( chan_list ) {
	ch_mask.o_val = 0xffff;
	for( ch = 0; ch < 16; ch++ ) {
	    if( in_range(ch+1, chan_list, 0, 0, NULL) ) {
		ch_mask.o_val &= ~(1 << ch);
	    }
	}
	define_macro("$ch_mask", hash_func("$ch_mask"),
		     root_thd, &ch_mask, NULL);
    }
    if( pos_begin.o_str ) {
	define_macro("$pos_begin", hash_func("$pos_begin"),
		     root_thd, &pos_begin, NULL);
    }
    if( pos_end.o_str ) {
	define_macro("$pos_end", hash_func("$pos_end"),
		     root_thd, &pos_end, NULL);
    }
    if( resolution.o_val >= 0 ) {
	define_macro("$resolution", hash_func("$resolution"),
		     root_thd, &resolution, NULL);
    }
    define_macro("$japan", hash_func("$japan"),
		 root_thd, japan ? &one : &zero, NULL);

    /* push back the "-e" option argument */
    addchar(src_text, 0);
    cur_srcpos = SRCPOS_CMDLN;
    pushbk_evalstr(src_text.buf);

    /* include start-up file */
    cur_srcpos = 0;
    if( !qflag ) {
	if( !startup_file ) {
	    if( !(startup_file = replace_extension(bname, SRC_EXT)) ) {
		err_nomem("main");
	    }
	}
	pushbk_file(startup_file, 1);
    }

    /* parser main */
    parse_main();
    end_parse();

    if( no_file_generated )  exit(3);
    else  exit(0);
}

static void
fprint_version(fp)
FILE  *fp;
{
    char  *p;

    p = malloc(sizeof(rcs_version));
    *p = 0;
    sscanf(rcs_version, "$Revision: %s", p);
    fprintf(fp, "PMML Compiler   Release %s (Compiler version %s)\n", 
	    release, p);
}

static void
simple_help()
{
    fprint_version(stderr);
    if( japan ) {
	fprintf(stderr, "ヘルプを見るには 'pmml -h' を実行して下さい。\n");
    } else {
	fprintf(stderr, "Type 'pmml -h' for help.\n");
    }
    exit(2);
}

static void
no_input_file(bname)
char  *bname;
{
    if( japan ) {
	fprintf(stderr, "%s: 入力ファイルが指定されていません\n", bname);
    } else {
	fprintf(stderr, "%s: No input file specified\n", bname);
    }
    exit(1);
}

static void
no_arg(bname, optstr)
char  *bname;
char  *optstr;
{
    if( japan ) {
	fprintf(stderr, "%s: '%s' に対する引数がありません\n", bname, optstr);
    } else {
	fprintf(stderr, "%s: Argument to '%s' is missing\n", bname, optstr);
    }
    exit(1);
}

static void
bad_track(bname, track_list)
char  *bname; 
char  *track_list;
{
    if( japan ) {
	fprintf(stderr, "%s: トラック番号指定が間違っています '-T %s'\n", 
		bname, track_list);
    } else {
	fprintf(stderr, "%s: Bad track number list '-T %s'\n", 
		bname, track_list);
    }
    exit(1);
}

static void
bad_chan(bname, chan_list)
char  *bname; 
char  *chan_list;
{
    if( japan ) {
	fprintf(stderr, "%s: チャネル番号指定が間違っています '-c %s'\n", 
		bname, chan_list);
    } else {
	fprintf(stderr, "%s: Bad channel number list '-c %s'\n", 
		bname, chan_list);
    }
    exit(1);
}

static void
bad_reso(bname)
char  *bname; 
{
    if( japan ) {
	fprintf(stderr, "%s: '-r': 分解能の値が間違っています\n", bname);
    } else {
	fprintf(stderr, "%s: '-r': Bad resolution value\n", bname);
    }
    exit(1);
}

static void
bad_songpos(bname, opt, pos)
char  *bname; 
char  *opt, *pos;
{
    if( japan ) {
	fprintf(stderr, "%s: 演奏位置指定が間違っています '%s %s'\n", 
		bname, opt, pos);
    } else {
	fprintf(stderr, "%s: Bad song position specifier '%s %s'\n", 
		bname, opt, pos);
    }
    exit(1);
}

static void
unknown_opt(bname, opt)
char  *bname; 
int  opt;
{
    if( japan ) {
	fprintf(stderr, "%s: '-%c' は有効なオプションではありません\n", 
		bname, opt);
    } else {
	fprintf(stderr, "%s: Unknown option '-%c'\n", bname, opt);
    }
    exit(1);
}

static void
too_many_args(bname)
char  *bname; 
{
    if( japan ) {
	fprintf(stderr, "%s: 余計なコマンドライン引数があります\n", bname);
    } else {
	fprintf(stderr, "%s: Too many command-line arguments\n", bname);
    }
    exit(1);
}

static void
help(bname)
char  *bname;
{
    static char *ehelp[] = {
	"Options:",
	"  -0       generate format-0 standard MIDI file",
	"  -1       generate format-1 standard MIDI file",
	"  -2       generate format-2 standard MIDI file",
	"  -c ch,ch-ch,...",
	"           output only the specified MIDI channel(s)",
	"  -d       delete empty tracks",
	"  -e PMML_source_text",
	"           evaluate the PMML source text",
	"  -f {bar[:tick], /marker, /marker/{+,-}bar[:tick]}",
	"           start playing from the specified location",
	"  -h       show help",
	"  -i file  change the name of start-up file",
	"  -l file  load standard MIDI file (same as -e 'load(\"file\")')",
	"  -n       do not adjust note events in case of note collision",
	"  -o file  change the name of output MIDI file",
	"  -p       do not sort events by their priority",
	"  -q       do not include the start-up file",
	"  -r N     set output resolution to N (default = 480)",
	"  -t {bar[:tick], /marker, /marker/{+,-}bar[:tick]}",
	"           stop playing at the specified location",
	"  -w       suppress warning output",
	"  -F file  read PMML source file (same as -e 'include(\"file\")')",
	"  -I dir   add dir to the head of the source file search path",
	"  -R       do not apply running status rule",
	"  -T track,track-track,/trackname/,...",
	"           output only the specified track(s)",
	NULL
    };
    static char *jhelp[] = {
	"オプション:",
	"  -0       format 0 の標準MIDIファイルを生成する",
	"  -1       format 1 の標準MIDIファイルを生成する",
	"  -2       format 2 の標準MIDIファイルを生成する",
	"  -c ch,ch-ch,...",
	"           指定されたMIDIチャネルだけを出力する",
	"  -d       空のトラックを削除する",
	"  -e PMMLソース文字列",
	"           PMMLソース文字列を評価する",
	"  -f {小節数[:ティック], /マーカ, /マーカ/{+,-}小節数[:ティック]}",
	"           指定された位置から演奏を開始する",
	"  -h       ヘルプメッセージを出力する",
	"  -i file  初期化ファイルの名前を変更する",
	"  -l file  標準MIDIファイルをロードする (-e 'load(\"file\")' に同じ)",
	"  -n       ノート衝突時におけるノートイベントの修正を行わない",
	"  -o file  出力標準MIDIファイルの名前を変更する",
	"  -p       優先度によるイベントのソートを行わない",
	"  -q       初期化ファイルを読み込まない",
	"  -r N     出力分解能を N に設定する (省略時: 480)",
	"  -t {小節数[:ティック], /マーカ, /マーカ/{+,-}小節数[:ティック]}",
	"           指定された位置で演奏を中止する",
	"  -w       警告出力を抑制する",
	"  -F file  PMMLソースファイルを読み込む (-e 'include(\"file\")' に同じ)",
	"  -I dir   dir をソースファイル検索パスの先頭に追加する",
	"  -R       ランニング・ステータス・ルールを適用しない",
	"  -T track,track-track,/trackname/,...",
	"           指定されたトラックだけを出力する",
	NULL
    };
    char  **p;

    fprint_version(stdout);
    printf("Copyright (C) 1997,1998  Satoshi Nishimura\n");
    printf("This program comes with ABSOLUTELY NO WARRANTY.\n\n");

    if( japan ) {
	printf("使用法: %s [オプション] ファイル名\n", bname);
	for( p = jhelp; *p != NULL; p++ )  puts(*p);
    } else {
	printf("usage: %s [options] filename\n", bname);
	for( p = ehelp; *p != NULL; p++ )  puts(*p);
    }
}
