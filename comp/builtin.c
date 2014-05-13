/*
 * PMML: A Compiler for the Practical Music Macro Language
 *
 * builtin.c: handlers for built-in macros 
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
#include <math.h>
#include <ctype.h>
#include <time.h>
#include <unistd.h>
#include "pmml.h"

static void   do_rand P((int, Object *));
static void   do_irand P((int, Object *));
static void   do_srand P((int, Object *));
static void   do_frand P((int, Object *));
static void   do_fgrand P((int, Object *));
static void   do_floor P((int, Object *));
static void   do_ceil P((int, Object *));
static void   do_abs P((int, Object *));
static void   do_sqrt P((int, Object *));
static void   do_sin P((int, Object *));
static void   do_cos P((int, Object *));
static void   do_exp P((int, Object *));
static void   do_log P((int, Object *));
static void   do_atan2 P((int, Object *));
static void   do_int P((int, Object *));
static void   do_float P((int, Object *));
static void   do_rational P((int, Object *));
static void   do_intg P((int, Object *));
static void   do_num P((int, Object *));
static void   do_den P((int, Object *));
static void   do_strlen P((int, Object *));
static void   do_substr P((int, Object *));
static void   do_index P((int, Object *));
static void   do_charcode P((int, Object *));
static void   do_atoi P((int, Object *));
static void   do_atof P((int, Object *));
static void   do_dumpdef P((int, Object *));
static void   do_defined P((int, Object *));
static void   do_idtype P((int, Object *));
static void   do_idstr P((int, Object *));
static void   do_type P((int, Object *));
static void   do_dup P((int, Object *));
static void   do_rep P((int, Object *));
static void   do_print P((int, Object *));
static void   do_eprint P((int, Object *));
static void   do_printf P((int, Object *));
static void   do_eprintf P((int, Object *));
static void   do_sprintf P((int, Object *));
static void   do_error P((int, Object *));
static void   do_warn P((int, Object *));
static void   do_exit P((int, Object *));
static void   do_ntrk P((int, Object *));
static void   do_format P((int, Object *));
static void   do_resolution P((int, Object *));
static void   do_toupper P((int, Object *));
static void   do_tolower P((int, Object *));
static void   do_notename P((int, Object *));
static void   do_nth_token P((int, Object *));
static void   do_concat P((int, Object *));
static void   do_showeffs P((int, Object *));
static void   do_dumpdict P((int, Object *));
static void   do_trace P((int, Object *));
static void   do_seteflags P((int, Object *));
#ifdef DEBUG
void  dump_pbstack();
#endif

/*
 * table of built-in macros
 */
static struct builtin {
    char  *name;
    char  *arg_spec;
    void (*handler)();
} builtin_tab[] = {
    { "rand",		"",	do_rand },
    { "irand",		"i:i",	do_irand },
    { "srand",		":i",	do_srand },
    { "frand",		"i:i",	do_frand },
    { "fgrand",		"i:ii",	do_fgrand },
    { "floor",		"n",	do_floor },
    { "ceil",		"n",	do_ceil },
    { "abs",		"n",	do_abs },
    { "sqrt", 		"f",	do_sqrt },
    { "sin", 		"f",	do_sin },
    { "cos", 		"f",	do_cos },
    { "exp", 		"f",	do_exp },
    { "log", 		"f",	do_log },
    { "atan2", 		"ff",	do_atan2 },
    { "int",		"i",	do_int },
    { "float",		"f",	do_float },
    { "rational",	"r:i",	do_rational },
    { "intg",		"r",	do_intg },
    { "num",		"r",	do_num },
    { "den",		"r",	do_den },
    { "strlen",		"s",	do_strlen },
    { "substr",		"si:i",	do_substr },
    { "index",		"ss",	do_index },
    { "charcode",	"s:i",	do_charcode },
    { "atoi",		"s",	do_atoi },
    { "atof",		"s",	do_atof },
    { "dumpdef",        "y",    do_dumpdef },
    { "defined", 	"y", 	do_defined },
    { "idtype", 	"y", 	do_idtype },
    { "idstr", 		"y", 	do_idstr },
    { "type", 		"e", 	do_type },
    { "dup",		"a",	do_dup },
    { "rep",		"ie",	do_rep },
    { "print",		"e*",	do_print },
    { "eprint",		"e*",	do_eprint },
    { "printf",         "sp*",	do_printf },
    { "eprintf",        "sp*",	do_eprintf },
    { "sprintf",        "sp*",	do_sprintf },
    { "error",          "sp*",	do_error },
    { "warn",           "sp*",	do_warn },
    { "exit",           "i",	do_exit },
    { "ntrk",		"s",	do_ntrk },
    { "format",		"s",	do_format },
    { "resolution",	"s",	do_resolution },
    { "toupper",	"s",	do_toupper },
    { "tolower",	"s",	do_tolower },
    { "notename",	"i:i",	do_notename },
    { "nth_token",	"yi",	do_nth_token },
    { "concat",		"a:s",	do_concat },
    { "showeffs",	"",	do_showeffs },
    { "dumpdict",	":sy",	do_dumpdict },
    { "trace",		"i",	do_trace },
    { "seteflags",	"n",	do_seteflags },
#ifdef DEBUG
    { "dump_pbstack",	"",	dump_pbstack },
#endif
};

