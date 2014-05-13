/*
 * util.c: string utilities
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
#include "util.h"

#define STR_BLKSIZ  32	    /* block size for dynamically-growing string */

static int find_extension P((char *, int));

/*  
 * complete the file name by appending extension if it has no extension
 *   The returned string must by free'ed manually.
 *   The return value becomes NULL if no memory is left.
 */
char *
attach_extension(fname, extstr)
char  *fname;
char  *extstr;
{
    int  len;
    char  *p;

    len = strlen(fname);
    if( strcmp(fname, "-") == 0 || find_extension(fname, len) > 0 ) {
	return strdup(fname);
    }
    p = (char *) malloc(len + strlen(extstr) + 1);
    if( !p )  return NULL;
    strcpy(p, fname);
    strcpy(p+len, extstr);
    return p;
}

/*
 * replace the extention part of a file name by a given extension
 *   The returned string must by free'ed manually.
 *   The return value becomes NULL if no memory is left.
 */
char *
replace_extension(fname, extstr)
char  *fname;
char  *extstr;
{
    int  len, i;
    char  *p;

    if( strcmp(fname, "-") == 0 )  return strdup(fname);
    len = strlen(fname);
    if( ! (i = find_extension(fname, len)) ) {
	/* if no extension, attach the given extension */
	p = (char *) malloc(len + strlen(extstr) + 1);
	if( !p )  return NULL;
	strcpy(p, fname);
	strcpy(p+len, extstr);
    } else {
	p = (char *) malloc(i + strlen(extstr));
	if( !p )  return NULL;
	strncpy(p, fname, i-1);
	strcpy(p+i-1, extstr);
    }
    return p;
}

/*
 * find the position of the extention part of a file name
 */
static int
find_extension(fname, len)
char  *fname;
int  len;
{
   char  *p;
   int	dotpos = 0;
   int	extlen = 0;
   int	bodylen = 0;
   
   for( p = fname + len; len > 0; len-- ) {
       switch( *--p ) {
       case '.':
	   if( !dotpos ) {
	       dotpos = len;
	   } else {
	       bodylen++;
	   }
	   break;
       case '/':
       case '\\':
       case ':':	
	   goto theend;
       default:
	   dotpos ? bodylen++ : extlen++;
       }
   }
 theend:
   return ( !dotpos || !extlen || !bodylen ) ? 0 : dotpos;
}

/*
 * buffer expansion routine for dynamically-growing strings
 */
int
expand_string(sp, c)
String  *sp;
int	c;
{
    if( sp->allocated ) {
	sp->buf = realloc(sp->buf, sp->allocated += STR_BLKSIZ);
    } else {
	sp->buf = malloc(sp->allocated = STR_BLKSIZ);
    }
    if( !sp->buf ) {
	err_nomem("expand_string");
    }
    return(sp->buf[sp->len++] = c);
}

/*
 * add a string to a dynamically-growing string
 */
char *
_addstr(sp, str)
String  *sp;
char  *str;
{
    int  len;

    len = strlen(str);
    if( (sp->len + len) > sp->allocated ) {
	int  n;

	for( n = sp->allocated; (sp->len + len) > n; n += STR_BLKSIZ );
	sp->buf = sp->allocated ? realloc(sp->buf, n) : malloc(n);
	if( !sp->buf )  err_nomem("addstr");
	sp->allocated = n;
    }
    strncpy(sp->buf + sp->len, str, len);
    sp->len += len;
}

/*
 * prepend a directory name to a file name 
 *   `path_ptr' is ptr to a variable keeping ';'-separated path list.
 *   After calling, the varible will be changed to the next path.
 *   Returns NULL if there is no more paths in the path list.
 *   The returned string must by free'ed manually.
 */
char *
prepend_path(path_ptr, fname)
char  **path_ptr;
char  *fname;
{
    char *p0, *p1, *rtn;
    int len;
    int insert_slash;

    p0 = *path_ptr;
    while( *p0 == ';' )  p0++;	/* skip empty path */
    if( *p0 == 0 )  return NULL;

    for( len = 0, p1 = p0; *p1 && *p1 != ';'; len++, p1++ );

    insert_slash = (p1[-1] != '/' && p1[-1] != '\\' && p1[-1] != ':');

    /* if the path is "." or "./" then remove it */
    if( *p0 == '.' && (len == 1 || (len == 2 && ! insert_slash)) ) {
	len = 0;
	insert_slash = 0;
    }

    if( !(rtn = malloc(len + strlen(fname) + insert_slash + 1)) ) {
	err_nomem("prepend_path");
    }
    strncpy(rtn, p0, len);
    if( insert_slash ) {
	rtn[len] = '/';
	strcpy(rtn + len + 1, fname);
    } else {
	strcpy(rtn + len, fname);
    }

    *path_ptr = p1;
    return rtn;
}

/*
 * strip the directry part of a file name
 */
char *
basename(s)
char  *s;
{
    char  *p;

    for( p = s + strlen(s); p >= s; p-- ) {
	if( *p == '/' || *p == '\\' || *p == ':' ) {
	    return p+1;
	}
    }
    return s;
}

#ifdef SELF_ERR_NOMEM
/*
 * no-memory error
 */
void
err_nomem(str)
char  *str;
{
    fprintf(stderr, "Memory exhausted during the execution of `%s'\n", str);
    exit(1);
}
#endif
