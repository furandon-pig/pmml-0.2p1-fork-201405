/*
 * PMML: A Compiler for the Practical Music Macro Language
 *
 * token.c: token & object management
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
#include "pmml.h"

/*
 * block size for token memory allocation
 */
#define TOK_BLOCK_SIZE  32

/* 
 * global variables
 */
static Token  *free_list = NULL;

/*
 * macros for token memory allocator
 */
#define  token_alloc(newtk) \
(free_list ? \
 ((newtk) = free_list, free_list = free_list->next) : \
 ((newtk) = _token_alloc()) )

#define  token_free(tp) \
    ((tp)->next = free_list, free_list = (tp))

/*
 * function prototypes
 */
static Token *  _token_alloc P((void));


/*********************************************************************
 * Token related routines
 *********************************************************************/
/*
 * memory allocator for tokens
 */
static Token *
_token_alloc()
{
    Token  *tp;
    char  *blk;
    int  i;

    if( !(blk = malloc(TOKEN_SIZE * TOK_BLOCK_SIZE)) ) {
	err_nomem("token_alloc");
    }
    for( i = 0; i < TOK_BLOCK_SIZE; i++ ) {
	tp = (Token *) (blk + i * TOKEN_SIZE);
	tp->next = free_list;
	free_list = tp;
    }
    
    tp = free_list;
    free_list = free_list->next;
    return tp;
}

/*
 * duplicate the token structure
 */
Token *
copy_token(tp)
Token *tp;
{
    Token  *new;

    token_alloc(new);
    memcpy(new, tp, TOKEN_SIZE);

    switch( tp->type ) {
    case T_STRING:
	if( !(new->u.obj.o_str = strdup(tp->u.obj.o_str)) ) { 
	    err_nomem("copy_token");
	}
	break;
	    
    case T_ARRAY:
	tp->u.obj.o_ap->ref++;
	break;
	
    case T_ID:
	if( !(new->u.id.name = strdup(tp->u.id.name)) ) { 
	    err_nomem("copy_token");
	}
	break;

    case T_LOCALID:
	tp->u.dp->active++;
	break;
    }

    return new;
}

/*
 * create a new token
 *   The `next' element is initialized to NULL.
 */
Token *
new_token(type, src_pos)
int  type;
long  src_pos;
{
    Token  *new;

    token_alloc(new);
    new->type = type;
    new->src_pos = src_pos;
    new->next = NULL;
    return new;
}

/*
 * check equality of two tokens
 */
int
token_equal(tp1, tp2)
Token  *tp1, *tp2;
{
    /* first check token type */ 
    if( tp1->type != tp2->type )  return 0;

    /* then examine the data part */
    switch( tp1->type ) {
    case T_NUMBER:
	if( tp1->u.obj.o_type == O_INT ) {
	    return( tp2->u.obj.o_type == O_INT 
		   && tp1->u.obj.o_val == tp2->u.obj.o_val );
	} else {
	    return( tp2->u.obj.o_type != O_INT 
		   && tp1->u.obj.fpval == tp2->u.obj.fpval );
	}

    case T_RATIONAL:
    case T_STRING:
    case T_ARRAY:
    case T_STOKENS:
    case T_WTOKENS:
    case T_TSHIFT:
    case T_ACCENT:
	return( compare_object(&tp1->u.obj, &tp2->u.obj) == 0 );

    case T_OCTAVE:
	return( tp1->u.oct == tp2->u.oct );

    case T_ID:
	return( tp1->u.id.hash == tp2->u.id.hash &&
	       strcmp(tp1->u.id.name, tp2->u.id.name) == 0 );

    case T_PITCH:
	if( tp1->u.pitch.flags != tp2->u.pitch.flags ||
	   tp1->u.pitch.note_num != tp2->u.pitch.note_num ||
	   tp1->u.pitch.octave != tp2->u.pitch.octave ) {
	    return 0;
	}
	break;

    case T_LOCALID:
	return( tp1->u.dp == tp2->u.dp );
    }
    return 1;
}

/*
 * print the contents of a token
 */
