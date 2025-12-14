/*
**  @file $RCSfile: shortcut.cpp,v $
**  general description of this module
**  $Id: shortcut.cpp,v 1.1.1.1 2008/05/28 07:35:00 kent Exp $
**  @author $Author: kent $
**  @date $Date: 2008/05/28 07:35:00 $
**  @version $Revision: 1.1.1.1 $
**  @note Editor: Vim 6.1, Gcc 4.0.1, tab=4
**  @note Platform: Linux
*/


#include "pal/tos.h"
#include "pal/terrno.h"
#include "pal/tstdio.h"

#ifdef WIN32

#include <shlobj.h>
#include <objbase.h>

#pragma warning(disable:4996)

#ifdef __cplusplus
extern "C"
{
#endif

BOOL shortcut_create(const char *pszSrc, const char *pszDest)
{
	/* Create shortcut */
	IShellLink *pLink=NULL;
	IPersistFile *pFile=NULL;
	WCHAR *pwszDest=NULL;
	char *pszFileLnk=NULL;
	HRESULT hRes;
						    
	CoInitialize(NULL);
							    
	if ((strlen(pszSrc) > _MAX_PATH) || (strlen(pszDest) + 4 > _MAX_PATH))
	{
		CoUninitialize();
		errno = ENAMETOOLONG;

		return FALSE;
	}
								    
	/* Create Shortcut-Object */
	if (CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER,
		IID_IShellLink, (void **) &pLink) != S_OK)
	{
		CoUninitialize();
		errno = ESTALE;
															      
		return FALSE;
	}
									  
	/* Set target path */
	pLink->SetPath(pszSrc);

	/* Get File-Object */
	if (pLink->QueryInterface(IID_IPersistFile, (void **) &pFile) != S_OK)
	{
		pLink->Release();
		CoUninitialize();

		errno = ESTALE; 
		return FALSE;
	}

	/* shortcuts have the extension .lnk */
	pszFileLnk = (char *) malloc(strlen(pszDest) + 5);

	sprintf(pszFileLnk, "%s.lnk", pszDest);
													  
	/* Turn filename into widechars */
	pwszDest = (WCHAR *) malloc((_MAX_PATH + 5) * sizeof(WCHAR));
	if( !pwszDest )
	{
		pLink->Release();
		pFile->Release();
		CoUninitialize();
		return FALSE;
	}

	MultiByteToWideChar(CP_ACP, 0, pszFileLnk, -1, pwszDest, _MAX_PATH);

	free(pszFileLnk);

	/* Save shortcut */
	if (FAILED(hRes = pFile->Save((LPCOLESTR) pwszDest, TRUE)))
	{
		free(pwszDest);
		pLink->Release();
		pFile->Release();
		CoUninitialize();
		terrno_set_from_hresult(hRes);
		return FALSE;
	}
																	  
	free(pwszDest);
																		    
	pFile->Release();
	pLink->Release();
	CoUninitialize();
	errno = 0;

	return TRUE;

}

