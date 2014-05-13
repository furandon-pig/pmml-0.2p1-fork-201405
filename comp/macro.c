/*
 * PMML: A Compiler for the Practical Music Macro Language
 *
 * macro.c: handlers for macro directives
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
#include "pmml.h"

/*
 * global variables
 */
extern int  trace_level;

/*
 * function prototypes
 */
static void  err_calls P((void));
static void  print_trace P((char *, MacroCall *, Object *));

/*
 * macros
 */
#define  check_calls()   {if( !cur_thd->pb->calls )  err_calls(); }

#define TRACE_MAXARGS  -1
#define TRACE_MAXELMS   20
#define TRACE_MAXTKLEN  50

/*********************************************************************
 * Routines for macro expansion
 *********************************************************************/

/*
 * evaluate ID, ID(arguments...), ID[exp], and assignment 
 */
int
identifier()
{
    DicEnt *dp;
    int  is_defined, assign_type, i;
    Object *op;
    MacroCall *mp;

    if( (is_defined = get_id(SD_ANY, DEF_ON_UNDEF, &dp)) == -1 ) {
	error(cur_srcpos, "Inappropriate identifier", "名前が不適切です");
    }
    assign_type = probe_next_token()->type;

    if( assign_type == '=' ) {
	/*
	 * simple assignment
	 */
	get_token();	/* skip '=' */
	if( !(op = get_expression()) )  parse_error(cur_token);
	dp->active--;
	if( is_defined == 1 && dp->type != D_LocalMacro && 
		  dp->scope.c != cur_thd ) {
	    DicEnt  *dp1;

	    /* the identifier is not mine! */
	    if( !(dp1 = (DicEnt *) malloc(sizeof(DicEnt))) ||
	        !(dp1->name = strdup(dp->name)) ) {
		err_nomem("identifier");
	    }
	    dp1->type = D_ThreadMacro;
	    dp1->scope.c = cur_thd;
	    dp1->hash = dp->hash;
	    dp1->active = 0;
	    dp1->dic_obj = *op;
	    dp1->dic_arg_spec = NULL;
	    insert_dict(dp1);
	} else {
	    redefine_macro(dp, op, NULL);
	}
	
    } else {
	if( !is_defined ) {
	    error(cur_srcpos, "Undefined macro '%s'",
		  "'%s' が定義されていません", dp->name);
	}

	if( tkinfo[assign_type].flags & F_CA ) {
	    /*
	     * compound assignment
	     */
	    if( (dp->type != D_LocalMacro && dp->type != D_ThreadMacro) || 
	       dp->dic_arg_spec || !isleftval(dp->dic_obj.o_type) ) {
		error(cur_srcpos, "%s: Illegal left value",
		      "%s: 左辺が間違っています", dp->name);
	    }
	    get_token();	/* skip assignment operator */
	    op = _get_expression(assign_type, &dp->dic_obj);
	    if( !op )  parse_error(cur_token);
	    dp->active--;
	    if( is_defined == 1 && dp->type != D_LocalMacro && 
	           dp->scope.c != cur_thd ) {
		DicEnt  *dp1;
		
		/* the identifier is not mine! */
		if( !(dp1 = (DicEnt *) malloc(sizeof(DicEnt))) ||
		   !(dp1->name = strdup(dp->name)) ) {
		    err_nomem("identifier");
		}
		dp1->type = D_ThreadMacro;
		dp1->scope.c = cur_thd;
		dp1->hash = dp->hash;
		dp1->active = 0;
		dp1->dic_obj = *op;
		dp1->dic_arg_spec = NULL;
		insert_dict(dp1);
	    } else {
		free_object(&dp->dic_obj);
		dp->dic_obj = *op;
	    }

	} else switch( dp->type ) {
	case D_BuiltIn:
	    if( dp->dic_arg_spec ) {
		/* with arguments */
		mp = scan_args(dp->dic_arg_spec, dp->name);
		(* dp->dic_handler)(mp->nargs, mp->arg);
		for( i = 0; i < mp->nargs; i++ ) {
		    free_object(&mp->arg[i]);
		}
		free(mp);
	    } else {
		/* no arguments */
		(* dp->dic_handler)(0, NULL);
	    }
	    dp->active--;
	    break;

	case D_LocalMacro:
	case D_ThreadMacro:
	    /*
	     * macro call
	     */
	    if( dp->dic_arg_spec ) {
		/* function macros */
		mp = scan_args(dp->dic_arg_spec, dp->name);
		mp->macro = dp;
		mp->lmacros = NULL;
		mp->src_pos = cur_srcpos;
		mp->arg_array = create_ngarray(mp->nargs, mp->arg);
		mp->ref = 1;

		if( trace_level >= 1 ) {
		    print_trace(dp->name, mp, &dp->dic_obj);
		}

		mp->next = cur_thd->pb->calls;
		cur_thd->pb->calls = mp;

		pushbk_tklist(dp->dic_obj.o_tp, TF_CALLEOM);

	    } else {
		/* variable macros */
		if( trace_level >= 2 ) {
		    print_trace(dp->name, NULL, &dp->dic_obj);
		}
		pushbk_object(&dp->dic_obj);
	    }
	    dp->active--;
	    break;

	case D_ThreadName:
	    pushbk_special_id(T_THREAD, dp);
	    /* dp->active remains increased */
	    break;

	case D_EffClass:
	    pushbk_special_id(T_EFFCLASS, dp);
	    /* dp->active remains increased */
	    break;

	case D_EffInst:
	    pushbk_special_id(T_EFFINST, dp);
	    /* dp->active remains increased */
	    break;

	case D_Channel:
	    error(cur_srcpos, "Parse error near `%s'", 
		  "`%s' の付近に文法の誤りがあります", dp->name);
	    break;
	}
    }

    return 0;
}

