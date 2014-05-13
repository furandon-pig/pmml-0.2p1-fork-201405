/*
 * PMML: A Compiler for the Practical Music Macro Language
 *
 * rational.c: rational number calculation routines
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
 * function prototypes
 */
static unsigned long  gcd P((unsigned long, unsigned long));
static void	den_overflow P((void));

/*
 * rational constants
 */
Rational  r_zero = RINIT(0,0,1);
Rational  r_one  = RINIT(1,0,1);
Rational  r_max =  RINIT(0x7fff,0,1);
Rational  r_min =  RINIT(-0x8000,0,1);

/*
 * add two rational numbers
 */
void
radd(a, b, y)
Rational  *a, *b;
Rational  *y;  /* result (possibly the same pointer as a or b) */
{
    unsigned long  u, v, w, w2;

    y->intg = a->intg + b->intg;

    if( a->den == b->den ) {
	u = (long)a->num + b->num;
	v = a->den;
    } else {
	w = gcd((unsigned long)a->den, (unsigned long)b->den);
	u = ((long)a->num * b->den + (long)b->num * a->den) / w; 
	v = ((long)a->den * b->den) / w;
	if( v >= 32768 ) {
	    w2 = gcd(u, v);
	    u /= w2;
	    v /= w2;
	    if( v >= 32768 ) {
		den_overflow();
		float_to_rational(rational_to_float(a) + rational_to_float(b),
				  y);
		return;
	    }
	}
    }

    if( u >= v ) {
	u -= v;
	y->intg++;
    }
    y->num = u;
    y->den = v;
    y->type = O_RATIONAL;
}

/*
 * subtract two rational numbers
 */
void
rsub(a, b, y)
Rational  *a, *b;
Rational  *y;  /* result (possibly the same pointer as a or b) */
{
    long  u, w;
    unsigned long  v, w2;

    y->intg = a->intg - b->intg;

    if( a->den == b->den ) {
	u = (long)a->num - b->num;
	v = a->den;
	if( u < 0 ) {
	    u += v;
	    y->intg--;
	}
    } else {
	w = gcd((unsigned long)a->den, (unsigned long)b->den);
	u = ((long)a->num * b->den - (long)b->num * a->den) / w; 
	v = (a->den * b->den) / w;
	if( u < 0 ) {
	    u += v;
	    y->intg--;
	}
	if( v >= 32768 ) {
	    w2 = gcd(u, v);
	    u /= w2; 
	    v /= w2;
	    if( v >= 32768 ) {
		den_overflow();
		float_to_rational(rational_to_float(a) - rational_to_float(b),
				  y);
		return;
	    }
	}
    }
    y->num = u;
    y->den = v;
    y->type = O_RATIONAL;
}

/*
 * multiply two rational numbers
 */
void
rmult(a, b, y)
Rational  *a, *b;
Rational  *y;  /* result (possibly the same pointer as a or b) */
{
    int  neg;
    unsigned long  d1, d2, u, v, w1, w2, w3, w4;
    
    if( a->intg < 0 ) {
	d1 = - (long)a->intg * a->den - a->num;
	neg = 1;
    } else {
	d1 = (long)a->intg * a->den + a->num;
	neg = 0;
    }
    if( b->intg < 0 ) {
	d2 = - (long)b->intg * b->den - b->num;
	neg ^= 1;
    } else {
	d2 = (long)b->intg * b->den + b->num;
    }

    w1 = gcd(d1, (unsigned long)b->den);
    d1 /= w1;
    v = a->den * (b->den / w1);
    if( v < 32768 ) {
	u = d1 * d2;
    } else {
	w2 = gcd(d2, (unsigned long)a->den);
	d2 /= w2;
	v /= w2;
	if( v < 32768 ) {
	    u = d1 * d2;
	} else {
	    w3 = gcd(d1, v);
	    v /= w3;
	    w4 = gcd(d2, v);
	    v /= w4;
	    if( v < 32768 ) {
		u = (d1 / w3) * (d2 / w4);
	    } else {
		den_overflow();
		float_to_rational(rational_to_float(a) * rational_to_float(b),
				  y);
		return;
	    }
	}
    }

    y->den = v;
    if( neg ) {
	unsigned long  t = u % v;
	if( t == 0 ) {
	    y->intg = - (u / v); 
	    y->num = 0;
	} else {
	    y->intg = - (u / v) - 1;
	    y->num = v - t;
	}
    } else {
	y->intg = u / v;
	y->num = u % v;
    }
    y->type = O_RATIONAL;
}