/*
 * type definitions for 1/f random number generators
 */
struct frand_stat {
    struct frand_stat  *next;
    int  key;
    long  last;
};

#define NDICESLOG2  3
#define NDICES      (1 << NDICESLOG2)

struct fgrand_stat {
    struct fgrand_stat  *next;
    int  key;
    long  dice[NDICES];
    long  cnt;
};

/*
 * global variables
 */
static String  strbuf;
static struct frand_stat  *frand_states;
static struct fgrand_stat  *fgrand_states;

/*********************************************************************
 * Initializer (install built-in macros to the dictionary)
 *********************************************************************/
void
init_builtin()
{
    int  i;
    struct builtin  *p;
    DicEnt  *dp;

    for( i = 0, p = builtin_tab; 
	 i < (int)(sizeof(builtin_tab)/sizeof(struct builtin));  i++, p++ ) {
	if( !(dp = (DicEnt *) malloc(sizeof(DicEnt))) ) {
	    err_nomem("init_builtin");
	}
	dp->type = D_BuiltIn;
	if( !(dp->name = strdup(p->name)) ) {
	    err_nomem("init_builtin");
	}
	dp->hash = hash_func(p->name);
	dp->scope.c = root_thd;
	dp->active = 0;
	dp->dic_arg_spec = p->arg_spec;
	dp->dic_handler = p->handler;
	insert_dict(dp);
    }
}

/*********************************************************************
 * Handlers for each built-in macro
 *********************************************************************/
/*
 * rand(): returns random floating-point number whose scope is [0, 1]
 */
static void
do_rand(argc, argv)
int  argc;
Object  *argv;
{
    pushbk_float(random() * 4.656612875e-10);	/* random * 1/(2**31-1) */
}

/*
 * irand(n [,m]): returns random integer whose scope is [1, n] or [n, m]
 */
static void
do_irand(argc, argv)
int  argc;
Object  *argv;
{
    if( argc == 2 ) {
	pushbk_int( random() % (argv[1].o_val - argv[0].o_val + 1) 
		   + argv[0].o_val );
    } else {
	pushbk_int( random() % argv[0].o_val + 1 ); 
    }
}

/*
 * srand([n]): set a seed for rand()
 *
 *  This also clears the history information of frand and fgrand.
 */
static void
do_srand(argc, argv)
int  argc;
Object  *argv;
{
    if( argc == 0 ) {
	srandom((int) time(0L) + getpid());
    } else {
	srandom((int) argv->o_val);
    }

    {
	struct frand_stat  *sp, *sp_next;

	for( sp = frand_states; sp; sp = sp_next ) {
	    sp_next = sp->next;
	    free(sp);
	}
	frand_states = NULL;
    }
	
    {
	struct fgrand_stat  *sp, *sp_next;

	for( sp = fgrand_states; sp; sp = sp_next ) {
	    sp_next = sp->next;
	    free(sp);
	}
	fgrand_states = NULL;
    }
}

/*
 * frand(n [, key]): generate 1/f random integers with uniform distribution
 *
 * n must be a power of 2.  The scope of generated numbers is [0, n-1]. 
 *
 * Reference: C. Dodge and T. Jerse, "Computer Music", Schirmer books, p. 290.
 */
static void
do_frand(argc, argv)
int  argc;
Object  *argv;
{
    int  key = (argc >= 2) ? argv[1].o_val : 0;
    int  n = argv->o_val;
    int  k, i;
    struct frand_stat  *sp;

    /* find the status entery that matches the key */ 
    for( sp = frand_states; sp; sp = sp->next ) {
	if( sp->key == key ) goto found;
    }
    if( !(sp = (struct frand_stat *) malloc(sizeof(struct frand_stat))) ) {
	err_nomem("do_frand");
    }
    sp->key = key;
    sp->next = frand_states;
    sp->last = random();
    frand_states = sp;
 found:

    /* k will be the maximum integer satisfying n >= 2 ** k */ 
    for( k = 0; n > 1; k++ )  n >>= 1;

    /* generate the next random number */
    for( i = 0; i < k; i++ ) {
	if( random() % (1 << (i+1)) == 0 ) {
	    sp->last ^= 1 << i;
	}
    }
    
    pushbk_int(sp->last & ((1 << k) - 1));
}

