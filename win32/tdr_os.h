#ifndef TDR_OS_H
#define TDR_OS_H

#ifdef WIN32
	#if(_WIN32_WINNT >= 0x0400)
		#include <winsock2.h>
		#include <windows.h>
	#else /* _WIN32_WINNT */
		#include <winsock2.h>
		#include <windows.h>
	#endif /* _WIN32_WINNT */

	#include <tchar.h>
#else	/* WIN32 */
	#include <unistd.h>
	#include <endian.h>
#endif	/* WIN32 */

#include <wchar.h>
#include <string.h>
#include <stdio.h>
#include "tdr/tdr_types.h"


#ifdef WIN32

	#if defined (LITTLEENDIAN) && LITTLEENDIAN >0 
		#define TDR_OS_LITTLEENDIAN

		#if defined (TDR_OS_BIGENDIAN)
		#undef TDR_OS_BIGENDIAN
		#endif
	#else
		#define TDR_OS_BIGENDIAN
		#if defined (TOS_LITTLEENDIAN)
		#undef TDR_OS_LITTLEENDIAN
		#endif
	#endif

#else /* WIN32 */

	#if __BYTE_ORDER == __LITTLE_ENDIAN
		#define TDR_OS_LITTLEENDIAN

		#if defined (TDR_OS_BIGENDIAN)
		#undef TDR_OS_BIGENDIAN
		#endif
	#else
		#define TDR_OS_BIGENDIAN
		#if defined (TDR_OS_LITTLEENDIAN)
		#undef TDR_OS_LITTLEENDIAN
	#endif
#endif

#endif /* WIN32 */


#define TDR_OS_DIRDOT			'.'
#define	TDR_OS_DIRBAR			'_'

#ifdef WIN32
	#define	TDR_OS_DIRSEP			'\\'
	#define tdr_stricmp(s1, s2)				stricmp(s1, s2)
	#define tdr_strnicmp(s1, s2, n)			strnicmp(s1, s2, n)
	#define tdr_strupr(sz)				_strupr(sz)

	#define tdr_snprintf				_snprintf

	
	size_t tdr_wcsnlen(const wchar_t *s, size_t maxlen);
	


	#define tdr_vsnprintf				_vsnprintf

	#if _MSC_VER < 1400
		#define	tdr_strnlen(sz,n)				strlen(sz)
	#else
		#define	tdr_strnlen(sz,n)				strnlen(sz, n)
	#endif	/* _MSC_VER */

#else /*#ifdef WIN32*/
	#define	TDR_OS_DIRSEP			'/'
	#define tdr_stricmp(s1, s2)			strcasecmp(s1, s2)
	#define tdr_strnicmp(s1, s2, n)		strncasecmp(s1, s2, n)
	#define tdr_strupr(p)			{			      \
		int i=0;					      \
		while(p[i])					      \
		{						      \
			p[i]	=	(char)toupper(p[i]);	      \
			i++;					      \
		}						      \
	}

	#define tdr_snprintf				snprintf
	#define tdr_vsnprintf				vsnprintf
	#define  tdr_strnlen	strnlen
	size_t tdr_wcsnlen(const wchar_t *s, size_t maxlen);

#endif/*#ifdef WIN32*/

#define TDR_STRNCPY(pszDst, pszSrc, iLen)					      \
	do								      \
	{								      \
		strncpy(pszDst, pszSrc, (iLen)-1);			      \
		pszDst[(iLen)-1] = 0;					      \
	}								      \
	while(0)

#define TDR_MEMCPY(d, s, size, min)			{			      \
	int i;								      \
	if( size<=min )							      \
	{								      \
		for(i=0; i<size; i++)					      \
		{							      \
			d[0]	=	s[0];				      \
			d++; s++;					      \
		}							      \
	}								      \
	else								      \
	{								      \
		memcpy(d, s, size);					      \
		s +=	size;						      \
		d +=	size;						      \
	}								      \
}




#if defined(WIN32) &&  _MSC_VER < 1300
#define tdr_os_swap64(x) \
	((((x) & (tdr_ulonglong)0xff00000000000000) >> 56)                                   \
	| (((x) & (tdr_ulonglong)0x00ff000000000000) >> 40)                                 \
	| (((x) & (tdr_ulonglong)0x0000ff0000000000) >> 24)                                 \
	| (((x) & (tdr_ulonglong)0x000000ff00000000) >> 8)                                  \
	| (((x) & (tdr_ulonglong)0x00000000ff000000) << 8)                                  \
	| (((x) & (tdr_ulonglong)0x0000000000ff0000) << 24)                                 \
	| (((x) & (tdr_ulonglong)0x000000000000ff00) << 40)                                 \
	| (((x) & (tdr_ulonglong)0x00000000000000ff) << 56))
