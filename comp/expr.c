/*
 * PMML: A Compiler for the Practical Music Macro Language
 *
 * expr.c: parse an expression and calculate its value
 *	   using the operater precedence parsing method
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
 * growing unit size of the operand & opeator stacks
 */
#define	OPS_BLKSIZ	16

/*
 * operand stack (dynamically-growing stack)
 */
static	Object	*opd_stack;
static  int  opd_n;
static  int  opd_nallocated;

#define	opd_push() { \
    if( opd_n >= opd_nallocated )  grow_opd_stack(); \
    opd_n++; \
}

#define opd_pop()	(--opd_n)
#define opd_top	  	(opd_stack[opd_n - 1])
#define opd_sec   	(opd_stack[opd_n - 2])
#define opd_third 	(opd_stack[opd_n - 3])

#define dest1  opd_top
#define src    opd_top
#define dest   opd_sec

/*
 * operator stack (dynamically-growing stack)
 */

static struct optrstk {
    unsigned char  op;
    char  skip_flag;	/* true if '{ ... }' should be skipped */
} *optr_stack;

static int  optr_n;
static int  optr_nallocated;

#define optr_push(t) { \
    if( optr_n >= optr_nallocated )  grow_optr_stack(); \
    optr_stack[optr_n].op = (t); \
    optr_stack[optr_n++].skip_flag = 0; \
}

#define optr_pop()  (optr_stack[--optr_n].op)
#define optr_top    optr_stack[optr_n - 1]
#define optr_sec    optr_stack[optr_n - 2]

/*
 * function prototypes
 */
static void  scan_single_quote P((Token *));
static void  scan_back_quote P((Token *));
static void  grow_opd_stack P((void));
static void  grow_optr_stack P((void));
static void  op_error P((void));
static void  op_badopd P((void));
static void  op_divzero P((void));
void  op_serror P((void));


/*********************************************************************
 * Initilization
 *********************************************************************/
void
init_expression()
{
    opd_stack = (Object *) malloc(sizeof(Object) * OPS_BLKSIZ);
    opd_nallocated = OPS_BLKSIZ;
    opd_n = 0;

    optr_stack = (struct optrstk *) 
	malloc(sizeof(struct optrstk) * OPS_BLKSIZ);
    optr_nallocated = OPS_BLKSIZ;
    optr_n = 0;

    if( !opd_stack || !optr_stack )  err_nomem("init_expression");

    opd_push();
    opd_top.o_type = O_NIL;  /* these data act as "barriers" */
    opd_push();
    opd_top.o_type = O_NIL;
}

/*********************************************************************
 * Expression evaluator
 *********************************************************************/
/*
 * read an expression and calculate its value
 *   The return value points to some location in the operand stack.
 *   When the retrun value is a string, the string contents (obj->u.str) 
 *   must be free'ed on the caller's responsiblity.
 *   Returns NULL if there is no expression.
 * 
 *   get_expression never returns T_WTOKENS object.
 */
Object *
_get_expression(asgn_op_type, lopd)
int  asgn_op_type;	/* If the expreesion is the right of a compund 
			   assignment operator, this indicates the type of  
			   the compound assignment operator; otherwise 0 */