/*
 * fgrand(n [, m[, key]]): generate 1/f random integers with Gaussian-like
 *                         distribution whose scope is [1,n] or [n,m] 
 *
 * Reference: M. Gardner, "White and Brown Music, Fractal Curves and 1/f
 * Fluctuations", Scientific American, 238(4), pp. 16-31, 1978. 
 */
static void
do_fgrand(argc, argv)
int  argc;
Object  *argv;
{
    int  key = (argc >= 3) ? argv[2].o_val : 0;
    struct fgrand_stat  *sp;
    int  k, n;
    long  s;

    n = (argc >= 2) ? (argv[1].o_val - argv[0].o_val + 1) : argv[0].o_val;
    if( n < 1 )  n = 1;

    /* find the status entery that matches the key */ 
    for( sp = fgrand_states; sp; sp = sp->next ) {
	if( sp->key == key ) goto found;
    }
    if( !(sp = (struct fgrand_stat *) malloc(sizeof(struct fgrand_stat))) ) {
	err_nomem("do_fgrand");
    }
    sp->key = key;
    sp->next = fgrand_states;
    sp->cnt = 0;
    fgrand_states = sp;
 found:

    /* Now, throw the dices */
    s = 0;
    for( k = 0; k < NDICES; k++ ) {
	if( (sp->cnt ^ (sp->cnt-1)) & (1L << k) ) {
	    sp->dice[k] = random();
	}
	s += sp->dice[k] % (((n - 1) >> NDICESLOG2 ) + 
			    (((n - 1) & (NDICES - 1)) > k) + 1);
    }
    sp->cnt++;

    pushbk_int(s + ((argc >= 2) ? argv[0].o_val : 1));
}

/*
 * floor(n): returns the greatest integer value less than or eqaul to n
 */
static void
do_floor(argc, argv)
int  argc;
Object  *argv;
{
    if( argv->o_type == O_INT ) {
	pushbk_int(argv->o_val);
    } else if( argv->o_type == O_RATIONAL ) {
	pushbk_int((PmmlInt)argv->r.intg);
    } else { /* float */
	pushbk_int((PmmlInt)floor(argv->fpval));
    }
}

/*
 * ceil(n): returns the smallest integer value greater than or equal to n
 */
static void
do_ceil(argc, argv)
int  argc;
Object  *argv;
{
    if( argv->o_type == O_INT ) {
	pushbk_int(argv->o_val);
    } else if( argv->o_type == O_RATIONAL ) {
	if( argv->r.num > 0 ) {
	    pushbk_int((PmmlInt)argv->r.intg + 1);
	} else {
	    pushbk_int((PmmlInt)argv->r.intg);
	}
    } else { /* float */
	pushbk_int((PmmlInt)ceil(argv->fpval));
    }
}

/*
 * abs(n): returns the absolute value of n
 */
static void
do_abs(argc, argv)
int  argc;
Object  *argv;
{
    if( argv->o_type == O_INT ) {
	pushbk_int(argv->o_val >= 0 ? argv->o_val : - argv->o_val);
    } else if( argv->o_type == O_RATIONAL ) {
	if( argv->r.intg < 0 ) {
	    rneg(&argv->r);
	}
	pushbk_object(argv);
    } else { /* float */
	pushbk_float(fabs(argv->fpval));
    }
}

/*
 * some mathematical functions
 */
static void
do_sqrt(argc, argv)
int  argc;
Object  *argv;
{
    if( argv->fpval < 0 ) {
	error(cur_srcpos, "sqrt: negative argument", "sqrt: 引数が負です");
    }
    pushbk_float(sqrt(argv->fpval));
}

static void
do_sin(argc, argv)
int  argc;
Object  *argv;
{
    pushbk_float(sin(argv->fpval));
}

static void
do_cos(argc, argv)
int  argc;
Object  *argv;
{
    pushbk_float(cos(argv->fpval));
}

static void
do_exp(argc, argv)
int  argc;
Object  *argv;
{
    pushbk_float(exp(argv->fpval));
}

static void
do_log(argc, argv)
int  argc;
Object  *argv;
{
    if( argv->fpval <= 0 ) {
	error(cur_srcpos, "log: non-positive argument", 
	      "log: 引数が０または負です");
    }
    pushbk_float(log(argv->fpval));
}

static void
do_atan2(argc, argv)
int  argc;
Object  *argv;
{
    pushbk_float(atan2(argv[0].fpval, argv[1].fpval));
}

/*
 * int(n): convert to an integer with rounding
 */
static void
do_int(argc, argv)
int  argc;
Object  *argv;
{
    pushbk_int(argv->o_val);
}

