/*
**  @file $RCSfile: tstring.c,v $
**  general description of this module
**  $Id: tstring.c,v 1.1 2009/01/23 09:35:03 kent Exp $
**  @author $Author: kent $
**  @date $Date: 2009/01/23 09:35:03 $
**  @version $Revision: 1.1 $
**  @note Editor: Vim 6.1, Gcc 4.0.1, tab=4
**  @note Platform: Linux
*/
#include "pal/pal.h"
#include "pal/tstring.h"

#ifdef WIN32

void bzero(void *s, size_t n)
{
	memset(s, 0, n);
}

#else /* WIN32 */

int stricmp(const char *string1, const char *string2)
{
	return strcasecmp(string1, string2);
}

int strnicmp(const char *string1, const char *string2,size_t count)
{
	return strncasecmp(string1, string2, count);
}

char *strupr(char *str)
{
	int i=0;	

	while(str[i])				
	{						      
		str[i]	=	(char)toupper(str[i]);	      
		i++;					     
	}						      
	
	return str;
}

char *strlwr(char *str)
{
	int i=0;	

	while(str[i])				
	{						      
		str[i]	=	(char)tolower(str[i]);	      
		i++;					     
	}						      
	
	return str;
}

#endif /* WIN32 */