/*
 * multiply a rational number by an integer
 */
void
rimult(a, b, y)
Rational  *a;
long  b;
Rational  *y;  /* result (possibly the same pointer as a) */
{
    int  neg;
    unsigned long  d1, u, v;
    
    if( a->intg < 0 ) {
	d1 = - (long)a->intg * a->den - a->num;
	neg = 1;
    } else {
	d1 = (long)a->intg * a->den + a->num;
	neg = 0;
    }
    if( b < 0 ) {
	b = -b;
	neg ^= 1;
    } 

    u = d1 * b;
    y->den = v = a->den;

    if( neg ) {
	unsigned long  t = u % v;
	if( t == 0 ) {
	    y->intg = - (u / v); 
	    y->num = 0;
	} else {
	    y->intg = - (u / v) - 1;
	    y->num = v - t;
	}
    } else {
	y->intg = u / v;
	y->num = u % v;
    }
    y->type = O_RATIONAL;
}

/*
 * divide two rational numbers
 *   returns non-zero if divisor is 0.
 */
int
rdiv(a, b, y)
Rational  *a, *b;
Rational  *y;  /* result (possibly the same pointer as a or b) */
{
    int  neg;
    unsigned long  d1, d2, u, v, w1, w2, w3, w4;
    short  s1, s2;
    
    if( a->intg < 0 ) {
	d1 = - (long)a->intg * a->den - a->num;
	neg = 1;
    } else {
	d1 = (long)a->intg * a->den + a->num;
	neg = 0;
    }
    if( b->intg < 0 ) {
	d2 = - (long)b->intg * b->den - b->num;
	neg ^= 1;
    } else {
	d2 = (long)b->intg * b->den + b->num;
    }

    if( d2 == 0 )  return 1;

    w1 = gcd(d1, d2);
    d1 /= w1;
    d2 /= w1;
    s1 = a->den;
    s2 = b->den;
    if( d2 < 32768 && (v = s1 * d2) < 32768 ) {
	u = d1 * s2;
    } else {
	w2 = gcd((unsigned long)s1, (unsigned long)s2);
	s1 /= w2;
	s2 /= w2;
	if( d2 < 32768 && (v = s1 * d2) < 32768 ) {
	    u = d1 * s2;
	} else {
	    w3 = gcd(d1, (unsigned long)s1);
	    w4 = gcd(d2, (unsigned long)s2);
	    s1 /= w3;
	    d2 /= w4;
	    if( d2 < 32768 && (v = s1 * d2) < 32768 ) {
		u = (d1 / w3) * (s2 / w4);
	    } else {
		den_overflow();
		float_to_rational(rational_to_float(a) / rational_to_float(b),
				  y);
		return 0;
	    }
	}
    }

    y->den = v;
    if( neg ) {
	unsigned long  t = u % v;
	if( t == 0 ) {
	    y->intg = - (u / v); 
	    y->num = 0;
	} else {
	    y->intg = - (u / v) - 1;
	    y->num = v - t;
	}
    } else {
	y->intg = u / v;
	y->num = u % v;
    }
    y->type = O_RATIONAL;
    return 0;
}

/*
 * divide a rational number by an integer
 *   returns non-zero if divisor is 0.
 */