/*
 * post-processing routine for macro expansion
 */
void
end_of_macro()
{
    MacroCall *oldcalls;

    /* first, update calls */
    oldcalls = cur_thd->pb->calls;
    cur_thd->pb->calls = cur_thd->pb->calls->next;

    /* delete MacroCall */
    delete_mc(oldcalls);
}

/*
 * delete MacroCall structure 
 */
void
delete_mc(mp)
MacroCall  *mp;
{
    Array  *ap;
    DicEnt  *dp, *next_dp;

    if( --mp->ref == 0 ) {
	/* delete the argument array */
	ap = mp->arg_array;
	if( --ap->ref > 0 ) {
	    /*
	     * If someone is referencing the '$*' array,
	     * make it to a normal array.
	     */
	    if( ap->ng_flag ) {
		ngarray_to_array(ap);
	    }
	} else {
	    destroy_array(ap);
	}

	/* delete all the local macros */
	for( dp = mp->lmacros; dp != NULL; dp = next_dp ) {
	    next_dp = dp->dnext;
	    if( dp->active ) {
		error(mp->src_pos, 
		      "Failed to delete local macro `%s' of macro `%s' due to extra reference (See manual for details)",
		      "ローカルマクロ`%s'（マクロ'%s'内）は他からの参照があるため消去できません（詳細はマニュアル参照）",
		      dp->name, mp->macro->name);
	    }
	    delete_dict(dp);
	}

	/* delete MacroCall struct */
	free(mp);
    }
}

/*
 * expand strong token list
 */
int
strong_token_list()
{
    pushbk_tklist(cur_token->u.obj.o_tp, TF_RPLPOS);

    return 0;
}

/*
 * evaluate array subscript
 */
int
array_subscript()
{
    Array  *ap;
    Object *op, *op1;
    int  assign_type;
    int  i;

    ap = cur_token->u.obj.o_ap;

    if( probe_next_token()->type == '[' ) {
	get_token();   /* skip '[' */
	if( !(op = get_expression()) )  parse_error(cur_token);
	if( !isnumber(op->o_type) || get_token()->type != ']' ) { 
	    error(cur_srcpos, "Parse error in array subscript",
		       "配列の添字に間違いがあります");
	}

	/* subscript range check */
	if( op->o_type != O_INT ) {
	    PmmlInt  tmp;
	    conv_to_int(op, &tmp);
	    i = tmp;
	} else {
	    i = op->o_val;
	}
	i--;	/* array subscript starts with 1 */
	if( i < 0 || i >= ap->size ) {
	    error(cur_srcpos, "Subscript out of range (value = %d)",
		   "配列の添字が範囲外です (添字値 = %d)", i+1);
	}

	assign_type = probe_next_token()->type;
	if( assign_type == '=' ) {
	    /* assignment to array element */
	    get_token();
	    op = &array_ref(ap, i);
	    if( !(op1 = get_expression()) )  parse_error(cur_token);
	    free_object(op);   /* get_expression must be called before this */
	    *op = *op1;

	} else if( tkinfo[assign_type].flags & F_CA ) {
	    /* compound assignment */
	    op = &array_ref(ap, i);
	    if( !isleftval(op->o_type) ) {
		error(cur_srcpos, "Illegal type of left value",
		      "左辺値の型が違います");
	    }
	    get_token();
	    op1 = _get_expression(assign_type, op);
	    if( !op1 )  parse_error(cur_token);
	    free_object(op);   /* get_expression must be called before this */
	    *op = *op1;

	} else {
	    /* element reference */
	    op = &array_ref(ap, i);
	    pushbk_object(op);
	}
	free_array(ap);
	return 0;

    } else {
	/* output the array as it is */

	/* recover cur_token which is destroyed by probe_next_token */
	/* This type of care is needed only when do_XXX function returns 1. */
	push_and_get_array(ap);

	return 1;
    }
}