/*
 * float(n): convert to a FP number
 */
static void
do_float(argc, argv)
int  argc;
Object  *argv;
{
    pushbk_float(argv->fpval);
}

/*
 * rational(n [,div]): convert to the nearest rational number
 */
static void
do_rational(argc, argv)
int  argc;
Object  *argv;
{
    if( argc == 2 ) {
	ridiv(&argv[0].r, argv[1].o_val, &argv[0].r);
    }
    pushbk_object(argv);
}

/*
 * intg(n): give the integer part of a rational number
 */
static void
do_intg(argc, argv)
int  argc;
Object  *argv;
{
    pushbk_int((PmmlInt)argv->r.intg);
}

/*
 * num(n): give the numerator of a rational number
 */
static void
do_num(argc, argv)
int  argc;
Object  *argv;
{
    Rational  tmp;

    rnorm2(&argv->r, &tmp);
    pushbk_int((PmmlInt)tmp.num);
}

/*
 * den(n): give the denominator of a rational number
 */
static void
do_den(argc, argv)
int  argc;
Object  *argv;
{
    Rational  tmp;

    rnorm2(&argv->r, &tmp);
    pushbk_int((PmmlInt)tmp.den);
}

/*
 * strlen(str): returns the length of string
 */
static void
do_strlen(argc, argv)
int  argc;
Object  *argv;
{
    pushbk_int((PmmlInt)strlen(argv->o_str));
}

/*
 * substr(str, from [,nchars]): returns a part of string
 */
static void
do_substr(argc, argv)
int  argc;
Object  *argv;
{
    char  *s;
    int	 ofs, slen, dlen;

    s = argv->o_str;
    slen = strlen(s);

    ofs = argv[1].o_val - 1;
    if( ofs < 0 )  ofs = 0;
    else if( ofs >= slen )  ofs = slen;

    if( argc == 3 ) {
	dlen = argv[2].o_val;
	if( dlen < 0 )  dlen = 0;
    } else {
	dlen = slen;
    }

    if( ofs + dlen > slen ) {
	dlen = slen - ofs;
    }
	
    resetstr(strbuf);
    for( s += ofs; --dlen >= 0; s++ ) {
	addchar(strbuf, *s);
    }
    addchar(strbuf, 0);
	
    pushbk_string(getstr(strbuf));
}

/*
 * index(str1, str2): returns the position of str2 in str1
 */
static void
do_index(argc, argv)
int  argc;
Object  *argv;
{
    char  *s, *p;
    int  slen, plen, i, ret;

    s = (argv++)->o_str;
    p = argv->o_str;
    plen = strlen(p);

    ret = 0;
    if( plen <= 1 ) {
	for( i = 1; *s; i++, s++ ) {
	    if( *s == *p ) {
		ret = i;
		break;
	    }
	}
    } else {
	slen = strlen(s);
	for( i = 1; plen <= slen; i++, slen--, s++ ) {
	    if( strncmp(s, p, plen) == 0 ) {
		ret = i;
		break;
	    } 
	}
    }

    pushbk_int((PmmlInt)ret);
}

/*
 * atoi(str): convert a string to an integer
 */
static void
do_atoi(argc, argv)
int  argc;
Object  *argv;
{
    pushbk_int((PmmlInt)atoi(argv->o_str));
}

/*
 * atof(str): convert a string to a FP number
 */
static void
do_atof(argc, argv)
int  argc;
Object  *argv;
{
    double  atof();

    pushbk_float(atof(argv->o_str));
}

/*
 * charcode(str [,n]): returns the ASCII code of the n-th charactor of string
 */
static void
do_charcode(argc, argv)
int  argc;
Object  *argv;
{
    int  n = 0;

    if( argc == 2 )  n = argv[1].o_val - 1;
    if( n < 0 || n >= (int)strlen(argv->o_str) ) {
	pushbk_int(0L);
    } else {
	pushbk_int((PmmlInt)argv->o_str[n]);
    }
}

/*
 * dumpdef(id): print the definition of macro (for debugging)
 */
static void
do_dumpdef(argc, argv)
int  argc;
Object  *argv;
{
    DicEnt  *dp = argv->id.dp;

    dp->active--;

    if( !argv->id.defined ) {
	printf("%s: undefined\n", dp->name);
	free(dp->name);
	free(dp);
    } else {
	print_def(dp, 1);
    }
}

/*
 * defined(id): returns true if the id is a defined name
 */
static void
do_defined(argc, argv)
int  argc;
Object  *argv;
{
    DicEnt  *dp = argv->id.dp;

    dp->active--;
    if( !argv->id.defined ) {
	free(dp->name);
	free(dp);
	pushbk_int(0L);
    } else {
	pushbk_int(1L);
    }
}

/*
 * idtype(id): returns the type of id
 */
