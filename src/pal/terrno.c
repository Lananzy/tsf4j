/*
**  @file $RCSfile: terrno.c,v $
**  general description of this module
**  $Id: terrno.c,v 1.1.1.1 2008/05/28 07:35:00 kent Exp $
**  @author $Author: kent $
**  @date $Date: 2008/05/28 07:35:00 $
**  @version $Revision: 1.1.1.1 $
**  @note Editor: Vim 6.1, Gcc 4.0.1, tab=4
**  @note Platform: Linux
*/

#include "pal/tos.h"
#include "pal/terrno.h"

#ifdef WIN32

void terrno_set_from_winerr(long a_lWinError)
{
	switch(a_lWinError)
	{
	case ERROR_SUCCESS:
		errno = 0;
		break;

	case ERROR_INVALID_FUNCTION:
		errno = EBADRQC;
		break;

	case ERROR_FILE_NOT_FOUND:
		errno = ENOENT;
		break;

	case ERROR_PATH_NOT_FOUND:
		errno = ENOENT;
		break;

	case ERROR_TOO_MANY_OPEN_FILES:
		errno = EMFILE;
		break;

	case ERROR_ACCESS_DENIED:
		errno = EACCES;
		break;

	case ERROR_INVALID_HANDLE:
		errno = EBADF;
		break;

	case ERROR_NOT_ENOUGH_MEMORY:
		errno = ENOMEM;
		break;

	case ERROR_INVALID_DATA:
		errno = EINVAL;
		break;

	case ERROR_OUTOFMEMORY:
		errno = ENOMEM;
		break;

	case ERROR_INVALID_DRIVE:
		errno = ENODEV;
		break;

	case ERROR_NOT_SAME_DEVICE:
		errno = EXDEV;
		break;

	case ERROR_NO_MORE_FILES:
		errno = EMFILE;
		break;

	case ERROR_WRITE_PROTECT:
		errno = EROFS;
		break;

	case ERROR_BAD_UNIT:
		errno = ENODEV;
		break;

	case ERROR_SHARING_VIOLATION:
		errno = EACCES;
		break;

	case ERROR_LOCK_VIOLATION:
		errno = EACCES;
		break;

	case ERROR_SHARING_BUFFER_EXCEEDED:
		errno = ENOLCK;
		break;

	case ERROR_HANDLE_EOF:
		errno = ENODATA;
		break;

	case ERROR_HANDLE_DISK_FULL:
		errno = ENOSPC;
		break;

	case ERROR_NOT_SUPPORTED:
		errno = ENOSYS;
		break;

	case ERROR_REM_NOT_LIST:
		errno = ENONET;
		break;

	case ERROR_DUP_NAME:
		errno = ENOTUNIQ;
		break;

	case ERROR_BAD_NETPATH:
		errno = EINVAL;
		break;

	case ERROR_BAD_NET_NAME:
		errno = EINVAL;
		break;

	case ERROR_FILE_EXISTS:
		errno = EEXIST;
		break;

	case ERROR_CANNOT_MAKE:
		errno = EPERM;
		break;

	case ERROR_INVALID_PARAMETER:
		errno = EINVAL;
		break;

	case ERROR_NO_PROC_SLOTS:
		errno = EAGAIN;
		break;

	case ERROR_BROKEN_PIPE:
		errno = EPIPE;
		break;

	case ERROR_OPEN_FAILED:
		errno = EIO;
		break;

	case ERROR_NO_MORE_SEARCH_HANDLES:
		errno = ENFILE;
		break;

	case ERROR_CALL_NOT_IMPLEMENTED:
		errno = ENOSYS;
		break;

	case ERROR_INVALID_NAME:
		errno = ENOENT;
		break;

	case ERROR_WAIT_NO_CHILDREN:
		errno = ECHILD;
		break;

	case ERROR_CHILD_NOT_COMPLETE:
		errno = EBUSY;
		break;

	case ERROR_DIR_NOT_EMPTY:
		errno = ENOTEMPTY;
		break;

	case ERROR_SIGNAL_REFUSED:
		errno = EIO;
		break;

	case ERROR_BAD_PATHNAME:
		errno = ENOENT;
		break;

	case ERROR_SIGNAL_PENDING:
		errno = EBUSY;
		break;

	case ERROR_MAX_THRDS_REACHED:
		errno = EAGAIN;
		break;

	case ERROR_BUSY:
		errno = EBUSY;
		break;

	case ERROR_ALREADY_EXISTS:
		errno = EEXIST;
		break;

	case ERROR_NO_SIGNAL_SENT:
		errno = EIO;
		break;

	case ERROR_FILENAME_EXCED_RANGE:
		errno = EINVAL;
		break;

	case ERROR_META_EXPANSION_TOO_LONG:
		errno = EINVAL;
		break;

	case ERROR_INVALID_SIGNAL_NUMBER:
		errno = EINVAL;
		break;

	case ERROR_THREAD_1_INACTIVE:
		errno = EINVAL;
		break;

	case ERROR_BAD_PIPE:
		errno = EINVAL;
		break;

	case ERROR_PIPE_BUSY:
		errno = EBUSY;
		break;

	case ERROR_NO_DATA:
		errno = EPIPE;
		break;

	case ERROR_PIPE_NOT_CONNECTED:
		errno = ECOMM;
		break;

	case ERROR_MORE_DATA:
		errno = EAGAIN;
		break;

	case ERROR_DIRECTORY:
		errno = ENOTDIR;
		break;

	case ERROR_PIPE_CONNECTED:
		errno = EBUSY;
		break;

	case ERROR_PIPE_LISTENING:
		errno = ECOMM;
		break;

	case ERROR_NO_TOKEN:
		errno = EINVAL;
		break;

	case ERROR_PROCESS_ABORTED:
		errno = EFAULT;
		break;

	case ERROR_BAD_DEVICE:
		errno = ENODEV;
		break;

	case ERROR_BAD_USERNAME:
		errno = EINVAL;
		break;

	case ERROR_NOT_CONNECTED:
		errno = ENOLINK;
		break;

	case ERROR_OPEN_FILES:
		errno = EAGAIN;
		break;

	case ERROR_ACTIVE_CONNECTIONS:
		errno = EAGAIN;
		break;

	case ERROR_DEVICE_IN_USE:
		errno = EAGAIN;
		break;

	case ERROR_INVALID_AT_INTERRUPT_TIME:
		errno = EINTR;
		break;

	case ERROR_IO_DEVICE:
		errno = EIO;
		break;

	case ERROR_NOT_OWNER:
		errno = EPERM;
		break;

	case ERROR_END_OF_MEDIA:
		errno = ENOSPC;
		break;

	case ERROR_EOM_OVERFLOW:
		errno = ENOSPC;
		break;

	case ERROR_BEGINNING_OF_MEDIA:
		errno = ESPIPE;
		break;

	case ERROR_SETMARK_DETECTED:
		errno = ESPIPE;
		break;

	case ERROR_NO_DATA_DETECTED:
		errno = ENOSPC;
		break;

	case ERROR_POSSIBLE_DEADLOCK:
		errno = EDEADLOCK;
		break;

	case ERROR_CRC:
		errno = EIO;
		break;

	case ERROR_NEGATIVE_SEEK:
		errno = EINVAL;
		break;

	case ERROR_NOT_READY:
		errno = ENOMEDIUM;
		break;

	case ERROR_DISK_FULL:
		errno = ENOSPC;
		break;

	case ERROR_NOACCESS:
		errno = EFAULT;
		break;

	case ERROR_FILE_INVALID:
		errno = ENXIO;
		break;

	case ERROR_INVALID_ADDRESS:
		errno = EFAULT;
		break;

	case ERROR_BUFFER_OVERFLOW:
		errno = ENOMEM;
		break;

	case ERROR_SERVICE_DOES_NOT_EXIST:
		errno = ESRCH;
		break;

	case ERROR_SERVICE_EXISTS:
		errno = EEXIST;
		break;

	default:
		errno = 0;
		break;
	}
}