/*
 * evaluate #(obj, obj, ...) and #array
 */
int
number_sign()
{
    int  sz;
    Token  *tp;
    MacroCall *mp;
    long  src_pos1;

    if( probe_next_token()->type == '(' ) {
	mp = scan_args("e*", "#(...)");
	pushbk_array(create_array(mp->nargs, mp->arg));
	free(mp);
	
    } else {
	src_pos1 = cur_srcpos;
	if( (tp = parse_cmds(MACRO_CMDS))->type != T_ARRAY ) {
	    error(src_pos1, "Missing array after #",
		   "# の次が配列ではありません");
	}
	sz = tp->u.obj.o_ap->size;	/* this must be saved */
	free_array(tp->u.obj.o_ap);
	pushbk_int((PmmlInt)sz);
    } 
    return 0;
}

/*
 * evaluate @array
 */
int
at_mark()
{
    Array  *ap;
    Token  *tp;
    int	 i;
    Object *op;
    long  src_pos1;

    src_pos1 = cur_srcpos;
    if( (tp = parse_cmds(MACRO_CMDS))->type != T_ARRAY ) {
	error(src_pos1, "Missing array after @",
	       "@ の次が配列ではありません");
    }
    ap = tp->u.obj.o_ap;

    for( i = ap->size; --i >= 0; ) {
	op = &array_ref(ap, i);
	pushbk_object(op);
	if( i != 0 )  pushbk_symbol(',');
    }
    
    free_array(ap);
    return 0;
}

/*
 * evaluate argument reference ($num, $#, $*, $@, $[expr])
 */
int
dollar_sign()
{
    int  i;
    Token  *tp;
    Array  *ap;
    Object *op;

    tp = get_token();
    if( tp->type == '$' ) {
    	/* escaped '$' */
	return 1;	/* return '$' as it is */
    } else {
	check_calls();
	switch( tp->type ) {
	case '#':
	    if( trace_level >= 3 ) {
		print_trace("$#", NULL, NULL);
		printf("%d\n", cur_thd->pb->calls->arg_array->size); 
	    }
	    pushbk_int((PmmlInt)cur_thd->pb->calls->arg_array->size);
	    break;
	case '*':
	    if( trace_level >= 3 ) {
		print_trace("$*", NULL, NULL);
		fprint_array(stdout, cur_thd->pb->calls->arg_array, 
			     TRACE_MAXELMS, TRACE_MAXTKLEN);
		printf("\n");
	    }
	    cur_thd->pb->calls->arg_array->ref++;
	    pushbk_array(cur_thd->pb->calls->arg_array);
	    break;
	case '@':
	    ap = cur_thd->pb->calls->arg_array;
	    if( trace_level >= 3 ) {
		print_trace("$@", NULL, NULL);
		for( i = 0; i < ap->size; i++ ) {
		    fprint_object(stdout, &array_ref(ap, i), 
				  TRACE_MAXELMS, TRACE_MAXTKLEN);
		    if( i != ap->size-1 )  printf(", ");
		}
		printf("\n");
	    }
	    for( i = ap->size; --i >= 0; ) {
		pushbk_object(&array_ref(ap, i));
		if( i != 0 )  pushbk_symbol(',');
	    }
	    break;
	case T_NUMBER:
	    if( tp->u.obj.o_type != O_INT || (i = tp->u.obj.o_val) < 1 || 
	       i > cur_thd->pb->calls->maxargs ) {
		error(cur_srcpos, "$%d: Illegal argument reference",
		      "$%d: 不正な引数参照です", i);
	    } else if( i > cur_thd->pb->calls->arg_array->size ) {
		/* ignore it! */
	    } else {
		if( trace_level >= 3 ) {
		    char  s[10];
		    sprintf(s, "$%d", i); 
		    print_trace(s, NULL, &array_ref(cur_thd->pb->calls
						    ->arg_array, i-1));
		}
		pushbk_object(&array_ref(cur_thd->pb->calls->arg_array, i-1));
	    }
	    break;
	case '[':
	    if( !(op = get_expression()) )  parse_error(cur_token);
	    if( !isnumber(op->o_type) || get_token()->type != ']' ) { 
		error(cur_srcpos, "Parse error in array subscript",
		      "配列の添字に間違いがあります");
	    }

	    if( op->o_type != O_INT ) {
		PmmlInt  tmp;
		conv_to_int(op, &tmp);
		i = tmp;
	    } else {
		i = op->o_val;
	    }

	    if( i < 1 || i > cur_thd->pb->calls->maxargs ) {
		error(cur_srcpos, "$[]: Subscript out of range (value = %d)",
		      "$[]: 配列の添字が範囲外です (添字値 = %d)", i);
	    } else if( i > cur_thd->pb->calls->arg_array->size ) {
		/* ignore it! */
	    } else {
		if( trace_level >= 3 ) {
		    char  s[10];
		    sprintf(s, "$[%d]", i); 
		    print_trace(s, NULL, &array_ref(cur_thd->pb->calls
						    ->arg_array, i-1));
		}
		pushbk_object(&array_ref(cur_thd->pb->calls->arg_array, i-1));
	    }
	    break;
	default:
	    parse_error(tp);
	}
    }
	
    return 0;
}