Object *lopd;		/* left-value operand (used for compound assignments)
			   O_ARRAY or O_STOKENS is not allowed. */
{
    Token  *tp;
    int	   type, prev_type;
    int	   flags;
    int	   tmp;

    opd_push();
    opd_top.o_type = O_NIL;	/* "barrier" data */
    optr_push(T_BOTTOM);	/* push `bottom of stack' mark */

    if( asgn_op_type ) {
	optr_push(asgn_op_type);
	opd_push();
	opd_top = *lopd;
	if( lopd->o_type == O_STRING ) {
	    if( !(opd_top.o_str = strdup(lopd->o_str)) ) {
		err_nomem("get_expression");
	    }
	}
    }

    for(prev_type = T_BOTTOM; ; prev_type = type) {

	if( tkinfo[prev_type].flags & F_PU ) { 
	    /* 
	     * If the previous token is an operator execpt ')' or at the
	     * beginning of expression, then an operand, unary operator 
	     * or '(' is expected (otherwise error).
	     */

	    /* do not expand strong token lists here */
	    tp = parse_cmds(MACRO_CMDS & ~F_TK);
	    type = tp->type;

	    if( type == '-' ) {
		type = T_UMINUS;   /* recognize '-' as a unary operator */
	    }

	    if( (flags = tkinfo[type].flags) & F_OPD ) { 
		if( flags & F_REG ) {
		    opd_push();
		    if( type == T_RT ) {
			rsub(&cur_thd->reg.t, &cur_thd->reg.tb, &opd_top.r);
		    } else if( type == T_DU ) {
			rset(cur_thd->reg.dp, 100, &opd_top.r);
			rmult(&cur_thd->reg.l, &opd_top.r, &opd_top.r);
			radd(&cur_thd->reg.dofs, &opd_top.r, &opd_top.r);
		    } else {
			tmp = tkinfo[type].regno;
			if( IsIntReg(tmp) ) {
			    opd_top.o_type = O_INT;
			    opd_top.o_val = 
				((ThreadRegs *)cur_thd)->ireg[tmp - NRREG];
			} else {
			    opd_top = ((Object *)
				       ((ThreadRegs *)cur_thd)->rreg)[tmp]; 
			}
		    }
		} else switch(type) {
		case T_PITCH:
		    opd_push();
		    opd_top.o_type = O_INT;
		    opd_top.o_val = pitch_to_val(tp);
		    break;

		case T_STRING:
		    opd_push();
		    opd_top.o_type = O_STRING;
		    if( !(opd_top.o_str = strdup(tp->u.obj.o_str)) ) {
			err_nomem("get_expression");
		    }
		    break;
		    
		case T_STOKENS:
		    opd_push();
		    opd_top.o_type = O_STOKENS;
		    opd_top.o_tp = copy_tklist(tp->u.obj.o_tp);
		    break;

		case '\'':
		    scan_single_quote(tp);
		    break;
		
		case '`':
		    scan_back_quote(tp);
		    break;
		
		default: /* T_NUMBER, T_RATIONAL, or T_ARRAY */
		    opd_push();
		    opd_top = tp->u.obj;
		}

	    } else if( type == '(' ) {
		optr_push(type);

	    } else if( flags & F_U ) {
		/* unary operator */
		/*
		 * while the operater of the stack top has higher
		 * precedence over `opr', pop the stack top and 
		 * do the calculation
		 */
		while( tkinfo[optr_top.op].prec > tkinfo[type].prec) { 
		    (*(tkinfo[optr_pop()].handler))();
		}
		optr_push(type);

	    } else if( type == '{' ) {
		Token  *tklist;

		if( !(tkinfo[optr_top.op].flags & F_L) ) {
		    parse_error(tp);
		}
		if( optr_top.skip_flag ) {
		    tklist = scan_tklist(0, SKIP, 
					 "(in expression)", -'{', '}');
		    opd_push();
		    opd_top.o_type = O_INT;
		    opd_top.o_val = 0;
		} else {
		    tklist = scan_tklist(0, COLLECT, 
					 "(in expression)", -'{', '}');
		    pushbk_tklist(tklist, TF_FREE);
		    type = prev_type;	/* fake as if there is no '{' */
		}
		
	    } else {
		/* there is no valid operand */
		if( prev_type == T_BOTTOM ) {
		    /* clean stack */
		    while( optr_pop() != T_BOTTOM );
		    while( opd_stack[--opd_n].o_type != O_NIL );
		    return NULL;
		} else {
		    parse_error(tp);
		}
	    }

	} else {
	    /* 
	     * If the previous token is an operand or ')',
	     * then a binary or ternary operator or ')' is expected.
	     * (otherwise the end of expression)
	     */
	    type = probe_next_token()->type;

	    if( type == ')' ) {
		while( (tmp = optr_pop()) != '(' && tmp != T_BOTTOM ) {
		    (*(tkinfo[tmp].handler))();
		}
		if( tmp == T_BOTTOM ) {
		    /* The ')' is not this expression's one. */
		    break;	/* end of expression */
		}
		get_token();

	    } else if( (flags = tkinfo[type].flags) & F_B ) {
		/* binary operator */
		get_token();
		while( tkinfo[optr_top.op].prec >= tkinfo[type].prec) { 
		    (*(tkinfo[optr_pop()].handler))();
		}
		optr_push(type);
		
		if( flags & F_L ) {
		    if( type == T_LOGAND ) {
			if( !istrue_object(&opd_top) ) {
			    optr_top.skip_flag = 1;
			}
		    } else { /* type == T_LOGOR */
			if( istrue_object(&opd_top) ) {
			    optr_top.skip_flag = 1;
			}
		    }
		}

	    } else if( flags & F_T ) {
		/* ternary operator */
		get_token();
		while( tkinfo[optr_top.op].prec > tkinfo[type].prec) { 
		    (*(tkinfo[optr_pop()].handler))();
		}
		if( optr_top.op == ':' && type == ':' ) { 
		    /* in the case like "a ? b ? x : y : z" */
		    (*(tkinfo[optr_pop()].handler))();
		}
		optr_push(type);

		if( type == '?' ) {
		    if( (optr_sec.op == '?' || optr_sec.op == ':') &&
		       optr_sec.skip_flag ) {
			/* In case of "1 ? {x} : b ? {y} : {z}",
			   neither '{y}' nor '{z}' should be evaluated. */
			optr_top.skip_flag = 2;
		    } else if( !istrue_object(&opd_top) ) {
			optr_top.skip_flag = 1;
		    }
		} else { /* type == ':' */
		    optr_top.skip_flag = 1 - optr_sec.skip_flag;
		}

	    } else {
		/* end of expression */
		/* process operators remaining in the operator stack */
		while( (tmp = optr_pop()) != T_BOTTOM ) {
		    (*(tkinfo[tmp].handler))();
		}
		break;
	    }
	}
    }

    /* 
     * pop the operand stack until "barrier data" is found
     *    Normally the barrier data is on the second of operand stack now, 
     *    but it may not be so if error occurs.
     */
    while( opd_stack[--opd_n].o_type != O_NIL );

    return &opd_stack[opd_n + 1];
}