int
ridiv(a, b, y)
Rational  *a;
long  b;
Rational  *y;  /* result (possibly the same pointer as a) */
{
    int  neg;
    unsigned long  d1, u, v, w1, w3;
    short  s1;
    
    if( a->intg < 0 ) {
	d1 = - (long)a->intg * a->den - a->num;
	neg = 1;
    } else {
	d1 = (long)a->intg * a->den + a->num;
	neg = 0;
    }
    if( b < 0 ) {
	b = -b;
	neg ^= 1;
    } 

    if( b == 0 )  return 1;

    w1 = gcd(d1, b);
    d1 /= w1;
    b /= w1;
    s1 = a->den;
    if( b < 32768 && (v = s1 * b) < 32768 ) {
	u = d1;
    } else {
	w3 = gcd(d1, (unsigned long)s1);
	s1 /= w3;
	if( b < 32768 && (v = s1 * b) < 32768 ) {
	    u = d1 / w3;
	} else {
	    den_overflow();
	    float_to_rational(rational_to_float(a) / b, y);
	    return 0;
	}
    }

    y->den = v;
    if( neg ) {
	unsigned long  t = u % v;
	if( t == 0 ) {
	    y->intg = - (u / v); 
	    y->num = 0;
	} else {
	    y->intg = - (u / v) - 1;
	    y->num = v - t;
	}
    } else {
	y->intg = u / v;
	y->num = u % v;
    }
    y->type = O_RATIONAL;
    return 0;
}

/*
 * calculate the rational-number remainder
 *   The result is (a - n * b), where n is the quotient (a / b)
 *   rounded towards zero to an integer.
 *   Returns non-zero if divisor is 0.
 */
int
rmod(a, b, y)
Rational  *a, *b;
Rational  *y;  /* result (possibly the same pointer as a or b) */
{
    Rational  n;

    if( rdiv(a, b, &n) )  return 1;
    if( n.intg < 0 && n.num != 0 )  n.intg++;
    n.num = 0;
    n.den = 1;
    rmult(&n, b, &n);
    rsub(a, &n, y);
    return 0;
}

/*
 * normalize a rational number
 */
void
rnorm(a)
Rational  *a;  /* source & result */
{
    unsigned long  w;

    a->intg += a->num / a->den;
    a->num = a->num % a->den;
    w = gcd((unsigned long)a->num, (unsigned long)a->den);
    a->num /= w;
    a->den /= w;
}

void
rnorm2(a, r)
Rational  *a;  /* source */
Rational  *r;  /* result */
{
    unsigned long  w;

    r->intg = a->intg + a->num / a->den;
    r->num = a->num % a->den;
    w = gcd((unsigned long)r->num, (unsigned long)a->den);
    r->num = a->num / w;
    r->den = a->den / w;
}

/*
 * create a rational number from an independent numerator and denominator
 */
void
rset(num, den, a)
long  num;
int  den;
Rational  *a;  /* result */
{
    a->den = den;
    if( num >= 0 ) {
	a->intg = num / den;
	a->num = num % den;
    } else {
	a->intg = num / den;
	a->num = -num % den;
	if( a->num != 0 ) {
	    a->intg--;
	    a->num = den - a->num;
	}
    }
    a->type = O_RATIONAL;
}

/*
 * negate a rational number
 */
void
rneg(a)
Rational  *a;  /* source & result */
{
   if( a->num == 0 ) {
       a->intg = - a->intg;
   } else {
       a->intg = - a->intg - 1;
       a->num = a->den - a->num;
   }
}

/*
 * convert integer to rational number
 */
void
int_to_rational(i, r)
long  i;
Rational  *r; /* result */
{
    if( i >= 0 ) {
	r->intg = i / (IRES * 4);
	r->num = (i % (IRES * 4)) * (LDEN / (IRES * 4)); 
    } else {
	r->intg = - (-i - 1) / (IRES * 4) - 1;
	r->num = ((IRES * 4 - 1) 
		  - (-i - 1) % (IRES * 4)) * (LDEN / (IRES * 4)); 
    }
    r->den = LDEN;
    r->type = O_RATIONAL;
}

/*
 * convert float number to rational number
 */