/*
 * def(id [, arg_spec]) { tokens ... } : define a macro
 * edef(id [, arg_spec]) { tokens ... } : define a macro ($-notations and 
 *                                        eval() in tokens are evaluated)
 */
int
do_def()
{
    char  *arg_spec;
    Object  obj;
    DicEnt  *dp;
    MacroCall *mp;
    char  *cmdname;
    int  scan_mode;

    if( cur_token->type == T_DEF ) {
	cmdname = "def";  scan_mode = 0;
    } else {  /* T_EDEF */
	cmdname = "edef"; scan_mode = ARGS_ONLY;
    }

    mp = scan_args("x:s", cmdname);
    if( mp->nargs == 2 ) {
	if( !valid_arg_spec(arg_spec = mp->arg[1].o_str) ) {
	    error(cur_srcpos, "%s: Incorrect argument specifier",
		  "%s: 引数仕様が間違っています", cmdname);
	}
    } else  arg_spec = DEF_ARG_SPEC;
    dp = mp->arg[0].id.dp;

    /* scan the following token list */
    obj.o_type = O_WTOKENS;
    obj.o_tp = scan_tklist(scan_mode, COLLECT, cmdname, '{', '}');

    dp->active--;
    redefine_macro(dp, &obj, arg_spec);
	
    free(mp);
    return 0;
}

/*
 * undef(id) : undefine a macro or delete a thread 
 */
int
do_undef()
{
    DicEnt  *dp;
    MacroCall *mp;

    mp = scan_args("y", "undef");
    dp = mp->arg[0].id.dp;

    dp->active--;
    if( !mp->arg[0].id.defined ) {
	free(dp->name);
	free(dp);
    } else {
	if( dp->type == D_ThreadName && dp->dic_thd == root_thd ) {
	    error(cur_srcpos, "undef: Can not delete the root thread",
		  "undef: ルートスレッドを消去することはできません。");
	}
	unlink_dnext_link(dp);
	delete_dict(dp);
    }

    free(mp);
    return 0;
}

/*
 * eval(expr) : evaluate an expression
 */
int
do_eval()
{
    MacroCall *mp;
    static String  strbuf;

    mp = scan_args("e", "eval");
    switch( mp->arg[0].o_type ) {
    case O_STOKENS:
	pushbk_tklist(mp->arg[0].o_tp, TF_FREE);
	break;
    case O_STRING:
	resetstr(strbuf);
	addstr(strbuf, mp->arg[0].o_str);
	addchar(strbuf, 0);
	pushbk_string(getstr(strbuf));
	free_object(&mp->arg[0]);
	break;
    default:
	pushbk_object(&mp->arg[0]);
	free_object(&mp->arg[0]);
	break;
    }

    free(mp);
    return 0;
}

/*
 * evalstr(string) : evaluate a string as the input stream
 */
int
do_evalstr()
{
    MacroCall *mp;

    mp = scan_args("s", "evalstr");
    pushbk_evalstr(mp->arg[0].o_str);
    free(mp);
    return 0;
}

/*
 * include(filename)
 */
int
do_include()
{
    MacroCall *mp;

    mp = scan_args("s", "include");
    pushbk_file(mp->arg[0].o_str, 1);
    strfree(mp->arg[0].o_str);
    free(mp);
    return 0;
}

/*
 * null(tokens)
 */