/*
 * scan between two single-quotes
 */
static void
scan_single_quote(tp)
Token  *tp;
{
    Token  *tklist, **nextp;
    long   src_pos1;
    
    src_pos1 = tp->src_pos;
    nextp = &tklist;

    while( (tp = get_token())->type != '\'' ) {
	if( tp->type == T_EOF ) {
	    error(src_pos1, "Closing single quote is not found",
		  "対応するシングルクオート(')がありません");
	}
	*nextp = copy_token(tp);
	nextp = &(*nextp)->next;
	if( tp->type == T_ARRAY ) {
	    free_array(tp->u.obj.o_ap);
	}
    }
    *nextp = NULL;
    opd_push();
    opd_top.o_type = O_STOKENS;
    opd_top.o_tp = tklist;
}

/*
 * scan between two back-quotes
 */
static void
scan_back_quote(tp)
Token  *tp;
{
    Token  *tklist, **nextp;
    long   src_pos1;
    
    src_pos1 = tp->src_pos;
    nextp = &tklist;

    while( (tp = parse_cmds(ARGS_ONLY))->type != '`' ) {
	if( tp->type == T_EOF ) {
	    error(src_pos1, "Closing back quote is not found",
		  "対応するバッククオート(`)がありません");
	}
	*nextp = copy_token(tp);
	nextp = &(*nextp)->next;
	if( tp->type == T_ARRAY ) {
	    free_array(tp->u.obj.o_ap);
	}
    }
    *nextp = NULL;
    opd_push();
    opd_top.o_type = O_STOKENS;
    opd_top.o_tp = tklist;
}

/*********************************************************************
 * Calculate the key number from PITCH token
 *********************************************************************/
int
pitch_to_val(tp)
Token  *tp;
{
    int  note_num, oct;
    static short  key_table[16] = {
	0x000,  /* C dur */   
	0x020,  /* G dur */  
	0x021,  /* D dur */  
	0x0a1,  /* A dur */  
	0x0a5,  /* E dur */  
	0x2a5,  /* B dur */  
	0x2b5,  /* Fis dur */
	0xab5,  /* Cis dur */
	0x000,  /* (C dur) */
	0xab5,  /* Ces dur */
	0xa95,  /* Ges dur */
	0xa94,  /* Des dur */
	0xa14,  /* As dur */ 
	0xa10,  /* Es dur */ 
	0x810,  /* B dur */  
	0x800,  /* F dur */  
    };

    if( tp->u.pitch.flags & RESTFLAG ) {
	return REST_NOTE;
    }
    note_num = tp->u.pitch.note_num;
#ifdef DEBUG
    if( note_num < 0 || note_num >= 12 ) {
	fprintf(stderr, "Internal error (pitch_to_val)\n");
	abort();
    }
#endif
    if( !(tp->u.pitch.flags & ACCIDENTAL) ) {
	if( cur_thd->reg.key >= 0 ) {
	    note_num += (key_table[cur_thd->reg.key & 15] >> note_num) & 1;
	} else {
	    note_num -= (key_table[cur_thd->reg.key & 15] >> note_num) & 1;
	}
    }
    oct = tp->u.pitch.octave + 1;
    if( !(tp->u.pitch.flags & ABS_OCT) ) {
	oct += cur_thd->reg.o;
    }
#ifdef ADD_TP_IN_CONST
    return( note_num + oct * 12 + cur_thd->reg.tp );
#else
    return( note_num + oct * 12 );
#endif
}

/*********************************************************************
 * Grow the size of the stacks
 *********************************************************************/
static void
grow_opd_stack()
{
    opd_nallocated += OPS_BLKSIZ;
    opd_stack = (Object *) realloc(opd_stack, 
				   sizeof(Object) * opd_nallocated );
    if( !opd_stack )  err_nomem("grow_opd_stack");
}

static void
grow_optr_stack()
{
    optr_nallocated += OPS_BLKSIZ;
    optr_stack = (struct optrstk *) 
	realloc(optr_stack, sizeof(struct optrstk) * optr_nallocated );
    if( !optr_stack )  err_nomem("grow_optr_stack");
}


/*********************************************************************
 * Handlers for each operator
 *********************************************************************/
