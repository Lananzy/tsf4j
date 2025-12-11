/*
**  @file $RCSfile: tsignal.c,v $
**  general description of this module
**  $Id: tsignal.c,v 1.1.1.1 2008/05/28 07:35:00 kent Exp $
**  @author $Author: kent $
**  @date $Date: 2008/05/28 07:35:00 $
**  @version $Revision: 1.1.1.1 $
**  @note Editor: Vim 6.1, Gcc 4.0.1, tab=4
**  @note Platform: Linux
*/

#include "pal/tos.h"
#include "pal/tstdio.h"
#include "pal/tsignal.h"
#include "pal/terrno.h"


#ifdef WIN32

#pragma warning(disable:4996)

char *sys_siglist[NSIG+1] = {
/* 0*/  "Zero signal",
/* 1*/  "Hangup",
/* 2*/  "Interrupt",
/* 3*/  "Quit",
/* 4*/  "Illegal instruction",
/* 5*/  "Trap",
/* 6*/  "Abort termination",
/* 7*/  "Bus error",
/* 8*/  "Floating-point exception",
/* 9*/  "Kill",
/*10*/  "User-defined signal 1",
/*11*/  "Segmentation violation",
/*12*/  "User-defined signal 2",
/*13*/  "Broken pipe",
/*14*/  "Alarm clock",
/*15*/  "Software termination signal",
/*16*/  "Stack fault",
/*17*/  "Child signal",
/*18*/  "Continue signal",
/*19*/  "Stop signal",
/*20*/  "Terminal stop signal",
/*21*/  "Background read",
/*22*/  "Background write",
/*23*/  "Urgent data signal",
/*24*/  "CPU limit exceeded",
/*25*/  "File size limit exceeded",
/*26*/  "Virtual timer expired",
/*27*/  "Profiling timer expired",
/*28*/  "SIGWINCH",
/*29*/  "Pollable event",
/*30*/  "Power fault",
/*31*/  "SIGUNUSED",
0
};

/* A counter to know when to re-initialize the static sets.  */
static int __sigprocmask_inited = 0;

/* Which signals are currently blocked (initially none).  */
static sigset_t __current_mask;

/* Which signals are pending (initially none).  */
static sigset_t __sigprocmask_pending_signals;

/* Previous handlers to restore when the blocked signals are unblocked.  */
static TSIGDESC s_astSigDesc[SIGMAX];

int kill_one_proc_i(pid_t pid, int sig)
{
	/*
	unsigned msg;
	*/

	HANDLE hProc = NULL;
	int iRet=0;

	if( 0==sig )	/* just test whether the process is exist. */
	{
		hProc	=	OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, pid );

		if( INVALID_HANDLE_VALUE==hProc || NULL==hProc )
		{
			errno	=	ESRCH;
			return -1;
		}
		else
		{
			CloseHandle(hProc);
			return 0;
		}
	}

	if (pid==getpid()) return traise(sig);

	if (sig==SIGKILL)
	{
		hProc	=	OpenProcess(PROCESS_TERMINATE, FALSE, pid);

		if (hProc==INVALID_HANDLE_VALUE || hProc==NULL)
		{
			errno	=	EPERM;
			return -1;
		}
		else
		{
			if (!TerminateProcess( hProc,SIGKILL) )
			{
				errno	=	EPERM;
				iRet	=	-1;
			}

			CloseHandle(hProc);

			return iRet;
		}
	}

	switch( sig )
	{
	case SIGTERM:
	case SIGQUIT:
	case SIGINT:
	case SIGABRT:
		if( GenerateConsoleCtrlEvent(CTRL_C_EVENT, (DWORD)pid) )
			iRet	=	0;
		else
			iRet	=	-1;

		break;

	default:
		break;
	}

	return 0;
}

int killpg(pid_t pgrp, int sig)
{
	int iProc;
	int i;
	pid_t aiPidList[TOS_MAX_CHILD];

	if( pgrp<0 )
		pgrp	=	-pgrp;
	else if( 0==pgrp )
		pgrp	=	getppid();

	iProc	=	tos_get_pidlist_i(TOS_MAX_CHILD, aiPidList, 0, pgrp);

	if( iProc<0 )
		return -1;
	
	if( 0==iProc )
	{
		errno	=	ESRCH;
		return -1;
	}

	for(i=0; i<iProc; i++)
	{
		kill_one_proc_i(aiPidList[i], sig);
	}

	return 0;
}


int kill(pid_t pid, int sig)
{
	int iProc;
	int i;
	pid_t aiPidList[TOS_MAX_CHILD];

	if( pid>0 )
		return kill_one_proc_i(pid, sig);

	if( -1==pid )
		pid	=	getpgrp();
	else if( pid<0 )
		pid	=	-pid;

	iProc	=	tos_get_pidlist_i(TOS_MAX_CHILD, aiPidList, 0, pid);

	if( iProc<0 )
		return -1;
	
	if( 0==iProc )
	{
		errno	=	ESRCH;
		return -1;
	}

	for(i=0; i<iProc; i++)
	{
		kill_one_proc_i(aiPidList[i], sig);
	}

	return 0;
}