int
do_null()
{
    Token  *tklist;

    tklist = scan_tklist(MACRO_CMDS, COLLECT, "null", '(', ')'); 
    pushbk_int(tklist == NULL ? 1L : 0L);
    free_tklist(tklist);

    return 0;
}

/*********************************************************************
 * Routines for conditional branch
 *********************************************************************/
/*
 * if(expr) {tokens} [elsif(expr) {tokens}]* [else {token}]
 */
int
do_if()
{
    int  done = 0;
    Token  *tp;
    MacroCall *mp;

    mp = scan_args("e", "if");
    done = istrue_object(&mp->arg[0]);
    free_object(&mp->arg[0]);
    free(mp);
    tp = scan_tklist(0, !done, "if", '{', '}');

		    /* We cannot expand the next token becuase 
		       "look ahead" tokens can not be expanded. */
    while( probe_next_token()->type == T_ELSIF ) {
	get_token();
	if( done ) {
	    /* skip from '(' to ')' */
	    mp = scan_args("q", "elsif");
	    free_tklist(mp->arg[0].o_tp);
	    free(mp);
	    (void) scan_tklist(0, SKIP, "elsif", '{', '}');
	} else {
	    mp = scan_args("e", "elsif");
	    done = istrue_object(&mp->arg[0]);
	    free_object(&mp->arg[0]);
	    free(mp);
	    tp = scan_tklist(0, !done, "elsif", '{', '}');
	}
    }

    if( probe_next_token()->type == T_ELSE ) {
	get_token();
	if( done ) {
	    (void) scan_tklist(0, SKIP, "else", '{', '}');
	} else {
	    tp = scan_tklist(0, COLLECT, "else", '{', '}');
	    done = 1;
	}
    }

    if( done ) {
	pushbk_tklist(tp, TF_FREE);
    }

    return 0;
}

/*
 * switch(expr) { case(expr,...) {tokens} ... [default {tokens}] }
 */
int
do_switch()
{
    int  done = 0;
    Token  *tp, *tklist;
    MacroCall *mp, *mp1;
    int  i, scanned;

    mp = scan_args("e", "switch");

    if( get_token()->type != '{' ) {
	error(cur_srcpos, "switch: Missing '{'", "switch: '{' がありません");
    }

    while( (tp = get_token())->type != '}' ) {
	switch( tp->type ) {
	case T_CASE:
	    mp1 = scan_args("e*", "case");
	    for( i = 0, scanned = 0; i < mp1->nargs; i++ ) {
		if( !done && compare_object(&mp->arg[0], &mp1->arg[i]) == 0 ) {
		    tklist = scan_tklist(0, COLLECT, "case", '{', '}'); 
		    done = 1;
		    scanned = 1;
		}
		free_object(&mp1->arg[i]);    
	    }
	    if( !scanned )  (void) scan_tklist(0, SKIP, "case", '{', '}');
	    free(mp1);
	    break;

	case T_DEFAULT:
	    if( !done ) {
		tklist = scan_tklist(0, COLLECT, "default", '{', '}'); 
		done = 1;
	    } else  (void) scan_tklist(0, SKIP, "default", '{', '}');
	    break;

	default:
	    parse_error(tp);
	}
    }

    if( done ) {
	pushbk_tklist(tklist, TF_FREE);
    }

    free_object(&mp->arg[0]);
    free(mp);
    return 0;
}

/*********************************************************************
 * Routines for loops
 *********************************************************************/
/*
 * while(expr) {tokens}
 */
int
do_while()
{
    Token  loop_tok, *cond;
    Object  *op;
    int  true;

    /* collect argument token list */
    cond = scan_tklist(0, COLLECT, "while", '(', ')');

    /* evaluate the conditional expresison */
    pushbk_symbol(')');   /* push sentinel in order to avoid `over-running' */
    pushbk_tklist(cond, 0);
    if( !(op = get_expression()) )  parse_error(cur_token);
    true = istrue_object(op);
    free_object(op);
    if( get_token()->type != ')' ) {
	error(cur_srcpos, "while: Bad conditional expression",
	      "while: 条件式が間違っています");
    }	
    
    if( true ) {
	/* loop is continued */
	loop_tok.type = T_EWHILE;
	loop_tok.src_pos = cur_srcpos;
	loop_tok.u.ewhile.body = scan_tklist(0, COLLECT, "while", '{', '}');
	loop_tok.u.ewhile.cond = cond;

	pushbk_token(&loop_tok);
	pushbk_tklist(loop_tok.u.ewhile.body, 0);
	cur_thd->pb->loop_count++;
    } else {
	/* termination of loop */
	scan_tklist(0, SKIP, "while", '{', '}');
	free_tklist(cond);
    }

    return 0;
}