void
fprint_token(fp, tp, maxelms, maxtklen)
FILE  *fp;
Token  *tp;
int  maxelms, maxtklen;
{
    int  i;
    extern struct keyword wordlist[];
    static char  *opstr[] = {
	">=", "<=", "&&", "||", "==", "!=",
	"+=", "-=", "*=", "/=", "%=", "shl=",
	"shr=", "&=", "xor=", "|=", "::", "-", 
	"<BOTTOM>", "[", "%{", "===", "..", "..."
    };
    static char  *notename[12] = {
	"C", "C#", "D", "D#", "E", "F", 
	"F#", "G", "G#", "A", "A#", "B"
    };

    if( (tp->type >= 33 && tp->type <= 64) ||
        (tp->type >= 65 + (int)(sizeof(opstr)/sizeof(char*)) && 
	 tp->type <= 127) ) {
	putc(tp->type, fp);

    } else if( tp->type >= 65 && tp->type < 128 ) {
	fprintf(fp, "%s", opstr[tp->type - 65]);

    } else if( tp->type >= 128 ) {
	for( i = 0; ; i++ ) {
	    if( wordlist[i].val == tp->type ) {
		fprintf(fp, "%s", wordlist[i].name);
		break;
	    }
	}

    } else switch( tp->type ) {
    case T_NUMBER:
    case T_RATIONAL:
    case T_STRING:
    case T_ARRAY:
    case T_STOKENS:
    case T_WTOKENS:
	fprint_object(fp, &tp->u.obj, maxelms, maxtklen);
	break;

    case T_ID:
	fprintf(fp, "%s", tp->u.id.name);
	break;

    case T_PITCH:
	if( !(tp->u.pitch.flags & ABS_OCT) ) {
	    if( tp->u.pitch.octave >= 0 ) {
		for( i = 0; i < tp->u.pitch.octave; i++ )  putc('^', fp);
	    } else {
		for( i = 0; i < -tp->u.pitch.octave; i++ )  putc('_', fp);
	    }
	} 
	if( tp->u.pitch.flags & RESTFLAG ) { 
	    putc('R', fp);
	} else {
	    fprintf(fp, "%s", notename[tp->u.pitch.note_num]);
	    if( (tp->u.pitch.flags & ACCIDENTAL) 
	       && notename[tp->u.pitch.note_num][1] != '#' ) {
		fprintf(fp, "%%");
	    }
	}
	if( tp->u.pitch.flags & ABS_OCT ) {
	    fprintf(fp, "%d", tp->u.pitch.octave);
	}
	break;

    case T_OCTAVE:
	if( tp->u.oct >= 0 ) {
	    for( i = 0; i < tp->u.oct; i++ )  putc('^', fp);
	} else {
	    for( i = 0; i < -tp->u.oct; i++ )  putc('_', fp);
	}
	break;

    case T_TSHIFT:
	i = tp->u.obj.r.intg * 2 + tp->u.obj.r.num;
	if( i > 0 ) {
	    for( ; i >= 0; i -= 2 )  putc('>', fp);
	    if( i == -1 )  putc('.', fp);
	} else {
	    for( ; i <= 0; i += 2 )  putc('<', fp);
	    if( i == 1 )  putc('.', fp);
	}
	break;

    case T_ACCENT:
	i = tp->u.obj.r.intg * 2 + tp->u.obj.r.num;
	if( i > 0 ) {
	    for( ; i >= 0; i -= 2 )  putc('+', fp);
	    if( i == -1 )  putc('.', fp);
	} else {
	    for( ; i <= 0; i += 2 )  putc('-', fp);
	    if( i == 1 )  putc('.', fp);
	}
	break;

    case T_THREAD:
	fprintf(fp, "<THREAD:%s>", tp->u.dp->name);
	break;

    case T_EFFCLASS:
	fprintf(fp, "<EFFCLASS:%s>", tp->u.dp->name);
	break;

    case T_EFFINST:
	fprintf(fp, "<EFFINST:%s>", tp->u.dp->name);
	break;

    case T_LOCALID:
	fprintf(fp, "<LOCAL:%s>", tp->u.dp->name);
	break;

    case T_EOF:
	fprintf(fp, "<EOF>");
	break;

    case T_BOT:
	fprintf(fp, "<BOT>");
	break;

    case T_EOINIT:
    case T_EOB:
    case T_EOT:
	fprintf(fp, "}");
	break;

    case T_EOP:
	fprintf(fp, "]");
	break;

    default:
	fprintf(fp, "<UNKNOWN>");
    }
}


/*********************************************************************
 * Token-list related routines
 *********************************************************************/
/*
 * make a replica of a token list 
 */
Token *
copy_tklist(tp)
Token  *tp;
{
    Token  *newlist;
    Token  **nextp = &newlist;

    for( ; tp != NULL; tp = tp->next ) {
	*nextp = copy_token(tp);
	nextp = &(*nextp)->next;
    }
    *nextp = NULL;

    return newlist;
}

/*
 * free the memory used in an token list
 */
