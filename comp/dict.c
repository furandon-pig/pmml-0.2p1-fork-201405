/*
 * PMML: A Compiler for the Practical Music Macro Language
 *
 * dict.c: name dictionary related routines
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

/*
 * size of the hash table (prime number is recommended)
 */
#define	HASH_SIZE   1021		

/*
 * global variables
 */
static DicEnt  *dictionary[HASH_SIZE];	/* dictionary body */

/*
 * prototypes
 */
static void  print_scope P((Thread *));


/*********************************************************************
 * Routines for adding entries
 *********************************************************************/
/*
 * insert an entry to the dictionary
 */
void
insert_dict(dp)
DicEnt *dp;
{
    DicEnt  **head;

    /* connect dnext link */
    if( dp->type == D_LocalMacro ) {
	dp->dnext = dp->scope.m->lmacros;
	dp->scope.m->lmacros = dp;
    } else if( dp->type != D_BuiltIn ) {
	dp->dnext = dp->scope.c->macros;
	dp->scope.c->macros = dp;
    }

    /* connet hnext link */
    head = &dictionary[dp->hash];
    dp->hnext = *head;
    *head = dp;
}

/* 
 * define a thread macro 
 */
DicEnt *
define_macro(name, hash, scope, objp, arg_spec)
char  *name;
int   hash;
Thread  *scope;		/* NULL for current thread */
Object  *objp;
char  *arg_spec;
{
    DicEnt  *dp;

    if( !scope ) {
	dp = search_dict(SD_ANY, name, hash, NULL, cur_thd->pb->calls);
	scope = cur_thd;
    } else {
	dp = search_dict(SD_THREAD, name, hash, scope, cur_thd->pb->calls);
    }

    if( !dp ) {
	/* define new macro */
	if( !(dp = (DicEnt *) malloc(sizeof(DicEnt))) ||
	   !(dp->name = strdup(name)) ) {
	    err_nomem("define_macro");
	}
	dp->type = D_ThreadMacro;
	dp->hash = hash;
	dp->scope.c = scope;
	dp->active = 0;
	dp->dic_obj = *objp;
	if( arg_spec != NULL && arg_spec != DEF_ARG_SPEC ) {
	    if( !(arg_spec = strdup(arg_spec)) )  err_nomem("define_macro");
	}
	dp->dic_arg_spec = arg_spec;
	insert_dict(dp);
    } else {
	redefine_macro(dp, objp, arg_spec);
    }

    return dp;
}

/*
 * redefine an existing macro
 */
void
redefine_macro(dp, objp, arg_spec)
DicEnt *dp;
Object *objp;
char  *arg_spec;
{
    if( dp->type != D_LocalMacro && dp->type != D_ThreadMacro ) {
	error(cur_srcpos, "%s: already defined as a different name type",
	      "%s: 他の種類の名前として既に定義済みです。", dp->name);
    }
    free_object(&dp->dic_obj);
    dp->dic_obj = *objp;
    if( dp->dic_arg_spec != NULL && dp->dic_arg_spec != DEF_ARG_SPEC ) {
	strfree(dp->dic_arg_spec);
    }
    dp->dic_arg_spec = arg_spec;
}

/*********************************************************************
 * Search routine
 *********************************************************************/
/*
 * search an entry
 *   returns NULL if the name is not registered
 *
 *    Search Mode:
 *    SD_ANY    - find local macro of the current macro call  or 
 *                non-local entry whose scope is the current thread or 
 *                one of its ancestors
 *    SD_MINE   - find local macro of the current macro call  or
 *                non-local entry whose scope is the current thread
 *    SD_LOCAL  - find local macro of the current macro call
 *    SD_THREAD - find non-local entry whose scope is the specified thread
 */