static void
do_idtype(argc, argv)
int  argc;
Object  *argv;
{
    DicEnt  *dp = argv->id.dp;

    dp->active--;
    if( !argv->id.defined ) {
	free(dp->name);
	free(dp);
	pushbk_int(0L);
    } else {
	pushbk_int((PmmlInt)dp->type + 1);
    }
}

/*
 * idstr(id): returns the string of id name 
 */
static void
do_idstr(argc, argv)
int  argc;
Object  *argv;
{
    DicEnt  *dp = argv->id.dp;

    resetstr(strbuf);
    addstr(strbuf, dp->name);
    addchar(strbuf, 0);

    dp->active--;
    if( !argv->id.defined ) {
	free(dp->name);
	free(dp);
    }
    pushbk_string(getstr(strbuf));
}

/*
 * type(exp): returns the type of expression
 */
static void
do_type(argc, argv)
int  argc;
Object  *argv;
{
    static char  s[2];

    switch( argv->o_type ) {
    case O_INT:
	s[0] = 'i';
	break;
    case O_RATIONAL:
	s[0] = 'r';
	break;
    case O_STRING:
	s[0] = 's';
	break;
    case O_ARRAY:
	s[0] = 'a';
	break;
    case O_STOKENS:
    case O_WTOKENS:  /* impossible */
	s[0] = 't';
	break;
    default: /* float */
	s[0] = 'f';
	break;
    }
    pushbk_string(s);
}

/*
 * dup(array) : make a clone of an array
 */
static void
do_dup(argc, argv)
int  argc;
Object  *argv;
{
    pushbk_array(dup_array(argv[0].o_ap));
}

/*
 * rep(count, elmement) : create a new array by replicating a single element
 */
static void
do_rep(argc, argv)
int  argc;
Object  *argv;
{
    pushbk_array(create_reparray(argv[0].o_val, &argv[1]));
}

/*
 * print(object, ...)
 */
static void
do_print(argc, argv)
int  argc;
Object  *argv;
{
    while( --argc >= 0 ) {
	fprint_object_nq(stdout, argv, -1, -1);
	if( argc )  putchar(' ');
	argv++;
    }
    putchar('\n');
}

/*
 * eprint(object, ...)
 */
static void
do_eprint(argc, argv)
int  argc;
Object  *argv;
{
    while( --argc >= 0 ) {
	fprint_object_nq(stderr, argv, -1, -1);
	if( argc )  putc(' ', stderr);
	argv++;
    }
    putc('\n', stderr);
}

/*
 * printf(fmt, args..): print formated string to stdout
 */
static void
do_printf(argc, argv)
int  argc;
Object  *argv;
{
    pmml_sprintf(&strbuf, "printf", argv->o_str, argc - 1, argv + 1);
    fwrite(strbuf.buf, strbuf.len - 1, 1, stdout);
}

/*
 * eprintf(fmt, args..): print formated string to stderr
 */
static void
do_eprintf(argc, argv)
int  argc;
Object  *argv;
{
    pmml_sprintf(&strbuf, "eprintf", argv->o_str, argc - 1, argv + 1);
    fwrite(strbuf.buf, strbuf.len - 1, 1, stderr);
}

/*
 * sprintf(fmt, args..): formated string conversion
 */
static void
do_sprintf(argc, argv)
int  argc;
Object  *argv;
{
    pmml_sprintf(&strbuf, "sprintf", argv->o_str, argc - 1, argv + 1);
    pushbk_string(getstr(strbuf));
}

/*
 * error(fmt, args..): do as if a compiling error has occured
 */
static void
do_error(argc, argv)
int  argc;
Object  *argv;
{
    pmml_sprintf(&strbuf, "error", argv->o_str, argc - 1, argv + 1);
    error(cur_srcpos, "%s", "%s", getstr(strbuf));
}

/*
 * warn(fmt, args..): do as if a compiler warnning is outputted
 */
static void
do_warn(argc, argv)
int  argc;
Object  *argv;
{
    pmml_sprintf(&strbuf, "warn", argv->o_str, argc - 1, argv + 1);
    warn(cur_srcpos, "%s", "%s", getstr(strbuf));
}

/*
 * exit(status): terminate compile process
 */
static void
do_exit(argc, argv)
int  argc;
Object  *argv;
{
    exit(argv->o_val);
}

/*
 * ntrk(filename): returns the number of tracks of a MIDI file
 */
static void
do_ntrk(argc, argv)
int  argc;
Object  *argv;
{
    pushbk_int((PmmlInt)get_ntrk(argv->o_str));
}

/*
 * format(filename): returns the format code of a MIDI file
 */
static void
do_format(argc, argv)
int  argc;
Object  *argv;
{
    pushbk_int((PmmlInt)get_format(argv->o_str));
}