void
free_tklist(tp)
Token  *tp;
{
    Token  *next;

    /* If the token list is active, the free'ing is "delayed". */
    if( check_active_tklist(tp) ) {
	return;
    }

#ifdef FREE_TEST
    printf("free_tklist: ");
    fprint_tklist(stdout, tp, O_STOKENS, -1, -1);
    printf("\n");
#endif

    while( tp != NULL ) {
	switch( tp->type ) {
	case T_STRING:
	    strfree(tp->u.obj.o_str);
	    break;

	case T_ARRAY:
	    free_array(tp->u.obj.o_ap);
	    break;
	    
	case T_ID:
	    free(tp->u.id.name);
	    break;
	    
	case T_LOCALID:
	    tp->u.dp->active--;
	    break;
	}
	
	next = tp->next;
	token_free(tp);
	tp = next;
    }
}

/*
 * print the contents of a token list
 */
void
fprint_tklist(fp, tp, type, maxelms, maxtklen)
FILE  *fp;
Token  *tp;
int  type;
int  maxelms, maxtklen;
{
    int  len = 0;

    if( type == O_STOKENS )  fprintf(fp, "'");
    for( ; tp; tp = tp->next ) {
	if( maxtklen >= 0 && len >= maxtklen ) {
	    fprintf(fp, "...");
	    break;
	}
	fprint_token(fp, tp, maxelms, maxtklen);
	if( tp->next )  fprintf(fp, " ");
	len++;
    }
    if( type == O_STOKENS )  fprintf(fp, "'");
}


/*********************************************************************
 * Object related routines
 *********************************************************************/
/*
 * copy object (if it is an array, just increase the ref counter)
 */
void
copy_object(dst, src)
Object  *dst, *src;
{
    switch( src->o_type ) {
    case O_STRING:
	dst->o_type = O_STRING;
	if( !(dst->o_str = strdup(src->o_str)) ) {
	    err_nomem("copy_object");
	}
	break;

    case O_ARRAY:
	*dst = *src;
	src->o_ap->ref++;
	break;

    case O_STOKENS:
    case O_WTOKENS:
	dst->o_type = src->o_type;
	dst->o_tp = copy_tklist(src->o_tp);
	break;

    default:
	*dst = *src;
	break;
    }
}

/*
 * free the memory used in an object
 */
void
free_object(op)
Object  *op;
{
    switch( op->o_type ) {
    case O_STRING:
	strfree(op->o_str);
	break;

    case O_ARRAY:
	free_array(op->o_ap);
	break;

    case O_STOKENS:
    case O_WTOKENS:
	free_tklist(op->o_tp);
	break;
    }
}

/*
 * examinue the truth value of an object
 *  Return value:  0 - false   1 - true   -1 - the object is NIL
 */
int
istrue_object(op)
Object  *op;
{
    switch( op->o_type ) {
    case O_INT:
	return( op->o_val != 0 );

    case O_RATIONAL:
	return( !rzerop(&op->r) );

    case O_STRING:
	return( op->o_str[0] != 0 );

    case O_ARRAY:
	return( op->o_ap->size != 0 );

    case O_STOKENS:
    case O_WTOKENS:
	return( op->o_tp != NULL );

    case O_NIL:  /* maybe error */
	return -1;

    default:  /* O_FLOAT */
	return( op->fpval != 0.0 );
    }
}

/*
 * print the contents of an object
 */
void
_fprint_object(fp, obj, maxelms, maxtklen, strquote)
FILE  *fp;
Object  *obj;
int  maxelms, maxtklen;
int  strquote;
{
    char  buf[15];    /* space enough for storing any %g-formatted number */
    
    switch( obj->o_type ) {
    case O_INT:
	fprintf(fp, "%ld", obj->o_val);
	break;
    case O_RATIONAL:
	fprintf(fp, "%s", rstring(&obj->r));
	break;
    case O_STRING:
	if( strquote ) {
	    fprintf(fp, "\"%s\"", obj->o_str);
	} else {
	    fprintf(fp, "%s", obj->o_str);
	}
	break;
    case O_ARRAY:
	fprint_array(fp, obj->o_ap, maxelms, maxtklen);
	break;
    case O_STOKENS:
    case O_WTOKENS:
	fprint_tklist(fp, obj->o_tp, obj->o_type, maxelms, maxtklen);
	break;
    default:
	sprintf(buf, "%g", obj->fpval);
	if( strchr(buf, '.') || strchr(buf, 'e') ) { 
	    fprintf(fp, "%s", buf);
	} else {
	    fprintf(fp, "%s.0", buf);
	}
	break;
    }
}

