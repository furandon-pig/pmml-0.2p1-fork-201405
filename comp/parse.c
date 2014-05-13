/*
 * PMML: A Compiler for the Practical Music Macro Language
 *
 * parse.c: command parser
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
 * initial value for thread
 */
static Thread  init_thd = { 
    {
	RINIT(0, 0, LDEN),	/* t = 0 */
	RINIT(0, 0, LDEN),	/* tb = 0 */
	RINIT(0, 0, LDEN),	/* dt = 0 */
	RINIT(0, LDEN/32, LDEN),/* sh = 1z = 1/32 */
	RINIT(0, LDEN/4,LDEN),	/* l  = 1q = 1/4 */
	RINIT(0, 0, LDEN),	/* do = 0 */
	2,  		/* tk = 2 */
    	1,  		/* ch = 1 */
	REST_NOTE, 	/* n = REST */
	0,		/* tp = 0 */
	4,		/* o = 4 */
	0,		/* key = 0 */
	80,		/* v = 80 (mezzo forte) */
	-1,		/* nv = -1 */
	16,		/* ac = 16 */
	100,		/* dp = 100 */
    },
    NULL,		/* effs = NULL */
    1L,			/* eebv = 1 */
    /* other members need not to be initialized */
};

/*
 * global variables
 */
Thread  *cur_thd;		/* current thread:
				   This is actually the top element of
				   ready thread list */
Thread  *root_thd;		/* root thread */

static Channel  *all_chnls = NULL;	/* sequential list of comm channels */
static int  total_blocked = 0; 		/* number of threads being blocked 
					   by wait() or alt */

int  default_eflags = 0;   /* effector flags OR'ed to all instance flags */ 

/* 
 * prototypes
 */
static int	cleanup_idle_threads P((Thread *));
static int	parse_modifier P((int, int *, Rational *));
static Channel *get_channel_name P((char *, char *, MacroCall **));
static void	check_dead_lock P((void));
static void	check_unrecv_msgs P((Channel *));
static void	parse_event_types P((EventSet, char *));
static void	parse_numlist P((EventSet, int, int, char *));
static void	attach_effector P((DicEnt *, MacroCall *, char *, int, int));
static void	def_se_macros P((EffInst *));
static void	enable_effector P((int, EffInst *));
static int	eval_action P((Token *, EffInst *, Event *));
static void	per_event_action P((Event *, EffInst *));
static void	reoutput_ctrl P((Event *, EffInst *));
static void	reoutput_excl P((Event *, EffInst *));
static void	reoutput_meta P((Event *, EffInst *, int));
static void	detach_all_effectors P((Thread *));
static void	detach_effector P((EffInst *));
static int	print_numlist P((EventSet, int, int, char *, char *));
static void	print_event_types P((EventSet));
static Thread  *create_thread P((Thread *, int));
static 		err_trk P((int));
static 		err_chan P((int));
static		err_funcno P((int));
static		warn_note P((int));
static void	err_undef P((DicEnt *));

/*
 * macros
 */
#ifdef DEBUG_THREAD 
#define sleep_cur_thd()  (printf("sleep thread %08x\n", cur_thd), \
			  cur_thd = cur_thd->rtl_next)
#define awake_thread(th) (printf("awake thread %08x\n", th), \
			  (th)->rtl_next = cur_thd, cur_thd = (th))
#else
#define sleep_cur_thd()  (cur_thd = cur_thd->rtl_next)
#define awake_thread(th) ((th)->rtl_next = cur_thd, cur_thd = (th))
#endif

/* move the current thread to the tail of RTL */
#define rotate_thread() { \
    Thread *th; \
    for( th = cur_thd; th->rtl_next != NULL; th = th->rtl_next ); \
    th->rtl_next = cur_thd; \
    cur_thd = cur_thd->rtl_next; \
    th->rtl_next->rtl_next = NULL; \
}

#define ret_to_parent() { \
    Thread  *th = cur_thd; \
    sleep_cur_thd(); \
    awake_thread(th->parent); \
    delete_thread(th, 0); \
}

#define lock_calls(cls) { \
    MacroCall  *mp; \
    for( mp = (cls); mp; mp = mp->next )  mp->ref++; \
}

#define unlock_calls(cls) { \
    MacroCall  *mp, *mp_next; \
    for( mp = (cls); mp; mp = mp_next ) { \
	mp_next = mp->next; \
	delete_mc(mp); \
    } \
}

#define check_time(t) \
    { if( (t).intg < 0 ) { warn_time(&(t)); (t) = r_zero; } }

#define  conv_track(tk) \
    ( ((tk) - 1) & ~(MAXTRACK - 1) ? err_trk(tk) : (tk) - 1 )

#define  conv_chan(ch) \
    ( ((ch) - 1) & ~15 ? err_chan(ch) : (ch) - 1 )

#define  conv_funcno(fn) \
    ( (fn) & ~255 ? err_funcno(fn) : (fn) )

#ifdef ADD_TP_IN_CONST
#  define  set_note(dest, n) \
    { register int  ntmp = (n); \
      if( ntmp < 0 ) { warn_note(ntmp); (dest) = 0; } \
      else if( ntmp >= 128 ) { warn_note(ntmp); (dest) = 127; } \
      else (dest) = ntmp; \
    }
#else
#  define  set_note(dest, n) \
    { register int  ntmp = (n) + cur_thd->reg.tp; \
      if( ntmp < 0 ) { warn_note(ntmp); (dest) = 0; } \
      else if( ntmp >= 128 ) { warn_note(ntmp); (dest) = 127; } \
      else (dest) = ntmp; \
    }
#endif

/*********************************************************************
 * Initialization & Ending
 *********************************************************************/
void
init_parse()
{
    DicEnt  *dp;

    root_thd = create_thread(&init_thd, 0);
    root_thd->parent = NULL;
    root_thd->rtl_next = NULL;
    cur_thd = root_thd;

    /* register the "global" thread to the dictionary */
    if( !(dp = (DicEnt *) malloc(sizeof(DicEnt))) ) {
	err_nomem("init_parse");
    }
    if( !(dp->name = strdup("global")) ) {
	err_nomem("init_parse");
    }
    dp->hash = hash_func(dp->name);
    dp->scope.c = root_thd;
    dp->active = 0;
    dp->dic_thd = root_thd;
    dp->type = D_BuiltIn; /* a trick to avoid connecting to the dnext link */
    insert_dict(dp);
    dp->type = D_ThreadName;
    root_thd->dp = dp;
}

/*
 * ending routine for the parser
 */
void
end_parse()
{
    Channel  *cp;

    /* Be aware that the root thread is already dead */

    /* check unreceived messages */
    for( cp = all_chnls; cp; cp = cp->next ) {
	check_unrecv_msgs(cp);
    }
}

/*********************************************************************
 * Parsing routines
 *********************************************************************/
/*
 * parse initilization block ('%{' .. '}')
 */
void
parse_initblk()
{
    Token  *tp, *tklist;
    
    if( (tp = get_token())->type != T_INITBLK ) {
	if( tp->type != T_EOF ) {
	    tp = copy_token(tp);
	    tp->next = NULL;
	    pushbk_tklist(tp, TF_FREE);
	}
    } else {
	tklist = scan_tklist(0, COLLECT, "%{", -'{', '}');
	pushbk_symbol(T_EOINIT);
	pushbk_tklist(tklist, TF_FREE);
	if( parse_cmds(ALL_CMDS)->type != T_EOINIT ) {
	    parse_error(cur_token);
	}
    }
}

/* 
 * parser main 
 */
void
parse_main()
{
    Token  *tp;

    tp = parse_cmds(ALL_CMDS);
    if( tp ) {
	parse_error(tp);
    }
}

/*
 * end of stream (called when push-back stack is exhausted)
 */
int
do_eof()
{
    Thread  *th = cur_thd;
    Rational  *tmp_t;

    sleep_cur_thd();	/* sleep current thread and pick up the next one */
    if( th->dp == NULL ) {
	/* in case of unnamed thread --- delete it */
	if( th->flags & SFLAG ) {
	    /* update parent thread's time information and awake him */
	    tmp_t = th->flags & PFLAG ? &th->maxtime : &th->reg.t;
	    if( !(th->parent->flags & PFLAG) ) {
		th->parent->reg.t = *tmp_t;
	    } else {
		if( rgreater(tmp_t, &th->parent->maxtime) )  
		    th->parent->maxtime = *tmp_t;
	    }
	    awake_thread(th->parent);
	}
	delete_thread(th, 0);
    }

    if( cur_thd == NULL ) {
	/* The current state is either end of parsing or deadlocked */

	/* Since detaching effectors may awake some blocked threads, we must 
	   detach remainning effectors before calling check_dead_lock */
	cleanup_idle_threads(root_thd);
	
	if( cur_thd == NULL ) {
	    /* No threads are awoken by detaching effectors */

	    check_dead_lock();	/* This may awake threads blocked by alt */ 

	    if( cur_thd == NULL ) {
		/* Now, everything is successfuly completed. */
		return 2;	/* make parse_cmd() return NULL */
	    }
	}
    }
    return 0;
}

/*
 * delete named threads sleeping due to token exhaution
 * in order to detach remainning effectors
 *
 *   Returns 1 if thread "hp" is actually deleted; otherwise 0 is returned. 
 */
static int
cleanup_idle_threads(hp)
Thread  *hp;
{
    DicEnt  *dp, **dpp;

    /* first apply cleanup_idle_threads to all the child named threads */
    for( dpp = &hp->macros; (dp = *dpp) != NULL; ) {
	if( dp->type == D_ThreadName ) {
	    if( cleanup_idle_threads(dp->dic_thd) ) {
		*dpp = dp->dnext;
		dp->type = D_BuiltIn; /* a trick to avoid double deletion */
		delete_dict(dp);
		continue;
	    }
	}
	dpp = &dp->dnext;
    }

    /* If the thread has no child, and if it is sleeping 
       due to token exhaution (namely `idle'), delete the thread */
    if( hp->nchild == 0 && hp->pb->top->type == T_EOF ) {
	delete_thread(hp, 1);
	return 1;
    }
    return 0;
}

/*
 * parse a sequence of commands   
 * 
 *   The `mode' parameter specifies the set of commands to be parsed.
 *      ALL_CMDS    - all type of commands
 *      MACRO_CMDS  - macro directives, $-notations, eval(), and evalstr() only
 *      ARGS_ONLY   - $-notations, eval() and evalstr() only
 *   In all cases, strong token lists are expanded.
 *   Returns the next token just after the parsed commands (which is eqaul 
 *   to cur_token).
 */
Token *
parse_cmds(mode)
int  mode;
{
    Token  *tp;
    int  ret;

    while( tkinfo[(tp = get_token())->type].flags & mode ) {
	if( (ret = (*tkinfo[tp->type].handler)(0)) != 0 ) {
	    if( ret == 2 )  tp = NULL;  /* at the very end */
	    break;
	}
    }
    return tp;
}

/*
 * defthread(th, ...) : create-new-threads command
 */
int
do_defthread()
{
    int  i;
    DicEnt  *dp;
    MacroCall *mp;
    Thread  *hp;

    mp = scan_args("y*", "defthread");
    for( i = 0; i < mp->nargs; i++ ) {
	dp = mp->arg[i].id.dp;
	dp->active--;
	if( mp->arg[i].id.defined ) { /* already defined */
	    if( mp->arg[i].id.defined == 1  /* w/o scope specifier */
	       && dp->type != D_LocalMacro && dp->scope.c != cur_thd ) {
		DicEnt  *dp1;
		
		/* the identifier is not mine! */
		if( !(dp1 = (DicEnt *) malloc(sizeof(DicEnt))) ||
		   !(dp1->name = strdup(dp->name)) ) {
		    err_nomem("do_defthread");
		}
		dp1->scope.c = cur_thd;
		dp1->hash = dp->hash;
		dp1->active = 0;
		dp = dp1;
	    } else {
		error(cur_srcpos, "%s: already defined",
		      "%s: 既に定義済みです", dp->name);
	    }
	} else if( dp->type == D_LocalMacro ) {
	    error(cur_srcpos, "%s: Can not define thread as a local name",
		  "%s: スレッドをローカル名としては定義できません", dp->name);
	}
	dp->type = D_ThreadName;
	hp = create_thread(dp->scope.c, 0);
	hp->dp = dp;
	dp->dic_thd = hp;

	/* Register the DicEnt structure to the dictionary.
	   If the parent is an unnamed thread, register the DicEnt to 
	   its most nearst ancestor that is named; otherwise, 
	   the recursive deletion of threads (delete_dict() in "dict.c")
	   does not work. */
	while( dp->scope.c->dp == NULL ) {
	    dp->scope.c = dp->scope.c->parent;
	}
	insert_dict(dp);
    }

    free(mp);
    return 0;
}

/*
 * parse note generation (pitch) command
 */
int
do_pitch()
{
    int  tmp_v;
    Rational  tmp_dt;
    Rational  tmp_t;
    int  thcreated;
    Event  *e_on, *e_off;

    if( cur_token->type != T_NOTE ) {
	cur_thd->reg.n = pitch_to_val(cur_token);
#ifdef SET_OCTAVE_BY_NOTE
	/* change the o-register value if the note is accompanied 
	   with an octave number */
	if( (cur_token->u.pitch.flags & (ABS_OCT | RESTFLAG)) == ABS_OCT ) {
	    cur_thd->reg.o = cur_token->u.pitch.octave;
	}
#endif
    }

    thcreated = parse_modifier(0, &tmp_v, &tmp_dt);

#ifdef DEBUG_NOTE
    {   Rational  tmp;
	radd(&cur_thd->reg.t, &tmp_dt, &tmp);
	printf("Note: t=%s ch=%d n=%d v=%d do=%s dp=%d nv=%d\n", 
	       rstring(&tmp), cur_thd->reg.ch, cur_thd->reg.n, tmp_v, 
	       rstring(&cur_thd->reg.dofs), cur_thd->reg.dp, cur_thd->reg.nv);
    }
#endif
    
    /* insert note-on & note-off events */
    if( cur_thd->reg.n != REST_NOTE ) {
	Rational  dur;

	event_alloc(e_on);
	e_on->type = E_NoteOn;
	radd(&cur_thd->reg.t, &tmp_dt, &e_on->time);
	check_time(e_on->time);
	e_on->track = conv_track(cur_thd->reg.tk);
	e_on->ch = conv_chan(cur_thd->reg.ch);
	set_note(e_on->note, cur_thd->reg.n);
	e_on->ev_veloc = tmp_v;
	e_on->next = NULL;
	
	rset(cur_thd->reg.dp, 100, &dur);
	rmult(&cur_thd->reg.l, &dur, &dur);
	radd(&cur_thd->reg.dofs, &dur, &dur);
	if( dur.intg < 0 )  dur = r_zero;

	event_alloc(e_off);
	radd(&e_on->time, &dur, &e_off->time);
	check_time(e_off->time);
	e_off->type = E_NoteOff;
	e_off->flags = rzerop(&dur) ? IrregularNoteOff : 0;
	e_off->track = e_on->track;
	e_off->ch = e_on->ch;
	e_off->note = e_on->note;
	e_off->ev_veloc = cur_thd->reg.nv;
	e_off->ev_partner = e_on;
	e_on->ev_partner = e_off;
	e_off->next = NULL;
	output_event(e_on);
	output_event(e_off);
    }
	       
    /* increase time */
    radd(&cur_thd->reg.t, &cur_thd->reg.l, &tmp_t);

    /* return to the parent thread */
    if( thcreated )  ret_to_parent();

    if( probe_next_token()->type == '&' ) {
	get_token();
    } else {
	if( !(cur_thd->flags & PFLAG) ) {
	    cur_thd->reg.t = tmp_t;
	} else { 
	    if( rgreater(&tmp_t, &cur_thd->maxtime) ) 
		cur_thd->maxtime = tmp_t;
	}
    }

    return 0;
}