void
op_uminus()
{
    switch( dest1.o_type ) {
    case O_INT:
	dest1.o_val = - dest1.o_val;
	break;
    case O_RATIONAL:
	rneg(&dest1.r);
	break;
    case O_STRING:
    case O_ARRAY:
    case O_STOKENS:
    case O_WTOKENS:
	op_badopd();
	break;
    case O_NIL:
	op_serror();
	break;
    default:  /* float */
	dest1.fpval = - dest1.fpval;
    }
}

void
op_lognot()
{
    int  true;

    if( (true = istrue_object(&dest1)) == -1 ) {
	op_serror();
    }
    free_object(&dest1);
    dest1.o_type = O_INT;
    dest1.o_val = ! true;
}

void
op_bitnot()
{
    if( dest1.o_type != O_INT ) {
	if( dest1.o_type == O_NIL ) {
	    op_serror();
	} else {
	    op_badopd();
	}
    } else {
	dest1.o_val = ~ dest1.o_val;
    }
}

void
op_mult()
{
    switch( src.o_type ) {
    case O_INT:
	switch( dest.o_type ) {
	case O_INT:
	    dest.o_val *= src.o_val;
	    break;
	case O_RATIONAL:
	    rimult(&dest.r, src.o_val, &dest.r);
	    break;
	case O_STRING:
	case O_ARRAY:
	case O_STOKENS:
	case O_WTOKENS:
	case O_NIL:
	    op_error();
	    break;
	default:  /* float */
	    dest.fpval *= src.o_val;
	    break;
	}
	break;
    case O_RATIONAL:
	switch( dest.o_type ) {
	case O_INT:
	    rimult(&src.r, dest.o_val, &dest.r);
	    break;
	case O_RATIONAL:
	    rmult(&dest.r, &src.r, &dest.r);
	    break;
	case O_STRING:
	case O_ARRAY:
	case O_STOKENS:
	case O_WTOKENS:
	case O_NIL:
	    op_error();
	    break;
	default:  /* float */
	    dest.fpval *= rational_to_float(&src.r);
	    break;
	}
	break;
    case O_STRING:
    case O_ARRAY:
    case O_STOKENS:
    case O_WTOKENS:
    case O_NIL:
	op_error();
	break;
    default:  /* float */
	switch( dest.o_type ) {
	case O_INT:
	    dest.fpval = dest.o_val * src.fpval;
	    break;
	case O_RATIONAL:
	    dest.fpval = rational_to_float(&dest.r) * src.fpval;
	    break;
	case O_STRING:
	case O_ARRAY:
	case O_STOKENS:
	case O_WTOKENS:
	case O_NIL:
	    op_error();
	    break;
	default:  /* float */
	    dest.fpval *= src.fpval;
	    break;
	}
	break;
    }
    opd_pop();
}

void
op_div()
{
    Rational  rtmp;

    switch( src.o_type ) {
    case O_INT:
	if( src.o_val == 0 )  op_divzero();
	switch( dest.o_type ) {
	case O_INT:
	    dest.o_val /= src.o_val;
	    break;
	case O_RATIONAL:
	    ridiv(&dest.r, src.o_val, &dest.r);
	    break;
	case O_STRING:
	case O_ARRAY:
	case O_STOKENS:
	case O_WTOKENS:
	case O_NIL:
	    op_error();
	    break;
	default:  /* float */
	    dest.fpval /= src.o_val;
	    break;
	}
	break;
    case O_RATIONAL:
	if( rzerop(&src.r) )  op_divzero();
	switch( dest.o_type ) {
	case O_INT:
	    rtmp.intg = dest.o_val;
	    rtmp.num = 0;
	    rtmp.den = LDEN;
	    rtmp.type = O_RATIONAL;
	    rdiv(&rtmp, &src.r, &dest.r);
	    break;
	case O_RATIONAL:
	    rdiv(&dest.r, &src.r, &dest.r);
	    break;
	case O_STRING:
	case O_ARRAY:
	case O_STOKENS:
	case O_WTOKENS:
	case O_NIL:
	    op_error();
	    break;
	default:  /* float */
	    dest.fpval /= rational_to_float(&src.r);
	    break;
	}
	break;
    case O_STRING:
    case O_ARRAY:
    case O_STOKENS:
    case O_WTOKENS:
    case O_NIL:
	op_error();
	break;
    default:  /* float */
	if( src.fpval == 0.0 )  op_divzero();
	switch( dest.o_type ) {
	case O_INT:
	    dest.fpval = dest.o_val / src.fpval;
	    break;
	case O_RATIONAL:
	    dest.fpval = rational_to_float(&dest.r) / src.fpval;
	    break;
	case O_STRING:
	case O_ARRAY:
	case O_STOKENS:
	case O_WTOKENS:
	case O_NIL:
	    op_error();
	    break;
	default:  /* float */
	    dest.fpval /= src.fpval;
	    break;
	}
	break;
    }
    opd_pop();
}

