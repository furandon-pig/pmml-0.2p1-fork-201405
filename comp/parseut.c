/*
 * PMML: A Compiler for the Practical Music Macro Language
 *
 * parseut.c: parsing utilities
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

static Token *	scan_q_arg P((Token **));

/*
 * scan macro arguments and create MacroCall structure
 * 
 *   The return value is ptr to newly-created MacroCall structure.
 *   Only 'nargs' and 'arg' elements are set.
 *
 *   All the argument objects should be free'ed on 
 *   the caller's responsiblity.
 */
MacroCall *
scan_args(arg_spec, cmd_name)
char  *arg_spec;	/* argument specification string */
char  *cmd_name;	/* command name (for error message) */
{
    Token  *tp;
    int   nargs = 0;
    int   maxargs = 0;
    MacroCall *mp;
    int   nalloc;
    Object  *op;
    DicEnt  *dp;
    long  src_pos1;
    int	  opts = 0;
    int   prev = 0;
    int   c, goodarg;

    if( arg_spec == DEF_ARG_SPEC )  arg_spec = "q*";

    if( !(mp = (MacroCall *) malloc(sizeof(MacroCall)) ) ) {
	err_nomem("scan_args");
    }
    nalloc = ARG_INITSIZ;

    src_pos1 = cur_srcpos;
    if( get_token()->type != '(' ) {
	error(src_pos1, "%s: '(' expected", "%s: '(' がありません", cmd_name);
    }

    for(;;) {
	if( ! *arg_spec ) {
	    if( get_token()->type != ')' ) {
		error(cur_srcpos, "%s: Too many arguments", 
		      "%s: 引数が多すぎます", cmd_name);
	    } else {
		break;
	    }
	}
	
	c = prev && *arg_spec == '*' ? prev : *arg_spec++;
	if( c == ':' ) {
	    opts = 1;
	    continue;
	}
	
	if( nargs >= nalloc ) {
	    nalloc += ARG_GROWSIZ;
	    if( !(mp = (MacroCall *) 
		  realloc(mp, MAC_BASE_SIZE + nalloc * sizeof(Object))) ) {
		err_nomem("scan_args");
	    }
	}
	
	prev = c;
	if( c == 'q' ) {
	    mp->arg[nargs].o_type = O_WTOKENS;
	    mp->arg[nargs].o_tp = scan_q_arg(&tp);
	    
	    if( !mp->arg[nargs].o_tp && tp->type == ')' ) {
		/* in case of "func()" or "func(... ,)" */
		arg_spec--;
		break;
	    }

	} else if( (c | 0x21) == 'y' ) {
	    /* 
	     * 'x': identifier (register it to the dict even if undefined)
	     * 'X': same as 'x' but "local::" is assumed (for loop vars) 
	     * 'y': identifer (if undefined, not registered to the dict)
	     *  These are only used ineternally
	     */
	    tp = parse_cmds(ARGS_ONLY);
	    if( tp->type == ')' ) {
		arg_spec--;
		break;
	    }
	    
	    if( (mp->arg[nargs].id.defined = 
		 get_id(c == 'X' ? SD_LOCAL : c == 'x' ? SD_MINE : SD_ANY, 
			c != 'y', &dp)) == -1 ) {
		error(cur_srcpos, "%s: Inappropriate identifier", 
		      "%s: 名前が不適切です", cmd_name);
	    }
	    mp->arg[nargs].id.dp = dp;
	    tp = get_token();
	    if( tp->type == T_DCOLON ) {
		error(cur_srcpos, "%s: Bad scope specifier",
		      "%s: 通用範囲指定が間違っています", cmd_name);
	    }
	    
	} else if( c == 'l' ) {
	    /* user version of 'X' */  
	    tp = parse_cmds(ARGS_ONLY);
	    if( tp->type == ')' ) {
		arg_spec--;
		break;
	    }
	    if( get_id(SD_LOCAL, DEF_ON_UNDEF, &dp) == -1 ) { 
		error(cur_srcpos, "%s: Inappropriate identifier in arguments", 
		      "%s: 引数中の名前が不適切です", cmd_name);
	    }
	    mp->arg[nargs].o_type = O_WTOKENS;
	    (mp->arg[nargs].o_tp = new_token(T_LOCALID, cur_srcpos))
		->u.dp = dp;

	    tp = get_token();
	    if( tp->type == T_DCOLON ) {
		error(cur_srcpos, "%s: Bad scope specifier",
		      "%s: 通用範囲指定が間違っています", cmd_name);
	    }
	    
	} else {
	    if( !(op = get_expression()) ) {
		if( cur_token->type == ')' ) {
		    arg_spec--;
		    break;
		} else {
		    parse_error(cur_token);
		}
	    }
	    goodarg = 1;
	    switch( op->o_type ) {
	    case O_INT:
		if( c == 'f' ) {
		    op->fpval = op->o_val;
		} else if( c == 'r' ) {
		    int_to_rational(op->o_val, &op->r);
		} else {
		    /* 'I' means an integer w/o automatic type conversion */
		    goodarg = ( c == 'i' || c == 'I' ||
			       c == 'n' || c == 'p' || c == 'e' );
		}
		break;
	    case O_RATIONAL:
		if( c == 'i' ) {
		    op->o_val = rational_to_int(&op->r);
		    op->o_type = O_INT;
		} else if( c == 'f' ) {
		    op->fpval = rational_to_float(&op->r);
		} else {
		    goodarg = ( c == 'r' || c == 'n' || c == 'p' || c == 'e' );
		}
		break;
	    case O_STRING:
		goodarg = ( c == 's' || c == 'p' || c == 'e' );
		break;
	    case O_ARRAY:
		goodarg = ( c == 'a' || c == 'e' );
		break;
	    case O_STOKENS:
		goodarg = ( c == 't' || c == 'e' );
		break;
	    default:  /* float */
		if( c == 'i' ) {
		    op->o_val = (PmmlInt) floor(op->fpval + .5);
		    op->o_type = O_INT;
		} else if( c == 'r' ) {
		    float_to_rational(op->fpval, &op->r);
		} else {
		    goodarg = ( c == 'f' || c == 'n' || c == 'p' || c == 'e' );
		}
		break;
	    }
	    if( !goodarg ) {
		error(cur_srcpos, "%s: Mismatched argument type",
		      "%s: 引数の型が違います", cmd_name);
	    }
	    tp = get_token();
	    mp->arg[nargs] = *op;
	}
	
	nargs++;
	
	if( tp->type == ')' ) {
	    break;
	} else if( tp->type == T_EOF ) {
	    error(src_pos1, "%s: Closing ')' is not found",
		  "%s: '(' に対応する ')' がありません", cmd_name);
	} else if( tp->type != ',' ) {
	    error(cur_srcpos, "%s: Parse error in arguments",
		  "%s: 引数に間違いがあります", cmd_name);
	}
    }

    if( !opts && isalpha(*arg_spec) && arg_spec[1] != '*' ) {
	error(cur_srcpos, "%s: Too few arguments", 
	      "%s: 引数が足りません", cmd_name);
    }
    
    for( maxargs = nargs; *arg_spec; arg_spec++ ) {
	if( *arg_spec == '*' ) {
	    maxargs = 0x7fff; 
	    break;
	} else if( *arg_spec != ':' )  maxargs++;
    }
	
    mp->nargs = nargs;
    mp->maxargs = maxargs;
    return mp;
}

