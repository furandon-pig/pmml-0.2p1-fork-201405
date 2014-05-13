/*
 * PMML: A Compiler for the Practical Music Macro Language
 *
 * error.c: error handlers
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
#if !defined(__STDC__)
#  define USE_VARARGS  1
#endif
#if defined(USE_VARARGS)
#  include <varargs.h>
#  define __ERROR_C__
#else
#  include <stdarg.h>
#endif
#include <errno.h>
#include "pmml.h"
#include "../common/smf.h"
		    
/*
 * Max number of array elements, tokens in token list, and arguments
 * in error messages
 */
#define ERR_MAXELMS    3
#define ERR_MAXTKLEN   5
#define ERR_MAXARGS    8

int	err_maxelms = ERR_MAXELMS;
int	err_maxtklen = ERR_MAXTKLEN;
int	err_maxargs = ERR_MAXARGS;

int	no_warning = 0;

/* file name directory */
static char  **file_names = NULL;
static int   nfilenames = 0;

/*
 * function prototypes
 */
static void  errhdrout P((long, char *, char *, Token *));

/*
 * register a new file name for error message outputs
 *  The return value is File Name ID, which is the index 
 *  to the file_names array.
 */
int
err_newfname(fname)
char  *fname;
{
    file_names = nfilenames ? 
	   (char **) realloc(file_names, sizeof(char *) * (nfilenames+1)) :
	   (char **) malloc(sizeof(char *));
    if( !file_names )  err_nomem("err_newfname");
    file_names[nfilenames] = fname;
    return nfilenames++;
}

/*
 * print file position information "filename:lineno"
 */
void
fprint_fname(fp, src_pos)
FILE  *fp;
long  src_pos;
{
    if( src_pos == SRCPOS_CMDLN ) {
	fprintf(fp, "cmdline");
    } else if( src_pos == SRCPOS_EOF ) {
	fprintf(fp, "At EOF");
    } else if( src_pos ) {
	fprintf(fp, "%s:%ld",
		file_names[(int)( (unsigned long)src_pos >> 24)], 
		src_pos & 0xffffffL);
    }
}

/*
 * error header output
 */
static void
errhdrout(src_pos, emtype, jmtype, tp)
long  src_pos;
char  *emtype, *jmtype;
Token  *tp;
{
    if( src_pos ) {
	fprint_fname(stderr, src_pos);
	fprintf(stderr, ": ");
    }
    fprintf(stderr, "<%s> ", japan ? jmtype : emtype);
    if( tp ) {
	fprintf(stderr, "`");
	fprint_token(stderr, tp, err_maxelms, err_maxtklen);
	fprintf(stderr, "': ");
    }
}

/*
 * general error
 *   error(src_pos, english_msg, japanese_msg, printf_args ..)
 */
void
#if defined(USE_VARARGS)
error(va_alist)
va_dcl
#else
error(long src_pos, char *emsg, char *jmsg, ...)
#endif
{
    va_list args;
#if defined(USE_VARARGS)
    long  src_pos;
    char  *emsg, *jmsg;
   
    va_start(args);
    src_pos = va_arg(args, long);
    emsg = va_arg(args, char *);
    jmsg = va_arg(args, char *);
#else
    va_start(args, jmsg);
#endif

    errhdrout(src_pos, "Error", "エラー", NULL);
    vfprintf(stderr, japan ? jmsg : emsg, args);
    fprintf(stderr, "\n");
    va_end(args);

    fprint_calls(stderr);
    exit(1);
}

/*
 * general error (printed with token)
 *   error(src_pos, token_ptr, english_msg, japanese_msg, printf_args ..)
 */
