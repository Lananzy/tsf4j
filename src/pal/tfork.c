/*
**  @file $RCSfile: tfork.c,v $
**  general description of this module
**  $Id: tfork.c,v 1.1.1.1 2008/05/28 07:35:00 kent Exp $
**  @author $Author: kent $
**  @date $Date: 2008/05/28 07:35:00 $
**  @version $Revision: 1.1.1.1 $
**  @note Editor: Vim 6.1, Gcc 4.0.1, tab=4
**  @note Platform: Linux
*/

#include "pal/tfork.h"

#ifdef WIN32

pid_t fork(void)
{
	return 0;
}

#endif /* WIN32 */