/*
 * check if an argument specification string is valid
 */
int
valid_arg_spec(str)
char  *str;
{
 state1:
    switch(*str++) {
    case 0:
	return 1;
    case 'q':
    case 'n':
    case 'i':
    case 'f':
    case 'r':
    case 's':
    case 'p':
    case 'a':
    case 't':
    case 'e':
    case 'l':
	goto state2;
    case ':':
	goto state1;
    default:
	return 0;
    }

 state2:
    switch(*str++) {
    case 0:
	return 1;
    case 'q':
    case 'n':
    case 'i':
    case 'f':
    case 'r':
    case 's':
    case 'p':
    case 'a':
    case 't':
    case 'e':
    case 'l':
	goto state2;
    case ':':
	goto state1;
    case '*':
	return( *str == 0 );
    default:
	return 0;
    }
}

/*
 * scan "q"-type argument
 */
static Token *
scan_q_arg(lasttp)
Token  **lasttp;
{
    Token  *tp, **nextp, *tklist;
    DicEnt  *dp;
    int  parcnt = 0;
    int  state = 0;	/* 0 - previous token is neither 'local' nor '::' 
			   1 - previous token is '::'
			   2 - previous token is 'local'
			   3 - previous two tokens are 'local ::'  */ 
    enum { Collect, Ignore, Finish }  action;

    nextp = &tklist;

    do {
	tp = parse_cmds(ARGS_ONLY);

	if( (state == 2 && tp->type != T_LOCAL && tp->type != T_DCOLON) ) {
	    /* output pending 'local' */
	    *nextp = new_token(T_LOCAL, cur_srcpos);
	    nextp = &(*nextp)->next;
	} else if( state == 3 && tp->type != T_LOCAL && tp->type != T_ID ) {
	    /* output pending 'local ::' */
	    *nextp = new_token(T_LOCAL, cur_srcpos);
	    nextp = &(*nextp)->next;
	    *nextp = new_token(T_DCOLON, cur_srcpos);
	    nextp = &(*nextp)->next;
	}

	switch( tp->type ) {
	case T_EOF:
	    action = Finish;
	    break;

	case ',':
	    if( !parcnt )  action = Finish;
	    else {
		state = 0;
		action = Collect;
	    }
	    break;

	case ')':
	    if( parcnt-- == 0 )  action = Finish;
	    else {
		state = 0;
		action = Collect;
	    }
	    break;

	case '(':
	    parcnt++;
	    state = 0;
	    action = Collect;
	    break;

	case T_LOCAL:
	    if( state == 2 ) {
		action = Collect;
	    } else {
		state = 2;
		action = Ignore;
	    } 
	    break;

	case T_DCOLON:
	    if( state == 2 ) {
		state = 3;  
		action = Ignore;
	    } else {
		state = 1;
		action = Collect;
	    }
	    break;

	case T_ID:
	    switch(state) {
	    case 1:
	    case 2:
		state = 1;
		action = Collect;
		break;
	    case 0:
		if( (dp = search_dict(SD_LOCAL, tp->u.id.name, tp->u.id.hash, 
				      NULL, cur_thd->pb->calls)) ) {
		    dp->active++;
		    (*nextp = new_token(T_LOCALID, cur_srcpos))->u.dp = dp;
		    nextp = &(*nextp)->next;
		    state = 1;
		    action = Ignore;
		} else {
		    state = 1;
		    action = Collect;
		}
		break;
	    case 3:
		if( !cur_thd->pb->calls ) {
		    warn(cur_srcpos, "'local::' will be ignored",
			 "'local::' は無視されます");
		    state = 1;
		    action = Collect;
		} else {
		    if( ! (dp = search_dict(SD_LOCAL, 
					    tp->u.id.name, tp->u.id.hash, 
					    NULL, cur_thd->pb->calls)) ) {
			/* The ID is not defined -- define it now */
			if( !(dp = (DicEnt *) malloc(sizeof(DicEnt))) ||
			   !(dp->name = strdup(tp->u.id.name)) ) {
			    err_nomem("scan_q_arg");
			}
			dp->type = D_LocalMacro;
			dp->scope.m = cur_thd->pb->calls;
			dp->hash = tp->u.id.hash;
			dp->active = 0;
			dp->dic_obj.o_type = O_INT;
			dp->dic_arg_spec = NULL;
			insert_dict(dp);
		    }
		    dp->active++;
		    (*nextp = new_token(T_LOCALID, cur_srcpos))->u.dp = dp;
		    nextp = &(*nextp)->next;
		    state = 1;
		    action = Ignore;
		}
		break;
	    }
	    break;

	default:
	    state = 0;
	    action = Collect;
	    break;
	}

	if( action == Collect ) {
	    *nextp = copy_token(tp);
	    nextp = &(*nextp)->next;
	}
	if( tp->type == T_ARRAY ) { 
	    free_array(tp->u.obj.o_ap);
	}

    } while( action != Finish );

    *nextp = NULL;
    *lasttp = tp;
    return tklist;
}