DicEnt *
search_dict(mode, name, hash, scope, cur_mc)
int   mode;		/* SD_ANY, SD_THREAD, or SD_LOCAL */
char  *name;		/* macro name */
int   hash;		/* hash value */
Thread *scope;		/* scope thread (for SD_THREAD only) */
MacroCall *cur_mc;	/* current MacroCall structure */
{
    DicEnt  *dp, *rtn;
    int  pri, maxpri = -1, n;
    Thread  *t;

    rtn = NULL;

    switch(mode) {
    case SD_ANY:
    case SD_MINE:
	/* set priority */
	if( mode == SD_MINE ) {
	    cur_thd->pri = 0x7fff;
	} else {
	    for( t = cur_thd, n = 0x7fff; t != NULL; t = t->parent ) {
		t->pri = n--;
	    }
	}
	/* 
	 * If a local macro is defined, return it. 
	 * Otherwise, return the entry with the highest non-zero priority
	 */
	for( dp = dictionary[hash]; dp != NULL; dp = dp->hnext ) {
	    if( dp->type == D_LocalMacro ) {
		if( dp->scope.m == cur_mc && strcmp(name, dp->name) == 0 ) {
		    rtn = dp;
		    break;
		}
	    } else if( ((pri = dp->scope.c->pri) != 0 
			|| dp->type == D_ThreadName)
		      && strcmp(name, dp->name) == 0 ) {
		if( pri > maxpri ) {
		    rtn = dp;
		    maxpri = pri;
		}
	    }
	}
	/* reset priority */
	for( t = cur_thd; t != NULL; t = t->parent ) {
	    t->pri = 0;
	}
	break;

    case SD_LOCAL:
	for( dp = dictionary[hash]; dp != NULL; dp = dp->hnext ) {
	    if( dp->type == D_LocalMacro && dp->scope.m == cur_mc &&
	       strcmp(name, dp->name) == 0 ) {
		rtn = dp;
		break;
	    }
	}
	break;

    case SD_THREAD:
	for( dp = dictionary[hash]; dp != NULL; dp = dp->hnext ) {
	    if( dp->type != D_LocalMacro &&
	       dp->scope.c == scope && strcmp(name, dp->name) == 0 ) {
		rtn = dp;
		break;
	    }
	}
	break;
    }

    return rtn;
}

/*********************************************************************
 * Routines for deleting entries
 *********************************************************************/
/*
 * delete a dictionary entry
 */
void
delete_dict(dp)
DicEnt  *dp;
{
    DicEnt **dpp, *dp1;

    switch(dp->type) {
    case D_LocalMacro:
    case D_ThreadMacro:
	free_object(&dp->dic_obj);
	if( dp->dic_arg_spec != NULL && dp->dic_arg_spec != DEF_ARG_SPEC ) {
	    strfree(dp->dic_arg_spec);
	}
	break;

    case D_BuiltIn:
	/* do nothing */
	break;

    case D_ThreadName:
	/* first delete DicEnt entries of all the child named threads */
	for( dpp = &dp->dic_thd->macros; (dp1 = *dpp) != NULL; ) {
	    if( dp1->type == D_ThreadName ) {
		*dpp = dp1->dnext;
		delete_dict(dp1);
	    } else {
		dpp = &dp1->dnext;
	    }
	}

	/* change named thread to unnamed thread */
	dp->dic_thd->dp = NULL;

	/* if the thread is sleeping due to token exhaution, 
	   delete it really */ 
	if( dp->dic_thd->pb->top->type == T_EOF ) {
	    /* Since this thread must be already sleeping, and all the named 
	       decendant threads no longer have their DicEnt entry, 
	       delete_thread can be called safely. */
	    delete_thread(dp->dic_thd, 0);
	}

	break;

    case D_EffClass:
	delete_effclass(dp->dic_eclass);
	break;

    case D_EffInst:
	/* indicate that the dictionary entry no longer exists */
	dp->dic_einst->dp = NULL;
	break;

    case D_Channel:
	destroy_channel(dp->dic_chnl);
	break;
    }

    /* unlink the hnext link */
    dpp = &dictionary[dp->hash];
    while( *dpp != dp )  dpp = &(*dpp)->hnext;
    *dpp = dp->hnext;

    if( dp->active ) {
	error(cur_srcpos, 
	      "%s: Unexpected deletion of an active dictionary entry (See manual for details)",
	      "%s: 活動中の辞書エントリが不慮に削除されました（詳細はマニュアル参照）",
	      dp->name);
    } else {	
	free(dp->name);
	free(dp);
    }
}

/*
 * unlink the dnext link
 *   (call before delete_dict if necessary)
 */
void
unlink_dnext_link(dp)
DicEnt  *dp;
{
    DicEnt  **dpp;

    if( dp->type == D_BuiltIn ) {
	return;
    } else if( dp->type == D_LocalMacro ) {
	dpp = & dp->scope.m->lmacros;
    } else {
	dpp = & dp->scope.c->macros;
    }

    while( *dpp != dp )  dpp = &(*dpp)->dnext;
    *dpp = dp->dnext;
}


/*********************************************************************
 * Hash function generator
 *   This routine is based on the hash function generator shown in the book 
 *   "Compilers" by Aho, Sethi, and Ullman
 *********************************************************************/
int
hash_func(str)
char  *str;
{
    char  *p;
    unsigned long  h = 0;
    unsigned long  t;

    for( p = str; *p; p++ ) {
	h = (h << 4) + *p;
	if( t = h & 0xf0000000 ) {
	    h ^= (t >> 24) ^ t;
	}
    }
    return( (int)(h % HASH_SIZE) );
}