void
#if defined(USE_VARARGS)
terror(va_alist)
va_dcl
#else
terror(long src_pos, Token *tp, char *emsg, char *jmsg, ...)
#endif
{
    va_list args;
   
#if defined(USE_VARARGS)
    long  src_pos;
    Token  *tp;
    char  *emsg, *jmsg;

    va_start(args);
    src_pos = va_arg(args, long);
    tp = va_arg(args, Token *);
    emsg = va_arg(args, char *);
    jmsg = va_arg(args, char *);
#else
    va_start(args, jmsg);
#endif

    errhdrout(src_pos, "Error", "エラー", tp);
    vfprintf(stderr, japan ? jmsg : emsg, args);
    fprintf(stderr, "\n");
    va_end(args);

    fprint_calls(stderr);
    exit(1);
}

/*
 * parsing error
 */
void
parse_error(next_token)
Token  *next_token;
{
    errhdrout(cur_srcpos, "Error", "エラー", NULL);
    if( japan ) {
	fprintf(stderr, "`");
	fprint_token(stderr, next_token, err_maxelms, err_maxtklen);
	fprintf(stderr, "' の付近に文法の誤りがあります\n");
    } else {
	fprintf(stderr, "Parse error near `");
	fprint_token(stderr, next_token, err_maxelms, err_maxtklen);
	fprintf(stderr, "'\n");
    }

    fprint_calls(stderr);
    exit(1);
}

/*
 * error related with standard MIDI file I/O
 */
void
mferror(fname)
char  *fname;
{
    char  *reason;

    errhdrout(cur_srcpos, "Error", "エラー", NULL);

    switch( errno ) {
    case EBADEOF_SMF:
	reason = "Unexpected EOF";
	goto bad_fmt;
    case EBADHEAD_SMF:
	reason = "Bad header";
	goto bad_fmt;
    case ENOHEAD_SMF:
	reason = "No header";
	goto bad_fmt;
    case EBADSTAT_SMF:
	reason = "No MIDI status";
	goto bad_fmt;
    case EBADMETA_SMF:	
	reason = "Short tempo or seqno event";
	goto bad_fmt;
    bad_fmt:
	if( japan ) {
	    fprintf(stderr, "%s: 正しいMIDIファイルではありません (%s)\n",
		    fname, reason);
	} else {
	    fprintf(stderr, "%s: Corrupted MIDI file (%s)\n", fname, reason);
	}
	break;

    case ESEEKFAIL_SMF:
	if( japan ) {
	    fprintf(stderr, "%s: シークできません\n", fname);
	} else {
	    fprintf(stderr, "%s: Can not seek\n", fname);
	}
	break;

    case ENOENT:
	if( japan ) {
	    fprintf(stderr, "%s: ファイルが見つかりません\n", fname);
	} else {
	    fprintf(stderr, "%s: No such file\n", fname);
	}
	break;
	    
    default:
	if( japan ) {
	    fprintf(stderr, "%s: MIDIファイル入出力エラー (%s)\n",
		    fname, sys_errlist[errno]);
	} else {
	    fprintf(stderr, "%s: MIDI file I/O error (%s)\n",
		    fname, sys_errlist[errno]);
	}
	break;
    }

    fprint_calls(stderr);
    exit(1);
}

/*
 * warning
 */
void
#if defined(USE_VARARGS)
warn(va_alist)
va_dcl
#else
warn(long src_pos, char *emsg, char *jmsg, ...)
#endif
{
    va_list args;
#if defined(USE_VARARGS)
    long  src_pos;
    char  *emsg, *jmsg;
#endif
   
    if( no_warning )  return;

#if defined(USE_VARARGS)
    va_start(args);
    src_pos = va_arg(args, long);
    emsg = va_arg(args, char *);
    jmsg = va_arg(args, char *);
#else
    va_start(args, jmsg);
#endif

    errhdrout(src_pos, "Warning", "警告", NULL);
    vfprintf(stderr, japan ? jmsg : emsg, args);
    fprintf(stderr, "\n");
    va_end(args);
}

/*
 * no-memory error
 */
void
err_nomem(str)
char  *str;
{
    if( japan ) {
	fprintf(stderr, "メモリが足りません (%s)\n", str);
    } else {
	fprintf(stderr, "Sorry. Can not continue compilation due to the lack of memory space (%s)\n", str);
    }
    exit(1);
}