#else
#define tdr_os_swap64(x) \
	((((x) & (tdr_ulonglong)0xff00000000000000LL) >> 56)                                   \
	| (((x) & (tdr_ulonglong)0x00ff000000000000LL) >> 40)                                 \
	| (((x) & (tdr_ulonglong)0x0000ff0000000000LL) >> 24)                                 \
	| (((x) & (tdr_ulonglong)0x000000ff00000000LL) >> 8)                                  \
	| (((x) & (tdr_ulonglong)0x00000000ff000000) << 8)                                  \
	| (((x) & (tdr_ulonglong)0x0000000000ff0000) << 24)                                 \
	| (((x) & (tdr_ulonglong)0x000000000000ff00) << 40)                                 \
	| (((x) & (tdr_ulonglong)0x00000000000000ff) << 56))

#endif


#define tdr_os_swap32(x) \
	((((x) & 0xff000000) >> 24)                                  \
	| (((x) & 0x00ff0000) >> 8)                                 \
	| (((x) & 0x0000ff00) << 8)                                 \
	| (((x) & 0x000000ff) << 24))                                  
	
#define tdr_os_swap16(x) \
	((((x) & 0xff00) >> 8)                                  \
	| (((x) & 0x00ff) << 8))                                 
              

#ifdef TDR_OS_LITTLEENDIAN
#define tdr_ntohq(x)                    tdr_os_swap64(x)
#define tdr_htonq(x)                    tdr_os_swap64(x)
#define tdr_ntohl_(x)                    tdr_os_swap32(x)
#define tdr_htonl_(x)                    tdr_os_swap32(x)
#define tdr_ntohs_(x)                    tdr_os_swap16(x)
#define tdr_htons_(x)                    tdr_os_swap16(x)
#else
#define tdr_ntohq(x)                    (x)
#define tdr_htonq(x)                    (x)
#define tdr_ntohl_(x)                   (x)
#define tdr_htonl_(x)                   (x)
#define tdr_ntohs_(x)                   (x)
#define tdr_htons_(x)                   (x)
#endif

#define TDR_NTOHNS(d, s, size)			{			      \
	int i;												\
	for(i=0; i<size; i++)										\
	{															\
		*(unsigned short*)d	=	ntohs(*(unsigned short*)s);   \
		d	+=	sizeof(unsigned short);			      \
		s	+=	sizeof(unsigned short);			      \
	}															\
}

#define TDR_HTONNS(d, s, size)			{			      \
	int i;								      \
	for(i=0; i<size; i++)						      \
	{																	\
		*(unsigned short*)d	=	ntohs(*(unsigned short*)s);   \
		d	+=	sizeof(unsigned short);			      \
		s	+=	sizeof(unsigned short);		 	      \
	}								      \
}

#define TDR_NTOHNL(d, s, size)			{			      \
	int i;								      \
	for(i=0; i<size; i++)						      \
	{								      \
		*(unsigned long*)d	=	ntohl(*(unsigned long*)s);    \
		d	+=	sizeof(unsigned long);			      \
		s	+=	sizeof(unsigned long);			      \
	}								      \
}

#define TDR_HTONNL(d, s, size)			{			      \
	int i;								      \
	for(i=0; i<size; i++)						      \
	{								      \
		*(unsigned long*)d	=	ntohl(*(unsigned long*)s);    \
		d	+=	sizeof(unsigned long);			      \
		s	+=	sizeof(unsigned long);			      \
	}								      \
}

#define TDR_NTOHNQ(d, s, size)			{			      \
	int i;								      \
	for(i=0; i<size; i++)						      \
	{								      \
		*(tdr_ulonglong*)d	=	tdr_ntohq(*(tdr_ulonglong*)s);		      \
		d	+=	sizeof(tdr_ulonglong);			      \
		s	+=	sizeof(tdr_ulonglong);			      \
	}								      \
}

#define TDR_HTONNQ(d, s, size)			{			      \
	int i;								      \
	for(i=0; i<size; i++)						      \
	{								      \
		*(tdr_ulonglong*)d	=	tdr_htonq(*(tdr_ulonglong*)s);		      \
		d	+=	sizeof(tdr_ulonglong);			      \
		s	+=	sizeof(tdr_ulonglong);			      \
	}								      \
}


void tdr_os_file_to_macro_i(char* pszMacro, int iMacro, const char* pszFile);
char * tdr_strptime( const char *buf,  const char *format, struct tm *timeptr);


#endif /* TDR_OS_H */
