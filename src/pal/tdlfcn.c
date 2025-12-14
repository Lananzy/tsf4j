/*
**  @file $RCSfile: tdlfcn.c,v $
**  general description of this module
**  $Id: tdlfcn.c,v 1.1 2009/01/23 09:35:03 kent Exp $
**  @author $Author: kent $
**  @date $Date: 2009/01/23 09:35:03 $
**  @version $Revision: 1.1 $
**  @note Editor: Vim 6.1, Gcc 4.0.1, tab=4
**  @note Platform: Linux
*/
#include "pal/pal.h"
#include "pal/tdlfcn.h"

#ifdef WIN32
	void *dlopen(const char *filename, int flag)
	{
		return (void*)LoadLibrary(filename);
	}

	char *dlerror(void)
	{
		return NULL;
	}

	void *dlsym(void *handle, const char *symbol)
	{
		return (void*) GetProcAddress((HMODULE)handle, symbol);
	}

	int dlclose(void *handle)
	{
		return CloseHandle((HANDLE)handle) ? 0 : -1;
	}
#endif /*WIN32*/