/*
 * parse modifiers (accent, time-shift, and '(commands)')
 */
static int
parse_modifier(thcreated, v_ptr, dt_ptr)
int  thcreated;
int   *v_ptr;
Rational  *dt_ptr;
{
    long  src_pos1;
    Token  *tp;
    int  tmp_v;
    Rational  tmp_dt;
    int  type;
    Thread  *thd1;

    tmp_v = cur_thd->reg.v;
    tmp_dt = cur_thd->reg.dt;

    for(;;) {
	type = probe_next_token()->type;

	if( type == T_ACCENT ) {
	    tp = get_token();
	    if( thcreated ) {
		cur_thd->reg.v += floor(rational_to_float(&tp->u.obj.r) 
					* (1.0 / (IRES * 4.0))
					* cur_thd->reg.ac + .5);
	    } else {
		tmp_v += floor(rational_to_float(&tp->u.obj.r)
			       * (1.0 / (IRES * 4.0))
			       * cur_thd->reg.ac + .5);
	    }

	} else if( type == T_TSHIFT ) {
	    Rational  tmp;

	    tp = get_token();
	    rmult(&tp->u.obj.r, &cur_thd->reg.sh, &tmp);
	    if( thcreated ) {
		radd(&cur_thd->reg.dt, &tmp, &cur_thd->reg.dt);
	    } else {
		radd(&tmp_dt, &tmp, &tmp_dt);
	    }

	} else if( type == '(' ) {
	    get_token();
	    if( !thcreated ) {
		Thread  *th;
		th = create_thread(cur_thd, MFLAG); 
		th->reg.v  = tmp_v;
		th->reg.dt = tmp_dt;
		sleep_cur_thd();
		awake_thread(th);
		thcreated = 1;
	    }
	    cur_thd->reg.tb = cur_thd->reg.t;

	    src_pos1 = cur_srcpos;
	    thd1 = cur_thd;
	    tp = parse_cmds(ALL_CMDS);  /* parse_cmds may return NULL (This
					   type of care is needed only if
					   it is called with ALL_CMDS) */
	    if( !tp || tp->type == T_EOF ) {
		error(src_pos1, "'(': Closing ')' is not found",
		      "'(' に対応する ')' がありません");
	    } else if( tp->type != ')' ) {
		parse_error(tp);
	    }
	    
	    cur_thd->reg.t = cur_thd->reg.tb;

	} else {
	    break;
	}
    }

    if( thcreated ) {
	if( v_ptr )  *v_ptr = cur_thd->reg.v;
	if( dt_ptr ) *dt_ptr = cur_thd->reg.dt;
    } else {
	if( v_ptr )  *v_ptr = tmp_v;
	if( dt_ptr ) *dt_ptr = tmp_dt;
    }

    return thcreated;
}

/*
 * 'THREAD_NAME {' : switch-thread command
 */
int
do_thread()
{
    DicEnt  *dp;
    Token  *tklist;
    Token  bot_tok;

    dp = cur_token->u.dp;
    bot_tok.src_pos = cur_srcpos;
    tklist = scan_tklist(0, COLLECT, dp->name, '{', '}');
    if( probe_next_token()->type == '+' ) {
	get_token();
	bot_tok.u.bot.avoid_sync = CFLAG;
    } else {
	bot_tok.u.bot.avoid_sync = 0;
    }
    dp->active--;

    bot_tok.type = T_BOT;
    bot_tok.u.bot.stime = cur_thd->reg.t;
    bot_tok.u.bot.calls = cur_thd->pb->calls;

    /* increase the reference counters for MacroCall structures
       -- The counter values will be resumed in end_of_named_thread() */
    lock_calls(cur_thd->pb->calls);

    /* if the specified thread is sleeping due to token exhaution, awake it */ 
    if( dp->dic_thd->pb->top->type == T_EOF ) {
	awake_thread(dp->dic_thd);
    }

    push_tklist_at_bottom(tklist, TF_FREE, dp->dic_thd, &bot_tok);

    return 0;
}

int
begin_of_named_thread()
{
    if( !(cur_thd->flags & CFLAG) ) {
	if( rgreater(&cur_token->u.bot.stime, &cur_thd->reg.t) ) {
	    cur_thd->reg.t = cur_token->u.bot.stime;
	}
    }

    cur_thd->pb->calls = cur_token->u.bot.calls;
    cur_thd->flags = (cur_thd->flags & ~CFLAG) | cur_token->u.bot.avoid_sync; 

    return 0;
}

int
end_of_named_thread()
{
    Thread  *th;

    unlock_calls(cur_thd->pb->calls);
    cur_thd->pb->calls = NULL;

    if( !(cur_thd->flags & CFLAG) ) {
	/* update maxtime for implementing '===' */
	for( th = cur_thd; th != NULL; th = th->parent ) {
	    if( rless(&th->maxtime, &cur_thd->reg.t) ) {
		th->maxtime = cur_thd->reg.t;
	    }
	}
    }
    return 0;
}

/*
 * '{' : do-in-a-new-thread command
 */
int
do_lbrace()
{
    Thread  *new;
    Token  *tklist;
    PBinfo  *pb_save;

    tklist = scan_tklist(0, COLLECT, "{", -'{', '}');

    /* create a new thread */
    new = create_thread(cur_thd, 0);

    /* To make it possible to refer the local macros of parent thread,
       copy calls and increase reference counters of MacroCall */
    new->pb->calls = cur_thd->pb->calls;
    lock_calls(new->pb->calls);

    /* temporarily use parent's push-back stack */
    pb_save = new->pb;
    new->pb = cur_thd->pb;

    /* process modifiers */
    sleep_cur_thd();
    awake_thread(new);
    parse_modifier(1, NULL, NULL);

    /* parse '&' */
    if( probe_next_token()->type == '&' ) {
	get_token();
    } else {
	new->flags |= SFLAG;

	/* allow to perform 'break' against parent thread's loop */
	pb_save->loop_count = new->pb->loop_count;
    }

    /* resume its own push-back stack */    
    new->pb = pb_save;

    /* awake parent thread if there is '&' option */
    if( !(new->flags & SFLAG) ) {
	/* insert new->parent (i.e. the old thread) at the second pos in RTL */
	sleep_cur_thd();
	awake_thread(new->parent);
	awake_thread(new);
    }

    pushbk_symbol(T_EOB);   /* for displaying a correct error message when 
			       parse_error is called near the end of block */
    pushbk_tklist(tklist, TF_FREE);

    return 0;
}

int
do_rbrace()
{
    error(cur_srcpos, "'}': No corresponding '{'",
	  "'}' に対応する '{' がありません");
}

int
end_of_block()
{
    /* nothing to do */
    return 0;
}

/*
 * [ ... ] : parse chord block 
 */
int
do_lbracket()
{
    Thread  *new;
    Token  *tklist;
    PBinfo  *pb_save;

    tklist = scan_tklist(0, COLLECT, "[", -'[', ']');

    /* create a new thread */
    new = create_thread(cur_thd, PFLAG);

    /* see do_lbrace() */
    new->pb->calls = cur_thd->pb->calls;
    lock_calls(new->pb->calls);

    /* temporarily use parent's push-back stack */
    pb_save = new->pb;
    new->pb = cur_thd->pb;

    /* process modifiers */
    sleep_cur_thd();
    awake_thread(new);
    parse_modifier(1, NULL, NULL);

    /* parse '&' */
    if( probe_next_token()->type == '&' ) {
	get_token();
    } else {
	new->flags |= SFLAG;

	/* allow to perform 'break' against parent thread's loop */
	pb_save->loop_count = new->pb->loop_count;
    }

    /* resume its own push-back stack */    
    new->pb = pb_save;

    /* awake parent thread if there is '&' option */
    if( !(new->flags & SFLAG) ) {
	/* insert new->parent (i.e. the old thread) at the second pos in RTL */
	sleep_cur_thd();
	awake_thread(new->parent);
	awake_thread(new);
    }

    pushbk_symbol(T_EOP);   /* for displaying a correct error message when 
			       parse_error is called near the end of block */
    pushbk_tklist(tklist, TF_FREE);

    return 0;
}

int
do_rbracket()
{
    error(cur_srcpos, "']': No corresponding '['",
	  "']' に対応する '[' がありません");
}

/*
 * REGISTER = val, REGISTER op= val : change-register command
 */
int
do_reg()
{
    int	 type, regno, assign_type;
    Object  *op;
    Token  err_token;

    type = cur_token->type;
    regno = tkinfo[type].regno;

    assign_type = get_token()->type;

    if( assign_type != '=' && !(tkinfo[assign_type].flags & F_CA) ) {
	err_token.type = type;
	terror(cur_srcpos, &err_token, 
	       "Isolated register", "レジスタが孤立しています");
    }

    if( assign_type == '=' ) {
	op = get_expression();
    } else {
	Object  tmp;
	if( type == T_RT ) {
	    rsub(&cur_thd->reg.t, &cur_thd->reg.tb, &tmp.r);
	    op = _get_expression(assign_type, &tmp);
	} else if( type == T_DU ) {
	    rset(cur_thd->reg.dp, 100, &tmp.r);
	    rmult(&cur_thd->reg.l, &tmp.r, &tmp.r);
	    radd(&cur_thd->reg.dofs, &tmp.r, &tmp.r);
	    op = _get_expression(assign_type, &tmp);
	} else {
	    if( IsIntReg(regno) ) {
		tmp.o_type = O_INT;
		tmp.o_val = ((ThreadRegs *)cur_thd)->ireg[regno - NRREG];
		op = _get_expression(assign_type, &tmp);
	    } else {
		op = _get_expression(assign_type, (Object *)
				     &((ThreadRegs *)cur_thd)->rreg[regno]);
	    }
	}
    }
    if( !op )  parse_error(cur_token);
    if( !isnumber(op->o_type) ) {
	err_token.type = type;
	terror(cur_srcpos, &err_token, "Illegel type for register value",
	       "レジスタ値の型が違います");
    }

    if( IsIntReg(regno) ) {
	conv_to_int(op, &((ThreadRegs *)cur_thd)->ireg[regno - NRREG]);
    } else {
	Rational  *r = &((ThreadRegs *)cur_thd)->rreg[regno];
	conv_to_rational(op, r);
    }
    
    switch( type ) {
    case T_RT:
	radd(&cur_thd->reg.t, &cur_thd->reg.tb, &cur_thd->reg.t); 
	break;
    case T_DU:
	cur_thd->reg.dp = 0;
	break;
    case T_DR:
	cur_thd->reg.dofs = r_zero;
	break;
    }

    return 0;
}

/*
 * RATIONAL_NUMBER : set-length command
 */
int
do_length()
{
    Object  *op;

    pushbk_object(&cur_token->u.obj);
    if( !(op = get_expression()) )  parse_error(cur_token);
    if( !isnumber(op->o_type) ) {
	error(cur_srcpos, "Bad length specifier", "音価指定が間違っています");
    }
    conv_to_rational(op, &cur_thd->reg.l);
    return 0;
}

/*
 * '^', '_' : increase(descrease)-octave command
 */
int
do_octave()
{
    cur_thd->reg.o += cur_token->u.oct;
    return 0;
}

/*
 * '++', '--' : accent command
 */
int
do_accent()
{
    cur_thd->reg.v += floor(rational_to_float(&cur_token->u.obj.r)
			    * (1.0 / (IRES * 4.0))
			    * cur_thd->reg.ac + .5);
    return 0;
}

/*
 * '>>', '<<' : time-shift command
 */
int
do_tshift()
{
    Rational  tmp;

    rmult(&cur_token->u.obj.r, &cur_thd->reg.sh, &tmp);
    radd(&cur_thd->reg.dt, &tmp, &cur_thd->reg.dt);
    return 0;
}

/*
 * ';' : no-operation
 */
int
do_semicolon()
{
    /* ignore it */
    return 0;
}

/*
 * '===' : wait until all the decendant threads go to sleep
 */
int
do_sync()
{
    Thread  *thd, *thd1;  
    int  has_ready_descendant;

    if( cur_thd->flags & (EFLAG | MFLAG) ) {
	error(cur_srcpos, 
	      "===: Unavailable in a modifier or effector action",   
	      "===: 修飾子またはエフェクタアクションの中では実行できません");
    }

    /* Are there any child threads in the RTL? */
    has_ready_descendant = 0;
    for( thd = cur_thd->rtl_next; thd != NULL; thd = thd->rtl_next ) {
	for( thd1 = thd->parent; thd1 != NULL; thd1 = thd1->parent ){ 
	    if( thd1 == cur_thd ) { 
		has_ready_descendant = 1; 
	    }
	}
	if( has_ready_descendant )  break;
    }

    if( has_ready_descendant ) {
	/* Then, suspend the execution of the current thread */

	/* push ESYNC token */
	pushbk_symbol(T_ESYNC);
	
	/* move the current thread to the tail of RTL */
	rotate_thread();
    } else { 
	/* Otherwise, set the maximum time among the descendants to t,
	   and continue */
	if( rgreater(&cur_thd->maxtime, &cur_thd->reg.t) ) {
	    cur_thd->reg.t = cur_thd->maxtime;
	}
    }

    return 0;
}

int
end_sync()
{
    do_sync();

    return 0;
}

/*
 * signal(channel_name [, msg]) : send a message to channel
 */