int
end_while(flags)
int  flags;
{
    Token  *cond, *body;
    Object  *op;
    int  true;
    
    cond = cur_token->u.ewhile.cond;
    body = cur_token->u.ewhile.body;

    if( !(flags & BREAK_LOOP) ) {
	/* evaluate the conditional expresison */
	pushbk_token(NULL);	/* keep the loop token */
	pushbk_symbol(')');
	pushbk_tklist(cond, 0);
	if( !(op = get_expression()) )  parse_error(cur_token);
	true = istrue_object(op);
	free_object(op);
	if( get_token()->type != ')' ) {
	    error(cur_srcpos, "while: Bad conditional expression",
		   "while: 条件式が間違っています");
	}	
	
	if( true ) {
	    /* loop is continued */
	    pushbk_tklist(body, 0);
	    return 0;
	} else {
	    /* termination of loop */
	    get_token();	/* dispose loop token */
	}
    }

    /* termination of loop */
    free_tklist(cond);
    free_tklist(body);
    cur_thd->pb->loop_count--;
    return 0;
}

/*
 * for(id,expr,expr) {tokens}
 */
int
do_for()
{
    Token  loop_tok;
    DicEnt  *dp;
    Object  lvar;
    MacroCall *mp;

    mp = scan_args("XII:I", "for");
    lvar.o_type = O_INT;
    lvar.o_val = mp->arg[1].o_val;
    loop_tok.u.efor.limit = mp->arg[2].o_val;
    loop_tok.u.efor.step = mp->nargs == 4 ? mp->arg[3].o_val : 1;
    if( loop_tok.u.efor.step == 0 ) {
	error(cur_srcpos, "for: Zero step value", "for: ステップ値が０です");
    }
    dp = mp->arg[0].id.dp;
    free(mp);

    if( loop_tok.u.efor.step > 0 ? 
       lvar.o_val > loop_tok.u.efor.limit : 
       lvar.o_val < loop_tok.u.efor.limit ) {
	/* termination of loop */
	scan_tklist(0, SKIP, "for", '{', '}');
	dp->active--;

    } else {
	/* loop is continued */
	redefine_macro(dp, &lvar, NULL);
	/* dp->active is remain incremated */

	loop_tok.type = T_EFOR;
	loop_tok.src_pos = cur_srcpos;
	loop_tok.u.efor.body = scan_tklist(0, COLLECT, "for", '{', '}');
	loop_tok.u.efor.loopvar = dp;

	pushbk_token(&loop_tok);
	pushbk_tklist(loop_tok.u.efor.body, 0);
	cur_thd->pb->loop_count++;
    }

    return 0;
}

int
end_for(flags)
int  flags;
{
    DicEnt  *dp = cur_token->u.efor.loopvar;

    if( (flags & BREAK_LOOP) || 
       (dp->dic_obj.o_val += cur_token->u.efor.step,
	cur_token->u.efor.step > 0 ? 
        dp->dic_obj.o_val > cur_token->u.efor.limit :
        dp->dic_obj.o_val < cur_token->u.efor.limit ) ) {

	/* termination of loop */
	free_tklist(cur_token->u.efor.body);
	dp->active--;
	cur_thd->pb->loop_count--;

    } else {
	/* loop is continued */
	pushbk_token(NULL); 	/* keep the loop token */
	pushbk_tklist(cur_token->u.efor.body, 0);
    }

    return 0;
}

/*
 * foreach(id [,array]) {tokens}
 */
int
do_foreach()
{
    Token  loop_tok;
    Array  *ap;
    DicEnt  *dp;
    Object  lvar;
    MacroCall *mp;

    mp = scan_args("X:a", "foreach");
    if( mp->nargs == 2 ) {
	ap = mp->arg[1].o_ap;
    } else {
	check_calls();
	ap = cur_thd->pb->calls->arg_array;
	ap->ref++;
    }
    dp = mp->arg[0].id.dp;
    free(mp);

    if( ap->size == 0 ) {
	/* termination of loop */
	scan_tklist(0, SKIP, "foreach", '{', '}');
	free_array(ap);
	dp->active--;

    } else {
	/* loop is continued */
	copy_object(&lvar, &array_ref(ap, 0));
	redefine_macro(dp, &lvar, NULL);
	/* dp->active++ is remain increased */

	loop_tok.type = T_EFOREACH;
	loop_tok.src_pos = cur_srcpos;
	loop_tok.u.eforeach.body = scan_tklist(0, COLLECT, 
					       "foreach", '{', '}');
	loop_tok.u.eforeach.loopvar = dp;
	loop_tok.u.eforeach.ap = ap;
	loop_tok.u.eforeach.count = 0;

	pushbk_token(&loop_tok);
	pushbk_tklist(loop_tok.u.eforeach.body, 0);
	cur_thd->pb->loop_count++;
    }

    return 0;
}

