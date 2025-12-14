/*
**  @file $RCSfile: tconvert.c,v $
**  general description of this module
**  $Id: tconvert.c,v 1.1 2008/07/29 10:38:06 steve Exp $
**  @author $Author: steve $
**  @date $Date: 2008/07/29 10:38:06 $
**  @version $Revision: 1.1 $
**  @note Editor: Vim 6.1, Gcc 4.0.1, tab=4
**  @note Platform: Linux
*/

#include <assert.h>
#include <limits.h>
#include <stdlib.h>
#include <errno.h>

#include "comm/tconvert.h" 

long tconvert_get_bytesl(const char *a_szStr, char** a_ppszEnd, int a_iRadix)
{
	char* szEnd;
	long lRet;
	long lMulti;

	assert(a_szStr);

	lRet = strtol(a_szStr, (char **) &szEnd, a_iRadix);

	if ( (lRet <= 0) || (LONG_MAX==lRet && ERANGE==errno) )
		return -1;

	switch (szEnd[0]) 
	{
	case 'K': 
	case 'k':
		lMulti	=	1024;
		break;
	case 'M':
	case 'm':
		lMulti	=	1024 * 1024;
		break;
	case 'G':
	case 'g':
		lMulti	= 1024 * 1024 * 1024;
		break;
	default:
		errno	=	EINVAL;

		return -1;
	}

	if( 'b'!=szEnd[1] && 'B'!=szEnd[1] )
	{
		errno	=	EINVAL;
		return -1;
	}

	if( LONG_MAX/lMulti < lRet )
	{
		errno	=	ERANGE;
		return -1;
	}

	lRet	*=	lMulti;

	if( a_ppszEnd ) *a_ppszEnd	=	szEnd+2;

	return lRet;
}

long tconvert_get_daysl(const char *a_szStr, char** a_ppszEnd, int a_iRadix)
{
	char* szEnd;
	long lRet;
	long lMulti;

	assert(a_szStr);

	lRet = strtol(a_szStr, (char **) &szEnd, a_iRadix);

	if ( (lRet <= 0) || (LONG_MAX==lRet && ERANGE==errno) )
		return -1;

	switch (szEnd[0]) 
	{
	case 'Y': 
	case 'y':
		lMulti	=	365;
		break;
	case 'M':
	case 'm':
		lMulti	=	30;
		break;
	case 'D':
	case 'd':
		lMulti	= 1;
		break;
	default:
		errno	=	EINVAL;

		return -1;
	}

	if( LONG_MAX/lMulti < lRet )
	{
		errno	=	ERANGE;
		return -1;
	}

	lRet	*=	lMulti;

	if( a_ppszEnd ) *a_ppszEnd	=	szEnd+2;

	return lRet;
}