int
do_signal()
{
    Channel  *chnl;
    Message  *msg, **mpp;
    WaitStatus  *wp;
    MacroCall  *mp;

    chnl = get_channel_name("signal", "y:e", &mp);
    
    /* create a new Message element and insert it to the time-orderd list */
    if( !(msg = (Message *) malloc(sizeof(Message))) ) {
	err_nomem("do_signal");
    }
    msg->time = cur_thd->reg.t;
    msg->src_pos = cur_srcpos;
    msg->received = 0;
    if( mp->nargs == 2 ) {
	msg->message = mp->arg[1];	/* structure copy */
    } else {
	msg->message.o_type = O_NIL;
    }
    for( mpp = &chnl->mtop; *mpp; mpp = &(*mpp)->next ) {
	if( rgreater(&(*mpp)->time, &msg->time) ) {
	    break;
	}
    }
    msg->next = *mpp;
    *mpp = msg;
    free(mp);

    /* wake up threads that are waiting for this message */
    for( wp = chnl->wtop; wp; wp = wp->next ) {
	if( wp->status == WS_WaitBlocked || wp->status == WS_AltBlocked ) {
	    awake_thread(wp->sleep_thd);
	    rotate_thread();
	    total_blocked--;
	    if( wp->status == WS_WaitBlocked ) {
		wp->status = WS_Executing;
	    } else { /* WS_AltBlocked */
		AltGuard  *gp;
		/* change the status of all the guards so that the threads
		   not to be doubly woke up.  Gp is on the top of PBstack */ 
		for( gp = wp->sleep_thd->pb->top->t.u.ealt.guards; 
		    gp != NULL; gp = gp->next ) {
		    gp->wp->status = WS_Executing;
		}
	    }
	}
    }

    return 0;
}

/*
 * get channel name and if it's a new one, create a new channel 
 */
static Channel *
get_channel_name(cmdname, arg_spec, mp_rtn)
char  *cmdname;
char  *arg_spec;
MacroCall  **mp_rtn;
{
    MacroCall  *mp;
    DicEnt  *dp;
    Channel  *chnl;

    mp = scan_args(arg_spec, cmdname);
    dp = mp->arg[0].id.dp;
    dp->active--;
    if( mp->arg[0].id.defined ) { /* already defined */
	if( dp->type != D_Channel ) {
	    error(cur_srcpos, "%s: Not a communication channel name",
		  "%s: 通信チャネル名ではありません", dp->name);
	}
	chnl = dp->dic_chnl;
    } else { /* not defined */
	if( dp->type == D_LocalMacro || 
	   (dp->scope.c != root_thd && dp->scope.c != cur_thd ) ) {
	    error(cur_srcpos, "%s: Bad scope specifier",
		  "%s: 通用範囲指定が間違っています", dp->name);
	}
	dp->type = D_Channel;
	dp->scope.c = root_thd;		/* Channel name is always global. */
	insert_dict(dp);
	
	/* make new Channel structure */
	if( !(chnl = (Channel *) malloc(sizeof(Channel))) ) {
	    err_nomem("get_channel_name");
	}
	chnl->dp = dp;
	chnl->mtop = NULL;
	chnl->wtop = NULL;
	dp->dic_chnl = chnl;
	chnl->next = all_chnls;
	all_chnls = chnl;
    }

    if( mp_rtn )  *mp_rtn = mp;
    else  free(mp);

    return chnl;
}

/*
 * destroy a channel
 */
void
destroy_channel(chnl)
Channel  *chnl;
{
    Message  *msg, *msgnext;
    WaitStatus  *wp, *wpnext;
    Channel  **cpp;
    
    /* check waiting threads */
    for( wp = chnl->wtop; wp; wp = wp->next ) {
	if( wp->status == WS_WaitBlocked || wp->status == WS_AltBlocked ) {
	    error(cur_srcpos,
		  "%s: Some threads are doing wait/alt for this communication channel",
		  "%s: この通信チャネルに対して wait または alt を実行しているスレッドがあります", chnl->dp->name);
	} else if( wp->status == WS_Executing ) {
	    error(cur_srcpos,
		  "%s: Some wait/alt have not finished receiving messages",
		  "%s: メッセージの受け取りをまだ終えていない wait または alt があります", chnl->dp->name);
	}
    }
   
    /* check unreceived messages */
    check_unrecv_msgs(chnl);

    /* free memory used by the wtop list */
    for( wp = chnl->wtop; wp; wp = wpnext ) {
	wpnext = wp->next;
	free(wp);
    }
    /* free memory used by the mtop list */
    for( msg = chnl->mtop; msg; msg = msgnext ) {
	free_object(&msg->message);
	msgnext = msg->next;
	free(msg);
    }

    /* unlink 'all_chnls' link */
    for( cpp = &all_chnls; *cpp != NULL; cpp = &(*cpp)->next ) {
	if( *cpp == chnl ) {
	    *cpp = chnl->next;
	    break;
	}
    }

    free(chnl);
}

/*
 * check dead lock
 */
static void
check_dead_lock()
{
    Channel  *cp;
    WaitStatus  *wp;

    if( total_blocked == 0 )  return; 

    for( cp = all_chnls; cp != NULL; cp = cp->next ) {
	for( wp = cp->wtop; wp; wp = wp->next ) {
	    if( wp->status == WS_WaitBlocked ) {
		error(wp->src_pos, "wait: Permanently blocked",
		      "wait: 永久待ち状態になりました");
	    } else if( wp->status == WS_AltBlocked ) {
		/* force to terminate alt loop */
		warn(wp->src_pos, 
		     "alt: Automatically terminated due to permanent blocking", 
		     "alt: 永久待ち状態のため自動的に終了します");
		awake_thread(wp->sleep_thd);
		total_blocked--;
		get_token();  /* get EALT token */
		end_alt(BREAK_LOOP);
	    }
	}
    }
}

/*
 * check unreceived messages
 */
static void
check_unrecv_msgs(chnl)
Channel  *chnl;
{
    Message  *msg;

    for( msg = chnl->mtop; msg; msg = msg->next ) {
	if( !msg->received ) {
	    warn(msg->src_pos, "signal: Message is never received",
		 "signal: メッセージが一度も受け取られていません");
	    break;
	}
    }
}

/*
 * wait(channel_name [, id]) : wait and receive a message
 */
int
do_wait()
{
    Thread  *wait_thd;
    Channel  *chnl;
    WaitStatus  *wp;
    Message  *nextmsg;
    Token  ewait;
    MacroCall  *mp;
    DicEnt  *dp;

    if( cur_thd->flags & (EFLAG | MFLAG) ) {
	error(cur_srcpos, 
	      "wait: Unavailable in a modifier or effector action",   
	      "wait: 修飾子またはエフェクタアクションの中では実行できません");
    }

    chnl = get_channel_name("wait", "y:x", &mp);
    dp = (mp->nargs == 2) ? mp->arg[1].id.dp : NULL;
    free(mp);
	    
    /* If wait() is executed in { } or [ ], the thread which is considered 
       as the subject of history search is changed to the most nearest 
       ancestor thread which is named or invoked with tailing ampersand. */
    wait_thd = cur_thd;
    while( wait_thd->dp == NULL && (wait_thd->flags & SFLAG) ) {
	wait_thd = wait_thd->parent;
    }

    wait_thd->flags |= WAITEX;

    /* Has wait_thd ever executed wait() or alt for this channel? */ 
    for( wp = chnl->wtop; wp; wp = wp->next ) {
	if( wp->wait_thd == wait_thd ) {
	    nextmsg = wp->lastmsg->next;
	    goto found;
	}
    }
    /* If no, make a new WaitStatus */
    if( !(wp = (WaitStatus *) malloc(sizeof(WaitStatus))) ) {
	err_nomem("do_wait");
    }
    wp->wait_thd = wait_thd;
    wp->status = WS_Idle;
    wp->lastmsg = NULL;
    wp->next = chnl->wtop;
    chnl->wtop = wp;
    nextmsg = chnl->mtop;
    
 found:
    ewait.type = T_EWAIT;
    ewait.src_pos = cur_srcpos;
    ewait.u.ewait.chnl = chnl;
    ewait.u.ewait.wp = wp;
    ewait.u.ewait.dp = dp;
    pushbk_token(&ewait);

    if( !nextmsg ) {
	/* corresponding message is not sent yet -- sleep current thread */
	wp->status = WS_WaitBlocked;
	wp->sleep_thd = cur_thd;
	wp->src_pos = cur_srcpos; 
	total_blocked++;
	sleep_cur_thd();
	if( cur_thd == NULL ) {
	    check_dead_lock();	/* this must not be returned */
	}
    } else {
	/* corresponsing message is already sent */
	wp->status = WS_Executing;
	wp->src_pos = cur_srcpos; 
	rotate_thread();
    }
    return 0;
}

int
end_wait()
{
    Message  *nextmsg;
    Object  obj;

    /* The value of nextmsg may be different from that in do_wait() 
       because other threads may insert a new Message element during
       between do_wait and end_wait() */
    nextmsg = cur_token->u.ewait.wp->lastmsg ?
	cur_token->u.ewait.wp->lastmsg->next : cur_token->u.ewait.chnl->mtop;
    if( rgreater(&nextmsg->time, &cur_thd->reg.t) ) {
	cur_thd->reg.t = nextmsg->time;
    }
    nextmsg->received = 1;
    cur_token->u.ewait.wp->lastmsg = nextmsg;
    cur_token->u.ewait.wp->status = WS_Idle;

    /* place message contents */
    if( cur_token->u.ewait.dp ) {
	cur_token->u.ewait.dp->active--;
	if( nextmsg->message.o_type == O_NIL ) {
	    error(cur_token->u.ewait.wp->src_pos,
		  "wait: Too many arguments", "wait: 引数が多すぎます");
	}
	copy_object(&obj, &nextmsg->message);
	redefine_macro(cur_token->u.ewait.dp, &obj, NULL); 
    }

    return 0;
}

/*
 * alt { case(channel_name) { ... } ... [default { ... }] }
 *       : wait for a message over multiple channels
 */
int
do_alt()
{
    Token  ealt, *tp;
    AltGuard  *gp, **gnextp;
    Thread  *wait_thd;
    WaitStatus  *wp;
    int  ready;
    long  src_pos1;
    MacroCall  *mp;

    if( cur_thd->flags & (EFLAG | MFLAG) ) {
	error(cur_srcpos, 
	      "alt: Unavailable in a modifier or effector action",   
	      "alt: 修飾子またはエフェクタアクションの中では実行できません");
    }

    if( get_token()->type != '{' ) {
	error(cur_srcpos, "alt: Missing '{'", "alt: '{' がありません");
    }

    /* If alt{} is executed in { } or [ ], the thread which is considered 
       as the subject of history search is changed to the most nearest 
       ancestor thread which is named or invoked with tailing ampersand. */
    wait_thd = cur_thd;
    while( wait_thd->dp == NULL && (wait_thd->flags & SFLAG) ) {
	wait_thd = wait_thd->parent;
    }

    wait_thd->flags |= WAITEX;

    ealt.type = T_EALT;
    ealt.src_pos = cur_srcpos;
    gnextp = &ealt.u.ealt.guards;
    ready = 0;

    while( (tp = get_token())->type != '}' ) {
	switch( tp->type ) {
	case T_CASE:
	    if( !(gp = (AltGuard *) malloc(sizeof(AltGuard))) ) {
		err_nomem("do_alt");
	    }
	    gp->chnl = get_channel_name("case", "y:x", &mp);
	    gp->dp = (mp->nargs == 2) ? mp->arg[1].id.dp : NULL;
	    free(mp);
	    src_pos1 = cur_srcpos;
	    gp->body = scan_tklist(0, COLLECT, "case", '{', '}');
	    *gnextp = gp;
	    gnextp = &gp->next;

	    /* Has wait_thd ever executed wait() or alt for this channel? */ 
	    for( wp = gp->chnl->wtop; wp; wp = wp->next ) {
		if( wp->wait_thd == wait_thd ) {
		    if( wp->lastmsg->next )  ready = 1;
		    goto found;
		}
	    }
	    /* If no, make a new WaitStatus */
	    if( !(wp = (WaitStatus *) malloc(sizeof(WaitStatus))) ) {
		err_nomem("do_alt");
	    }
	    wp->wait_thd = wait_thd;
	    wp->status = WS_Idle;
	    wp->lastmsg = NULL;
	    wp->next = gp->chnl->wtop;
	    gp->chnl->wtop = wp;
	    if( gp->chnl->mtop )  ready = 1;
	found:
	    wp->src_pos = src_pos1;
	    gp->wp = wp;
	    break;

	case T_DEFAULT:
	    if( !(gp = (AltGuard *) malloc(sizeof(AltGuard))) ) {
		err_nomem("do_alt");
	    }
	    gp->chnl = NULL;
	    gp->dp = NULL;
	    src_pos1 = cur_srcpos;
	    gp->body = scan_tklist(0, COLLECT, "default", '{', '}');
	    *gnextp = gp;
	    gnextp = &gp->next;

	    /* make a dummy WaitStatus */
	    if( !(gp->wp = (WaitStatus *) malloc(sizeof(WaitStatus))) ) {
		err_nomem("do_alt");
	    }
	    gp->wp->src_pos = src_pos1;
	    ready = 1;
	    break;

	default:
	    parse_error(tp);
	}
    }
    *gnextp = NULL;

    pushbk_token(&ealt);
    cur_thd->pb->loop_count++;

    if( !ready ) {
	/* no ready guards -- sleep current thread */
	for( gp = ealt.u.ealt.guards; gp != NULL; gp = gp->next ) { 
	    gp->wp->status = WS_AltBlocked;
	    gp->wp->sleep_thd = cur_thd;
	}
	total_blocked++;
	sleep_cur_thd();
	if( cur_thd == NULL ) {
	    check_dead_lock();	/* this must terminate alt */
	}
    } else {
	/* corresponsing message is already sent */
	for( gp = ealt.u.ealt.guards; gp != NULL; gp = gp->next ) { 
	    gp->wp->status = WS_Executing;
	}
	rotate_thread();
    }

    return 0;
}

