/*
 * util.h: header string utilities
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

#ifndef __UTIL_H__
#define __UTIL_H__

#if defined(__STDC__) && !defined(PROTOTYPE)
#   define PROTOTYPE 1 
#endif
#if !defined(P) 
#   ifdef PROTOTYPE
#      define P(arg)  arg
#   else
#      define P(arg)  ()
#   endif
#endif

char	*malloc();
char	*realloc();
char	*strdup();

/*
 * dynamically-growing string 
 */
typedef struct  {
    char *buf;
    int  len;
    int  allocated;
} String;

char *	attach_extension P((char *, char *));
char *	replace_extension P((char *, char *));
int	expand_string P((String *, int));
char *  _addstr P((String *, char *));
char *  prepend_path P((char **, char *));
char *  basename P((char *));

#ifdef FREE_TEST
#   define strfree(p)  (printf("free_string: %s\n", p), free(p))
#else
#   define strfree(p)  free(p)
#endif

#define initstr(s)   ((s).allocated = (s).len = 0)
#define resetstr(s)  ((s).len = 0)
#define getstr(s)    ((s).buf)
#define addchar(s, c) \
    ((s).len < (s).allocated ? \
     (s).buf[(s).len++] = (c) : expand_string(&(s), (c)))
#define addstr(s, str)  _addstr(&(s), (str))
#define freestr(s)   { if( (s).allocated )  free((s).buf); }

#ifdef SELF_ERR_NOMEM
void	err_nomem P((char *));
#endif

#endif /* __UTIL_H__ */
