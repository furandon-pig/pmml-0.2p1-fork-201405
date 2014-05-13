/*
 * PMML: A Compiler for the Practical Music Macro Language
 *
 * array.c: array operation routines
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

static void  grow_array P((Array *));

#define AVOID_ZERO(x)  ((x) <= 0 ? 1 : (x))

/*
 * PMML arrays are implemented with a ring buffer whose size can be 
 * dynamically extended.  The size of the ring buffer is restricted to
 * 2^k (k is an integer) in order to eliminate division operations
 * in element access.  The ring buffer structure enables the execution of 
 * element access, insert, append, and shift operations in O(1), i.e.,
 * independently of the number of elements, except the overheads due to
 * array expansion.
 *
 * Because of the buffer size restriction to 2^k, up to half of the buffer 
 * may be unused.  To overcome this difficulty, buffer size is initially
 * set just equal to the number of elements.  On the first call of insert() 
 * or append(), the buffer size is adjusted to 2^k.
 */

/*********************************************************************
 * Routines for new array generation
 *********************************************************************/
/*
 * create a new array
 */
Array *
create_array(nelms, elms)
int  nelms;
Object  *elms;
{
    Array  *ap;

    if( !(ap = (Array *) malloc(sizeof(Array)) )) {
	err_nomem("create_array");
    }
    ap->size = nelms;
    ap->offset = 0;
    ap->alloc_size = -1;
    ap->ref = 1;	/* assumed to be referenced at once */
    ap->ng_flag = 0;
    
    if( !(ap->elms = (Object *) malloc(sizeof(Object) * AVOID_ZERO(nelms))) ) {
	err_nomem("create_array");
    }
    memcpy(ap->elms, elms, sizeof(Object) * nelms);
    
    return ap;
}

/*
 * make a clone of an array
 */