void
op_rem()
{
    Rational  rtmp;

    switch( src.o_type ) {
    case O_INT:
	if( src.o_val == 0 )  op_divzero();
	switch( dest.o_type ) {
	case O_INT:
	    dest.o_val %= src.o_val;
	    break;
	case O_RATIONAL:
	    rtmp.intg = src.o_val;
	    rtmp.num = 0;
	    rtmp.den = LDEN;
	    rtmp.type = O_RATIONAL;
	    rmod(&dest.r, &rtmp, &dest.r);
	    break;
	case O_STRING:
	case O_ARRAY:
	case O_STOKENS:
	case O_WTOKENS:
	case O_NIL:
	    op_error();
	    break;
	default:  /* float */
	    dest.fpval = fmod(dest.fpval, (double)src.o_val);
	    break;
	}
	break;
    case O_RATIONAL:
	if( rzerop(&src.r) )  op_divzero();
	switch( dest.o_type ) {
	case O_INT:
	    rtmp.intg = dest.o_val;
	    rtmp.num = 0;
	    rtmp.den = LDEN;
	    rtmp.type = O_RATIONAL;
	    rmod(&rtmp, &src.r, &dest.r);
	    break;
	case O_RATIONAL:
	    rmod(&dest.r, &src.r, &dest.r);
	    break;
	case O_STRING:
	case O_ARRAY:
	case O_STOKENS:
	case O_WTOKENS:
	case O_NIL:
	    op_error();
	    break;
	default:  /* float */
	    dest.fpval = fmod(dest.fpval, rational_to_float(&src.r));
	    break;
	}
	break;
    case O_STRING:
    case O_ARRAY:
    case O_STOKENS:
    case O_WTOKENS:
    case O_NIL:
	op_error();
	break;
    default:  /* float */
	if( src.fpval == 0.0 )  op_divzero();
	switch( dest.o_type ) {
	case O_INT:
	    dest.fpval = fmod((double)dest.o_val, src.fpval);
	    break;
	case O_RATIONAL:
	    dest.fpval = fmod(rational_to_float(&dest.r), src.fpval);
	    break;
	case O_STRING:
	case O_ARRAY:
	case O_STOKENS:
	case O_WTOKENS:
	case O_NIL:
	    op_error();
	    break;
	default:  /* float */
	    dest.fpval = fmod(dest.fpval, src.fpval);
	    break;
	}
	break;
    }
    opd_pop();
}

void
op_add()
{
    Rational  rtmp;

    switch( src.o_type ) {
    case O_INT:
	switch( dest.o_type ) {
	case O_INT:
	    dest.o_val += src.o_val;
	    break;
	case O_RATIONAL:
	    int_to_rational(src.o_val, &rtmp);
	    radd(&dest.r, &rtmp, &dest.r);
	    break;
	case O_STRING:
	case O_ARRAY:
	case O_STOKENS:
	case O_WTOKENS:
	case O_NIL:
	    op_error();
	    break;
	default:  /* float */
	    dest.fpval += src.o_val;
	    break;
	}
	break;
    case O_RATIONAL:
	switch( dest.o_type ) {
	case O_INT:
	    int_to_rational(dest.o_val, &rtmp);
	    radd(&rtmp, &src.r, &dest.r);
	    break;
	case O_RATIONAL:
	    radd(&dest.r, &src.r, &dest.r);
	    break;
	case O_STRING:
	case O_ARRAY:
	case O_STOKENS:
	case O_WTOKENS:
	case O_NIL:
	    op_error();
	    break;
	default:  /* float */
	    dest.fpval += rational_to_float(&src.r);
	    break;
	}
	break;
    case O_STRING:
	if( dest.o_type == O_STRING ) {
	    int  len1, len2;
	    len1 = strlen(src.o_str);
	    len2 = strlen(dest.o_str);
	    if( !(dest.o_str = realloc(dest.o_str, len1 + len2 + 1))) {
		err_nomem("op_add");
	    }
	    strcpy(dest.o_str + len2, src.o_str);
	    strfree(src.o_str);
	} else  op_error();
	break;
    case O_ARRAY:
    case O_STOKENS:
    case O_WTOKENS:
    case O_NIL:
	op_error();
	break;
    default:  /* float */
	switch( dest.o_type ) {
	case O_INT:
	    dest.fpval = dest.o_val + src.fpval;
	    break;
	case O_RATIONAL:
	    dest.fpval = rational_to_float(&dest.r) + src.fpval;
	    break;
	case O_STRING:
	case O_ARRAY:
	case O_STOKENS:
	case O_WTOKENS:
	case O_NIL:
	    op_error();
	    break;
	default:  /* float */
	    dest.fpval += src.fpval;
	    break;
	}
	break;
    }
    opd_pop();
}