int
end_alt(flags)
int  flags;
{
    Rational  min_time;
    AltGuard  *min_guard, *default_guard, *gp, *gpnext;
    Message  *nextmsg, *min_nextmsg;
    Object  obj;

    if( !(flags & BREAK_LOOP) ) {
	/* keep EALT token */
	pushbk_token(NULL);

	/* find the earliest guard among ready guards */
	min_time = r_max;
	min_guard = NULL;
	default_guard = NULL;
	for( gp = cur_token->u.ealt.guards; gp != NULL; gp = gp->next ) { 
	    if( !gp->chnl ) {  /* default guard? */
		default_guard = gp;
	    } else {
		nextmsg = gp->wp->lastmsg ? 
		    gp->wp->lastmsg->next : gp->chnl->mtop;
		if( nextmsg && rless(&nextmsg->time, &min_time) ) {
		    min_time = nextmsg->time;
		    min_guard = gp;
		    min_nextmsg = nextmsg;
		}
	    }
	}

	if( min_guard && 
	   (!default_guard || rlesseq(&min_time, &cur_thd->reg.t)) ) {  
	    /* channel guard is selected */
	    /* update t register just like wait() */
	    if( rgreater(&min_time, &cur_thd->reg.t) ) {
		cur_thd->reg.t = min_time;
	    }

	    min_nextmsg->received = 1;
	    min_guard->wp->lastmsg = min_nextmsg;

	    /* place message contents */
	    if( min_guard->dp ) {
		if( min_nextmsg->message.o_type == O_NIL ) {
		    error(min_guard->wp->src_pos, "case: Too many arguments", 
			  "case: 引数が多すぎます");
		}
		copy_object(&obj, &min_nextmsg->message);
		redefine_macro(min_guard->dp, &obj, NULL);
	    }
	    
	    pushbk_tklist(min_guard->body, 0);
	    rotate_thread();

	} else if( default_guard ) {  
	    /* default guard is selected */
	    pushbk_tklist(default_guard->body, 0);
	    rotate_thread();

	} else {  /* No ready guards --- sleep current thread */
	    for( gp = cur_token->u.ealt.guards; gp != NULL; gp = gp->next ) { 
		gp->wp->status = WS_AltBlocked;
		gp->wp->sleep_thd = cur_thd;
	    }
	    total_blocked++;
	    sleep_cur_thd();
	    if( cur_thd == NULL ) {
		check_dead_lock();	/* this must terminate alt */
	    }
	}

    } else {
	/* 'break' is executed inside the alt loop 
	   or alt is forced to terminated at the end of program */
	for( gp = cur_token->u.ealt.guards; gp != NULL; gp = gpnext ) { 
	    gp->wp->status = WS_Idle;
	    free_tklist(gp->body);
	    if( gp->dp )  gp->dp->active--;
	    if( gp->chnl == NULL ) { /* default guard */
		free(gp->wp);
	    }
	    gpnext = gp->next;
	    free(gp);
	}
	cur_thd->pb->loop_count--;
    }

    return 0;
}

/*
 * note_on(note_num [, velocity [,ch]]) : generate note-on event 
 */
int
do_note_on()
{
    MacroCall  *mp;
    int  vel, chan;
    Event  *ep;
    int  thcreated;
    int  tmp_v;
    Rational  tmp_dt;
   
    mp = scan_args("i:ii", "note_on");
    thcreated = parse_modifier(0, &tmp_v, &tmp_dt);

    vel = mp->nargs >= 2 ? mp->arg[1].o_val : tmp_v;
    chan = mp->nargs == 3 ? mp->arg[2].o_val : cur_thd->reg.ch;

    if( mp->arg[0].o_val != REST_NOTE ) {
	event_alloc(ep);
	ep->type = E_NoteOn;
	radd(&cur_thd->reg.t, &tmp_dt, &ep->time);
	check_time(ep->time);
	ep->track = conv_track(cur_thd->reg.tk);
	ep->ch = conv_chan(chan);
	set_note(ep->note, mp->arg[0].o_val);
	ep->ev_veloc = vel;
	ep->ev_partner = NULL;
	ep->next = NULL;
	output_event(ep);
    }

    if( thcreated )  ret_to_parent();
    free(mp);
    return 0;
}

/*
 * note_off(note_num [, velocity [,ch]]) : generate note-off event 
 */
int
do_note_off()
{
    MacroCall  *mp;
    int  vel, chan;
    Event  *ep;
    int  thcreated;
    Rational  tmp_dt;
   
    mp = scan_args("i:ii", "note_off");
    thcreated = parse_modifier(0, NULL, &tmp_dt);

    vel = mp->nargs >= 2 ? mp->arg[1].o_val : cur_thd->reg.nv;
    chan = mp->nargs == 3 ? mp->arg[2].o_val : cur_thd->reg.ch;

    if( mp->arg[0].o_val != REST_NOTE ) {
	event_alloc(ep);
	ep->type = E_NoteOff;
	ep->flags = IrregularNoteOff;
	radd(&cur_thd->reg.t, &tmp_dt, &ep->time);
	check_time(ep->time);
	ep->track = conv_track(cur_thd->reg.tk);
	ep->ch = conv_chan(chan);
	set_note(ep->note, mp->arg[0].o_val);
	ep->ev_veloc = vel;
	ep->ev_partner = NULL;
	ep->next = NULL;
	output_event(ep);
    }

    if( thcreated )  ret_to_parent();
    free(mp);
    return 0;
}

/*
 * prog(prog_no) : generate program change event
 * ctrl(func_no, val) : generate control change event
 * kp(val) : generate key-pressure event
 * cpr(val) : generate channel-pressure event
 * bend(val) : generate pitch-bend event
 * tempo(val) : generate tempo-change meta event
 */
int
do_ctrl()
{
    MacroCall  *mp;
    Event  *ep;
    int  thcreated;
    Rational  tmp_dt;
   
    event_alloc(ep);

    switch( cur_token->type ) {
    case T_PROG:
	mp = scan_args("n", "prog");
	ep->type = E_Prog;
	ep->u.obj = mp->arg[0];
	break;
    case T_CTRL:
	mp = scan_args("ie", "ctrl");
	ep->type = E_Ctrl + conv_funcno(mp->arg[0].o_val);
	ep->u.obj = mp->arg[1];
	break;
    case T_KP:
	mp = scan_args("n", "kp");
	ep->type = E_Kp;
	ep->u.obj = mp->arg[0];
	break;
    case T_CPR:
	mp = scan_args("n", "cpr");
	ep->type = E_Cpr;
	ep->u.obj = mp->arg[0];
	break;
    case T_BEND:
	mp = scan_args("n", "bend");
	ep->type = E_Bend;
	ep->u.obj = mp->arg[0];
	break;
    case T_TEMPO:
	mp = scan_args("n", "tempo");
	ep->type = E_Tempo;
	ep->u.obj = mp->arg[0];
	break;
    }

    thcreated = parse_modifier(0, NULL, &tmp_dt);

    radd(&cur_thd->reg.t, &tmp_dt, &ep->time);
    check_time(ep->time);
    ep->track = conv_track(cur_thd->reg.tk);
    ep->ch = conv_chan(cur_thd->reg.ch);
    if( ep->type == E_Kp ) {
	set_note(ep->note, cur_thd->reg.n);
    }
    ep->next = NULL;
    output_event(ep);
    
    if( thcreated )  ret_to_parent();
    free(mp);
    return 0;
}

/*
 * ctrl_to(func_no, val, tmstep, minval) 
 * ctrl_cto(func_no, val, tmstep, minval [, slope1 [, slope2]]) 
 * ctrl_pt(func_no, val) 
 *     : generate continuous control change event 
 */
int
do_cont_ctrl()
{
    MacroCall  *mp;
    Event  *ep;
    int  thcreated;
    Rational  tmp_dt;

    event_alloc(ep);
    if( !(ep->u.cp = (ContEvent *) malloc(sizeof(ContEvent))) ) {
	err_nomem("do_cont_ctrl");
    }

    if( cur_token->type == T_CTRL_PT ) {
	mp = scan_args("if", "ctrl_pt");
	ep->type = (E_Ctrl + conv_funcno(mp->arg[0].o_val)) | E_Pt;
    } else {
	if( cur_token->type == T_CTRL_TO ) {
	    mp = scan_args("ifrf", "ctrl_to");
	    ep->type = (E_Ctrl + conv_funcno(mp->arg[0].o_val)) | E_To;
	} else {  /* T_CTRL_CTO */ 
	    mp = scan_args("ifrf:ff", "ctrl_cto");
	    ep->type = (E_Ctrl + conv_funcno(mp->arg[0].o_val)) | E_CTo;
	}
	if( !rgtzero(&mp->arg[2].r) ) {
	    error(cur_srcpos, "ctrl_(c)to: Illegal time step", 
		  "ctrl_(c)to: 時間刻み幅が不正な値です");
	} 
	ep->u.cp->tmstep = mp->arg[2].r;
	if( mp->arg[3].fpval < 0 ) {
	    error(cur_srcpos, "ctrl_(c)to: Illegal threshold value", 
		  "ctrl_(c)to: 閾値が不正です");
	} 
	ep->u.cp->thres = mp->arg[3].fpval;

	ep->u.cp->slope1 = ep->u.cp->slope2 = 0;
	if( mp->nargs > 4 ) {
	    ep->u.cp->slope1 = mp->arg[4].fpval;
	    if( mp->nargs > 5 ) {
		ep->u.cp->slope2 = mp->arg[5].fpval;
	    }
	}
    }

    if( (ep->type & ETYPE) >= E_ModeMsg && (ep->type & ETYPE) < E_VCtrl ) {
	err_funcno(mp->arg[0].o_val);
    }

    thcreated = parse_modifier(0, NULL, &tmp_dt);

    radd(&cur_thd->reg.t, &tmp_dt, &ep->time);
    check_time(ep->time);
    ep->track = conv_track(cur_thd->reg.tk);
    ep->ch = conv_chan(cur_thd->reg.ch);
    if( (ep->type & ETYPE) == E_Kp ) {
	set_note(ep->note, cur_thd->reg.n);
    }
    ep->u.cp->cval = mp->arg[1].fpval;
    /*
       if( ep->u.cp->cval == LASTVAL ) {
           error(cur_srcpos, "ctrl_to/cto/pt: Illegal control value",
	   	 "ctrl_to/cto/pt: コントロール値が不正です");
       }
    */
    ep->next = NULL;
    output_event(ep);
    
    if( thcreated )  ret_to_parent();
    free(mp);
    return 0;
}

/*
 * excl(data_array) : generate exclusive message event
 * excl2(data_array) : generate exclusive message event (no F7 is sent)
 * arbit(data_array) : generate arbitrary message event
 */
int
do_excl()
{
    MacroCall  *mp;
    Event  *ep;
    Array  *ap;
    uchar  *p;
    int   len, attach_f7, dmask, i;
    PmmlInt  ival;
    char  *name;
    int  thcreated;
    Rational  tmp_dt;

    event_alloc(ep);

    switch( cur_token->type ) {
    case T_EXCL:
	name = "excl";
	ep->type = E_Excl;
	attach_f7 = 1;
	dmask = 0x7f;
	break;

    case T_EXCL2:
	name = "excl2";
	ep->type = E_Excl;
	attach_f7 = 0;
	dmask = 0xff;
	break;

    case T_ARBIT:
	name = "arbit";
	ep->type = E_Arbit;
	attach_f7 = 0;
	dmask = 0xff;
	break;
    }

    mp = scan_args("a", name);
    ap = mp->arg[0].o_ap;
    len = attach_f7 ? ap->size + 1 : ap->size;

    thcreated = parse_modifier(0, NULL, &tmp_dt);

    if( !len )  ep->ev_data = NULL;
    else if( !(ep->ev_data = (uchar *) malloc(len)) ) {
	err_nomem("do_excl");
    }

    for( p = ep->ev_data, i = 0; i < ap->size; i++ ) {
	if( !isnumber(array_ref(ap,i).o_type) ) {
	    error(cur_srcpos, "%s: invalid type of array element",
		  "%s: 配列要素の型が違います", name);
	}
	conv_to_int(&array_ref(ap,i), &ival);
	*p++ = ival & dmask;
    }
    if( attach_f7 )  *p = 0xf7;

    radd(&cur_thd->reg.t, &tmp_dt, &ep->time);
    check_time(ep->time);
    ep->track = conv_track(cur_thd->reg.tk);
    ep->ev_len = len;
    ep->next = NULL;
    output_event(ep);
    
    if( thcreated )  ret_to_parent();
    free_array(ap);
    free(mp);
    return 0;
}

/*
 * seqno(num) : generate sequence number meta event
 */
int
do_seqno()
{
    MacroCall  *mp;
    Event  *ep;
    int  thcreated;
    Rational  tmp_dt;

    mp = scan_args("i", "seqno");
    thcreated = parse_modifier(0, NULL, &tmp_dt);

    event_alloc(ep);
    ep->type = E_Seqno;

    if( !(ep->ev_data = (uchar *) malloc(sizeof(short))) ) {
	err_nomem("do_seqno");
    }
    ep->ev_data[0] = mp->arg[0].o_val >> 8;
    ep->ev_data[1] = mp->arg[0].o_val;
    ep->ev_len = 2;
    radd(&cur_thd->reg.t, &tmp_dt, &ep->time);
    check_time(ep->time);
    ep->track = conv_track(cur_thd->reg.tk);
    ep->next = NULL;
    output_event(ep);

    if( thcreated )  ret_to_parent();
    free(mp);
    return 0;
}

/*
 * text(num, string) : generic text meta event
 */
int
do_text()
{
    MacroCall  *mp;
    Event  *ep;
    int  thcreated;
    Rational  tmp_dt;

    mp = scan_args("is", "text");
    thcreated = parse_modifier(0, NULL, &tmp_dt);

    if( mp->arg[0].o_val < 1 || mp->arg[0].o_val >= 16 ) {
	error(cur_srcpos, "text: Bad text-type number",
	      "text: テキスト種別番号が間違っています");
    }

    event_alloc(ep);
    ep->type = E_Text + mp->arg[0].o_val - 1;
    ep->ev_data = (uchar *) mp->arg[1].o_str;
    ep->ev_len = strlen(mp->arg[1].o_str);
    radd(&cur_thd->reg.t, &tmp_dt, &ep->time);
    check_time(ep->time);
    ep->track = conv_track(cur_thd->reg.tk);
    ep->next = NULL;
    output_event(ep);
    
    if( thcreated )  ret_to_parent();
    free(mp);
    return 0;
}

/*
 * meta(num, data_array) : generic meta event
 */
int
do_meta()
{
    MacroCall  *mp;
    Event  *ep;
    Array  *ap;
    uchar  *p;
    int  i;
    int  thcreated;
    Rational  tmp_dt;
    PmmlInt  ival;

    mp = scan_args("ia", "meta");
    thcreated = parse_modifier(0, NULL, &tmp_dt);

    if( mp->arg[0].o_val < 0 || mp->arg[0].o_val >= 128 ) {
	error(cur_srcpos, "meta: Bad function number",
	      "meta: ファンクション番号が間違っています");
    }

    event_alloc(ep);
    ep->type = E_Meta + mp->arg[0].o_val;
    ap = mp->arg[1].o_ap;
    if( !ap->size )  ep->ev_data = NULL;
    else if( !(ep->ev_data = (uchar *) malloc(ap->size)) ) {
	err_nomem("do_meta");
    }
    for( p = ep->ev_data, i = 0; i < ap->size; i++ ) {
	if( !isnumber(array_ref(ap,i).o_type) ) {
	    error(cur_srcpos, "meta: invalid type of array element",
		  "meta: 配列要素の型が違います");
	}
	conv_to_int(&array_ref(ap,i), &ival);
	*p++ = ival;
    }

    radd(&cur_thd->reg.t, &tmp_dt, &ep->time);
    check_time(ep->time);
    ep->track = conv_track(cur_thd->reg.tk);
    ep->ev_len = ap->size;
    ep->next = NULL;
    output_event(ep);
    
    if( thcreated )  ret_to_parent();
    free_array(ap);
    free(mp);
    return 0;
}