/*
 * scan from left-parenthesis to right-parenthesis
 */
Token *
scan_tklist(mode, skip_flag, cmd_name, begin_char, end_char)
int  mode;		/* scanning mode */ 
int  skip_flag;		/* If ture, token list is not collected. */
char  *cmd_name;	/* command name (for error message) */
int  begin_char;	/* openning parenthesis charactor '{', '[', etc.
			   negative value for no initial parenthesis */
int  end_char;		/* closing parenthesis charactor '}', ']', etc. */
{
    Token  *tp;
    Token  *tklist, **nextp;
    long  src_pos1;
    int   paren_cnt;

    src_pos1 = cur_srcpos;
    if( begin_char < 0 ) { 
	begin_char = - begin_char;
    } else {
	if( get_token()->type != begin_char ) {
	    error(src_pos1, "%s: Missing '%c'", "%s: '%c' がありません", 
		  cmd_name, begin_char);
	}
    }

    nextp = &tklist;
    for(paren_cnt = 0;;) {
	tp = (mode == 0) ? get_token() : parse_cmds(mode);
	if( tp->type == end_char ) {
	    if( paren_cnt-- == 0 )  break;
	} else if( tp->type == begin_char ) {
	    paren_cnt++;
	} else if( tp->type == T_EOF ) {
	    error(src_pos1, "%s: Closing '%c' is not found",
		  "%s: 閉じ括弧 '%c' がありません", cmd_name, end_char);
	}
	if( !skip_flag ) {
	    *nextp = copy_token(tp);
	    nextp = &(*nextp)->next;
	}
	if( tp->type == T_ARRAY ) {
	    free_array(tp->u.obj.o_ap);
	}
    }
    *nextp = NULL;

    return tklist;
}

