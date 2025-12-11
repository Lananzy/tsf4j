
#include <ctype.h>
#include <stdio.h>

static unsigned char upcase[256] = {
    0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15,
   16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31,
  ' ','!','"','#','$','%','&', 39,'(',')','*','+',',','-','.','/',
  '0','1','2','3','4','5','6','7','8','9',':',';','<','=','>','?',
  '@','A','B','C','D','E','F','G','H','I','J','K','L','M','N','O',
  'P','Q','R','S','T','U','V','W','X','Y','Z','[', 92,']','^','_',
  '`','A','B','C','D','E','F','G','H','I','J','K','L','M','N','O',
  'P','Q','R','S','T','U','V','W','X','Y','Z','{','|','}','~',127,
  128,129,130,131,132,133,134,135,136,137,138,139,140,141,142,143,
  144,145,146,147,148,149,150,151,152,153,154,155,156,157,158,159,
  160,161,162,163,164,165,166,167,168,169,170,171,172,173,174,175,
  176,177,178,179,180,181,182,183,184,185,186,187,188,189,190,191,
  192,193,194,195,196,197,198,199,200,201,202,203,204,205,206,207,
  208,209,210,211,212,213,214,215,216,217,218,219,220,221,222,223,
  224,225,226,227,228,229,230,231,232,233,234,235,236,237,238,239,
  240,241,242,243,244,245,246,247,248,249,250,251,252,253,254,255};

#define UPC(x) (upcase[(unsigned char)(x)])

/*
 * Same as strcmp, but case-insensitive.
 */
int
cistrcmp(s1,s2)
register char *s1,*s2;
{
    register char c1;
    while ((c1 = *s1) && UPC(c1) == UPC(*s2)) {
	s1++;
	s2++;
    }
    return UPC(c1) - UPC(*s2);
}

/*
 * Same as strstr, but case-insensitive.
 */
char*
cistrstr(s1,s2)
register char *s1,*s2;
{
	 register char *p1,*p2;
	 p1 = s1;
	 p2 = s2;
	 while (*s1 && *s2) {
		if (UPC(*s1) == UPC(*s2)) {
			s1++;
			s2++;
		} else {
			s1 = ++p1;
			s2 = p2;
		}
	}
	if (!*s2) return p1;
	return NULL;		
}


/*
 * Same as strncmp, but case-insensitive.
 */
int
cistrncmp(s1,s2,n)
register char *s1,*s2;
int n;
{
    register char c1;
    while (n-- && (c1 = *s1) && UPC(c1) == UPC(*s2)) {
	s1++;
	s2++;
    }
    if (n == -1) return 0;
    return UPC(c1) - UPC(*s2);
}

char* upperstr(str)
char *str;
{
	register char *s1;
	s1 = str;
	while (*s1) *(s1++) = UPC(*s1);
	return str;
}