/**
* @brief Set errno according to a Winsock error
* @param lWinError Error code defined in winsock.h
*/
void terrno_set_from_sockerr(long a_lWinError)
{
	switch(a_lWinError)
	{
	case 0:
		errno = 0;
		break;
	case WSAEINTR:
		errno = EINTR;
		break;

	case WSAEWOULDBLOCK:
		errno = EWOULDBLOCK;
		break;

	case WSAEINPROGRESS:
		errno = EINPROGRESS;
		break;

	case WSAEALREADY:
		errno = EALREADY;
		break;

	case WSAENOTSOCK:
		errno = ENOTSOCK;
		break;

	case WSAEDESTADDRREQ:
		errno = EDESTADDRREQ;
		break;

	case WSAEMSGSIZE:
		errno = EMSGSIZE;
		break;

	case WSAEPROTOTYPE:
		errno = EPROTOTYPE;
		break;

	case WSAENOPROTOOPT:
		errno = ENOPROTOOPT;
		break;

	case WSAEPROTONOSUPPORT:
		errno = EPROTONOSUPPORT;
		break;

	case WSAESOCKTNOSUPPORT:
		errno = ESOCKTNOSUPPORT;
		break;

	case WSAEOPNOTSUPP:
		errno = EOPNOTSUPP;
		break;

	case WSAEPFNOSUPPORT:
		errno = EPFNOSUPPORT;
		break;

	case WSAEAFNOSUPPORT:
		errno = EAFNOSUPPORT;
		break;

	case WSAEADDRINUSE:
		errno = EADDRINUSE;
		break;

	case WSAEADDRNOTAVAIL:
		errno = EADDRNOTAVAIL;
		break;

	case WSAENETDOWN:
		errno = ENETDOWN;
		break;

	case WSAENETUNREACH:
		errno = ENETUNREACH;
		break;

	case WSAENETRESET:
		errno = ENETRESET;
		break;

	case WSAECONNABORTED:
		errno = ECONNABORTED;
		break;

	case WSAECONNRESET:
		errno = ECONNRESET;
		break;

	case WSAENOBUFS:
		errno = ENOBUFS;
		break;

	case WSAEISCONN:
		errno = EISCONN;
		break;

	case WSAENOTCONN:
		errno = ENOTCONN;
		break;

	case WSAESHUTDOWN:
		errno = ESHUTDOWN;
		break;

	case WSAETOOMANYREFS:
		errno = ETOOMANYREFS;
		break;

	case WSAETIMEDOUT:
		errno = ETIMEDOUT;
		break;

	case WSAECONNREFUSED:
		errno = ECONNREFUSED;
		break;

	case WSAELOOP:
		errno = ELOOP;
		break;

	case WSAENAMETOOLONG:
		errno = ENAMETOOLONG;
		break;

	case WSAEHOSTDOWN:
		errno = EHOSTDOWN;
		break;

	case WSAEHOSTUNREACH:
		errno = EHOSTUNREACH;
		break;

	case WSAENOTEMPTY:
		errno = ENOTEMPTY;
		break;

	case WSAEPROCLIM:
		errno = EAGAIN;
		break;

	case WSAEUSERS:
		errno = EUSERS;
		break;

	case WSAEDQUOT:
		errno = EDQUOT;
		break;

	case WSAESTALE:
		errno = ESTALE;
		break;

	case WSAEREMOTE:
		errno = EREMOTE;
		break;

	case WSAEINVAL:
		errno = EINVAL;
		break;

	case WSAEFAULT:
		errno = EFAULT;
		break;

	case WSANO_DATA:
		errno = ENODATA;
		break;

	default:
		errno = 0;
		break;
	}
}

void terrno_set_from_hresult(HRESULT a_hRes)
{
	switch(a_hRes)
	{
	case NOERROR:
		errno = 0;
		break;
	case E_UNEXPECTED:
	case E_FAIL:
	case S_FALSE:
		errno = ESTALE;
	case E_NOTIMPL:
		errno = ENOSYS;
		break;
	case E_OUTOFMEMORY:
		errno = ENOMEM;
		break;
	case E_INVALIDARG:
	case E_NOINTERFACE:
		errno = EINVAL;
		break;
	case E_POINTER:
	case E_ABORT:
		errno = EFAULT;
		break;
	case E_HANDLE:
		errno = EBADF;
		break;
	case E_ACCESSDENIED:
		errno = EACCES;
		break;
	case E_PENDING:
		errno = EBUSY;
		break;
	default:
		terrno_set_from_winerr(HRESULT_CODE(a_hRes));
	}
}

#endif /* WIN32 */


