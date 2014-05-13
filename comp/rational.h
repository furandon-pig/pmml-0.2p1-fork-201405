/*
 * PMML: A Compiler for the Practical Music Macro Language
 *
 * rational.h: header for rational number calculation routines
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

/*
 * rational numbers represented as "a + b / c", where 
 *   -32768 <= a <= 32767 
 *   0 <= b <= 32767
 *   0 < c <= 32767
 *   b < c
 *   gcd(b,c) is not always 1 
 *
 * Because keeping rational numbers always irreducible leads to significant 
 * performance degradation, redundant rational numbers are allowed.
 */
typedef struct rational {
#if PMML_BIG_ENDIAN
    unsigned short  type;	/* must be O_RATIONAL */
    short  intg;		/* integer part */
    short  num;			/* numerator */
    short  den;			/* denominator */
#else
    short  num;			/* numerator */
    short  den;			/* denominator */
    short  intg;		/* integer part */
    unsigned short  type;	/* must be O_RATIONAL */
#endif
} Rational;

/* initializer */
#if PMML_BIG_ENDIAN
#  define RINIT(_intg,_num,_den) {O_RATIONAL, _intg, _num, _den}
#else
#  define RINIT(_intg,_num,_den) {_num, _den, _intg, O_RATIONAL}
#endif

/* compare two rational numbers */
#define rcomp(a,b)   ((a)->intg == (b)->intg ? (\
		      (a)->den == (b)->den ? (long)(a)->num - (b)->num : \
		      (long)(a)->num*(b)->den - (long)(b)->num*(a)->den ) : \
		      (long)(a)->intg - (b)->intg)
#define requal(a,b)  ((a)->intg == (b)->intg && \
		      (*(long *)&(a)->num == *(long *)&(b)->num || \
		      (a)->num * (b)->den == (b)->num * (a)->den))
#define rless(a,b)   ((a)->intg == (b)->intg ? (\
		      (a)->den == (b)->den ? (a)->num < (b)->num : \
		      (a)->num * (b)->den < (b)->num * (a)->den ) : \
		      (a)->intg < (b)->intg)
#define rlesseq(_a,_b)     (!rless((_b),(_a)))
#define rgreater(_a,_b)    (rless((_b),(_a)))
#define rgreatereq(_a,_b)  (!rless((_a),(_b)))
#define rzerop(a)    ((a)->intg == 0 && (a)->num == 0)
#define rgtzero(a)   ((a)->intg > 0 || ((a)->intg == 0 && (a)->num != 0))

/* convert a rational number to another type of number */
#define rational_to_int(r) \
	((long)floor(rational_to_float(r) + 0.5))
#define rational_to_float(r) \
	(((r)->intg + (double) (r)->num / (r)->den) * (IRES * 4.0))

/* rational constants */
extern Rational  r_zero, r_one, r_max, r_min; 

/* prototypes */
void	radd P((Rational *, Rational *, Rational *));
void	rsub P((Rational *, Rational *, Rational *));
void	rmult P((Rational *, Rational *, Rational *));
void	rimult P((Rational *, long, Rational *));
int	rdiv P((Rational *, Rational *, Rational *));
int	ridiv P((Rational *, long, Rational *));
int	rmod P((Rational *, Rational *, Rational *));
void	rnorm P((Rational *));
void	rnorm2 P((Rational *, Rational *));
void	rset P((long, int, Rational *));
void	rneg P((Rational *));
void	int_to_rational P((long, Rational *));
void	float_to_rational P((double, Rational *));
char *	rstring P((Rational *));