void
op_sub()
{
    Rational  rtmp;

    switch( src.o_type ) {
    case O_INT:
	switch( dest.o_type ) {
	case O_INT:
	    dest.o_val -= src.o_val;
	    break;
	case O_RATIONAL:
	    int_to_rational(src.o_val, &rtmp);
	    rsub(&dest.r, &rtmp, &dest.r);
	    break;
	case O_STRING:
	case O_ARRAY:
	case O_STOKENS:
	case O_WTOKENS:
	case O_NIL:
	    op_error();
	    break;
	default:  /* float */
	    dest.fpval -= src.o_val;
	    break;
	}
	break;
    case O_RATIONAL:
	switch( dest.o_type ) {
	case O_INT:
	    int_to_rational(dest.o_val, &rtmp);
	    rsub(&rtmp, &src.r, &dest.r);
	    break;
	case O_RATIONAL:
	    rsub(&dest.r, &src.r, &dest.r);
	    break;
	case O_STRING:
	case O_ARRAY:
	case O_STOKENS:
	case O_WTOKENS:
	case O_NIL:
	    op_error();
	    break;
	default:  /* float */
	    dest.fpval -= rational_to_float(&src.r);
	    break;
	}
	break;
    case O_STRING:
    case O_ARRAY:
    case O_STOKENS:
    case O_WTOKENS:
    case O_NIL:
	op_error();
	break;
    default:  /* float */
	switch( dest.o_type ) {
	case O_INT:
	    dest.fpval = dest.o_val - src.fpval;
	    break;
	case O_RATIONAL:
	    dest.fpval = rational_to_float(&dest.r) - src.fpval;
	    break;
	case O_STRING:
	case O_ARRAY:
	case O_STOKENS:
	case O_WTOKENS:
	case O_NIL:
	    op_error();
	    break;
	default:  /* float */
	    dest.fpval -= src.fpval;
	    break;
	}
	break;
    }
    opd_pop();
}

void
op_shr()
{
    if( dest.o_type == O_INT && src.o_type == O_INT ) { 
	dest.o_val >>= src.o_val;
    } else  op_error();
    opd_pop();
}

void
op_shl()
{
    if( dest.o_type == O_INT && src.o_type == O_INT ) { 
	dest.o_val <<= src.o_val;
    } else  op_error();
    opd_pop();
}

void
op_lt()
{
    Rational  rtmp;

    switch( src.o_type ) {
    case O_INT:
	switch( dest.o_type ) {
	case O_INT:
	    dest.o_val = dest.o_val < src.o_val;
	    break;
	case O_RATIONAL:
	    int_to_rational(src.o_val, &rtmp);
	    dest.o_val = rless(&dest.r, &rtmp);
	    dest.o_type = O_INT;
	    break;
	case O_STRING:
	case O_ARRAY:
	case O_STOKENS:
	case O_WTOKENS:
	case O_NIL:
	    op_error();
	    break;
	default:  /* float */
	    dest.o_val = dest.fpval < src.o_val;
	    dest.o_type = O_INT;
	    break;
	}
	break;
    case O_RATIONAL:
	switch( dest.o_type ) {
	case O_INT:
	    int_to_rational(dest.o_val, &rtmp);
	    dest.o_val = rless(&rtmp, &src.r);
	    break;
	case O_RATIONAL:
	    dest.o_val = rless(&dest.r, &src.r);
	    dest.o_type = O_INT;
	    break;
	case O_STRING:
	case O_ARRAY:
	case O_STOKENS:
	case O_WTOKENS:
	case O_NIL:
	    op_error();
	    break;
	default:  /* float */
	    dest.o_val = dest.fpval < rational_to_float(&src.r);
	    dest.o_type = O_INT;
	    break;
	}
	break;
    case O_STRING:
	if( dest.o_type == O_STRING ) {
	    int  cmp = strcmp(dest.o_str, src.o_str);
	    strfree(dest.o_str);
	    strfree(src.o_str);
	    dest.o_val = cmp < 0;
	    dest.o_type = O_INT;
	} else  op_error();
	break;
    case O_ARRAY:
    case O_STOKENS:
    case O_WTOKENS:
    case O_NIL:
	op_error();
	break;
    default:  /* float */
	switch( dest.o_type ) {
	case O_INT:
	    dest.o_val = dest.o_val < src.fpval;
	    break;
	case O_RATIONAL:
	    dest.o_val = rational_to_float(&dest.r) < src.fpval;
	    dest.o_type = O_INT;
	    break;
	case O_STRING:
	case O_ARRAY:
	case O_STOKENS:
	case O_WTOKENS:
	case O_NIL:
	    op_error();
	    break;
	default:  /* float */
	    dest.o_val = dest.fpval < src.fpval;
	    dest.o_type = O_INT;
	    break;
	}
	break;
    }
    opd_pop();
}

