/*
**  @file $RCSfile: tconndkeymgr_conn.c,v $
**  general description of this module
**  $Id: tconndkeymgr_conn.c,v 1.1 2009/01/23 01:51:45 sean Exp $
**  @author $Author: sean $
**  @date $Date: 2009/01/23 01:51:45 $
**  @version $Revision: 1.1 $
**  @note Editor: Vim 6.1, Gcc 4.0.1, tab=4
**  @note Platform: Linux
*/

#include <stdio.h>
#include "pal/pal.h"

#include "tlog/tlog.h"
#include "tloghelp/tlogload.h"

#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <sys/time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "pal/queue.h"
#include "tconndkeymgr_conn.h"

#define TCP_TIMEOUT 600

#ifdef HAVE_SETFD
#define FD_CLOSEONEXEC(x) do { \
        if (fcntl(x, F_SETFD, 1) == -1) \
                printf("fcntl(%d, F_SETFD)", x); \
} while (0)
#else
#define FD_CLOSEONEXEC(x)
#endif

extern LPTLOGCATEGORYINST g_pstLogCat;

static int fdevent_event_check(int socket)
{
	int ret = 0, count = 0;
	fd_set set;
	struct timeval tv;
	
	if (socket < 0)
	{
		return -1;
	}
	
	tv.tv_sec = 0;
	tv.tv_usec = 500000;
	
	FD_ZERO(&set);
	FD_SET (socket, &set);
    
	while((ret = select(socket+1, NULL, &set, NULL, &tv)) == 0) 
	{
		if(ret < 0) 
		{
			if (errno == EINTR)
			{
				continue;
			}
			
			fprintf(stderr, "ERROR:%s\n", strerror(errno));
			return -1;
		}
		else if(ret > 0) break;
		else if(count > 30) 
		{
			return 0;
		}
		
		count++;
		FD_ZERO(&set);
		FD_SET (socket, &set);
		tv.tv_sec = 0;
		tv.tv_usec = 500000;
	}
	
	return 1;
}

static int make_socket(int should_bind, int type, const char * const cp, u_short port)
{
	struct hostent *he;
	struct sockaddr_in address;
	int fd, on = 1, r;
	int serrno;
	
	socklen_t err_len;
	int err;
	
	address.sin_family 	= AF_INET;
	address.sin_port 	= htons(port);
	
	if (NULL == (he = gethostbyname(cp)))
	{
		return -1;
	}

	if (he->h_addrtype != AF_INET)
	{
		return -1;
	}

	if (he->h_length != sizeof(struct in_addr))
	{
		return -1;
	}
	
	memcpy(&(address.sin_addr.s_addr), he->h_addr_list[0], he->h_length);
	
	/* Create listen socket */
	fd = socket(AF_INET, type, 0);
	if (fd == -1)
	{
		return(-1);
	}
	
	/* set nonblock  */
	if (fcntl(fd, F_SETFL, O_NONBLOCK) == -1) 
	{
		goto out;
	}
	/* set  close-exec*/
	FD_CLOSEONEXEC(fd);
	
	setsockopt(fd, SOL_SOCKET, SO_KEEPALIVE, (void *)&on, sizeof(on));
	setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (void *)&on, sizeof(on));
	
	if (should_bind)
	{
		r = bind(fd, (struct sockaddr *)&address, sizeof(address));
	}
	else
	{
		r = connect(fd, (struct sockaddr *)&address, sizeof(address));
	}
	
	if (r == -1) 
	{
		if (errno != EINPROGRESS || should_bind) 
		{
			goto out;
		}
		
		/* check for non-block mode */
		if (fdevent_event_check(fd)<0)
		{
			goto out;
		}
		
		err_len = sizeof(int);
		r =  getsockopt(fd,SOL_SOCKET,SO_ERROR,&err,&err_len);
		
		if(err) 
		{
			goto out;
		}
	}
	
	return fd;
	
out:
	serrno = errno;
	close(fd);
	errno = serrno;
	return (-1);
}

static int accept_connect(SERVER *pstSrv, int iSock, uint32_t iAddr)
{
	LPCONNECTION pstConn;

	(void)iAddr;
	if (iSock>= 1024 || iSock < 0) 
	{
		tlog_log(g_pstLogCat, TLOG_PRIORITY_FATAL, 0,0, "Too many connections or fd==%d\n", iSock);
		if (iSock > 0)
		{
			close (iSock);
		}
		
		return -1;
	}
	
	/* set nonblock */
	fcntl(iSock, F_SETFL, fcntl(iSock, F_GETFL) | O_NONBLOCK);
	pstConn = (LPCONNECTION)malloc(sizeof(CONNECTION));
	if (0 == pstConn)
	{
		tlog_log(g_pstLogCat, TLOG_PRIORITY_FATAL, 0,0, "%s\n", strerror(errno));
		exit(-1);
	}
	
	memset (pstConn, 0x00, sizeof(*pstConn));
	pstConn->iSock		= iSock;
	pstConn->iTimeout	= time(NULL) + TCP_TIMEOUT;
	
	TAILQ_INSERT_TAIL(&pstSrv->stConnQueue, pstConn, next);
	
	return 0;
}