/*
 * resolution(filename): returns the resolution of a MIDI file
 */
static void
do_resolution(argc, argv)
int  argc;
Object  *argv;
{
    pushbk_int((PmmlInt)get_resolution(argv->o_str));
}

/*
 * toupper(str): convert a string to upper case
 */
static void
do_toupper(argc, argv)
int  argc;
Object  *argv;
{
    char  *s, c;
	
    resetstr(strbuf);
    for( s = argv->o_str; (c = *s++) != 0; ) {
	c = islower(c) ? c - 'a' + 'A' : c;
	addchar(strbuf, c);
    }
    addchar(strbuf, 0);
	
    pushbk_string(getstr(strbuf));
}

/*
 * tolower(str): convert a string to lower case
 */
static void
do_tolower(argc, argv)
int  argc;
Object  *argv;
{
    char  *s, c;
	
    resetstr(strbuf);
    for( s = argv->o_str; (c = *s++) != 0; ) {
	c = isupper(c) ? c - 'A' + 'a' : c;
	addchar(strbuf, c);
    }
    addchar(strbuf, 0);
	
    pushbk_string(getstr(strbuf));
}

/*
 * notename(n): convert a note number to the note string
 */
#define MAX_NOTENAME_LEN  20

static void
do_notename(argc, argv)
int  argc;
Object  *argv;
{
    int  note_num, oct, idx, cnt; 
    static  char *name[12] = { "C", "C#", "D", "D#", "E", "F", 
			       "F#", "G", "G#", "A", "A#", "B" };
    static  char buf[MAX_NOTENAME_LEN];
    char  *p;
    
    note_num = argv->o_val;
    if( note_num < 0 ) {
	oct = - ((-note_num - 1) / 12) - 2;
	idx = 11 - ((-note_num - 1) % 12);
    } else {
	oct = note_num / 12 - 1;
	idx = note_num % 12;
    }

    if( argc == 1 ) {
	sprintf(buf, "%s%d", name[idx], oct);
    } else {
	p = buf;
	cnt = 0;
	if( argv[1].o_val > 0 ) {
	    while( oct > argv[1].o_val ) {
		if( ++cnt > MAX_NOTENAME_LEN - 3 )  break;
		*p++ = '^';
		oct--;
	    }
	    while( oct < argv[1].o_val ) {
		if( ++cnt > MAX_NOTENAME_LEN - 3 )  break;
		*p++ = '_';
		oct++;
	    }
	}
	sprintf(p, "%s", name[idx]);
    }

    pushbk_string(buf);
}

/*
 * nth_token(id, n): the n-th token in the definition of a macro
 */
static void
do_nth_token(argc, argv)
int  argc;
Object  *argv;
{
    DicEnt  *dp = argv[0].id.dp;
    Token  *tp, *tp1;
    int  i;

    dp->active--;

    if( !argv[0].id.defined ) {
	error(cur_srcpos, "nth_token: '%s' is undefined",
	      "nth_token: '%s' が定義されていません", dp->name );
    } else if( (dp->type != D_LocalMacro && 
		dp->type != D_ThreadMacro) ||
	      (dp->dic_obj.o_type != O_STOKENS && 
	       dp->dic_obj.o_type != O_WTOKENS) ) {
	error(cur_srcpos, "nth_token: '%s' is not a token-list macro",
	      "nth_token: '%s' はトークン列型のマクロではありません",dp->name);
    }
    
    for( i = argv[1].o_val, tp = dp->dic_obj.o_tp; --i > 0; ) {
	if( !tp )  break;
	tp = tp->next;
    }

    if( tp ) {
	tp1 = copy_token(tp);
	tp1->next = NULL;
	pushbk_tklist(tp1, TF_FREE | TF_RPLPOS);
    }
}

/*
 * concat(array [, delimiter]) : concatenate all string elements in an array
 */
static void
do_concat(argc, argv)
int  argc;
Object  *argv;
{
    int  i;
    Array  *ap = argv[0].o_ap;

    resetstr(strbuf);
    for( i = 0; i < ap->size; i++ ) {
	if( array_ref(ap,i).o_type != O_STRING ) {
	    error(cur_srcpos, "concat: Array element is not a string",
		  "concat: 配列要素が文字列ではありません");
	}
	addstr(strbuf, array_ref(ap,i).o_str);
	if( argc == 2 && i != ap->size - 1 ) {
	    addstr(strbuf, argv[1].o_str);
	}
    }
    addchar(strbuf, 0);
    pushbk_string(getstr(strbuf));
}

/*
 * showeffs() : show attached effectors
 */