void
op_lteq()
{
    Rational  rtmp;

    switch( src.o_type ) {
    case O_INT:
	switch( dest.o_type ) {
	case O_INT:
	    dest.o_val = dest.o_val <= src.o_val;
	    break;
	case O_RATIONAL:
	    int_to_rational(src.o_val, &rtmp);
	    dest.o_val = rlesseq(&dest.r, &rtmp);
	    dest.o_type = O_INT;
	    break;
	case O_STRING:
	case O_ARRAY:
	case O_STOKENS:
	case O_WTOKENS:
	case O_NIL:
	    op_error();
	    break;
	default:  /* float */
	    dest.o_val = dest.fpval <= src.o_val;
	    dest.o_type = O_INT;
	    break;
	}
	break;
    case O_RATIONAL:
	switch( dest.o_type ) {
	case O_INT:
	    int_to_rational(dest.o_val, &rtmp);
	    dest.o_val = rlesseq(&rtmp, &src.r);
	    break;
	case O_RATIONAL:
	    dest.o_val = rlesseq(&dest.r, &src.r);
	    dest.o_type = O_INT;
	    break;
	case O_STRING:
	case O_ARRAY:
	case O_STOKENS:
	case O_WTOKENS:
	case O_NIL:
	    op_error();
	    break;
	default:  /* float */
	    dest.o_val = dest.fpval <= rational_to_float(&src.r);
	    dest.o_type = O_INT;
	    break;
	}
	break;
    case O_STRING:
	if( dest.o_type == O_STRING ) {
	    int  cmp = strcmp(dest.o_str, src.o_str);
	    strfree(dest.o_str);
	    strfree(src.o_str);
	    dest.o_val = cmp <= 0;
	    dest.o_type = O_INT;
	} else  op_error();
	break;
    case O_ARRAY:
    case O_STOKENS:
    case O_WTOKENS:
    case O_NIL:
	op_error();
	break;
    default:  /* float */
	switch( dest.o_type ) {
	case O_INT:
	    dest.o_val = dest.o_val <= src.fpval;
	    break;
	case O_RATIONAL:
	    dest.o_val = rational_to_float(&dest.r) <= src.fpval;
	    dest.o_type = O_INT;
	    break;
	case O_STRING:
	case O_ARRAY:
	case O_STOKENS:
	case O_WTOKENS:
	case O_NIL:
	    op_error();
	    break;
	default:  /* float */
	    dest.o_val = dest.fpval <= src.fpval;
	    dest.o_type = O_INT;
	    break;
	}
	break;
    }
    opd_pop();
}

void
op_gt()
{
    op_lteq();
    opd_top.o_val = !opd_top.o_val; 
}

void
op_gteq()
{
    op_lt();
    opd_top.o_val = !opd_top.o_val; 
}

void
op_eq()
{
    Rational  rtmp;

    switch( src.o_type ) {
    case O_INT:
	switch( dest.o_type ) {
	case O_INT:
	    dest.o_val = dest.o_val == src.o_val;
	    break;
	case O_RATIONAL:
	    int_to_rational(src.o_val, &rtmp);
	    dest.o_val = requal(&dest.r, &rtmp);
	    dest.o_type = O_INT;
	    break;
	case O_STRING:
	case O_ARRAY:
	case O_STOKENS:
	case O_WTOKENS:
	    free_object(&dest);
	    dest.o_val = 0;
	    dest.o_type = O_INT;
	    break;
	case O_NIL:
	    op_error();
	    break;
	default:  /* float */
	    dest.o_val = dest.fpval == src.o_val;
	    dest.o_type = O_INT;
	    break;
	}
	break;
    case O_RATIONAL:
	switch( dest.o_type ) {
	case O_INT:
	    int_to_rational(dest.o_val, &rtmp);
	    dest.o_val = requal(&rtmp, &src.r);
	    break;
	case O_RATIONAL:
	    dest.o_val = requal(&dest.r, &src.r);
	    dest.o_type = O_INT;
	    break;
	case O_STRING:
	case O_ARRAY:
	case O_STOKENS:
	case O_WTOKENS:
	    free_object(&dest);
	    dest.o_val = 0;
	    dest.o_type = O_INT;
	    break;
	case O_NIL:
	    op_error();
	    break;
	default:  /* float */
	    dest.o_val = dest.fpval == rational_to_float(&src.r);
	    dest.o_type = O_INT;
	    break;
	}
	break;
    case O_STRING:
	if( dest.o_type == O_STRING ) {
	    int  cmp = strcmp(dest.o_str, src.o_str);
	    strfree(dest.o_str);
	    strfree(src.o_str);
	    dest.o_val = cmp == 0;
	    dest.o_type = O_INT;
	} else if( dest.o_type == O_NIL ) {
	    op_error();
	} else {
	    free_object(&dest);
	    free_object(&src);
	    dest.o_val = 0;
	    dest.o_type = O_INT;
	}
	break;
    case O_ARRAY:
    case O_STOKENS:
    case O_WTOKENS:
	if( dest.o_type == src.o_type ) {
	    int  cmp = compare_object(&dest, &src);
	    free_object(&dest);
	    free_object(&src);
	    dest.o_val = cmp == 0;
	    dest.o_type = O_INT;
	} else if( dest.o_type == O_NIL ) {
	    op_error();
	} else {
	    free_object(&dest);
	    free_object(&src);
	    dest.o_val = 0;
	    dest.o_type = O_INT;
	}
	break;
    case O_NIL:
	op_error();
	break;
    default:  /* float */
	switch( dest.o_type ) {
	case O_INT:
	    dest.o_val = dest.o_val == src.fpval;
	    break;
	case O_RATIONAL:
	    dest.o_val = rational_to_float(&dest.r) == src.fpval;
	    dest.o_type = O_INT;
	    break;
	case O_STRING:
	case O_ARRAY:
	case O_STOKENS:
	case O_WTOKENS:
	case O_NIL:
	    op_error();
	    break;
	default:  /* float */
	    dest.o_val = dest.fpval == src.fpval;
	    dest.o_type = O_INT;
	    break;
	}
	break;
    }
    opd_pop();
}