BOOL shortcut_derefer(char *pszShortcut)
{
	IShellLink *pLink;
	IPersistFile *pFile;
	WCHAR *pwszShortcut;
	char *pszLnk;
	int iLen;
	HRESULT hRes;
	HANDLE hLink;
	char szTarget[_MAX_PATH + 1];

	if (! *pszShortcut )
		return TRUE;

	CoInitialize(NULL);
	szTarget[0] = 0;

	/* Create Shortcut-Object */
	if (CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER,
		IID_IShellLink, (void **) &pLink) != S_OK )
	{
		CoUninitialize();
		errno = ESTALE;

		return FALSE;
	}

	/* Get File-Object */
	if (pLink->QueryInterface(IID_IPersistFile, (void **) &pFile) != S_OK)
	{
		pLink->Release();
		CoUninitialize();
		errno = ESTALE;

		return FALSE;
	}

	pwszShortcut = (WCHAR *) malloc((_MAX_PATH + 1) * sizeof(WCHAR));

	/* Shortcuts have the extension .lnk
	If it isn't there, append it */
	iLen = strlen(pszShortcut);
	if (iLen > 4 && (strcmp(pszShortcut + iLen - 4, ".lnk") != 0))
	{
		pszLnk = (char *) malloc(iLen + 5);
		sprintf(pszLnk, "%s.lnk", pszShortcut);
	}
	else
		pszLnk = strdup(pszShortcut);

	/* Make sure the path refers to a file */
	hLink = CreateFile(pszLnk, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE,
		NULL, OPEN_EXISTING, 0, NULL);
	if (hLink == INVALID_HANDLE_VALUE)
	{
		free(pszLnk);
		terrno_set_from_winerr(GetLastError());

		if (errno == ENOENT)
		{
			/* There's no path with the ".lnk" extension.
			We don't quit here, because we have to decide whether the path doesn't
			exist or the path isn't a link. */

			/* Is it a directory? */
			if (GetFileAttributes(pszShortcut) & FILE_ATTRIBUTE_DIRECTORY)
			{
				errno = EINVAL;

				pLink->Release();
				pFile->Release();
				free(pwszShortcut);
				CoUninitialize();

				return FALSE;
			}

			pszLnk = strdup(pszShortcut);

			hLink = CreateFile(pszLnk, GENERIC_READ, FILE_SHARE_READ |
				FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
			terrno_set_from_winerr(GetLastError());      
		}
		else
		{
			pLink->Release();
			pFile->Release();
			free(pwszShortcut);
			CoUninitialize();

			return FALSE; /* File/link is there but unaccessible */
		}
	}

	MultiByteToWideChar(CP_ACP, 0, pszLnk, -1, pwszShortcut, _MAX_PATH);

	/* Open shortcut */
	if (FAILED(hRes = pFile->Load((LPCOLESTR) pwszShortcut, STGM_READ)))
	{
		pLink->Release();
		pFile->Release();
		free(pwszShortcut);
		CoUninitialize();

		/* For some reason, opening an invalid link sometimes fails with ACCESSDENIED.
		Since we have opened the file previously, insufficient priviledges
		are rather not the problem. */
		if (hRes == E_FAIL || hRes == E_ACCESSDENIED)
		{
			/* Check file magic */
			if (hLink != INVALID_HANDLE_VALUE)
			{
				DWORD dwRead;
				char pMagic[4] = {0, 0, 0, 0};

				ReadFile(hLink, pMagic, 4, &dwRead, NULL);
				if (memcmp(pMagic, "L\0\0\0", 4) == 0)
					terrno_set_from_hresult(hRes);
				else
					errno = EINVAL; /* No link */
			}
			/* else: errno was set above! */
		}
		else
			terrno_set_from_hresult(hRes);

		free(pszLnk);

		CloseHandle(hLink);
		return FALSE;
	}

	CloseHandle(hLink);
	free(pszLnk);
	free(pwszShortcut);

	/* Get target file */
	if (FAILED(hRes = pLink->GetPath(szTarget, _MAX_PATH, NULL, 0)))
	{
		pLink->Release();
		pFile->Release();
		CoUninitialize();

		if (hRes == E_FAIL)
			errno = EINVAL; /* Not a symlink */
		else
			terrno_set_from_hresult(hRes);

		return FALSE;
	}

	pFile->Release();
	pLink->Release();
	CoUninitialize();
	errno = 0;

	if (szTarget[0] != 0)
	{
		strcpy(pszShortcut, szTarget);
		return TRUE;
	}
	else
	{
		/* GetPath() did not return a valid path */
		errno = EINVAL;
		return FALSE;
	}
}

/**
   * @brief Dereference a symlink recursively
    */
int win_deref(char *path)
{
	int iDepth = 0;

	errno = 0;

	while (shortcut_derefer(path))
	{
		if (iDepth++ > 10)
		{
			errno = ELOOP;
			return -1;
		}
	}

	if (iDepth != 0 && errno == EINVAL)
		errno = 0;

	return errno ? -1 : 0;
}


#ifdef __cplusplus
}
#endif

#endif /* WIN32 */

