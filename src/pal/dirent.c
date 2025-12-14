#include "pal/tos.h"
#include "pal/ttypes.h"
#include "pal/tdirent.h"
#include "pal/tdir.h"

#ifdef WIN32

static struct dirent _dirent_buf;

#define isupperxdigit(c) (isdigit(c) || (c)>='A' && (c)<='F')

DIR *opendir(const char *dir)
{
	HANDLE h;
	int len;
	DIR *d;

	len	=	strlen( dir );

	d	=	(DIR*) malloc( sizeof(DIR)+len+3 );

	if (d)
	{
		char *p=(char*)d + sizeof(DIR);

		memcpy(p, dir, len);

		if (p[len-1]!='\\')
			p[len++]	=	'\\';

		p[len++]	=	'*';
		p[len]		=	0;

		d->name		=	p;
		d->count	=	0;
	}

	if( !d )	return NULL;

	h	=	FindFirstFile(d->name, (WIN32_FIND_DATA*)&_dirent_buf);

	if( h==INVALID_HANDLE_VALUE )
	{
		free(d);
		return NULL;
	}

	d->handle	=	h;

	_dirent_buf.dwReserved0	=	1;

	return d;
}

struct dirent *readdir(DIR *dir)
{
	if (!_dirent_buf.dwReserved0)
	{
		if (!FindNextFile(dir->handle,(WIN32_FIND_DATA*)&_dirent_buf))
			return NULL;
	}

	_dirent_buf.dwReserved0=0;
	_dirent_buf.d_ino++;
	/* urldecode name */
	{
		char *p=strchr(_dirent_buf.d_name,'%');
		if (p && p[1] && p[2])
		{
			char *to=p; 

			do
			{
				if (*p=='%' && isupperxdigit(p[1]) && isupperxdigit(p[2]))
				{
					int a,b;
					a=*++p-'0'; if (a>9) a-='A'-('9'+1);
					b=*++p-'0'; if (b>9) b-='A'-('9'+1);
					*to++=(a<<4)|b;
					++p;
				}
				else
				{
					*to++=*p++;
				}
			} while (*p);

			*to=0;
		}
	}

	_dirent_buf.d_namlen=strlen(_dirent_buf.d_name);

	++dir->count;

	return (struct dirent*)&_dirent_buf;
}


int closedir(DIR *dir)
{
	HANDLE h=dir->handle;

	free(dir);

	if (FindClose(h)) return 0;

	return 1;
}


void rewinddir(DIR *dir)
{
	FindClose(dir->handle);

	dir->handle	=	FindFirstFile(dir->name, (WIN32_FIND_DATA*)&_dirent_buf);

	dir->count	=	0;

	_dirent_buf.dwReserved0=1;

	return;
}


long telldir(DIR *dir)
{
	return dir->count;
}


void seekdir(DIR *dir, long loc)
{
	int i;

	rewinddir(dir);

	for (i=0; i<loc; i++)
		readdir(dir);
}

int dirfd(DIR *dir)
{
	return (int)dir->handle;
}

#endif /* WIN32 */