void
op_neq()
{
    op_eq();
    opd_top.o_val = !opd_top.o_val; 
}

void
op_bitand()
{
    if( dest.o_type == O_INT && src.o_type == O_INT ) { 
	dest.o_val &= src.o_val;
    } else  op_error();
    opd_pop();
}

void
op_xor()
{
    if( dest.o_type == O_INT && src.o_type == O_INT ) { 
	dest.o_val ^= src.o_val;
    } else  op_error();
    opd_pop();
}

void
op_bitor()
{
    if( dest.o_type == O_INT && src.o_type == O_INT ) { 
	dest.o_val |= src.o_val;
    } else  op_error();
    opd_pop();
}

void
op_logand()
{
    int  dtrue, strue;

    if( (dtrue = istrue_object(&dest)) == -1 ||
        (strue = istrue_object(&src)) == -1 ) {
	op_serror();
    }
    free_object(&dest);
    free_object(&src);
    dest.o_type = O_INT;
    dest.o_val = dtrue && strue;
    opd_pop();
}

void
op_logor()
{
    int  dtrue, strue;

    if( (dtrue = istrue_object(&dest)) == -1 ||
        (strue = istrue_object(&src)) == -1 ) {
	op_serror();
    }
    free_object(&dest);
    free_object(&src);
    dest.o_type = O_INT;
    dest.o_val = dtrue || strue;
    opd_pop();
}

void
op_cond()
{
    int  true;

    if( optr_top.op != '?' ) {
	op_serror();
    } 
    --optr_n;

    if( (true = istrue_object(&opd_third)) == -1 ) {
	op_serror();
    }
    free_object(&opd_third);

    if( true ) {
	opd_third = opd_sec;
	free_object(&opd_top);
    } else {
	opd_third = opd_top;
	free_object(&opd_sec);
    }
    opd_pop();
    opd_pop();
}


/*********************************************************************
 * Error routines
 *********************************************************************/
void
op_serror() 
{
    Token  token;
    token.type = optr_stack[optr_n].op;
    parse_error(&token);
}


void
op_fatal()
{
    error(cur_srcpos,
	  "Internal error (get_expression)", "内部エラー(get_expression)");
}

static void
op_error()
{
    Token  token;
    token.type = optr_stack[optr_n].op;

    if( opd_top.o_type == O_NIL || opd_sec.o_type == O_NIL ) {
	parse_error(&token);
    } else if( (isfloat(opd_top.o_type) && isfloat(opd_sec.o_type)) || 
	      opd_top.o_type == opd_sec.o_type ) {
	op_badopd();
    } else {
	terror(cur_srcpos, &token, "Operand type mismatch", 
	       "演算数の型が合っていません");
    }
}

static void
op_badopd()
{
    Token  token;
    token.type = optr_stack[optr_n].op;
    terror(cur_srcpos, &token, "Illegal operand type", 
	       "演算数の型が不適当です");
}

static void
op_divzero()
{
    Token  token;
    token.type = optr_stack[optr_n].op;
    terror(cur_srcpos, &token, "Division by zero", "０による除算");
}

/*********************************************************************
 * Self tester
 *********************************************************************/
#ifdef EXPR_SELF_TEST
/* To do self test, compile expr.c with input.c, error.c, token.c, util.c, 
   array.c, rational.c, table.c, parseut.c and keyword.c. 
   Use the '-Xlinker -noinhibit-exec' option of the gcc compiler. */
int  japan = 0;
void fprint_calls(fp) FILE *fp; {}
void end_of_macro() {}
int hash_func(p) char *p; { return 0; }
/* Token * scan_tklist(a,b,c,d,e) int a,b,d,e; char *c; {} */
Thread  *cur_thd;

Token *
parse_cmds(dummy)
int dummy;
{
    return get_token();
}

main()
{
    Object  *op;

    init_input();
    init_expression();
    cur_thd = (Thread *) malloc(sizeof(Thread));
    init_pbstack(cur_thd);
    pushbk_file("-", 0);

    op = get_expression();
    fprint_object(stdout, op, -1, -1);
    printf("\n");
}
#endif