static void
do_showeffs(argc, argv)
int  argc;
Object  *argv;
{
    EffInst  *eip;
    EEBV  eebv = cur_thd->eebv;
    int  cnt = 1;

    for( eip = cur_thd->effs; eip != NULL; eip = eip->next ) {
	printf("[%d]\tinstance name = ", cnt++);
	if( !eip->dp )  printf("<no name>\n");
	else  printf("\"%s\"\n", eip->dp->name);
	print_effinst(eip);
	printf("\tstatus = %s\n", eebv & 1 ? "Enabled" : "Disabled");
	eebv >>= 1;
    }
}

/*
 * dumpdict() : dump dictionary contents
 */
static void
do_dumpdict(argc, argv)
int  argc;
Object  *argv;
{
    char  *typestr = argc > 0 ? argv[0].o_str : "lm";
    int   types = 0;
    Thread  *scope = NULL;
    DicEnt  *dp;

    while( *typestr ) {
	switch( *typestr ) {
	case 'l':  types |= 1 << D_LocalMacro;  break; 
	case 'm':  types |= 1 << D_ThreadMacro;  break; 
	case 'b':  types |= 1 << D_BuiltIn;  break; 
	case 't':  types |= 1 << D_ThreadName;  break; 
	case 'e':  types |= 1 << D_EffClass;  break; 
	case 'i':  types |= 1 << D_EffInst;  break; 
	case 'c':  types |= 1 << D_Channel;  break; 
	case 's':  types |= 0x8000; break;	/* show statistics */
	default:
	    warn(cur_srcpos, "dumpdict: `%c': No such dictionary entry type",
		 "dumpdict: `%c' という辞書エントリの型はありません", 
		 *typestr);
	    break;
	}
	typestr++;
    }

    if( argc >= 2 ) {
	if( !argv[1].id.defined ||
	   (dp = argv[1].id.dp)->type != D_ThreadName ) {
	    error(cur_srcpos, "dumpdict: Bad scope specifier",
		  "dumpdict: 通用範囲指定が間違っています");
	}
	dp->active--;
	scope = dp->dic_thd;
    }

    dump_dict(types, scope);
}

/*
 * trace(level) : set trace level
 */
static void
do_trace(argc, argv)
int  argc;
Object  *argv;
{
    extern int  trace_level;

    trace_level = argv->o_val;
}

/*
 * seteflags(flags) : set default effector flags
 */
static void
do_seteflags(argc, argv)
int  argc;
Object  *argv;
{
    extern int  default_eflags;

    default_eflags = argv->o_val;
}

/*********************************************************************
 * printf routine for PMML
 *********************************************************************/

#define get_intarg(dst) \
{ PmmlInt  tmp; \
    if( !argc )  goto no_arg; \
    else { \
	argc--; \
	if( conv_to_int(argv++, &tmp) )  goto type_mismatch; \
	(dst) = tmp; \
    } \
}

#define get_strarg(dst) \
{ \
    if( !argc )  goto no_arg; \
    else if( argv->o_type != O_STRING )  goto type_mismatch; \
    else { \
	argc--; \
	(dst) = (argv++)->o_str; \
    } \
}

#define get_floatarg(dstp) \
{ \
    if( !argc )  goto no_arg; \
    else { \
	argc--; \
	if( conv_to_float(argv++, dstp) )  goto type_mismatch; \
    } \
}

#define MAXPREC  15	/* maximum precision for %f, %g, and %e */