void
float_to_rational(x, r)
double  x;
Rational  *r; /* result */
{
    int   den1 = 1, den2 = 0, num1 = 0, num2 = 1;
    long  a, tmp;

    x *= 1.0 / (IRES * 4);

    if( x >= 32767.0 ) {
	*r = r_max;
	return;
    } else if( x <= -32768.0 ) {
	*r = r_min;
	return;
    }

    r->intg = floor(x);
    x -= r->intg;	/* now, 0 <= x < 1 */

    /* convert x to rational by expanding it into a continued fraction */
    while( x > 1.0/32768.0 ) {
	x = 1.0 / x;
	a = floor(x);
	x -= a;

	tmp = a * den1 + den2;
	if( tmp >= 32768 )  break;
	den2 = den1;
	den1 = tmp;

	tmp = a * num1 + num2;
	num2 = num1;
	num1 = tmp;
    }

    if( num1 == den1 ) {  /* in case of x == 0.9999... */
	num1 = 0;
	r->intg++;
    }
    r->num = num1;
    r->den = den1;
    r->type = O_RATIONAL;
}

/*
 * convert a rational number to a string
 *   Returned string points to a static storage area.
 */
char *
rstring(a)
Rational  *a;
{
    Rational  tmp;
    static char  buf[21];

#ifdef DO_NOT_NORMALIZE
    tmp = *a;
#else
    rnorm2(a, &tmp);
#endif
    sprintf(buf, "%d+%d/%d", tmp.intg, tmp.num, tmp.den);

    return buf;
}


#ifndef BINARY_GCD
/*
 * calculate G.C.D. by Euclid's algorithm
 */
static unsigned long 
gcd(u, v)
unsigned long  u, v;
{
    unsigned long  tmp;
    
    if( v > u ) {
	tmp = u; u = v; v = tmp;
    }
    for(;;) {
	if( v == 0 )  return u;
	else if( v == 1 )  return v;
	else {
	    tmp = v;
	    v = u % v;
	    u = tmp;
	}
    }
}

#else

/* The binary GCD algorithm shown in "The Art of Computer Programming
   Volume II" by D. Knuth. */
static unsigned long 
gcd(u, v)
unsigned long  u, v;
{
    int  k;
    long  t;

    if( u == 0 )  return v;
    else if( v == 0 )  return u;

    for( k = 0; ((u | v) & 1) == 0; k++ ) { 
	u >>= 1; v >>= 1; 
    } 

    if( u & 1 ) {
	t = -v;
    } else {
	t = u >> 1;
    }

    for(;;) {
	while( (t & 1) == 0 )  t >>= 1;

	if( t > 0 ) {
	    u = t;
	} else {
	    v = -t;
	}

	if( (t = u - v) == 0 )  break;
	t >>= 1;
    }

    return(u << k);
}
#endif

/*
 * warning output on the overflow of denominator
 */
static void
den_overflow()
{
    warn(cur_srcpos, 
	 "Rational number rounded due to denominator overflow", 
	 "有理数の最大精度を越えたため丸めが行われます"); 
}

/*
 * tester
 */
#ifdef RATIONAL_SELF_TEST
main()
{
    Rational  x, y, r;
    double  d;
    long  i;

    fprintf(stderr, "d = ");  scanf("%lf", &d);
    float_to_rational(d, &r); 
    printf("%d+%d/%d\n", r.intg, r.num, r.den);

    fprintf(stderr, "i = ");  scanf("%d", &i);
    ridiv(&r, i, &r);
    printf("%d+%d/%d\n", r.intg, r.num, r.den);
    
    fprintf(stderr, "a1 = ");  scanf("%d", &x.intg);
    fprintf(stderr, "b1 = ");  scanf("%d", &x.num);
    fprintf(stderr, "c1 = ");  scanf("%d", &x.den);

    fprintf(stderr, "a2 = ");  scanf("%d", &y.intg);
    fprintf(stderr, "b2 = ");  scanf("%d", &y.num);
    fprintf(stderr, "c2 = ");  scanf("%d", &y.den);

    rnorm(&x);
    printf("%d+%d/%d\n", x.intg, x.num, x.den);
    radd(&x, &y, &r);
    printf("+:  %d+%d/%d\n", r.intg, r.num, r.den);
    rsub(&x, &y, &r);
    printf("-:  %d+%d/%d\n", r.intg, r.num, r.den);
    rmult(&x, &y, &r);
    printf("*:  %d+%d/%d\n", r.intg, r.num, r.den);
    rdiv(&x, &y, &r);
    printf("/:  %d+%d/%d\n", r.intg, r.num, r.den);
}
#endif