int sigaddset(sigset_t *_set, int _signo)
{
	if (_set == 0)
	{
		errno = EINVAL;
		return -1;
	}
	if (_signo < 0 || _signo >= NSIG)
	{
		errno = EINVAL;
		return -1;
	}
	_set->__bits[_signo>>5] |= (1U << (_signo&31));

	return 0;
}

int sigdelset(sigset_t *_set, int _signo)
{
	if (_set == 0)
	{
		errno = EINVAL;
		return -1;
	}
	if (_signo < 0 || _signo >= NSIG)
	{
		errno = EINVAL;
		return -1;
	}
	_set->__bits[_signo>>5] &= ~(1U << (_signo&31));
	return 0;
}

int sigemptyset(sigset_t *_set)
{
	if (_set == 0)
	{
		errno = EINVAL;
		return -1;
	}

	memset(_set,0,sizeof(*_set));

	return 0;
}

int sigfillset(sigset_t *_set)
{
	if (_set == 0)
	{
		errno = EINVAL;
		return -1;
	}
	memset(_set,0xff,sizeof(*_set));
	return 0;
}

int sigismember(const sigset_t *_set, int _signo)
{
	if (_set == 0)
	{
		errno = EINVAL;
		return -1;
	}

	if (_signo < 0 || _signo >= NSIG)
		return 0;

	return _set->__bits[_signo >> 5] & (1U<<(_signo&31)) ? 1 : 0;
}

int sigpending(sigset_t *set)
{
	if (set == (sigset_t *)0)
	{
		errno = EFAULT;
		return -1;
	}

	*set = __sigprocmask_pending_signals;

	return 0;
}


/* A signal handler which just records that a signal occured
(it will be raised later, if and when the signal is unblocked).  */
static void sig_suspender (int signo)
{
	sigaddset (&__sigprocmask_pending_signals, signo);
}

int signal_default_i(int a_iSig)
{
	switch (a_iSig)
	{
    case SIGALRM:
    case SIGHUP:
    case SIGINT:
    case SIGKILL:
    case SIGPIPE:
    case SIGTERM:
    case SIGUSR1:
    case SIGUSR2:
    case SIGPOLL:
    case SIGPROF:
    case SIGVTALRM:
		ExitProcess(a_iSig & 0x7f);
		
    case SIGABRT:
    case SIGFPE:
    case SIGILL:
    case SIGQUIT:
    case SIGSEGV:
    case SIGBUS:
    case SIGTRAP:
		ExitProcess(a_iSig & 0x7f);

	default:
		ExitProcess(a_iSig & 0x7f);
		break;
	}

	return 0;
}

int signal_fire_one_i(int a_iSig)
{
	TSIGDESC *pstDesc=NULL;
	siginfo_t stInfo;
	sigset_t stOldMask;
	sighandler_t pfnHandler=NULL;

	memset(&stInfo, 0, sizeof(stInfo));

	stInfo.si_signo	=	a_iSig;
	stInfo.si_errno	=	0;

	sigdelset(&__sigprocmask_pending_signals, a_iSig);

	pstDesc	=	s_astSigDesc + a_iSig;
	
	if( SIG_IGN==pstDesc->stAct.sa_handler )
	{
		return 0;
	}
	else if( SIG_ERR==pstDesc->stAct.sa_handler )
	{
		errno	=	EINVAL;
		return -1;
	}
	else if( SIG_DFL==pstDesc->stAct.sa_handler )
	{
		return signal_default_i(a_iSig);
	}

	stOldMask	=	pstDesc->stAct.sa_mask;

	memcpy(&__current_mask, &stOldMask, sizeof(__current_mask));

	pfnHandler	=	pstDesc->stAct.sa_handler;
	
	if( pstDesc->stAct.sa_flags & SA_RESETHAND )
		pstDesc->stAct.sa_handler	=	SIG_DFL;

	if( pstDesc->stAct.sa_flags & SA_SIGINFO )
		((void (*)(int, siginfo_t*, void*))pfnHandler)(a_iSig, &stInfo, NULL);
	else
		pfnHandler(a_iSig);

	memcpy(&stOldMask, &__current_mask, sizeof(__current_mask));

	return 0;
}

int signal_fire_i(int a_iSig)
{
	int i;

	if( a_iSig>0 )
	{
		if( sigismember(&__sigprocmask_pending_signals, a_iSig) )
		{
			return signal_fire_one_i(a_iSig);
		}
		else
		{
			return 0;
		}
	}

	for(i=1; i<SIGMAX; i++)
	{
		if( sigismember(&__sigprocmask_pending_signals, i) )
		{
			return signal_fire_one_i(i);
		}
	}

	return 0;
}