/*
 * get an identifer optionally preceded by scope specifiers
 *   If the ID is a defined name, get_id sets *dp_rtn to the pointer 
 *   to the DicEnt structure in the dictionary, and returns non-null; more 
 *   precisely, returns 1 if there are no scope specifiers or 2 otherwise.
 *   If the ID is an undefined name, create a new entry in the dictionary,
 *   sets *dp_rtn to the pointer to the new strucutre, and returns 0. 
 *   The type of the new entry becomes D_LocalMacro or D_ThreadMacro 
 *   depending on the scope specifier, although the D_ThreadMacro type 
 *   can be later changed to D_EffClass or D_EffInst.
 *   The definition value of the new macro entry is an integer whose value
 *   is undefined.
 * 
 *   returns -1 on error
 */ 
int
get_id(search_mode, def_on_undef, dp_rtn)
int  search_mode;    /* initial search mode (SD_ANY, SD_MINE or SD_LOCAL) */
int  def_on_undef;   /* insert undefined name to dictionary or not */
DicEnt  **dp_rtn;	
{
    Thread  *scope = cur_thd;
    DicEnt  *dp;
    Token   *tp;
    MacroCall *cur_mc = cur_thd->pb->calls;
    int     state = 0;	/* 1: `local::' is specified.
			   3: `some_thread::' is specified. */

    if( cur_token->type == T_LOCALID ) {
	cur_token->u.dp->active++;
	*dp_rtn = cur_token->u.dp;
	return 2;
    }

    if( search_mode == SD_LOCAL && !cur_mc ) { 
	search_mode = SD_MINE;
    }

    for( tp = cur_token; ; tp = parse_cmds(ARGS_ONLY) ) {
	switch( tp->type ) {
	case T_ID:
	    if( (dp = search_dict(search_mode, tp->u.id.name, 
				  tp->u.id.hash, scope, cur_mc)) ) {
		/* 
		 * The ID is already defined.
		 */
		if( dp->type == D_ThreadName
		   && probe_next_token()->type == T_DCOLON ) {

		    get_token();	/* skip '::' */
		    if( state == 1 ) {
			error(cur_srcpos, "Illegal domain specifier",
			      "通用範囲指定が間違っています");
		    }
		    search_mode = SD_THREAD;
		    scope = dp->dic_thd;
		    state = 3;

		} else {
		    dp->active++;
		    *dp_rtn = dp;
		    return( state == 0 ? 1 : 2 );
		}

	    } else {
		/*
		 * The ID is not defined yet.
		 */
		if( !(dp = (DicEnt *) malloc(sizeof(DicEnt))) ||
		    !(dp->name = strdup(tp->u.id.name)) ) {
		    err_nomem("get_id");
		}
		if( search_mode == SD_LOCAL ) {
		    dp->type = D_LocalMacro;
		    dp->scope.m = cur_mc;
		} else {
		    dp->type = D_ThreadMacro;
		    dp->scope.c = scope;
		}
		dp->hash = tp->u.id.hash;
		dp->active = 1;
		dp->dic_obj.o_type = O_INT;
		dp->dic_arg_spec = NULL;

		if( def_on_undef ) {
		    insert_dict(dp);
		}

		*dp_rtn = dp;
		return 0;
	    }
	    break;
	    
	case T_LOCAL:
	    if( state != 0 ) {
		error(cur_srcpos, "Illegal domain specifier",
		      "通用範囲指定が間違っています");
	    } else if( !cur_mc ) {
		warn(cur_srcpos, "'local::' will be ignored",
		     "'local::' は無視されます");
	    } else {
		search_mode = SD_LOCAL;
		state = 1;
	    }
	    goto skip_dcolon;

	case T_DCOLON:
	    search_mode = SD_THREAD;
	    scope = root_thd;
	    state = 3;
	    break;

	case '.':
	    if( state == 1 ) { 
		warn(cur_srcpos, "'.::' will be ignored",
		     "'.::' は無視されます");
	    } else if( state != 3 ) {
		search_mode = SD_THREAD;
		scope = cur_thd;
		state = 3;
	    }
	    goto skip_dcolon;
	    
	case T_DDOT:
	    if( state == 1 ) { 
		warn(cur_srcpos, "'..::' will be ignored",
		     "'..::' は無視されます");
	    } else {
		if( state != 3 ) {
		    search_mode = SD_THREAD;
		    scope = cur_thd;
		    state = 3;
		}
		if( scope->parent )  scope = scope->parent;
	    }
	    goto skip_dcolon;
	    
	case T_TDOT:
	    if( state == 1 ) { 
		warn(cur_srcpos, "'...::' will be ignored",
		     "'...::' は無視されます");
	    } else {
		if( state != 3 ) {
		    search_mode = SD_THREAD;
		    scope = cur_thd;
		    state = 3;
		}
		while( scope->parent && 
		      ( !scope->dp || scope->dp->type != D_ThreadName ) ) {
		    scope = scope->parent;
		}
	    }
	    goto skip_dcolon;
	    
	default:
	    return -1;

	skip_dcolon:
	    if( get_token()->type != T_DCOLON ) {
		error(cur_srcpos, "Illegal domain specifier",
		       "通用範囲指定が間違っています");
	    }
	    break;
	}
    }
    /* not reached */
}