/*********************************************************************
 * Print-out routines
 *********************************************************************/
#define DUMP_MAXELMS   6
#define DUMP_MAXTKLEN  10

/*
 * print the definition of macro
 */
void
print_def(dp, verbose)
DicEnt *dp;
int  verbose;
{
    char  *p;

    if( dp->type == D_LocalMacro )  printf("local");
    else if( dp->type == D_ThreadName )  print_scope(dp->dic_thd->parent);
    else print_scope(dp->scope.c);
    
    if(((dp->type == D_LocalMacro || 
	 dp->type == D_ThreadMacro || 
	 dp->type == D_BuiltIn ) &&
	(p = dp->dic_arg_spec) != NULL) ||
       ((dp->type == D_EffClass) &&
	(p = dp->dic_eclass->arg_spec) != NULL) ) {
	if( p == DEF_ARG_SPEC ) {
	    printf("::%s() = ", dp->name);
	} else {
	    printf("::%s(\"%s\") = ", dp->name, p);
	}
    } else {
	printf("::%s = ", dp->name);
    }

    switch(dp->type) {
    case D_LocalMacro:
    case D_ThreadMacro:
	if( verbose ) {
	    fprint_object(stdout, &dp->dic_obj, -1, -1);
	} else {
	    fprint_object(stdout, &dp->dic_obj, DUMP_MAXELMS, DUMP_MAXTKLEN);
	}
	printf("\n");
	break;

    case D_BuiltIn:
	printf("<built-in>\n");
	break;

    case D_ThreadName:	
	printf("<thread-name>\n");
	break;
    
    case D_EffClass:
	printf("<effector-class>\n");
	if( verbose )  print_effclass(dp->dic_eclass);
	break;
    
    case D_EffInst:
	printf("<effector-instance>\n");
	if( verbose )  print_effinst(dp->dic_einst);
	break;

    case D_Channel:
	printf("<channel-name>\n");
	break;
    }
}

/*
 * print scope thread
 */
static void
print_scope(hp)
Thread *hp;
{
    if( !hp || !hp->parent ) {  /* root thread */
	return;
    } else {
	print_scope(hp->parent);
	if( hp->dp ) {
	    printf("::%s", hp->dp->name);
	} else {
	    printf("::<noname>");
	}
    }
}

/*
 * print dictionary contents
 */
#define NHIST  10

static int
dp_comp(dp1, dp2)
DicEnt  **dp1, **dp2;
{
    return strcmp(dp1[0]->name, dp2[0]->name);
}

void
dump_dict(types, scope)
int  types;	/* bit vector representing which type of DicEnt is displayed */
Thread  *scope;	/* NULL for every thread */
{
    int  i, cnt, total;
    int  histgram[NHIST];
    DicEnt  *dp;
    DicEnt  **dpbuf;
    int  dpsize;

    total = 0;
    for( i = 0; i < NHIST; i++ )  histgram[i] = 0;

    dpsize = 0;
    dpbuf = (DicEnt **) malloc(sizeof(DicEnt *));

    for( i = 0; i < HASH_SIZE; i++ ) {
	cnt = 0;
	for( dp = dictionary[i]; dp != NULL; dp = dp->hnext ) {
	    cnt++;
	    if( !((1 << dp->type) & types) )  continue;
	    if( scope && dp->type != D_LocalMacro && scope != dp->scope.c ) {
		continue;
	    }
	    dpbuf = (DicEnt **) realloc(dpbuf, sizeof(DicEnt *) * (dpsize+1));
	    dpbuf[dpsize++] = dp;

	    if( dp->hash != i ) {
		printf("<hash value is inappropriate> %s\n", dp->name);
	    }
	}
	total += cnt;
	if( cnt >= NHIST )  cnt = NHIST - 1;
	histgram[cnt]++;
    }

    qsort((char *)dpbuf, dpsize, sizeof(DicEnt *), dp_comp);
    for( i = 0; i < dpsize; i++ ) {
	dp = dpbuf[i];
#ifdef PRINT_ACTIVE
	printf("%d  ", dp->active);
#endif
	if( dp->type == D_LocalMacro ) {
	    printf("[%s] ", dp->scope.m->macro->name);
	}
	print_def(dp, 0);
    }
	
    if( types & 0x8000 ) {
	printf("total entries = %d\n", total);
	printf("hash histgram:");
	for( i = 0; i < NHIST-1; i++ ) printf(" [%d]%d", i, histgram[i]);
	printf(" [>=%d]%d", i, histgram[i]);
	printf("\n");
    }

    free(dpbuf);
}