int
end_foreach(flags)
int  flags;
{
    Object  *op;
    DicEnt  *dp = cur_token->u.eforeach.loopvar;

    if( (flags & BREAK_LOOP) ||
       ++cur_token->u.eforeach.count >= cur_token->u.eforeach.ap->size ) {

	/* termination of loop */
	free_tklist(cur_token->u.eforeach.body);
	free_array(cur_token->u.eforeach.ap);
	dp->active--;
	cur_thd->pb->loop_count--;

    } else {
	/* loop is continued */
	op = & dp->dic_obj;
	free_object(op);
	copy_object(op, &array_ref(cur_token->u.eforeach.ap, 
				   cur_token->u.eforeach.count));
	pushbk_token(NULL);	/* keep the loop token */
	pushbk_tklist(cur_token->u.eforeach.body, 0);
    }

    return 0;
}

/*
 * repeat(expr) {tokens}
 */
int
do_repeat()
{
    int  times;
    Token  loop_tok;
    MacroCall *mp;

    mp = scan_args("i", "repeat");
    times = mp->arg[0].o_val;
    free(mp);

    if( times <= 0 ) {
	/* termination of loop */
	scan_tklist(0, SKIP, "repeat", '{', '}');
    } else {
	/* loop is continued */
	loop_tok.type = T_EREPEAT;
	loop_tok.src_pos = cur_srcpos;
	loop_tok.u.erepeat.count = times;
	loop_tok.u.erepeat.body = scan_tklist(0, COLLECT, "repeat", '{', '}');

	pushbk_token(&loop_tok);
	pushbk_tklist(loop_tok.u.erepeat.body, 0);
	cur_thd->pb->loop_count++;
    }

    return 0;
}

int
end_repeat(flags)
int  flags;
{
    if( (flags & BREAK_LOOP) || --cur_token->u.erepeat.count == 0 ) {
	/* termination of loop */
	free_tklist(cur_token->u.erepeat.body);
	cur_thd->pb->loop_count--;
    } else {
	/* loop is continued */
	pushbk_token(NULL);	/* keep the loop token */
	pushbk_tklist(cur_token->u.erepeat.body, 0);
    }

    return 0;
}

/*
 * break[(expr)] : loop termination
 */
int
do_break()
{
    int  level = 1;
    Token  *tp;
    MacroCall *mp;

    if( probe_next_token()->type == '(' ) {
	mp = scan_args("i", "break");
	level = mp->arg[0].o_val;
	free(mp);
    }

    if( level > cur_thd->pb->loop_count ) {
	error(cur_srcpos, "break: No target loops",
	     "break: 対象となるループがありません");
    }

    while( --level >= 0 ) {
	/* skip to the next EOL token */
	do {
	    tp = get_token();
	    if( tp->type == T_EOF ) {
		if( do_eof() == 1 ) {
		    error(cur_srcpos, "break: Internal error (Unexpected EOF)",
			  "break: 内部エラー (Unexpected EOF)");
		}
	    }
	} while( !(tkinfo[tp->type].flags & F_EOL) );
	(*tkinfo[tp->type].handler)(BREAK_LOOP);
    }

    return 0;
}

/*********************************************************************
 * Routines for array operation
 *********************************************************************/
/*
 * append(array, elm, elm, ...) : append elements after the end of array
 */
int
do_append()
{
    int  i;
    Array  *ap;
    MacroCall *mp;

    mp = scan_args("aee*", "append");

    ap = mp->arg[0].o_ap;
    for(i = 1; i < mp->nargs; i++ ) {
	append_array(ap, &mp->arg[i]);
    }

    free_array(ap);
    free(mp);
    return 0;
}

/*
 * insert(array, elm, elm, ...) : insert elements before the top of array
 */
int
do_insert()
{
    int  i;
    Array  *ap;
    MacroCall *mp;

    mp = scan_args("aee*", "insert");

    ap = mp->arg[0].o_ap;
    for(i = mp->nargs; --i >= 1; ) {
	insert_array(ap, &mp->arg[i]);
    }

    free_array(ap);
    free(mp);
    return 0;
}

/*
 * shift[([array [, count]])] : reduce array size
 */
