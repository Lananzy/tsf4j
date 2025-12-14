/*
**  @file $RCSfile: tuio.c,v $
**  general description of this module
**  $Id: tuio.c,v 1.1.1.1 2008/05/28 07:35:00 kent Exp $
**  @author $Author: kent $
**  @date $Date: 2008/05/28 07:35:00 $
**  @version $Revision: 1.1.1.1 $
**  @note Editor: Vim 6.1, Gcc 4.0.1, tab=4
**  @note Platform: Linux
*/

#ifdef WIN32

#include "pal/terrno.h"
#include "pal/tos.h"
#include "pal/tuio.h"

ssize_t readv(int fd, const struct iovec *vector, int count)
{
	int i;
	ssize_t iRead;
	ssize_t iTotal;
	const struct iovec* p;

	iTotal	=	0;

	for(i=0; i<count; i++)
	{
		p	=	vector + i;

		iRead	=	read(fd, p->iov_base, p->iov_len);

		if( -1==iRead )
			break;

		iTotal	+=	iRead;
	}

	return iTotal;
}

ssize_t writev(int fd, const struct iovec *vector, int count)
{
	int i;
	ssize_t iWrite;
	ssize_t iTotal;
	const struct iovec* p;

	iTotal	=	0;

	for(i=0; i<count; i++)
	{
		p	=	vector + i;

		iWrite	=	write(fd, p->iov_base, p->iov_len);

		if( -1==iWrite )
			break;

		iTotal	+=	iWrite;
	}

	return iTotal;
}

#endif /* WIN32 */