Array *
dup_array(ap)
Array  *ap;
{
    Array  *new;
    int	 nelms = ap->size;
    int  i;
    Object  *src, *dst;

    if( !(new = (Array *) malloc(sizeof(Array)) )) {
	err_nomem("dup_array");
    }
    new->size = nelms;
    new->offset = 0;
    new->alloc_size = -1;
    new->ref = 1;	/* assumed to be referenced at once */
    new->ng_flag = 0;
    
    if( !(new->elms = (Object *) malloc(sizeof(Object) * AVOID_ZERO(nelms)))) {
	err_nomem("dup_array");
    }
    
    for( i = 0, dst = new->elms; i < nelms; i++, dst++ ) {
	src = &array_ref(ap, i);

	switch( src->o_type ) {
	case O_STRING:
	    dst->o_type = O_STRING;
	    if( !(dst->o_str = strdup(src->o_str)) ) {
		err_nomem("dup_array");
	    }
	    break;
	    
	case O_ARRAY:
	    dst->o_type = O_ARRAY;
	    dst->o_ap = dup_array(src->o_ap);
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
    
    return new;
}

/*
 * create a new array by replicating a single element
 */
Array *
create_reparray(nelms, elm)
int  nelms;
Object  *elm;
{
    Array  *ap;
    int  i;
    Object  *dst;

    if( nelms < 0 )  nelms = 0;

    if( !(ap = (Array *) malloc(sizeof(Array)) )) {
	err_nomem("create_reparray");
    }
    ap->size = nelms;
    ap->offset = 0;
    ap->alloc_size = -1;
    ap->ref = 1;	/* assumed to be referenced at once */
    ap->ng_flag = 0;

    if( !(ap->elms = (Object *) malloc(sizeof(Object) * AVOID_ZERO(nelms))) ) {
	err_nomem("create_reparray");
    }

    switch(elm->o_type) {
    case O_STRING:
	for( i = 0, dst = ap->elms; i < nelms; i++, dst++ ) {
	    dst->o_type = O_STRING;
	    if( !(dst->o_str = strdup(elm->o_str)) ) {
		err_nomem("create_reparray");
	    }
	}
	break;

    case O_ARRAY:
	for( i = 0, dst = ap->elms; i < nelms; i++, dst++ ) {
	    dst->o_type = O_ARRAY;
	    dst->o_ap = dup_array(elm->o_ap);
	}
	break;

    case O_STOKENS:
    case O_WTOKENS:
	for( i = 0, dst = ap->elms; i < nelms; i++, dst++ ) {
	    dst->o_type = elm->o_type;
	    dst->o_tp = copy_tklist(elm->o_tp);
	}
	break;

    default:
	for( i = 0, dst = ap->elms; i < nelms; i++, dst++ ) {
	    *dst = *elm;
	}
	break;
    }

    return ap;
}

/*
 * make a non-growable array (used for implementing '$*')
 *   The '$*' array can not be expanded, because the data part (elms)
 *   points to somewhere in the argument stack.
 */
Array *
create_ngarray(nelms, elms)
int  nelms;
Object  *elms;
{
    Array  *ap;

    if( !(ap = (Array *) malloc(sizeof(Array)) )) {
	err_nomem("create_ngarray");
    }
    ap->size = nelms;
    ap->offset = 0;
    ap->alloc_size = -1;
    ap->ref = 1;
    ap->ng_flag = 1;
    ap->elms = elms;

    return ap;
}

/*
 * change a non-growable array to a normal array
 */
void
ngarray_to_array(ap)
Array  *ap;
{
    Object  *elms;

    if( !(elms = (Object *) malloc(sizeof(Object) * AVOID_ZERO(ap->size))) ) {
	err_nomem("ngarray_to_array");
    }
    memcpy(elms, ap->elms + ap->offset, sizeof(Object) * ap->size);
    ap->elms = elms;
    ap->offset = 0;
    ap->ng_flag = 0;
}

/*
 * create a new array from unsigned char array
 */
Array *
create_array_from_uchar(nelms, data)
int  nelms;
uchar  *data;
{
    Array  *ap;
    int  i;

    if( !(ap = (Array *) malloc(sizeof(Array)) )) {
	err_nomem("create_array_from_uchar");
    }
    ap->size = nelms;
    ap->offset = 0;
    ap->alloc_size = -1;
    ap->ref = 1;	/* assumed to be referenced at once */
    ap->ng_flag = 0;
    
    if( !(ap->elms = (Object *) malloc(sizeof(Object) * AVOID_ZERO(nelms))) ) {
	err_nomem("create_array");
    }
    for( i = 0; i < nelms; i++ ) {
	ap->elms[i].o_type = O_INT;
	ap->elms[i].o_val = *data++;
    }
    
    return ap;
}

/*********************************************************************
 * Free the memory used by an array
 *********************************************************************/
void
destroy_array(ap)
Array  *ap;
{
    int  i;

#ifdef FREE_TEST
    printf("destroy_array: ");
    fprint_array(stdout, ap, -1, -1);
    printf("\n");
#endif

    for( i = 0; i < ap->size; i++ ) {
	free_object( &array_ref(ap, i) );
    }
    if( ! ap->ng_flag )  free(ap->elms);
    free(ap);
}

/*********************************************************************
 * Routines for changing array size
 *********************************************************************/
/*
 * append one element after the end of an array
 */	
void
append_array(ap, elm)
Array  *ap;
Object *elm;
{
    if( ap->size > ap->alloc_size ) {
	grow_array(ap);
    }
    array_ref(ap, ap->size++) = *elm;
}

/*
 * insert one element before the top of an array
 */
void
insert_array(ap, elm)
Array  *ap;
Object *elm;
{
    if( ap->size > ap->alloc_size ) {
	grow_array(ap);
    }
    ap->size++;
    ap->offset = (ap->offset - 1) & ap->alloc_size;
    ap->elms[ap->offset] = *elm;
}

/*
 * reduce array size from the top (n > 0) or tail (n < 0)
 */
void
shift_array(ap, n)
Array  *ap;
int    n;
{
    if( n > 0 ) {
	if( n > ap->size )  n = ap->size;

	ap->size -= n;

	while( --n >= 0 ) {
	    /* free the shrinked part of the array */
	    free_object( &ap->elms[ap->offset] );
	    ap->offset = (ap->offset + 1) & ap->alloc_size;
	}
    } else {
	if( n < -ap->size )  n = -ap->size;
	
	while( ++n <= 0 ) {
	    --ap->size;
	    /* free the shrinked part of the array */
	    free_object( &array_ref(ap, ap->size) );
	}
    }
}

/*
 * expand array size
 */
static void
grow_array(ap)
Array  *ap;
{
    register int  asize;
    
#ifdef ARRAY_TEST
    printf("array size expansion\n");
#endif
    if( ap->ng_flag ) {
	error(cur_srcpos, "Can not increase the size of '$*' or message array",
	      "'$*' またはメッセージ配列のサイズを拡大することはできません");
    }

    if( (asize = ap->alloc_size) < 0 ) { /* no append/insert is applied yet */
	/* 
	 * alloc_size = (minimum 2^k satisfying 2^k >= size+offset+1) - 1
	 */
	register int t = ap->size + ap->offset;
	asize = 2;
	while( t >>= 1 )  asize <<= 1;

	ap->elms = (Object *) realloc(ap->elms, sizeof(Object) * asize);
	ap->alloc_size = asize - 1;

    } else {  /* size == alloc_size + 1 */
	/*
	 * double the array size
	 */
	register int oldsize = asize + 1;
	asize = oldsize << 1;

	ap->elms = (Object *) realloc(ap->elms, sizeof(Object) * asize);
	ap->alloc_size = asize - 1;

	if( ap->offset < (oldsize >> 2) ) {
	    /*
	     * If the offset position was in the first half of the old array,
	     * copy the span between the beginning and offset position 
	     * to the second half of the new array.
	     */

#ifdef ARRAY_TEST
	    printf("%d elements moved (Type A)\n", ap->offset);
#endif
	    memcpy(ap->elms + oldsize, ap->elms, ap->offset * sizeof(Object));

	} else {
	    /*
	     * If the offset position was in the second half of the old array,
	     * copy the span between the offset position and old array's end 
	     * to the last of the new array.
	     */

#ifdef ARRAY_TEST
	    printf("%d elements moved (Type B)\n", oldsize - ap->offset);
#endif
	    memcpy(ap->elms + ap->offset + oldsize, ap->elms + ap->offset,
		   (oldsize - ap->offset) * sizeof(Object));
	    ap->offset += oldsize;
	}
    }
}

/*********************************************************************
 * Print-out routines
 *********************************************************************/
void
fprint_array(fp, ap, maxelms, maxtklen)
FILE  *fp;
Array  *ap;
int  maxelms, maxtklen;
{
    int  i;

    fprintf(fp, "#(");
#ifdef PRINT_REF
    fprintf(fp, "ref=%d: ", ap->ref);
#endif
    for( i = 0; i < ap->size; i++ ) {
	if( i > 0 ) fprintf(fp, ",");
	if( maxelms >= 0 && i >= maxelms ) {
	    fprintf(fp, "...");
	    break;
	}
	fprint_object(fp, &array_ref(ap, i), maxelms, maxtklen);
    }
    fprintf(fp, ")");
}

/*********************************************************************
 * Self tester
 *********************************************************************/
#ifdef ARRAY_TEST
main()
{
    int	 i;
    char cmd[2], line[80];
    int  par = 1;
    static Object  o[3], t;
    Array  *ap;

    o[0].o_type = O_INT;
    o[0].o_val = 1;
    o[1].o_type = O_INT;
    o[1].o_val = 2;
    o[2].o_type = O_INT;
    o[2].o_val = 3;
    t.o_type = O_INT;
    t.o_val = 0;
    
    ap = create_array(3, &o);
	
    for(;;) {
	printf("size=%d ofs=%d alloc=%d : ", 
	       ap->size, ap->offset, ap->alloc_size);
	for( i = 0; i < ap->size; i++ ) printf("%ld ", array_ref(ap,i).o_val);
	printf("\n");

	if( !gets(line) )  exit(0);
	sscanf(line, "%1s%d", cmd, &par); 
	t.o_val = par;
	switch(cmd[0]) {
	case 'a':
	    append_array(ap, &t);
	    break;
	case 'i':
	    insert_array(ap, &t);
	    break;
	case 's':
	    shift_array(ap, par);
	    break;
	default:
	    exit(0);
	}
    }
}
#endif
