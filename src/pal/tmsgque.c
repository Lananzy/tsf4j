/*
**  @file $RCSfile: tmsgque.c,v $
**  general description of this module
**  $Id: tmsgque.c,v 1.1.1.1 2008/05/28 07:35:00 kent Exp $
**  @author $Author: kent $
**  @date $Date: 2008/05/28 07:35:00 $
**  @version $Revision: 1.1.1.1 $
**  @note Editor: Vim 6.1, Gcc 4.0.1, tab=4
**  @note Platform: Linux
*/

#include "pal/tos.h"
#include "pal/ttypes.h"
#include "pal/tipc.h"
#include "pal/tmsgque.h"

#ifdef WIN32

int msgget(key_t key, int msgflg)
{
	return -1;
}

int msgctl(int msqid, int cmd, struct msqid_ds *buf)
{
	return -1;
}

int msgsnd(int msqid, struct msgbuf *msgp, size_t msgsz, int msgflg)
{
	return -1;
}

ssize_t msgrcv(int msqid, struct msgbuf *msgp, size_t msgsz, long msgtyp, int msgflg)
{
	return -1;
}

#endif /* WIN32 */

