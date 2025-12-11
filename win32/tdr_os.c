#include <assert.h>
#include <ctype.h>

#include "tdr_ctypes_info_i.h"
#include "tdr_os.h"

#ifdef WIN32
#pragma warning(disable:4996)
#endif



void tdr_os_file_to_macro_i(char* pszMacro, int iMacro, const char* pszFile)
{
	const char *pszTitle;
	int i;

	assert( pszMacro && pszFile && iMacro>0 );
	
	pszTitle	=	strrchr(pszFile, TDR_OS_DIRSEP);

	if( pszTitle )
		pszTitle++;
	else
		pszTitle=	pszFile;

	i	=	0;

	while(i<iMacro && i<(int)strlen(pszTitle) )
	{
		if( TDR_OS_DIRDOT==pszTitle[i] )
			pszMacro[i]	=	TDR_OS_DIRBAR;
		else
			pszMacro[i]	=	(char)toupper(pszTitle[i]);

		i++;
	}

	pszMacro[i]	=	'\0';
}



//#ifndef WIN32

size_t tdr_wcsnlen(const wchar_t *s, size_t maxlen)
{
	size_t i;
	tdr_wchar_t *pwch;

	assert(NULL != s);
	pwch = (tdr_wchar_t *)s;
	for (i = 0; i < maxlen; i++)
	{
		if (*pwch == (tdr_wchar_t)0)
		{
			break;
		}
		pwch++;
	}

	return i;
}

//#endif /*#ifndef WIN32*/