/*
 * smpte(he, mn, se, fr, ff) : generate SMPTE offset meta event 
 */
int
do_smpte()
{
    MacroCall  *mp;
    Event  *ep;
    int  thcreated;
    Rational  tmp_dt;

    mp = scan_args("iiiii", "smpte");
    thcreated = parse_modifier(0, NULL, &tmp_dt);

    event_alloc(ep);
    ep->type = E_Smpte;
    if( !(ep->ev_data = (uchar *) malloc(5)) ) {
	err_nomem("do_smpte");
    }
    ep->ev_data[0] = mp->arg[0].o_val;
    ep->ev_data[1] = mp->arg[1].o_val;
    ep->ev_data[2] = mp->arg[2].o_val;
    ep->ev_data[3] = mp->arg[3].o_val;
    ep->ev_data[4] = mp->arg[4].o_val;

    radd(&cur_thd->reg.t, &tmp_dt, &ep->time);
    check_time(ep->time);
    ep->track = conv_track(cur_thd->reg.tk);
    ep->ev_len = 5;
    ep->next = NULL;
    output_event(ep);
    
    if( thcreated )  ret_to_parent();
    free(mp);
    return 0;
}

/*
 * timesig(nn, dd [,cc [,bb]]) : generate time-signature meta event
 */
int
do_timesig()
{
    MacroCall  *mp;
    Event  *ep;
    int  dd, logdd, cc;
    int  thcreated;
    Rational  tmp_dt;

    mp = scan_args("ii:ii", "timesig");
    thcreated = parse_modifier(0, NULL, &tmp_dt);

    dd = mp->arg[1].o_val;
    cc = (mp->nargs >= 3) ? mp->arg[2].o_val : 1;

    if( mp->arg[0].o_val <= 0 || dd <= 0 || cc <= 0 ||
       (mp->nargs >= 4 && mp->arg[3].o_val <= 0) ) {
	error(cur_srcpos, "timesig: wrong argument value",
	      "timesig: 引数の値が不適当です");
    }

    for( logdd = -1; dd != 0; logdd++ ) {
	dd >>= 1;
    }
    if( (1 << logdd) != mp->arg[1].o_val ) {
	warn(cur_srcpos, "timesig: Denominator is adjusted to %d",
	     "timesig: 分母は %d に修正されます", 1 << logdd);
    }

    event_alloc(ep);
    ep->type = E_TimeSig;
    if( !(ep->ev_data = (uchar *) malloc(4)) ) {
	err_nomem("do_timesig");
    }
    ep->ev_data[0] = mp->arg[0].o_val;
    ep->ev_data[1] = logdd;
    ep->ev_data[2] = (cc * 24 * 4) >> logdd;
    ep->ev_data[3] = (mp->nargs >= 4) ? mp->arg[3].o_val : 8; 

    radd(&cur_thd->reg.t, &tmp_dt, &ep->time);
    check_time(ep->time);
    ep->track = conv_track(cur_thd->reg.tk);
    ep->ev_len = 4;
    ep->next = NULL;
    output_event(ep);
    
    if( thcreated )  ret_to_parent();
    free(mp);
    return 0;
}

/*
 * keysig(sf, mi) : generate key-signature meta event
 */
int
do_keysig()
{
    MacroCall  *mp;
    Event  *ep;
    int  thcreated;
    Rational  tmp_dt;

    mp = scan_args("ii", "keysig");
    thcreated = parse_modifier(0, NULL, &tmp_dt);

    if( mp->arg[0].o_val < -7 || mp->arg[0].o_val >= 8 ||
        mp->arg[1].o_val < 0 || mp->arg[1].o_val >= 2 ) {
	warn(cur_srcpos, "keysig: argument value out of range",
	     "keysig: 引数の値が範囲外です");
    }

    event_alloc(ep);
    ep->type = E_KeySig;
    if( !(ep->ev_data = (uchar *) malloc(2)) ) {
	err_nomem("do_keysig");
    }
    ep->ev_data[0] = mp->arg[0].o_val;
    ep->ev_data[1] = mp->arg[1].o_val;

    radd(&cur_thd->reg.t, &tmp_dt, &ep->time);
    check_time(ep->time);
    ep->track = conv_track(cur_thd->reg.tk);
    ep->ev_len = 2;
    ep->next = NULL;
    output_event(ep);
    
    if( thcreated )  ret_to_parent();
    free(mp);
    return 0;
}

/*
 * end : end of track
 */
int
do_end()
{
    Event  *ep;
    int  thcreated;
    Rational  tmp_dt;

    thcreated = parse_modifier(0, NULL, &tmp_dt);

    event_alloc(ep);
    ep->type = E_TrkEnd;
    ep->ev_data = NULL;
    radd(&cur_thd->reg.t, &tmp_dt, &ep->time);
    check_time(ep->time);
    ep->track = conv_track(cur_thd->reg.tk);
    ep->ev_len = 0;
    ep->next = NULL;
    output_event(ep);
    
    if( thcreated )  ret_to_parent();
    return 0;
}

/*
 * load(filename) : load a starndard MIDI file
 * loadtrk(filename, track) : load a track of a starndard MIDI file
 */
int
do_load()
{
    MacroCall  *mp;
    Rational  tmp_t;

    switch( cur_token->type ) {
    case T_LOAD:
	mp = scan_args("s", "load");
	load_midi_file(mp->arg[0].o_str, &tmp_t);
	break;
    case T_LOADTRK:
	mp = scan_args("si", "loadtrk");
	load_single_track(mp->arg[0].o_str, mp->arg[1].o_val, 
			  conv_track(cur_thd->reg.tk), &tmp_t);
	break;
    }
    strfree(mp->arg[0].o_str);
    free(mp);

    if( probe_next_token()->type == '&' ) {
	get_token();
    } else {
	if( !(cur_thd->flags & PFLAG) ) {
	    cur_thd->reg.t = tmp_t;
	} else { 
	    if( rgreater(&tmp_t, &cur_thd->maxtime) ) 
		cur_thd->maxtime = tmp_t;
	}
    }

    return 0;
}

/*
 * close(filename) : close standard MIDI file
 */
int
do_close()
{
    MacroCall  *mp;

    mp = scan_args("s", "close");
    close_midi_file(mp->arg[0].o_str);
    strfree(mp->arg[0].o_str);
    free(mp);
    return 0;
}

/*
 * defeff(name [,arg_spec [,flags]] ) { ... } : define an effector
 */
int
do_defeff()
{
    MacroCall *mp;
    DicEnt  *dp;
    EffClass *ecp;
    EffRule  *rp, **rnextp;
    Token  *tp;

    mp = scan_args("x:si", "defeff");
    dp = mp->arg[0].id.dp;
    if( mp->arg[0].id.defined /* is already defined */ ) {
	if( dp->type != D_EffClass ) {
	    error(cur_srcpos, "%s: already defined as a different name type",
		  "%s: 他の種類の名前として既に定義済みです", dp->name);
	} else {
	    delete_effclass(dp->dic_eclass);
	}
    }
    if( dp->type == D_LocalMacro ) {
	error(cur_srcpos, "%s: Can not define effector as a local name",
	      "%s: エフェクタをローカル名としては定義できません", dp->name);
    }

    if( !(ecp = (EffClass *) malloc(sizeof(EffClass))) ) {
	err_nomem("do_defeff");
    }
    ecp->type = EC_Normal;
    ecp->dp = dp;
    ecp->flags = mp->nargs >= 3 ? mp->arg[2].o_val : 0;
    ecp->instances = NULL;
    if( mp->nargs >= 2 ) {
	if( !valid_arg_spec(ecp->arg_spec = mp->arg[1].o_str) ) {
	    error(cur_srcpos, "defeff: Incorrect argument specifier",
		  "defeff: 引数仕様が間違っています");
	}
    } else  ecp->arg_spec = DEF_ARG_SPEC;
    ecp->u.n.init = ecp->u.n.detach = ecp->u.n.wrap = NULL;
    e_allset(ecp->u.n.ethru);
    rnextp = &ecp->u.n.rules;

    if( get_token()->type != '{' ) {
	error(cur_srcpos, "defeff: Missing '{'", "defeff: '{' がありません");
    }

    while( (tp = get_token())->type != '}' ) {
	switch( tp->type ) {
	case T_ATTACH:
	case T_INIT:
	    ecp->u.n.init = scan_tklist(0, COLLECT, "attach", '{', '}');
	    break;
	case T_WRAP:
	    ecp->u.n.wrap = scan_tklist(0, COLLECT, "wrap", '{', '}');
	    break;
	case T_DETACH:
	    ecp->u.n.detach = scan_tklist(0, COLLECT, "detach", '{', '}');
	    break;
	case T_CASE:
	    if( !(rp = (EffRule *) malloc(sizeof(EffRule))) ) {
		err_nomem("do_defeff");
	    }
	    e_allclr(rp->scope);
	    parse_event_types(rp->scope, "case");
	    e_andnot(ecp->u.n.ethru, rp->scope);
	    /* if( e_isset(rp->scope, E_NotePair) )  ecp->flags |= EF_PairNotes; */
	    rp->action = scan_tklist(0, COLLECT, "case", '{', '}');
	    *rnextp = rp;
	    rnextp = &rp->next;
	    break;
	default:
	    parse_error(tp);
	}
    }
    *rnextp = NULL;

    dp->active--;
    dp->type = D_EffClass;
    dp->dic_eclass = ecp;

    free(mp);
    return 0;
}

/*
 * parse '(event_type, ...)' 
 */
static void
parse_event_types(evset, cmd_name)
EventSet  evset;
char  *cmd_name;
{
    int  i, etype;

    if( get_token()->type != '(' ) {
	error(cur_srcpos,"%s: '(' expected", "%s: '(' がありません", cmd_name);
    }

    for(;;) {
	etype = parse_cmds(MACRO_CMDS)->type;

	switch( etype ) {
	case T_NOTE:
	    e_bitset(evset, E_NoteOn);
	    e_bitset(evset, E_NoteOff);
	    e_bitset(evset, E_NotePair);
	    break;
	case T_NOTE_ON:
	    e_bitset(evset, E_NoteOn);
	    break;
	case T_NOTE_OFF:
	    e_bitset(evset, E_NoteOff);
	    break;
	case T_PROG:
	    e_bitset(evset, E_Prog);
	    break;
	case T_CTRL:
	    if( probe_next_token()->type == '(' ) {
		parse_numlist(&evset[e_idx(E_Ctrl)], 0, 255, "ctrl");
	    } else {
		for( i = e_idx(E_Ctrl); i < e_idx(E_Meta); i++ ) {
		    evset[i] = ~0L;
		}
	    }
	    break;
	case T_KP:
	    e_bitset(evset, E_Kp);
	    break;
	case T_CPR:
	    e_bitset(evset, E_Cpr);
	    break;
	case T_BEND:
	    e_bitset(evset, E_Bend);
	    break;
	case T_TEMPO:
	    e_bitset(evset, E_Tempo);
	    break;
	case T_EXCL:
	    e_bitset(evset, E_Excl);
	    break;
	case T_ARBIT:
	    e_bitset(evset, E_Arbit);
	    break;
	case T_SEQNO:
	    e_bitset(evset, E_Seqno);
	    break;
	case T_TEXT:
	    if( probe_next_token()->type == '(' ) {
		parse_numlist(&evset[e_idx(E_Meta)], 1, 15, "text");
	    } else {
		e_setinclusive(evset, E_Text, E_TextEnd + 1);
	    }
	    break;
	case T_META:
	    if( probe_next_token()->type == '(' ) {
		parse_numlist(&evset[e_idx(E_Meta)], 0, 127, "meta");
	    } else {
		for( i = e_idx(E_Meta); i < e_idx(E_NoteExcl); i++ ) {
		    evset[i] = ~0L;
		}
	    }
	    e_bitset(evset, E_RawMeta);
	    break;
	case T_SMPTE:
	    e_bitset(evset, E_Smpte);
	    break;
	case T_TIMESIG:
	    e_bitset(evset, E_TimeSig);
	    break;
	case T_KEYSIG:
	    e_bitset(evset, E_KeySig);
	    break;
	case T_ALL:
	    e_allset(evset);	/* This also sets E_RawMeta. */
	    e_bitclr(evset, E_NotePair);
	    break;
	default:
	    error(cur_srcpos, "%s: Parse error in arguments",
		   "%s: 引数に間違いがあります", cmd_name);
	}
	
	if( get_token()->type == ')' ) {
	    break;
	} else if( cur_token->type != ',' ) {
	    error(cur_srcpos, "%s: Parse error in arguments",
		   "%s: 引数に間違いがあります", cmd_name);
	}
    }
}

/*
 * parse '(num, num-num, ...)' 
 */
static void
parse_numlist(evset, low_limit, high_limit, cmd_name)
EventSet  evset;
int  low_limit, high_limit;	/* number range for error processing */
char  *cmd_name;
{
    int  n1, n2;
    Token  *tp;

    if( get_token()->type != '(' ) {
	error(cur_srcpos,"%s: '(' expected", "%s: '(' がありません", cmd_name);
    }

    for(;;) { 
	if( (tp = parse_cmds(MACRO_CMDS))->type != T_NUMBER 
	   || tp->u.obj.o_type != O_INT ) {
	    goto err_parse;
	}
	n1 = tp->u.obj.o_val;
	if( n1 < low_limit || n1 > high_limit ) {
	    goto err_range;
	}

	if( (tp = get_token())->type == '-' ) {
	    if( (tp = parse_cmds(MACRO_CMDS))->type != T_NUMBER 
		|| tp->u.obj.o_type != O_INT ) {
		goto err_parse;
	    }
	    n2 = tp->u.obj.o_val;
	    if( n2 < low_limit || n2 > high_limit ) {
		goto err_range;
	    }
	    e_setinclusive(evset, n1, n2 + 1);
	    tp = get_token();
	} else {
	    e_bitset(evset, n1);
	}

	if( tp->type == ')' ) {
	    break;
	} else if( tp->type != ',' ) {
	    goto err_parse;
	}
    }
    return;

 err_parse:
    error(cur_srcpos, "%s: Parse error in arguments",
	   "%s: 引数に間違いがあります", cmd_name);
 err_range:
    error(cur_srcpos, "%s: Number out of range",
	   "%s: 番号が範囲外です", cmd_name);
}

/*
 * EFFCLASS(args..) : attach an effector
 */
int
do_effclass()
{
    DicEnt  *dp;
    MacroCall  *mp;

    dp = cur_token->u.dp;	/* dp is class name */
    mp = scan_args(dp->dic_eclass->arg_spec, dp->name);

    dp->active--;
    attach_effector(dp, mp, NULL, 0, 1);

    return 0;
}