static int read_request(SERVER *pstSrv, CONNECTION *pstConn)
{
	(void)pstSrv;

	int iBytesRead = 0;
	memset(pstConn->szBuffRead, 0x00, sizeof(pstConn->szBuffRead));
	
again:
	iBytesRead=read(pstConn->iSock, pstConn->szBuffRead, 4096);
	printf ("%s\n", pstConn->szBuffRead);
	
	if (iBytesRead == -1)
	{
		if (errno == EWOULDBLOCK || errno == EAGAIN)
		{
			return 0;
		}
		else if (errno == EINTR) 
		{
			goto again;
		}
		else
		{
			return -1;
		}
	}
	else if (iBytesRead==0)
	{
		/* peer has close socket */
		return -2;
	}
	
	pstConn->iBytesRead = iBytesRead;
	//return process_console_dispatch(pstSrv, pstConn);
	return 0;
}

static void release_connect(SERVER *pstSrv, CONNECTION *pstConn) 
{
	FD_CLR(pstConn->iSock, &pstSrv->master_set);
	close(pstConn->iSock);
	
	TAILQ_REMOVE(&pstSrv->stConnQueue, pstConn, next);
	free (pstConn);
}

int tconndkeymgr_connection_init(SERVER *pstSrv)
{
	pstSrv->srvfd = make_socket (1, SOCK_STREAM, pstSrv->pstConfinst->szBindAddr, pstSrv->pstConfinst->wBindPort);
	if (-1 == pstSrv->srvfd)	
	{
		return -1;
	}
	
	if (-1 == listen(pstSrv->srvfd, 128 * 8))
	{		
		return -1;	
	}
	
	FD_ZERO(&pstSrv->master_set);
	FD_SET(pstSrv->srvfd, &pstSrv->master_set);
	
	TAILQ_INIT(&pstSrv->stConnQueue);
	TAILQ_INIT(&pstSrv->stTcndQueue);
	
	return 0;
}

int tconndkeymgr_connection_poll (SERVER *pstSrv)
{
	int iSock = 0; 
	int nfds = 0;
	int iRet = 0;
	
	CONNECTION *pstConn;
	
	fd_set sel_set;
	struct sockaddr_in addr;
	socklen_t len;
	
	time_t ticks;
	struct timeval timeout;

again:	
	timeout.tv_sec =0;
	timeout.tv_usec=50000;
	
	sel_set = pstSrv->master_set;
	nfds = select(1024, &sel_set, NULL, NULL, &timeout);	
	if(nfds < 0)
	{
		if (errno == EINTR)
		{
			goto again;
		}
		
		return -1;
	}
	
	if (FD_ISSET(pstSrv->srvfd, &sel_set))
	{
		nfds--;
		iSock = accept(pstSrv->srvfd, (struct sockaddr *)&addr,(len=sizeof(addr),&len));
		if (iSock < 0) 
		{
			if (errno == EWOULDBLOCK || errno == EAGAIN || errno == EINTR)
			{
				goto skip;
			}
			else 
			{
				/* big error*/
				return -1;
			}
		}
		
		if (accept_connect(pstSrv, iSock, addr.sin_addr.s_addr) == -1) 
		{
			goto skip;
		}
		
		FD_SET(iSock, &pstSrv->master_set);
	}
	
skip:
	time(&ticks);

	TAILQ_FOREACH(pstConn, &pstSrv->stConnQueue, next)
	{
		if (nfds && FD_ISSET(pstConn->iSock, &sel_set))
		{
			nfds--;
			iRet = read_request(pstSrv, pstConn);
			
			if (iRet < 0)
			{
				tlog_log(g_pstLogCat, TLOG_PRIORITY_ERROR, 0,0, "read_request error:%s\n", strerror(errno));
				release_connect(pstSrv, pstConn);
			}
			else 
			{
				// update timeout
				pstConn->iTimeout = ticks+TCP_TIMEOUT;
			}
		}
		else
		{
			// timeout
			if (pstConn->iTimeout <= (unsigned long)ticks)
			{
				release_connect (pstSrv, pstConn);
			}
		}
	}
	
	return 0;
}

void tconndkeymgr_connection_free(SERVER *pstSrv)
{
	CONNECTION *pstConn;
	TAILQ_FOREACH(pstConn, &pstSrv->stConnQueue, next)
	{
		release_connect(pstSrv, pstConn);
	}
}

