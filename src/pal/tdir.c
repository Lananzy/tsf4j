/*
**  @file $RCSfile: tdir.c,v $
**  general description of this module
**  $Id: tdir.c,v 1.1.1.1 2008/05/28 07:35:00 kent Exp $
**  @author $Author: kent $
**  @date $Date: 2008/05/28 07:35:00 $
**  @version $Revision: 1.1.1.1 $
**  @note Editor: Vim 6.1, Gcc 4.0.1, tab=4
**  @note Platform: Linux
*/

#include "pal/tstdio.h"
#include "pal/tstring.h"
#include "pal/tdir.h"

#if 0

static char  gs_w32_mnt_pnt[20]="/mnt";
static int   gs_w32_mnt_pnt_len=4;
static char  gs_w32_root_drv='c';
static int   gs_w32_file_name_case=0;


#define CHAR2INT(a,b,c,d) ((d<<24)+(c<<16)+(b<<8)+a)

#define isupperxdigit(c) (isdigit(c) || (c)>='A' && (c)<='F')


static char *urlencode(char *p, unsigned char c)
{
  static char n2h[]="0123456789ABCDEF";
  *p++='%';
  *p++=n2h[c>>4];
  *p++=n2h[c&0xf];
  return p;
}


static int is_drive_semicolon(const char *posix, const char *win32, const char *win32beg)
{
     return win32==(win32beg+1) && (posix[1]=='\\' || posix[1]=='/');
}


static int is_special_point(char *p, char *win32beg)
{
  return p==win32beg || p[-1]=='\\' || p[-1]=='.' ;
}


int tdir_posix2win32(const char* a_pszPosix, char* a_pszWin32, int* a_piLen)
{
	char *b=a_pszWin32;
	int len;

	len=strlen(a_pszPosix);

	if (!memcmp(a_pszPosix,"/dev/null",10))
	{
		memcpy(a_pszWin32,"NUL",4);
		return 0;
	}

	if (len>=4 &&
		(*(int*)a_pszPosix==CHAR2INT('/','m','n','t') || *(int*)a_pszPosix==CHAR2INT('/','M','N','T')) &&
		a_pszPosix[4]=='_' &&
		(a_pszPosix[6]=='/' || !a_pszPosix[6]))
	{
		a_pszPosix+=5;
		*a_pszWin32++=*a_pszPosix++; *a_pszWin32++=':';
	}

	/* Checking for '$MNT_PNT/ syntax */
	else if (len>=gs_w32_mnt_pnt_len &&
		!memcmp(a_pszPosix,gs_w32_mnt_pnt,gs_w32_mnt_pnt_len) &&
		(!a_pszPosix[len] && gs_w32_mnt_pnt_len>1) || a_pszPosix[len]=='/')
	{
		a_pszPosix+=gs_w32_mnt_pnt_len;
		if (!*a_pszPosix || *a_pszPosix=='/'&&!a_pszPosix[1])
		{
			memcpy(a_pszWin32,a_pszPosix-gs_w32_mnt_pnt_len,gs_w32_mnt_pnt_len);
			a_pszWin32[gs_w32_mnt_pnt_len]=0;
			return 0;
		}
		++a_pszPosix;
		*a_pszWin32++=*a_pszPosix++; *a_pszWin32++=':';
		++a_pszPosix;
	}
	else if (*a_pszPosix=='/')
	{
		if (a_pszPosix[1]=='/' && a_pszPosix[2]!='/')
		{
			/* Exactly 2 leading slashes are special */
			*a_pszWin32++='\\'; *a_pszWin32++='\\'; a_pszPosix+=2;
		}
		else if (gs_w32_root_drv)
		{
			*a_pszWin32++=gs_w32_root_drv; *a_pszWin32++=':';
		}
	}

	while (*a_pszPosix)
	{
		static char disallowed_chars[]="*?:<>|";

		if (*a_pszPosix=='/')
		{
			/* This gives problems with /./ & /../ . Too late (1:30am) to work around */
			//      if (a_pszWin32[-1]=='.') { a_pszWin32[-1]='%'; *a_pszWin32++='2'; *a_pszWin32++='E'; }

			/* Collapse multiple slashes into single */
			while (*a_pszPosix=='/') ++a_pszPosix;
			--a_pszPosix;
			*a_pszWin32='\\';
		}
		/* Urlencode characters disallowed in win32 filenames */
		/* If in filename-case preserving mode and this is 9x, urlencode
		uppercase letters */
		else if (memchr(disallowed_chars,*a_pszPosix,sizeof(disallowed_chars)-1)
			|| (gs_w32_file_name_case && isupper(*a_pszPosix)))
		{
			/* Don't urlencode drive's ':' */
			if (*a_pszPosix==':' && is_drive_semicolon(a_pszPosix,a_pszWin32,b)) *a_pszWin32=*a_pszPosix;
			/* Don't urlencode drive letter */
			else if (a_pszWin32!=b || a_pszPosix[1]!=':') { a_pszWin32=urlencode(a_pszWin32,*a_pszPosix)-1; }
		}
		else *a_pszWin32=*a_pszPosix;
		++a_pszWin32; ++a_pszPosix;
	}

	/* Here may be \r, because read(STDIN) returns \r\n as EOL */
	if (a_pszWin32[-1]=='\r') --a_pszWin32;

	/* We should kill trailing \, unless it's only */
	if (a_pszWin32[-1]=='\\' && a_pszWin32-1!=b) --a_pszWin32;

	/* We should urlencode trailing dot, unless it's only */
	if (a_pszWin32[-1]=='.')
	{
		if (!is_special_point(a_pszWin32-1,b))
		{ a_pszWin32[-1]='%'; *a_pszWin32++='2'; *a_pszWin32++='E'; }
	}

	/* We should add trailing \, if there's <drive>: syntax */
	if (a_pszWin32-1!=b && a_pszWin32[-1]==':') *a_pszWin32++='\\';


	*a_pszWin32=0;

	return 0;
}


int tdir_win322posix(const char *win32,char *buf)
{
  char *b=buf;
  int len=strlen(win32);

/* Checking for '<drive_letter>:' syntax */
  if (len>=2 && win32[1]==':')
  {
    if (toupper(*win32)!=gs_w32_root_drv)
    {
      memcpy(buf,"/mnt_",5); buf+=5;
      *buf++=*win32;
    }
    win32+=2;
  }

  while (*win32)
  {
    if (*win32=='\\') *buf='/';
    else if (*win32=='%' && isupperxdigit(win32[1]) && isupperxdigit(win32[2]))
    {
      int a,b;
      a=*++win32-'0'; if (a>9) a-='A'-('9'+1);
      b=*++win32-'0'; if (b>9) b-='A'-('9'+1);
      *buf=(a<<4)|b;
    }
    else *buf=*win32;

    ++buf; ++win32;
  }

  *buf++=0;
  return buf-b;
}

#endif