/*
 * attach(EFFCLASS(args..), INSTANCE_NAME [, enable]) : attach an effector 
 */
int
do_attach()
{
    DicEnt  *dp;
    MacroCall  *mp;
    char  *iname;
    int   ihash;
    Object  *op;
    Token  *tp;
    int  enable;

    if( get_token()->type != '(' ) {
	error(cur_srcpos, "attach: '(' expected", "attach: '(' がありません");
    }
    if( (tp = parse_cmds(MACRO_CMDS))->type != T_EFFCLASS ) {
	error(cur_srcpos, "attach: effector class name expected", 
	       "attach: エフェクタクラス名がありません");
    }

    dp = tp->u.dp;	/* dp is class name */
    mp = scan_args(dp->dic_eclass->arg_spec, dp->name);

    if( get_token()->type != ',' ) {
	error(cur_srcpos, "attach: Too few arguments", 
	       "attach: 引数が足りません");
    }

    /* get instance name */
    if( (tp = get_token())->type != T_ID ) {
	error(cur_srcpos, "attach: Bad instance name",
	       "attach: インスタンス名が間違っています");
    }
    if( !(iname = strdup(tp->u.id.name)) ) {
	err_nomem("do_attach");
    }
    ihash = tp->u.id.hash;

    if( (tp = get_token())->type == ',' ) {
	if( !(op = get_expression()) || op->o_type != O_INT ||
	   get_token()->type != ')' ) {
	    error(cur_srcpos, "attach: Bad enable/disable specifier",
		  "attach: enable/disable 指定が間違っています");
	}
	enable = (op->o_val != 0);
    } else if( tp->type == ')' ) {
	enable = 1;
    }

    dp->active--;
    attach_effector(dp, mp, iname, ihash, enable);

    return 0;
}

/*
 * enable(EFFCLASS or INSTANCE_NAME)  : enable effector instance(s)
 * disable(EFFCLASS or INSTANCE_NAME) : disable effector instance(s)
 */
int
do_enable()
{
    int	 disable = (cur_token->type == T_DISABLE);
    MacroCall *mp;
    DicEnt  *dp;
    EffInst  *eip;

    mp = scan_args("y", disable ? "disable" : "enable");
    dp = mp->arg[0].id.dp;
    dp->active--;
    if( !mp->arg[0].id.defined ) {
	err_undef(dp);
    }

    if( dp->type == D_EffInst ) {
	enable_effector(disable, dp->dic_einst);
    } else if( dp->type == D_EffClass ) {
	for( eip = dp->dic_eclass->instances; eip != NULL; eip = eip->cnext ) {
	    enable_effector(disable, eip);
	}
    } else {
	error(cur_srcpos, "%s: Not an effector (class or instance) name",
	      "%s: エフェクタ名ではありません", dp->name);
    }

    free(mp);
    return 0;
}

/*
 * EFFINST_NAME : synonym of enable(EFFINST_NAME)
 */
int
do_effinst()
{
    DicEnt  *dp;

    dp = cur_token->u.dp;	/* dp is an instance name */
    dp->active--;
    enable_effector(0, dp->dic_einst);

    return 0;
}

/*
 * ecode(event_type) : returns the code of an event type
 */
int
do_ecode()
{
    EventSet  es;
    int	 ecode, idx, i, cnt;
    unsigned long  d;

    e_allclr(es);
    parse_event_types(es, "ecode");
    e_bitclr(es, E_RawMeta);

    cnt = 0;
    for( i = 0; i < EVSET_SIZE; i++ ) {
	if( es[i] ) {
	    idx = i;
	    cnt++;
	}
    }
    if( cnt == 1 ) {
	ecode = idx << 5;
	for( d = es[idx]; (d & 1) == 0; d >>= 1) {
	    ecode++;
	}
    }
    if( cnt != 1 || d != 1 ) {
	error(cur_srcpos, "ecode: Must be just one event type",
	      "ecode: イベント型は１つでなければなりません");
    }
    
    pushbk_int((PmmlInt)ecode);
    return 0;
}

/*
 * set_thru_etypes([EFFCLASS or INSTANCE_NAME])(event_type, ...) 
 * set_eff_etypes([EFFCLASS or INSTANCE_NAME])(event_type, ...) 
 * add_eff_etypes([EFFCLASS or INSTANCE_NAME])(event_type, ...) 
 * del_eff_etypes([EFFCLASS or INSTANCE_NAME])(event_type, ...) : 
 *   commands for changing event through bits
 */
int
do_eff_etypes(flags, eip)
int  flags;
EffInst  *eip;
{
    int  cmd = cur_token->type;
    char  *cmdname = (cmd == T_SET_THRU_ETYPES ? "set_thru_etypes" :
		      cmd == T_SET_EFF_ETYPES ? "set_eff_etypes" :
		      cmd == T_ADD_EFF_ETYPES ? "add_eff_etypes" : 
		      "del_eff_etypes");
    MacroCall *mp;
    DicEnt  *dp;
    EventSet  evset;
    int  eclass = 0;
    
    mp = scan_args(":y", cmdname);
    if( mp->nargs == 0 ) {
	if( !(flags & EFF_ACTION) ) {
	    error(cur_srcpos, "%s: No effector name",
		  "%s: エフェクタ名がありません", cmdname);
	}
	/* eip must be already set */
    } else {
	dp = mp->arg[0].id.dp;
	dp->active--;
	if( !mp->arg[0].id.defined ) {
	    err_undef(dp);
	}
	if( dp->type == D_EffInst ) {
	    eip = dp->dic_einst;
	} else if( dp->type == D_EffClass ) {
	    eip = cur_thd->effs;
	    eclass = 1;
	} else {
	    error(cur_srcpos, "%s: Not an effector (class or instance) name",
		  "%s: エフェクタ名ではありません", dp->name);
	}
    }

    e_allclr(evset);
    parse_event_types(evset, cmdname);

    for( ; eip != NULL; eip = eip->next ) {
	if( eclass && eip->class != dp->dic_eclass )  continue;
	switch( cmd ) {
	case T_SET_THRU_ETYPES:
	    e_copy(eip->ethru, evset);
	    break;
	case T_SET_EFF_ETYPES:
	    e_copy(eip->ethru, evset);
	    e_comp(eip->ethru);
	    break;
	case T_ADD_EFF_ETYPES:
	    e_andnot(eip->ethru, evset);
	    break;
	case T_DEL_EFF_ETYPES:
	    e_or(eip->ethru, evset);
	    break;
	}
	if( !eclass )  break;
    }

    free(mp);
    return 0;
}

/*
 * set_thru_chs([EFFCLASS or INSTANCE_NAME])(ch, ...) 
 * set_eff_chs([EFFCLASS or INSTANCE_NAME])(ch, ...) 
 * add_eff_chs([EFFCLASS or INSTANCE_NAME])(ch, ...) 
 * del_eff_chs([EFFCLASS or INSTANCE_NAME])(ch, ...) : 
 *   commands for changing chanel through bits
 */
int
do_eff_chs(flags, eip)
int  flags;
EffInst  *eip;
{
    int  cmd = cur_token->type;
    char  *cmdname = (cmd == T_SET_THRU_CHS ? "set_thru_chs" :
		      cmd == T_SET_EFF_CHS ? "set_eff_chs" :
		      cmd == T_ADD_EFF_CHS ? "add_eff_chs" : "del_eff_chs");
    MacroCall *mp;
    DicEnt  *dp;
    unsigned long  chs = 0;
    int  eclass = 0;
    
    mp = scan_args(":y", cmdname);
    if( mp->nargs == 0 ) {
	if( !(flags & EFF_ACTION) ) {
	    error(cur_srcpos, "%s: No effector name",
		  "%s: エフェクタ名がありません", cmdname);
	}
	/* eip must be already set */
    } else {
	dp = mp->arg[0].id.dp;
	dp->active--;
	if( !mp->arg[0].id.defined ) {
	    err_undef(dp);
	}
	if( dp->type == D_EffInst ) {
	    eip = dp->dic_einst;
	} else if( dp->type == D_EffClass ) {
	    eip = cur_thd->effs;
	    eclass = 1;
	} else {
	    error(cur_srcpos, "%s: Not an effector (class or instance) name",
		  "%s: エフェクタ名ではありません", dp->name);
	}
    }

    parse_numlist(&chs, 1, 16, cmdname);
    chs >>= 1;

    for( ; eip != NULL; eip = eip->next ) {
	if( eclass && eip->class != dp->dic_eclass )  continue;
	switch( cmd ) {
	case T_SET_THRU_CHS:
	    eip->cthru = chs;
	    break;
	case T_SET_EFF_CHS:
	    eip->cthru = ~chs;
	    break;
	case T_ADD_EFF_CHS:
	    eip->cthru &= ~chs;
	    break;
	case T_DEL_EFF_CHS:
	    eip->cthru |= chs;
	    break;
	}
	if( !eclass )  break;
    }

    free(mp);
    return 0;
}

/*
 * ctrl_any(func_no, val) : generate a normal or continuous control 
 *   change event whichever is the same kind as the original event 
 *   in an effector action
 */
int
do_ctrl_any(flags, eip, ep)
int  flags;
EffInst  *eip;   /* unused */
Event  *ep;
{
    MacroCall  *mp;
    Event  *ep1;
    int  thcreated;
    Rational  tmp_dt;

    mp = scan_args("ie", "ctrl_any");

    if( !(flags & EFF_ACTION) || ep == NULL || (ep->type & ETYPE) >= E_Meta ) {
	error(cur_srcpos, 
	      "ctrl_any: Must be executed in an action for control change",
	      "ctrl_any: コントロールチェンジ用アクション以外では実行できません");
    }

    ep1 = copy_event(ep);
    ep1->type = (E_Ctrl + conv_funcno(mp->arg[0].o_val)) | (ep->type & CTYPE);
    if( ep1->type & CTYPE ) {
	PmmlFloat  f;
	if( !isnumber(mp->arg[1].o_type) ) {
	    error(cur_srcpos, "ctrl_any: Inappropriate type of control value",
		  "ctrl_any: コントロール値が型が不適当です");
	}
	conv_to_float(&mp->arg[1], &f);
	ep1->u.cp->cval = f;
    } else {
	free_object(&ep1->u.obj);
	ep1->u.obj = mp->arg[1];
    }

    thcreated = parse_modifier(0, NULL, &tmp_dt);

    radd(&cur_thd->reg.t, &tmp_dt, &ep1->time);
    check_time(ep1->time);
    ep1->track = conv_track(cur_thd->reg.tk);
    ep1->ch = conv_chan(cur_thd->reg.ch);
    if( ep1->type == E_Kp ) {
	set_note(ep1->note, cur_thd->reg.n);
    }
    ep1->next = NULL;
    output_event(ep1);
    
    if( thcreated )  ret_to_parent();
    free(mp);
    return 0;
}

/*********************************************************************
 * Effector related routines
 *********************************************************************/
/*
 * attach an effector
 */
static void
attach_effector(dp, mp, iname, ihash, enable)
DicEnt  *dp;	   /* dictionary entry defining effector class */
MacroCall *mp;	   /* arguments for init action */
char  *iname;	   /* instance name (NULL for "anonymous" instance) */
int  ihash;	   /* hash value of iname */
int  enable;       /* initial value of effector enable flag (1 or 0) */
{
    EffClass *ecp = dp->dic_eclass;
    EffInst  *eip;
    DicEnt  *idp;	/* dictionary entry defining effector instacne */
    int  i;

    /* create effector instance */
    if( !(eip = (EffInst *) malloc(sizeof(EffInst))) ) {
	err_nomem("attach_effector");
    }
    eip->class = ecp;
    eip->flags = ecp->flags | default_eflags;
    if( ecp->type == EC_BuiltIn ) {
	e_allclr(eip->ethru);
    } else {
	memcpy(eip->ethru, ecp->u.n.ethru, sizeof(EventSet));
    }
    eip->cthru = 0;

    /* register the instance name to the dictionary */
    if( iname ) {
	if( (idp = search_dict(SD_MINE, iname, ihash, 
			       NULL, cur_thd->pb->calls)) ) {
	    error(cur_srcpos, "%s: already defined",
		  "%s: 既に定義済みです", iname);
	}
	if( !(idp = (DicEnt *) malloc(sizeof(DicEnt))) ) {
	    err_nomem("attach_effector");
	}
	idp->type = D_EffInst;
	idp->name = iname;
	idp->hash = ihash;
	idp->scope.c = cur_thd;
	idp->dic_einst = eip;
	idp->active = 0;
	insert_dict(idp);
	eip->dp = idp;
    } else {
	eip->dp = NULL;
    }

    /* create effector thread */
    eip->thd = create_thread(cur_thd, EFLAG);
    eip->thd->dp = eip->dp;
    eip->thd->reg.dt = r_zero;
#ifndef ADD_TP_IN_CONST
    eip->thd->reg.tp = 0;
#endif

    if( ecp->type != EC_BuiltIn ) {
	def_se_macros(eip);
    }

    /* register the instance to the class */
    eip->cnext = ecp->instances;
    ecp->instances = eip;

    /* call init action */
    if( ecp->type == EC_BuiltIn ) {
	if( ecp->u.b.init ) {
	    (* ecp->u.b.init)(mp->nargs, mp->arg, eip);
	}
	for( i = 0; i < mp->nargs; i++ ) {
	    free_object(&mp->arg[i]);
	}
	free(mp);
    } else {
	/* set-up MacroCall structure for argument reference */
	mp->macro = ecp->dp;  /* Altough ecp->dp is not a DicEnt for macro,
			     it's OK because only 'name' element is refered */ 
	mp->lmacros = NULL;
	mp->src_pos = cur_srcpos;
	mp->arg_array = create_ngarray(mp->nargs, mp->arg);
	mp->ref = 1;
	mp->next = NULL;
	eip->thd->pb->calls = mp;

	if( ecp->u.n.init ) {
	    /* thread switch */
	    sleep_cur_thd();
	    awake_thread(eip->thd);

	    /* call init action */
	    eval_action(ecp->u.n.init, eip, NULL);
	    
	    /* resume thread */
	    sleep_cur_thd();
	    awake_thread(eip->thd->parent);
	}
    }

    /* initialize event buffer */
    eip->ebuf.ntrks = 0;
    eip->ebuf.trks = NULL;
    memset(&eip->ebuf.marks, 0, sizeof(Track));

    /* attach the instance to the current thread */
    eip->next = cur_thd->effs;
    cur_thd->effs = eip;
    if( cur_thd->eebv & 0x80000000 ) {
	error(cur_srcpos, "More than 31 effectors are attached",
	      "32個以上のエフェクタがアタッチされました");
    }
    cur_thd->eebv = (cur_thd->eebv << 1) | enable;
}

/*
 * define special effector-instance macros
 */