/*
 * examine the identity of two objects
 *
/* Return value:  0 - equal    1 - not equal    2 - type mismatch */
int
compare_object(op1, op2)
Object  *op1, *op2;
{
    int  i;
    Token  *tp1, *tp2;
    Rational  rtmp;

    switch( op1->o_type ) {
    case O_INT:
	switch( op2->o_type ) {
	case O_INT:
	    return( op2->o_val != op1->o_val );
	case O_RATIONAL:
	    int_to_rational(op1->o_val, &rtmp);
	    return( !requal(&rtmp, &op2->r) );
	case O_STRING:
	case O_ARRAY:
	case O_STOKENS:
	case O_WTOKENS:
	case O_NIL:
	    return 2;
	default:  /* float */
	    return( op2->fpval != op1->o_val );
	}
    case O_RATIONAL:
	switch( op2->o_type ) {
	case O_INT:
	    int_to_rational(op2->o_val, &rtmp);
	    return( !requal(&op1->r, &rtmp) );
	case O_RATIONAL:
	    return( !requal(&op2->r, &op1->r) );
	case O_STRING:
	case O_ARRAY:
	case O_STOKENS:
	case O_WTOKENS:
	case O_NIL:
	    return 2;
	default:  /* float */
	    return( op2->fpval != rational_to_float(&op1->r) );
	}
    case O_STRING:
	if( op2->o_type == O_STRING ) {
	    return( strcmp(op2->o_str, op1->o_str) != 0 );
	} else {
	    return 2;
	}
    case O_ARRAY:
	if( op2->o_type == O_ARRAY ) {
	    /* first check the pointer-level equality */
	    if( op1->o_ap == op2->o_ap )  return 0; 

	    /* then check the size equaility */
	    if( op1->o_ap->size != op2->o_ap->size )  return 1;
	
	    /* check equality of each element */
	    for( i = 0; i < op1->o_ap->size; i++ ) {
		if( compare_object(&array_ref(op1->o_ap,i), 
				   &array_ref(op2->o_ap,i)) != 0 ) {
		    return 1;
		}
	    }
	    return 0;
	} else {
	    return 2;
	}
    case O_STOKENS:
    case O_WTOKENS:
	if( op2->o_type == op1->o_type ) {
	    for( tp1 = op1->o_tp, tp2 = op2->o_tp; 
		tp1 || tp2; tp1 = tp1->next, tp2 = tp2->next ) {
		if( tp1 == NULL || tp2 == NULL ) {
		    return 1;   /* list length is different */
		}
		if( !token_equal(tp1, tp2) )  return 1;
	    }
	    return 0;
	} else {
	    return 2;
	}
    case O_NIL:
	return 2;
    default:  /* float */
	switch( op2->o_type ) {
	case O_INT:
	    return( op2->o_val != op1->fpval );
	case O_RATIONAL:
	    return( rational_to_float(&op2->r) != op1->fpval );
	case O_STRING:
	case O_ARRAY:
	case O_STOKENS:
	case O_WTOKENS:
	case O_NIL:
	    return 2;
	default:  /* float */
	    return( op2->fpval != op1->fpval );
	}
    }

    /* not reached */
    return 0;
}

/*
 * convert int/rational/float to integer by rounding
 *  Return value: non-zero = type-mismatch error
 */
int
conv_to_int(op, intp)
Object  *op;
PmmlInt  *intp;  /* result */
{
    if( op->o_type == O_INT ) {
	*intp = op->o_val;
	return 0;
    } else if( op->o_type == O_RATIONAL ) {
	*intp = rational_to_int(&op->r);
	return 0;
    } else if( isfloat(op->o_type) ) {
	*intp = (PmmlInt) floor(op->fpval + .5);
	return 0;
    }
    return 1;
}

/*
 * convert int/rational/float to rational
 *  Return value: non-zero = type-mismatch error
 */
int
conv_to_rational(op, r)
Object  *op;
Rational  *r;	/* result ('r' may be 'op') */
{
    if( op->o_type == O_INT ) {
	int_to_rational(op->o_val, r);
	return 0;
    } else if( op->o_type == O_RATIONAL ) {
	*r = op->r;
	return 0;
    } else if( isfloat(op->o_type) ) {
	float_to_rational(op->fpval, r);
	return 0;
    }
    return 1;
}

/*
 * convert int/rational/float to float
 *  Return value: non-zero = type-mismatch error
 */
int
conv_to_float(op, floatp)
Object  *op;
PmmlFloat  *floatp;  /* result */
{
    if( op->o_type == O_INT ) {
	*floatp = op->o_val;
	return 0;
    } else if( op->o_type == O_RATIONAL ) {
	*floatp = rational_to_float(&op->r);
	return 0;
    } else if( isfloat(op->o_type) ) {
	*floatp = op->fpval;
	return 0;
    }
    return 1;
}
