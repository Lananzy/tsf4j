/*
**  @file $RCSfile: tstdlib.c,v $
**  general description of this module
**  $Id: tstdlib.c,v 1.2 2009/01/23 09:35:03 kent Exp $
**  @author $Author: kent $
**  @date $Date: 2009/01/23 09:35:03 $
**  @version $Revision: 1.2 $
**  @note Editor: Vim 6.1, Gcc 4.0.1, tab=4
**  @note Platform: Linux
*/
#include "pal/tstdlib.h"

#ifdef WIN32

#include <math.h>
#include <stdlib.h>

#define        _RAND48_SEED_0  (0x330e)
#define        _RAND48_SEED_1  (0xabcd)
#define        _RAND48_SEED_2  (0x1234)
#define        _RAND48_MULT_0  (0xe66d)
#define        _RAND48_MULT_1  (0xdeec)
#define        _RAND48_MULT_2  (0x0005)
#define        _RAND48_ADD     (0x000b)

struct _rand48
{
	unsigned short aushSeed[3];
	unsigned short aushMult[3];
	unsigned short aushAdd;
};


static struct _rand48 s_stRand48 = {
	_RAND48_SEED_0,
	_RAND48_SEED_1, 
	_RAND48_SEED_2, 
	_RAND48_MULT_0, 
	_RAND48_MULT_1, 
	_RAND48_MULT_2, 
	_RAND48_ADD,
};

#define __rand48_seed			s_stRand48.aushSeed
#define __rand48_mult			s_stRand48.aushMult
#define __rand48_add			s_stRand48.aushAdd

longlong atoll(const char *nptr)
{
	return strtoll(nptr, (char**)0, 10);
}

longlong atoq(const char *nptr)
{
	return strtoll(nptr, (char**)0, 10);
}

void dorand48( unsigned short xseed[3] )
{
	unsigned long accu;
	unsigned short temp[2];
	
	accu = (unsigned long) __rand48_mult[0] * (unsigned long) xseed[0] +
		(unsigned long) __rand48_add;
	temp[0] = (unsigned short) accu;     /* lower 16 bits */
	accu >>= sizeof(unsigned short) * 8;
	accu += (unsigned long) __rand48_mult[0] * (unsigned long) xseed[1] +
		(unsigned long) __rand48_mult[1] * (unsigned long) xseed[0];
	temp[1] = (unsigned short) accu;     /* middle 16 bits */
	accu >>= sizeof(unsigned short) * 8;
	accu += __rand48_mult[0] * xseed[2] + __rand48_mult[1] * xseed[1] + __rand48_mult[2] * xseed[0];
	xseed[0] = temp[0];
	xseed[1] = temp[1];
	xseed[2] = (unsigned short) accu;
}

double erand48( unsigned short xseed[3] )
{
	dorand48(xseed);
	return ldexp((double) xseed[0], -48) +
		ldexp((double) xseed[1], -32) +
		ldexp((double) xseed[2], -16);
}

double drand48(void)
{
	return erand48(__rand48_seed);
}

long jrand48( unsigned short xseed[3] )
{
	dorand48(xseed);
	return ((long) xseed[2] << 16) + (long) xseed[1];
}

void lcong48( unsigned short p[7] )
{
	__rand48_seed[0] = p[0];
	__rand48_seed[1] = p[1];
	__rand48_seed[2] = p[2];
	__rand48_mult[0] = p[3];
	__rand48_mult[1] = p[4];
	__rand48_mult[2] = p[5];
	__rand48_add = p[6];
}

long lrand48(void)
{
	dorand48(__rand48_seed);
	return (long)((unsigned long) __rand48_seed[2] << 15) +
		((unsigned long) __rand48_seed[1] >> 1);
}

long mrand48(void)
{
	dorand48(__rand48_seed);
	return ((long) __rand48_seed[2] << 16) + (long) __rand48_seed[1];
}

long nrand48( unsigned short xseed[3] )
{
	dorand48 (xseed);
	return (long)((unsigned long) xseed[2] << 15) +
		((unsigned long) xseed[1] >> 1);
}

unsigned short * seed48( unsigned short xseed[3] )
{
	static unsigned short sseed[3];
	
	sseed[0] = __rand48_seed[0];
	sseed[1] = __rand48_seed[1];
	sseed[2] = __rand48_seed[2];
	__rand48_seed[0] = xseed[0];
	__rand48_seed[1] = xseed[1];
	__rand48_seed[2] = xseed[2];
	__rand48_mult[0] = _RAND48_MULT_0;
	__rand48_mult[1] = _RAND48_MULT_1;
	__rand48_mult[2] = _RAND48_MULT_2;
	__rand48_add = _RAND48_ADD;
	
	return sseed;
}


void srand48( long seed )
{
	__rand48_seed[0] = _RAND48_SEED_0;
	__rand48_seed[1] = (unsigned short) seed;
	__rand48_seed[2] = (unsigned short) ((unsigned long)seed >> 16);
	__rand48_mult[0] = _RAND48_MULT_0;
	__rand48_mult[1] = _RAND48_MULT_1;
	__rand48_mult[2] = _RAND48_MULT_2;
	__rand48_add = _RAND48_ADD;
}

#endif /* WIN32 */