static void
def_se_macros(eip)
EffInst  *eip;
{
    static int  hash_calculated = 0;
    static int  hash_val, hash_etype, hash_ctype;
    Object  zero;

    if( ! hash_calculated ) {
	hash_val = hash_func("val");
	hash_etype = hash_func("etype");
	hash_ctype  = hash_func("ctype");
	hash_calculated = 1;
    }

    zero.o_type = O_INT;
    zero.o_val = 0;

    eip->val = define_macro("val", hash_val, eip->thd, &zero, NULL);
    eip->val->active++;
    eip->etype = define_macro("etype", hash_etype, eip->thd, &zero, NULL);
    eip->etype->active++;
    eip->ctype = define_macro("ctype", hash_ctype, eip->thd, &zero, NULL);
    eip->ctype->active++;
}

/* 
 * enable or disable effector instance
 */
static void
enable_effector(disable, eip)
int  disable; 		/* 0 (enable) or 1 (disable) */
EffInst  *eip;
{
    EffInst  *eip1;
    EEBV  t_eebv;

    /* search the instance in the current effector list */
    t_eebv = 1;
    for( eip1 = cur_thd->effs; eip1 != NULL; eip1 = eip1->next ) {
	if( eip1 == eip ) {
	    if( disable ) {
		cur_thd->eebv &= ~t_eebv;
	    } else {
		cur_thd->eebv |= t_eebv;
	    }
	    break;
	}
	t_eebv <<= 1;
    }
}

/*
 * detach all the effectors attached in a thread
 */
static void
detach_all_effectors(hp)
Thread  *hp;
{
    EffInst  *eip;

    while( hp->effs && hp->effs->thd->parent == hp ) {
	/* remove effector instance from the effector list 
	   before calling detach_effector! */
	eip = hp->effs;
	hp->effs = eip->next;
	hp->eebv >>= 1;
	detach_effector(eip);
    }
}

/*
 * detach an effector
 */
static void
detach_effector(eip)
EffInst  *eip;
{
    EffClass *ecp = eip->class;
    int  tk;
    EventSeq *sp, *sp_next;
    Event  *ep, *ep_next;
    EffInst  **eipp;

    /* pair note-on and note-off events (this also sorts events) */
    pair_notes(&eip->ebuf);

    /* apply ExpandCtrl and/or Retrigger operations */
    if( eip->flags & (EF_ExpandCtrl | EF_Retrigger) ) {
	post_process(&eip->ebuf, eip->flags & (EF_ExpandCtrl | EF_Retrigger),
		     NULL, NULL, NULL);
	/* re-sort all tracks */
	for( tk = 0; tk < eip->ebuf.ntrks; tk++ ) {
	    sort_event(&eip->ebuf.trks[tk]);
	}
    }

    /* merge tracks if EF_MergeTracks is active */
    if( (eip->flags & EF_MergeTracks) && eip->ebuf.ntrks > 1 ) {
	merge_all_tracks(&eip->ebuf);
	/* Retrigerring after merging tracks is omitted, assuming that
	   EF_MergeTracks and EF_Retrigger are not set simultaneously. */
    }
     
    if( ecp->type == EC_BuiltIn ) {
	/* call detach action */
	if( ecp->u.b.detach )   (* ecp->u.b.detach)(eip);

	/* call action for each event */
	if( ecp->u.b.action ) {
	    for( tk = 0; tk < eip->ebuf.ntrks; tk++ ) {
		for( sp = eip->ebuf.trks[tk].eseq; sp != NULL; sp = sp_next ) {
		    for( ep = sp->events; ep != NULL; ep = ep_next ) {
			ep_next = ep->next;
			(* ecp->u.b.action)(ep, eip);
		    }
		    sp_next = sp->next;
		    free(sp);
		}
	    }
	}
	    
	/* call wrap action */
	if( ecp->u.b.wrap )   (* ecp->u.b.wrap)(eip);

    } else {
	/* thread switch */
	awake_thread(eip->thd);
	cur_thd->reg.t = cur_thd->parent->reg.t;  /* set detach time to t */

	/* call detach action */
	if( ecp->u.n.detach )  eval_action(ecp->u.n.detach, eip, NULL);

	/* call action for each event */
	for( tk = 0; tk < eip->ebuf.ntrks; tk++ ) {
	    for( sp = eip->ebuf.trks[tk].eseq; sp != NULL; sp = sp_next ) {
		for( ep = sp->events; ep != NULL; ep = ep_next ) {
		    ep_next = ep->next;
		    per_event_action(ep, eip);
		}
		sp_next = sp->next;
		free(sp);
	    }
	}

	/* call wrap action */
	if( ecp->u.n.wrap )  eval_action(ecp->u.n.wrap, eip, NULL);

	/* resume thread */
	sleep_cur_thd();

	/* clear active flag of special macro */
	eip->val->active--;
	eip->etype->active--;
	eip->ctype->active--;
    }

    /* delete effector thread */
    delete_thread(eip->thd, 0);

    /* delete dictionary entry for instacne name */
    if( eip->dp ) {
	unlink_dnext_link(eip->dp);
	delete_dict(eip->dp);
    }

    /* free event buffer */
    /* We can assume that there are no events in the event buffers */
    if( eip->ebuf.trks )  free(eip->ebuf.trks);
    destroy_track(&eip->ebuf.marks);

    /* unregister the instance from the class */
    eipp = &ecp->instances;
    while( *eipp != eip )  eipp = &(*eipp)->cnext;
    *eipp = eip->cnext;

    /* free effector instance struct */
    free(eip);
}

/* 
 * evaluate an effector action
 *   Retunrs 1 if "reject" command is found
 */
static int
eval_action(tklist, eip, ep)
Token  *tklist;
EffInst  *eip;
Event  *ep;
{
    int  rejected = 0;
    Token  *tp;
    long  src_pos1;

    /* push-back the action contents */
    pushbk_tklist(tklist, 0);

    /* parse the action */
    for(;;) {
	src_pos1 = cur_srcpos;
	tp = get_token();
	if( tp->type == T_EOF && cur_thd == eip->thd ) { 
	    cur_srcpos = src_pos1;	/* to avoid "At EOF:" in error msgs */
	    break;
	} else if( tp->type == T_REJECT ) {
	    rejected = 1;
	} else if( tkinfo[tp->type].flags & ALL_CMDS ) {
	    if( (* tkinfo[tp->type].handler)(EFF_ACTION, eip, ep) ) {
		parse_error(tp);
	    }
	} else {
	    parse_error(tp);
	}
    }

    return rejected;
}

/*
 * exectute effector action for an event
 */
static void
per_event_action(ep, eip)
Event  *ep;
EffInst  *eip;
{
    EffRule  *rp;
    int  etype;

    /* check if it is already deleted (possible in case of "note" action) */
    etype = ep->type & ETYPE;
    if( etype == E_Deleted ) {
	event_free(ep);
	return;
    }

    /* search rules */
    if( (etype == E_NoteOn || etype == E_NoteOff) && !ep->ev_partner ) {
	/* isolated note-on or note-off event */
	int  noterule_exist = 0;
	for( rp = eip->class->u.n.rules; rp != NULL; rp = rp->next ) {
	    if( e_isset(rp->scope, etype) ) {
		noterule_exist = 1;
		if( ! e_isset(rp->scope, E_NotePair) ) {
		    break;
		}
	    }
	}
	if( !rp ) {
	    if( noterule_exist ) {
		/* If there is "case(note)" rule but no "case(note_on)" or
		   "case(note_off)", pass through the event */
		ep->next = NULL;
		output_event(ep);
	    } else {
		/* Otherwise, dispose the event. */
		event_free(ep);
	    }
	    return;
	}
    } else {
	for( rp = eip->class->u.n.rules; rp != NULL; rp = rp->next ) {
	    if( e_isset(rp->scope, etype) ) {
		break;
	    }
	}
	if( !rp ) {
	    /* If there are no rules, dispose the event.  */
	    event_free(ep);
	    return;
	}
    }

    /* if the rule is empty, pass the event unmodified */
    if( !rp->action ) {
	ep->next = NULL;
	output_event(ep);
	return;
    }

    /* set t & k registers and eebv */
    cur_thd->reg.t = ep->time;
    cur_thd->reg.tk = ep->track + 1;
    cur_thd->eebv = ep->eebv;

    /* set "etype" macro */
    free_object(&eip->etype->dic_obj);
    eip->etype->dic_obj.o_type = O_INT;
    eip->etype->dic_obj.o_val = etype;

    /* processings for each event type */
    if( etype < E_Meta ) {  /* control change */
	/* set "ctype" macro */
	free_object(&eip->ctype->dic_obj);
	eip->ctype->dic_obj.o_type = O_INT;
	eip->ctype->dic_obj.o_val = ep->type >> CTYPE_SHIFT;

	/* set "val" macro */
	free_object(&eip->val->dic_obj);
	if( ep->type & CTYPE ) {	/* continuous type */
	    eip->val->dic_obj.fpval = ep->u.cp->cval;
	} else {			/* normal type */
	    copy_object(&eip->val->dic_obj, &ep->u.obj);
	}

	/* set registers */
	cur_thd->reg.ch = ep->ch + 1;
	if( etype == E_Kp ) {
	    cur_thd->reg.n = ep->note;
	}
	/* call the action */
	if( eval_action(rp->action, eip, ep) ) {
	    /* rejected */
	    event_free(ep);
	} else {
	    /* modify event information and output event */
	    reoutput_ctrl(ep, eip);
	}

    } else if( etype < E_NoteExcl ) {  /* meta event */
	/* set "val" macro */
	free_object(&eip->val->dic_obj);
	if( etype > E_TextEnd || e_isset(rp->scope, E_RawMeta) ) {
	    eip->val->dic_obj.o_type = O_ARRAY;
	    eip->val->dic_obj.o_ap = create_array_from_uchar(ep->ev_len,
							     ep->ev_data);
	} else if( etype == E_Seqno ) {
	    eip->val->dic_obj.o_type = O_INT;
	    eip->val->dic_obj.o_val = (ep->ev_data[0] << 8) | ep->ev_data[1];
	} else {  /* text meta event */
	    eip->val->dic_obj.o_type = O_STRING;
	    if( !(eip->val->dic_obj.o_str = malloc(ep->ev_len + 1)) ) {
		err_nomem("per_event_action");
	    }
	    strncpy(eip->val->dic_obj.o_str, ep->ev_data, ep->ev_len);
	    eip->val->dic_obj.o_str[ep->ev_len] = 0;
	}

	/* call the action */
	if( eval_action(rp->action, eip, ep) ) {
	    /* rejected */
	    event_free(ep);
	} else {
	    /* modify event information and output event */
	    reoutput_meta(ep, eip, e_isset(rp->scope, E_RawMeta));
	}

    } else {
	switch( etype ) {
	case E_NoteOn:
	    /* set register */
	    cur_thd->reg.ch = ep->ch + 1;
	    cur_thd->reg.n = ep->note;
	    cur_thd->reg.v = ep->ev_veloc;
	    /* If the rule is "note" (not "note_on"), set more registers */
	    if( e_isset(rp->scope, E_NotePair) ) {
		rsub(&ep->ev_partner->time, &ep->time, &cur_thd->reg.dofs);
		cur_thd->reg.dp = 0;
		cur_thd->reg.nv = ep->ev_partner->ev_veloc;
	    }
	    /* call the action */
	    if( eval_action(rp->action, eip, ep) ||
	       cur_thd->reg.n == REST_NOTE ) {
		/* rejected */
		if( e_isset(rp->scope, E_NotePair) ) {
		    ep->ev_partner->type = E_Deleted;
		}
		event_free(ep);
		return;
	    }
	    /* modify event infomation */
	    ep->time = cur_thd->reg.t;
	    check_time(ep->time);
	    ep->track = conv_track(cur_thd->reg.tk);
	    ep->ch = conv_chan(cur_thd->reg.ch);
	    set_note(ep->note, cur_thd->reg.n);
	    ep->ev_veloc = cur_thd->reg.v;
	    /* If the rule is "note" (not "note_on"), modify information 
	       of the corresponding note_off event */
	    if( e_isset(rp->scope, E_NotePair) ) {
		if( cur_thd->reg.dofs.intg < 0 ) {
		    /* avoid negative duration */
		    ep->ev_partner->time = ep->time;
		} else {
		    radd(&ep->time, &cur_thd->reg.dofs, &ep->ev_partner->time);
		}
		check_time(ep->ev_partner->time);
		ep->ev_partner->flags = 
		    rgtzero(&cur_thd->reg.dofs) ? 0 : IrregularNoteOff;
		ep->ev_partner->track = ep->track;
		ep->ev_partner->ch = ep->ch;
		ep->ev_partner->note = ep->note;
		ep->ev_partner->ev_veloc = cur_thd->reg.nv;
	    }
	    /* output the modified note-on event */
	    ep->next = NULL;
	    output_event(ep);
	    break;

	case E_NoteOff:
	    /* perform this action only if the rule is "note_off" */
	    if( ! e_isset(rp->scope, E_NotePair) ) {
		/* set register values */
		cur_thd->reg.ch = ep->ch + 1;
		cur_thd->reg.n = ep->note;
		cur_thd->reg.nv = ep->ev_veloc;
		/* call the action */
		if( eval_action(rp->action, eip, ep) ||
		   cur_thd->reg.n == REST_NOTE ) {
		    /* rejected */
		    event_free(ep);
		    return;
		}
		/* modify event infomation */
		ep->time = cur_thd->reg.t;
		if( ep->ev_partner ) {
		    long  cmp = rcomp(&ep->time, &ep->ev_partner->time);
		    if( cmp < 0 ) {
			/* if note-off time is eariler than note-on time,
			   do not consider as a pair any more. */
			ep->ev_partner->ev_partner = NULL;
			ep->ev_partner = NULL;
		    }
		    ep->flags = cmp <= 0 ? IrregularNoteOff : 0;
		}
		check_time(ep->time);
		ep->track = conv_track(cur_thd->reg.tk);
		ep->ch = conv_chan(cur_thd->reg.ch);
		set_note(ep->note, cur_thd->reg.n);
		ep->ev_veloc = cur_thd->reg.nv;
	    }
	    /* output the note-off event */
	    ep->next = NULL;
	    output_event(ep);
	    break;

	case E_Excl:
	case E_Arbit:
	    /* set "val" macro */
	    free_object(&eip->val->dic_obj);
	    eip->val->dic_obj.o_type = O_ARRAY;
	    eip->val->dic_obj.o_ap = create_array_from_uchar(ep->ev_len,
							     ep->ev_data);
	    /* call the action */
	    if( eval_action(rp->action, eip, ep) ) {
		/* rejected */
		event_free(ep);
	    } else {
		/* modify event information and output event */
		reoutput_excl(ep, eip);
	    }
	    break;

	case E_Deleted:
	    /* never reached */
	    break;
	}
    }
}