void
pmml_sprintf(sptr, cmdname, format, argc, argv)
String  *sptr;
char  *cmdname;		/* command name (for error message) */
char  *format;
int  argc;
Object  *argv;
{
    char *fmt;
    int  leftadj, padzero;
    char sign;
    char fchar;
    int  width, base;
    int	 prec;			/* precision: negative if not specified */
    unsigned long  num;
    PmmlFloat  fpnum;
    char buf[MAXPREC+41], *b;	/* buffer for integer-part digits 
				   or formatted floating-point number.
				   This must be enough for storing formatted 
				   MAXDOUBLE number with MAXPREC. */
    resetstr(*sptr);

    for( fmt = format; *fmt != 0; ) {
	/* normal characters */
	if( *fmt != '%' ) {
	    addchar(*sptr, *fmt);
	    ++fmt;
	    continue;
	}
	
	/* handle '%%' */
	++fmt;
	if( *fmt == '%' ) {
	    ++fmt;
	    addchar(*sptr, '%');
	    continue;
	}
	
	/* get options */
	sign = '\0';
	leftadj = padzero = 0;
	for(;; fmt++) {
	    switch(*fmt) {
	    case '+': sign = '+';   break;
	    case '-': leftadj = 1;  break;
	    case '0': padzero = 1;  break;
	    case ' ': sign = ' ';   break;
	    default: goto opts_end;
	    }
	}
    opts_end:
	if( leftadj )  padzero = 0;
	
	/* get field width */
	width = 0;
	if( *fmt == '*' ) {
	    get_intarg(width);
	    if( width < 0 ) {
		width = -width;
		leftadj = 1;
	    }
	    ++fmt;
	} else {
	    while( isdigit(*fmt) ) {
		width = width * 10 + *fmt++ - '0';
	    }
	}
	
	/* get precision */
	prec = -1;
	if( *fmt == '.' ) {
	    ++fmt;
	    if( *fmt == '*' ) {
		get_intarg(prec);
		++fmt;
	    } else if( isdigit(*fmt) ) {
		prec = 0;
		while( isdigit(*fmt) ) {
		    prec = prec * 10 + *fmt++ - '0';
		}
	    }
	}
	
	/* format character */
	switch( (fchar = *fmt++) | 0x20 ) {
	case 'd':
	    base = 10;
	    get_intarg(num);
	    if( (long)num < 0 ) {
		num = - (long)num;
		sign = '-';
	    } 
	    goto number;
	    
	case 'o':
	    base = 8;
	    goto unsigned_number;
	    
	case 'x':
	    base = 16;

	unsigned_number:
	    get_intarg(num);
	    sign = '\0';
	    
	number:
	    {
		char *digits = (fchar == 'X') ? 
		    "0123456789ABCDEF" : "0123456789abcdef";
		
		if( prec < 0 )  prec = 1;

		for( b = buf; num > 0; ) {
		    *b++ = digits[num % base];
		    num /= base;
		}
		
		width -= b - buf;
		prec -= b - buf;
		
		if( prec > 0 ) {
		    width -= prec;
		}
		
		if( sign )  --width;
		
		if( !leftadj && !padzero )
		    while( width-- > 0 )  addchar(*sptr, ' ');
		
		if( sign )  addchar(*sptr, sign);
		
		if( !leftadj && padzero )
		    while( width-- > 0 )  addchar(*sptr, '0');
		
		while( prec-- > 0 )  addchar(*sptr, '0');
		while( --b >= buf )  addchar(*sptr, *b);
		
		if( leftadj ) {
		    while( width-- > 0 )  addchar(*sptr, ' ');
		}
	    }
	    break;
	    
	case 'f':
	case 'g':
	case 'e':
	    {
		static char  fpfmt[5] = "%.*X";

		if( prec < 0 )  prec = 6;
		else if( prec > MAXPREC )  prec = MAXPREC;

		get_floatarg(&fpnum);
		if( fpnum < 0 ) {
		    fpnum = - fpnum;
		    sign = '-';
		}

		fpfmt[3] = fchar; 
		sprintf(buf, fpfmt, prec, fpnum);
		width -= strlen(buf);
		
		if( sign )  --width;
		
		if( !leftadj && !padzero )
		    while( width-- > 0 )  addchar(*sptr, ' ');
		
		if( sign )  addchar(*sptr, sign);
		
		if( !leftadj && padzero )
		    while( width-- > 0 )  addchar(*sptr, '0');
		
		for( b = buf; *b != 0; b++ )  addchar(*sptr, *b);
		
		if( leftadj ) {
		    while( width-- > 0 )  addchar(*sptr, ' ');
		}
	    }
	    break;
		
	case 'c':
	    get_intarg(num);
	    
	    if( !leftadj )
		while( --width > 0 )  addchar(*sptr, ' ');
	    
	    addchar(*sptr, (unsigned char) num);
	    
	    if( leftadj )
		while( --width > 0 )  addchar(*sptr, ' ');
	    break;
	    
	case 's':
	    {
		int  len;
		char *str;
		
		get_strarg(str);
		len = strlen(str);
		
		if( prec >= 0 && prec < len )  len = prec;
		width -= len;
		
		if( !leftadj )
		    while( width-- > 0 )  addchar(*sptr, ' ');
		
		while( len-- > 0 ) {
		    addchar(*sptr, *str);
		    str++;
		}
		
		if( leftadj ) 
		    while( width-- > 0 )  addchar(*sptr, ' ');
	    }
	    break;
	    
	default:
	    error(cur_srcpos, "%s: Unrecognized format specifier '%c'",
		  "%s: '%c' は有効なフォーマット文字ではありません", 
		  cmdname, fchar);
	}
    }

    if( argc != 0 ) {
	error(cur_srcpos, "%s: Too many arguments",
	      "%s: 引数が多すぎます", cmdname);
    }

    addchar(*sptr, 0);
    return;

 no_arg:
    error(cur_srcpos, "%s: Too few arguments",
	  "%s: 引数が足りません", cmdname);
 type_mismatch:
    error(cur_srcpos, "%s: Mismatched argument type",
	  "%s: 引数の型が違います", cmdname);
}
