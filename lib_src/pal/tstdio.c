/*
**  @file $RCSfile: tstdio.c,v $
**  general description of this module
**  $Id: tstdio.c,v 1.2 2009/01/23 09:35:03 kent Exp $
**  @author $Author: kent $
**  @date $Date: 2009/01/23 09:35:03 $
**  @version $Revision: 1.2 $
**  @note Editor: Vim 6.1, Gcc 4.0.1, tab=4
**  @note Platform: Linux
*/

#include "pal/tstdio.h"

#ifdef WIN32

#include <stdio.h>

int fpurge(FILE *stream)
{
	return fflush(stream);
}

void  __fpurge(FILE *stream)
{
	fflush(stream);
}

void setbuffer(FILE *a_fpStream, char *a_pszBuf, size_t a_iSize)
{
	setvbuf(a_fpStream, a_pszBuf, a_pszBuf ? _IOFBF : _IONBF, a_iSize);
}

void setlinebuf(FILE *a_fpStream)
{
	setvbuf(a_fpStream, (char *)NULL, _IOLBF, 0);
}

#else /* WIN32 */

#endif /* WIN32 */