static void
reoutput_ctrl(ep, eip)
Event  *ep;
EffInst  *eip;
{
    int  etype;

    /* set time, etc. */
    ep->time = cur_thd->reg.t;
    check_time(ep->time);
    ep->track = conv_track(cur_thd->reg.tk);
    ep->ch = conv_chan(cur_thd->reg.ch);

    /* take event type from "etype" */
    if( eip->etype->dic_obj.o_type != O_INT ) {
	error(cur_srcpos, "Illegal type of 'etype'", "'etype' の型が違います");
    }
    etype = E_Ctrl + conv_funcno(eip->etype->dic_obj.o_val);
    if( (ep->type & CTYPE) && etype >= E_ModeMsg && etype < E_VCtrl ) {
	err_funcno(eip->etype->dic_obj.o_val);
    }
    ep->type = etype | (ep->type & CTYPE);

    /* set note number if the event is a key pressure event */
    if( etype == E_Kp ) {
	set_note(ep->note, cur_thd->reg.n);
    }
   
    /* take control value from "val" */
    if( ep->type & CTYPE ) {	/* continuous type */
	PmmlFloat  f;
	if( !isnumber(eip->val->dic_obj.o_type) ) { 
	    error(cur_srcpos, "Illegal type of 'val'", "'val' の型が違います");
	}
	conv_to_float(&eip->val->dic_obj, &f);
	ep->u.cp->cval = f;
    } else {				/* normal type */
	free_object(&ep->u.obj);
	copy_object(&ep->u.obj, &eip->val->dic_obj);
    }

    /* output event */
    ep->next = NULL;
    output_event(ep);
}

static void
reoutput_excl(ep, eip)
Event  *ep;
EffInst  *eip;
{
    int  dmask = (ep->type == E_Excl) ? 0x7f : 0xff;
    Array  *ap;
    uchar  *p;
    int   i;
    PmmlInt  ival;

    /* set time, etc. */
    ep->time = cur_thd->reg.t;
    check_time(ep->time);
    ep->track = conv_track(cur_thd->reg.tk);

    /* take data array from "val" */
    if( eip->val->dic_obj.o_type != O_ARRAY ) {
	error(cur_srcpos, "Illegal type of 'val'", "'val' の型が違います");
    }
    ap = eip->val->dic_obj.o_ap;
    if( ep->ev_data )  free(ep->ev_data);
    if( !ap->size )  ep->ev_data = NULL;
    else if( !(ep->ev_data = (uchar *) malloc(ap->size)) ) {
	err_nomem("reoutput_excl");
    }
    for( p = ep->ev_data, i = 0; i < ap->size; i++ ) {
	if( !isnumber(array_ref(ap,i).o_type) ) {
	    error(cur_srcpos, "val: Invalid type of array element",
		  "val: 配列要素の型が違います");
	}
	conv_to_int(&array_ref(ap,i), &ival); 
	*p++ = ival & dmask;
    }

    /* set length */
    ep->ev_len = ap->size;

    /* output event */
    ep->next = NULL;
    output_event(ep);
}

static void
reoutput_meta(ep, eip, raw_meta)
Event  *ep;
EffInst  *eip;
int  raw_meta;
{
    Array  *ap;
    uchar  *p;
    int   i;
    PmmlInt  ival;

    /* set time, etc. */
    ep->time = cur_thd->reg.t;
    check_time(ep->time);
    ep->track = conv_track(cur_thd->reg.tk);

    /* take data array from "val" */
    if( ep->type > E_TextEnd || raw_meta ) {
	if( eip->val->dic_obj.o_type != O_ARRAY ) {
	    error(cur_srcpos, "Illegal type of 'val'", "'val' の型が違います");
	}
	ap = eip->val->dic_obj.o_ap;
	if( ep->ev_data )  free(ep->ev_data);
	if( !ap->size )  ep->ev_data = NULL;
	else if( !(ep->ev_data = (uchar *) malloc(ap->size)) ) {
	    err_nomem("reoutput_meta");
	}
	for( p = ep->ev_data, i = 0; i < ap->size; i++ ) {
	    if( !isnumber(array_ref(ap,i).o_type) ) {
		error(cur_srcpos, "val: Invalid type of array element",
		      "val: 配列要素の型が違います");
	    }
	    conv_to_int(&array_ref(ap,i), &ival); 
	    *p++ = ival;
	}
	ep->ev_len = ap->size;
    } else if( ep->type == E_Seqno ) {
	if( !isnumber(eip->val->dic_obj.o_type) ) {
	    error(cur_srcpos, "Illegal type of 'val'", "'val' の型が違います");
	}
	conv_to_int(&eip->val->dic_obj, &ival); 
	ep->ev_data[0] = ival >> 8;
	ep->ev_data[1] = ival;
    } else {  /* text meta event */
	if( eip->val->dic_obj.o_type != O_STRING ) {
	    error(cur_srcpos, "Illegal type of 'val'", "'val' の型が違います");
	}
	i = strlen(eip->val->dic_obj.o_str);
	if( ep->ev_data )  free(ep->ev_data);
	if( i == 0 )  ep->ev_data = NULL;
	else {
	    if( !(ep->ev_data = (uchar *) malloc(i)) ) {
		err_nomem("reoutput_meta");
	    }
	    strncpy(ep->ev_data, eip->val->dic_obj.o_str, i);
	}
	ep->ev_len = i;
    }

    /* output event */
    ep->next = NULL;
    output_event(ep);
}

/*
 * free memory used in an effector class
 */
void
delete_effclass(ecp)
EffClass  *ecp;
{
    EffRule  *rp, *next_rp;

    if( ecp->instances ) {
	error(cur_srcpos, "%s: can not delete/redefine active effector class",
	      "%s: 使用中のエフェクタクラスを削除／再定義することはできません",
	      ecp->dp->name);
    }

    if( ecp->type == EC_Normal ) {
	if( ecp->arg_spec != DEF_ARG_SPEC )  free(ecp->arg_spec);
	free_tklist(ecp->u.n.init);
	free_tklist(ecp->u.n.detach);
	free_tklist(ecp->u.n.wrap);
	for( rp = ecp->u.n.rules; rp != NULL; rp = next_rp ) {
	    next_rp = rp->next;
	    free_tklist(rp->action);
	    free(rp);
	}
    }

    free(ecp);
}

/*
 * dump the contents of effector class
 */
void
print_effclass(ecp)
EffClass *ecp;
{
    EffRule  *rp;
    EventSet  etmp;

    if( ecp->type == EC_BuiltIn ) {
	printf("\t<built-in effector class>\n");
	return;
    }

    printf("\teffect event types = ");
    e_allset(etmp);
    e_andnot(etmp, ecp->u.n.ethru);
    print_event_types(etmp);
    printf("\n");

    printf("\tflags = 0x%02x\n", ecp->flags);

    if( ecp->u.n.init ) {
	printf("\tinit = ");
	fprint_tklist(stdout, ecp->u.n.init, O_WTOKENS, -1, -1);
	printf("\n");
    }
    if( ecp->u.n.detach ) {
	printf("\tdetach = ");
	fprint_tklist(stdout, ecp->u.n.detach, O_WTOKENS, -1, -1);
	printf("\n");
    }
    for( rp = ecp->u.n.rules; rp != NULL; rp = rp->next ) {
	printf("\tcase(");
	print_event_types(rp->scope);
	printf(") = ");
	fprint_tklist(stdout, rp->action, O_WTOKENS, -1, -1);
	printf("\n");
    }
    if( ecp->u.n.wrap ) {
	printf("\twrap = ");
	fprint_tklist(stdout, ecp->u.n.wrap, O_WTOKENS, -1, -1);
	printf("\n");
    }
}

/*
 * dump the contents of effector instance
 */
void
print_effinst(eip)
EffInst  *eip;
{
    EventSet  etmp;
    unsigned long  ltmp;

    printf("\tclass name = \"%s\"", eip->class->dp->name);
    if( eip->class->type != EC_BuiltIn ) {
	printf("   args = (");
	fprint_args(stdout, eip->thd->pb->calls, -1);
	printf(")\n");
    } else {
	printf("\n");
    }
    printf("\teffect event types = ");
    e_allset(etmp);
    e_andnot(etmp, eip->ethru);
    print_event_types(etmp);
    printf("\n");
    ltmp = ~eip->cthru;
    printf("\teffect MIDI channels = ");
    print_numlist(&ltmp, 1, 16, "", "");
    printf("\n");
}

/*
 * print list of numbers
 *   Returns 0 if nothing is outputed
 */
static int
print_numlist(evset, offset, limit, leftstr, rightstr)
EventSet  evset;
int  offset, limit;
char  *leftstr, *rightstr;
{
    int  i, n0, cnt = 0, out = 0;

    for( i = 0; i < limit; i++ ) {
	if( e_isset(evset, i) ) {
	    if( !cnt )  n0 = i;
	    cnt++;
	} else {
	    if( cnt ) {
		if( out++ )  printf(","); 
		else  printf("%s", leftstr);
		if( cnt == 1 )  printf("%d", n0 + offset);
		else  printf("%d-%d", n0 + offset, n0 + cnt - 1 + offset);
		cnt = 0;
	    } 
	}
    }
    if( cnt ) {
	if( out++ )  printf(","); 
	else  printf("%s", leftstr);
	if( cnt == 1 )  printf("%d", n0 + offset);
	else  printf("%d-%d", n0 + offset, n0 + cnt - 1 + offset);
    } 
    if( out )  printf("%s", rightstr);

    return out;
}

/*
 * print event types
 */
static void
print_event_types(evset)
EventSet  evset;
{
    int  out = 0;

    if( e_isset(evset, E_NotePair) ) {
	if( out++ )  printf(",");
	printf("note");
    } else {
	if( e_isset(evset, E_NoteOn) ) {
	    if( out++ )  printf(",");
	    printf("note_on");
	}
	if( e_isset(evset, E_NoteOff) ) {
	    if( out++ )  printf(",");
	    printf("note_off");
	}
    }
    if( e_isset(evset, E_Excl) ) {
	if( out++ )  printf(",");
	printf("excl");
    }
    if( e_isset(evset, E_Arbit) ) {
	if( out++ )  printf(",");
	printf("arbit");
    }
    if( print_numlist(&evset[e_idx(E_Ctrl)], 0, 256, 
		      out ? ",ctrl(" : "ctrl(", ")") ) {
	out++;
    }
    if( print_numlist(&evset[e_idx(E_Meta)], 0, 128, 
		      out ? ",meta(" : "meta(", ")") ) {
	out++;
    }
}

/*********************************************************************
 * Thread operation routines 
 *********************************************************************/
/*
 * create a new thread
 */
static Thread *
create_thread(parent, flags)
Thread  *parent;
int  flags;
{
    Thread  *new;

    /* allocate thread's memory */
    if( !(new = (Thread *) malloc(sizeof(Thread))) ) {
	err_nomem("create_thread");
    }

    /* copy registers */
    memcpy(new, parent, sizeof(struct registers));

    /* set-up other members */
    new->effs = parent->effs;
    new->eebv = parent->eebv;
    new->pri = 0;
    new->maxtime = new->reg.t;
    new->flags = flags;
    new->parent = parent;
    new->macros = NULL;
    new->dp = NULL;
    new->nchild = 0;
    if( flags & MFLAG ) { 
	/* for modifiers, we do not make a new push-back stack;
	   instead, we share it with the parent. */ 
	new->pb = parent->pb;
    } else {
	init_pbstack(new);
    }

    /* increase the number of child threads */
    if( !(flags & EFLAG) ) {
	parent->nchild++;
    }

    return new;
}

/*
 * delete a thread
 */
void
delete_thread(hp, nofree)
Thread  *hp;	/* Note: The thread to be deleted must not be in RTL. */
int  nofree;	/* if ture, delete_thread does not free memory for macros.  
		   This is activated at the end of program for using 
		   delete_thread only for the purpose of detaching effectors,
		   and thus, for preventing wasteful free oprations. */
{
    DicEnt  *dp, *next_dp;
    Channel  *sp;
    WaitStatus  **wpp, *wp;
    
    /* if child threads are still alive, postpone until they are exited.*/
    if( hp->nchild > 0 ) {
	hp->flags |= EXITING;
	return;
    }

    /* set the maximum time among the descendants to t */
    if( rgreater(&hp->maxtime, &hp->reg.t) ) {
	hp->reg.t = hp->maxtime;
    }

    /* detach effectors */
    detach_all_effectors(hp);
    
    if( ! nofree ) {
	/* delete all the dictionary entries defined in this thread */
	/* Here we can assume all the D_ThreadName entries no longer exist. */
	for( dp = hp->macros; dp != NULL; dp = next_dp ) {
	    next_dp = dp->dnext;
	    delete_dict(dp);
	}

	/* remove WaitStatus elements of this thread */
	if( hp->flags & WAITEX ) {
	    for( sp = all_chnls; sp != NULL; sp = sp->next ) {
		for( wpp = &sp->wtop; (wp = *wpp) != NULL; wpp = &wp->next ) {
		    if( wp->wait_thd == hp ) {
			*wpp = wp->next;
			free(wp);
			break;
		    }
		}
	    }
	}
    
	if( !(hp->flags & MFLAG) ) {
	    /* free MacroCall structures */ 
	    unlock_calls(hp->pb->calls);
	    
	    /* delete push-back stack */
	    /* The push-back stack must already be exhausted */
	    free_pbstack(hp);
	}
    }
    
    /* if the parent is exiting, delete it recursively */
    if( hp->parent 
       && --hp->parent->nchild == 0 && (hp->parent->flags & EXITING) ) {
	delete_thread(hp->parent, nofree);
    }

    /* free itself */
    free(hp);
}


/*********************************************************************
 * Error handlers
 *********************************************************************/
void
warn_time(time)
Rational  *time;
{
    warn(cur_srcpos, "Time is negative (adjusted to zero)  (t=%s)",
	 "時刻の値が負です(０に修正されます)  (t=%s)", rstring(time));
}

static
err_trk(tk)
int  tk;
{
    error(cur_srcpos, "Track number out of range (tk=%d)", 
	  "トラック番号が範囲外です (tk=%d)", tk);
}

static
err_chan(ch)
int  ch;
{
    error(cur_srcpos, "Channel number out of range (ch=%d)", 
	  "チャネル番号が範囲外です (ch=%d)", ch);
}

static
err_funcno(fn)
int  fn;
{
    error(cur_srcpos, "Function number out of range (func_no=%d)", 
	  "ファンクション番号が範囲外です (func_no=%d)", fn);
}

static
warn_note(note)
int  note;
{
    warn(cur_srcpos, "Note number out of range (note=%d)", 
	 "ノート番号が範囲外です (note=%d)", note);
}

static void
err_undef(dp)
DicEnt  *dp;
{
    error(cur_srcpos, "'%s' is undefined",
	  "'%s' が定義されていません", dp->name);
}