int sigprocmask (int how, const sigset_t *new_set, sigset_t *old_set)
{
	int signo;
	sigset_t new_mask;

	/* If called for the first time, initialize.  */
	if (!__sigprocmask_inited)
	{
		__sigprocmask_inited = 1;
		sigemptyset (&__sigprocmask_pending_signals);
		sigemptyset (&__current_mask);

		memset(s_astSigDesc, 0, sizeof(s_astSigDesc));
		for (signo = 0; signo < SIGMAX; signo++)
			s_astSigDesc[signo].stAct.sa_handler	=	SIG_ERR;
	}

	if (old_set)
		*old_set = __current_mask;

	if (new_set == 0)
		return 0;

	if (how != SIG_BLOCK && how != SIG_UNBLOCK && how != SIG_SETMASK)
	{
		errno = EINVAL;
		return -1;
	}

	sigemptyset (&new_mask);

	for (signo = 0; signo < SIGMAX; signo++)
	{
		if (sigismember (&__current_mask, signo))
			sigaddset (&new_mask, signo);
		else if (sigismember (new_set, signo) && how != SIG_UNBLOCK)
		{
			sigaddset (&new_mask, signo);

			/* SIGKILL is silently ignored, as on other platforms.  */
			if (signo != SIGKILL && s_astSigDesc[signo].stAct.sa_handler == SIG_ERR)
				s_astSigDesc[signo].stAct.sa_handler = tsignal (signo, sig_suspender);
		}
		if ((   how == SIG_UNBLOCK
			&& sigismember (&new_mask, signo)
			&& sigismember (new_set, signo))
			|| (how == SIG_SETMASK
			&& sigismember (&new_mask, signo)
			&& !sigismember (new_set, signo)))
		{
			sigdelset (&new_mask, signo);
			if (s_astSigDesc[signo].stAct.sa_handler != SIG_ERR)
			{
				tsignal (signo, s_astSigDesc[signo].stAct.sa_handler);
				s_astSigDesc[signo].stAct.sa_handler = SIG_ERR;
			}
			if (sigismember (&__sigprocmask_pending_signals, signo))
			{
				sigdelset (&__sigprocmask_pending_signals, signo);
				traise (signo);
			}
		}
	}

	__current_mask = new_mask;

	return 0;
}

int sigsuspend(const sigset_t *_set)
{
	errno	=	EINTR;
	return -1;
}


int sigaction(int _sig, const struct sigaction *_act, struct sigaction *_oact)
{
	TSIGDESC *pstDesc;

	/* note that sigaction always fails for SIGKILL */

	if( _sig<=0 || _sig>=SIGMAX )
	{
		errno	=	EINVAL;
		return -1;
	}

	if( SIGKILL==_sig || SIGSTOP==_sig )
	{
		errno	=	EINVAL;
		return -1;
	}

	signal_fire_i(_sig);

	pstDesc	=	s_astSigDesc + _sig;

	if (_oact)
	{
		memcpy(_oact, &pstDesc->stAct, sizeof(pstDesc->stAct));
	}

	if (_act)
	{
		memcpy(&pstDesc->stAct, _act, sizeof(pstDesc->stAct));
	}

	return 0;
}

int sigwaitinfo(const sigset_t *set, siginfo_t *info)
{
	return sigtimedwait(set, info, NULL);
}

int sigtimedwait(const sigset_t *set,  siginfo_t *info, const struct timespec *timeout)
{
	DWORD dwTimeout;

	if( timeout )
		dwTimeout	=	timeout->tv_sec*1000 + timeout->tv_nsec/1000000;
	else
		dwTimeout	=	INFINITE;


	return 0;
}

void psignal (int sig, const char *msg)
{
	if (sig >= 0 && sig < NSIG)
		fprintf (stderr, "%s: %s\n", msg, sys_siglist[sig]);
	else
		fprintf (stderr, "%s: signal %d\n", msg, sig);
}



#endif /* WIN32 */

sighandler_t tsignal(int a_iSig, sighandler_t a_pfnHandler)
{
#ifdef WIN32
	/* use sigaction to simulate. */
	struct sigaction stAct;
	struct sigaction stOld;

	if( a_iSig<=0 || a_iSig>=SIGMAX)
	{
		errno	=	EINVAL;
		return SIG_ERR;
	}

	memset(&stAct, 0, sizeof(stAct));

	stAct.sa_handler	=	a_pfnHandler;
	stAct.sa_flags		=	SA_RESETHAND;

	if( sigaction(a_iSig, &stAct, &stOld)<0 )
		return SIG_ERR;

	if( stOld.sa_flags & SA_SIGINFO )
		return SIG_ERR;
	else
		return stOld.sa_handler;

#else /* WIN32 */
	return signal(a_iSig, a_pfnHandler);
#endif /* WIN32 */
}

int traise(int a_iSig)
{
#ifdef WIN32

	if( a_iSig<=0 || a_iSig>=SIGMAX)
	{
		errno	=	EINVAL;
		return -1;
	}

	signal_fire_i( 0 );

	if( sigismember(&__current_mask, a_iSig) )
	{
		sigaddset(&__sigprocmask_pending_signals, a_iSig);

		return 0;
	}

	return signal_fire_one_i(a_iSig);

#else /* WIN32 */
	return raise(a_iSig);
#endif/* WIN32 */
}