int
do_shift()
{
    int  cnt;
    Array  *ap;
    MacroCall *mp;

    if( probe_next_token()->type == '(' ) {
	mp = scan_args("a:i", "shift");
	cnt = (mp->nargs == 2) ? mp->arg[1].o_val : 1;
	ap = mp->arg[0].o_ap;
	free(mp);
    } else {
	check_calls();
	ap = cur_thd->pb->calls->arg_array;
	ap->ref++;
	cnt = 1;
    }
    shift_array(ap, cnt);
    free_array(ap);
    return 0;
}

/*********************************************************************
 * Error handlers etc.
 *********************************************************************/
/*
 * error unless there is at least one macro call in argument referencing
 */
static void
err_calls()
{
    error(cur_srcpos, "Argument reference on the top level",
	  "トップレベルでの引数参照はできません");
}

/*
 * print currently expanding macros
 */
void
fprint_calls(fp)
FILE  *fp;
{
    MacroCall *mp;
    Thread  *th;
    int  first = 1;

    for(th = cur_thd; th != NULL; th = th->parent) {
	for( mp = th->pb->calls; mp != NULL; mp = mp->next ) {
	    if( first ) {
		fprintf(fp, japan ? "\n*** マクロの呼び出し過程 ***\n" :
			"\n*** Macro Calling History ***\n");
		first = 0;
	    }
	    fprint_fname(fp, mp->src_pos);
	    fprintf(fp, ": %s(", mp->macro->name);
	    fprint_args(fp, mp, err_maxargs);
	    fprintf(fp, ")\n"); 
	}
	if( !(th->flags & EFLAG) )  break;
    }
}

/*
 * print the argument of macros
 */
void
fprint_args(fp, mp, maxargs)
FILE  *fp;
MacroCall  *mp;
int  maxargs;
{
    int  i;

    for( i = 0; i < mp->nargs; i++ ) {
	if( maxargs >= 0 && i >= maxargs ) {
	    fprintf(fp, "...");
	    break;
	}
	if( i >= mp->arg_array->offset && 
	   i < mp->arg_array->offset + mp->arg_array->size ) {
	    fprint_object(fp, &mp->arg_array->elms[i], 
			  err_maxelms, err_maxtklen);
	} else {
	    /* the object is already free'ed by `shift($*)' */
	    fprintf(fp, "???");
	}
	if( i != mp->nargs - 1 )  fprintf(fp, ", ");
    }
}

/*
 * print trace messages
 */
static void
print_trace(name, mp, op)
char  *name;
MacroCall  *mp;
Object  *op;
{
    static long prev_srcpos = 0;
    static String  prev_macname;
    char  *macname;

    macname = cur_thd->pb->calls ? cur_thd->pb->calls->macro->name : "";
    if( cur_srcpos != prev_srcpos ||
       !prev_macname.buf || strcmp(macname, prev_macname.buf) != 0 ) {
	printf("TRACE: [");
	if( *macname )  printf("in `%s' ", macname);
	printf("at ");
	fprint_fname(stdout, cur_srcpos);
	printf("]\n");
    }
    prev_srcpos = cur_srcpos;
    resetstr(prev_macname);
    addstr(prev_macname, macname);
    addchar(prev_macname, 0);
    
    printf("TRACE: %s", name);
    if( mp ) {
	printf("(");
	fprint_args(stdout, mp, TRACE_MAXARGS);
	printf(")");
    }
    printf("\nTRACE:\t\t=> ");
    if( op ) {
	fprint_object(stdout, op, TRACE_MAXELMS, TRACE_MAXTKLEN); 
	printf("\n");
    }
}

/*********************************************************************
 * Self tester
 *********************************************************************/
#ifdef MACRO_SELF_TEST
/* To do self test, compile macro.c with input.c, error.c, token.c, util.c, 
   array.c, rational.c, table.c, parseut.c, expr.c, dict.c, builtin.c 
   and keyword.c. 
   Use the '-Xlinker -noinhibit-exec' option of the gcc compiler. */
int  japan = 0;
int  trace_level = 0;
Thread  *cur_thd, *root_thd;

Token *
parse_cmds(mode)
int  mode;
{
    Token  *tp;

    while( tkinfo[(tp = get_token())->type].flags & mode ) {
	if( (*tkinfo[tp->type].handler)(0) ) {
	    break;
	}
    }
    return tp;
}

main()
{
    init_input();
    init_expression();
    root_thd = cur_thd = (Thread *) malloc(sizeof(Thread));
    init_pbstack(cur_thd);
    init_builtin();
    pushbk_file("-", 0);

    parse_cmds(MACRO_CMDS);
}
#endif
