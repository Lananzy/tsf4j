#include <assert.h>
#include <string.h>

#include "tdr_os.h"
#include "tdr_auxtools.h"
#include "tdr_define_i.h"
#include "tdr/tdr_define.h"
#include "tdr/tdr_error.h"

#ifdef WIN32
#include <locale.h>
#else
#include <iconv.h>
#endif

#ifdef WIN32
#pragma warning(disable:4996)
#endif

int tdr_normalize_string(char *a_pszDstStr, int a_iDstStrSize, const char *a_pszSrcStr)
{
	const char *pb;
    const char *pe;
	int iTempLength = 0;

	assert(NULL != a_pszDstStr);
	assert(0 < a_iDstStrSize);
	assert(NULL != a_pszSrcStr);


	*a_pszDstStr = '\0';
	iTempLength = strlen(a_pszSrcStr);
	if( iTempLength == 0 )
	{
		return 0;
	}

    pb = a_pszSrcStr;

    while (((*pb == ' ') || (*pb == '\t') || (*pb == '\n') || (*pb == '\r')) && (*pb != 0))
    {
        pb ++;
    }

    pe = &a_pszSrcStr[iTempLength-1];
    while ((pe >= pb) && ((*pe == ' ') || (*pe == '\t') || (*pe == '\n') || (*pe == '\r')))
    {
        pe --;
    }

	iTempLength = pe - pb + 2;  /*包括'\0'的空间*/
	iTempLength = TDR_MIN(a_iDstStrSize, iTempLength);
	TDR_STRNCPY(a_pszDstStr, pb, iTempLength);

	return iTempLength;
}

void tdr_trim_str( char *a_strInput )
{
    char *pb;
    char *pe;
	int iTempLength;

	
	assert(NULL != a_strInput);
	iTempLength = strlen(a_strInput);
	if( iTempLength == 0 )
	{
		return;
	}

    pb = a_strInput;

    while (((*pb == ' ') || (*pb == '\t') || (*pb == '\n') || (*pb == '\r')) && (*pb != 0))
    {
        pb ++;
    }

    pe = &a_strInput[iTempLength-1];
    while ((pe >= pb) && ((*pe == ' ') || (*pe == '\t') || (*pe == '\n') || (*pe == '\r')))
    {
        pe --;
    }

	*(pe+1) = '\0';

	if (pb != a_strInput)
	{
		strcpy( a_strInput, pb );
	}

}

int tdr_chinesembstowcs(OUT char *a_pszWcstr, IN int *a_piWcsLen, IN char *a_pszMbs, IN int a_iMbsLen)
{
	size_t iLen;
#ifdef WIN32
	char *pszLocal;
#else
	//static TDRBOOLEAN bTestd = TDR_FALSE;
	char *pszIn;
	char *pszOut;
	size_t iInLeft;
	size_t iOutLeft;
	iconv_t hMb2Unicode;
#endif

	assert(NULL != a_pszWcstr);
	assert(NULL != a_pszMbs);
	assert(NULL != a_piWcsLen);

#ifdef WIN32
	pszLocal = setlocale(LC_ALL, NULL);
	setlocale(LC_ALL,"Chinese-simplified");

	iLen = mbstowcs(NULL, a_pszMbs, 0);
	if (iLen > *a_piWcsLen/sizeof(tdr_wchar_t))
	{/*保存unicode字符串的缓冲区不够*/
		setlocale(LC_ALL,pszLocal);
		return TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_FAILED_CONVERT_CHINESE_TO_UNICODE);
	}
	iLen = mbstowcs((wchar_t *)a_pszWcstr, a_pszMbs, a_iMbsLen);
	setlocale(LC_ALL,pszLocal);
	if (iLen < 0)
	{
		return TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_FAILED_CONVERT_CHINESE_TO_UNICODE);
	}
	
	*a_piWcsLen = (iLen + 1) * sizeof(tdr_wchar_t);
	return TDR_SUCCESS;
#else
	hMb2Unicode = iconv_open("UNICODE", "GB2312");
	if (hMb2Unicode == (iconv_t)-1)
	{
		return TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_FAILED_CONVERT_CHINESE_TO_UNICODE);
	}

	//if (TDR_FALSE == bTestd)
	{
		/*第一次转换iconv会添加0xFF,0xFE两个字符，通过这个方法可以避免后续的字符转换也包含这两个字符*/
		wchar_t szTemp[12];

		pszIn = "a中";
		pszOut = (char *)&szTemp[0];
		iInLeft = 2;
		iOutLeft = sizeof(szTemp);

		iLen = iconv(hMb2Unicode, &pszIn, &iInLeft, &pszOut, &iOutLeft);
		//bTestd = TDR_TRUE;
	}

	pszIn = a_pszMbs;
	pszOut = (char *)a_pszWcstr;
	iInLeft = a_iMbsLen;
	iOutLeft = *a_piWcsLen;

	iLen = iconv(hMb2Unicode, &pszIn, &iInLeft, &pszOut, &iOutLeft);
	iconv_close(hMb2Unicode);
	*a_piWcsLen -= iOutLeft;
	if (0 >= *a_piWcsLen)
	{
		return TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_FAILED_CONVERT_CHINESE_TO_UNICODE);
	}
	
	return TDR_SUCCESS;
#endif
}

int tdr_wcstochinesembs(OUT char *a_pszMbs, INOUT int *a_piMbsLen, IN tdr_wchar_t *a_pszWcstr, IN int a_iWcsLen)
{
	int iLen;
#ifdef WIN32
	char *pszLocal;
#else
	static TDRBOOLEAN bTestd = TDR_FALSE;
	char *pszIn;
	char *pszOut;
	size_t iInLeft;
	size_t iOutLeft;
	iconv_t hUnicode2Mb;
#endif

	assert(NULL != a_pszMbs);
	assert(NULL != a_piMbsLen);
	assert(NULL != a_pszMbs);
	assert(0 < a_iWcsLen);

#ifdef WIN32
	pszLocal = setlocale(LC_ALL, NULL);
	setlocale(LC_ALL,"Chinese-simplified");

	iLen = wcstombs(NULL, a_pszWcstr, 0);
	if (iLen > *a_piMbsLen)
	{/*保存unicode字符串的缓冲区不够*/
		setlocale(LC_ALL,pszLocal);
		return TDR_ERR_ERROR;
	}
	iLen = wcstombs(a_pszMbs, a_pszWcstr, iLen + 1);
	setlocale(LC_ALL,pszLocal);
	if (iLen < 0)
	{
		return TDR_ERR_ERROR;
	}

	*a_piMbsLen = iLen;
	return TDR_SUCCESS;
#else
	hUnicode2Mb = iconv_open("GB2312", "UNICODE");
	if (hUnicode2Mb == (iconv_t)-1)
	{
		return TDR_ERR_ERROR;
	}

	if (TDR_FALSE == bTestd)
	{
		/*第一次转换iconv会添加0xFF,0xFE两个字符，通过这个方法可以避免后续的字符转换也包含这两个字符*/
		wchar_t szTemp[] = {0xFFFE, 0x2d4e, 0};
		char szMbs[12];

		pszIn = (char *)&szTemp[0];
		pszOut = &szMbs[0];
		iInLeft = 3;
		iOutLeft = sizeof(szMbs);

		iLen = iconv(hUnicode2Mb, &pszIn, &iInLeft, &pszOut, &iOutLeft);
		bTestd = TDR_TRUE;
	}

	pszIn = (char *)a_pszWcstr;
	pszOut = a_pszMbs;
	iInLeft = a_iWcsLen*sizeof(tdr_wchar_t);
	iOutLeft = *a_piMbsLen;

	iLen = iconv(hUnicode2Mb, &pszIn, &iInLeft, &pszOut, &iOutLeft);
	iconv_close(hUnicode2Mb);
	*a_piMbsLen = *a_piMbsLen - iOutLeft;
	if (0 >= *a_piMbsLen)
	{
		return TDR_ERR_ERROR;
	}

	return TDR_SUCCESS;
#endif
}

char *tdr_wcstochinesembs_i(IN tdr_wchar_t *a_pszWcstr, IN int a_iWcsLen)
{
	size_t iLen;	
	char *pszMbs;

#ifdef WIN32
	char *pszLocal;
#else
	static TDRBOOLEAN bTestd = TDR_FALSE;
	char *pszIn;
	char *pszOut;
	size_t iInLeft;
	size_t iOutLeft;
	iconv_t hUnicode2Mb;
#endif

	assert(0 < a_iWcsLen);

#ifdef WIN32
	pszLocal = setlocale(LC_ALL, NULL);
	setlocale(LC_ALL,"Chinese-simplified");

	iLen = wcstombs(NULL, a_pszWcstr, 0);

	/* Add one to leave room for the null terminator. */
	pszMbs = (char *)malloc( iLen + 1);
	if (NULL == pszMbs)
	{
		return NULL;
	}


	iLen = wcstombs(pszMbs, a_pszWcstr, iLen + 1);
	setlocale(LC_ALL,pszLocal);
	if (iLen < 0)
	{
		free(pszMbs);
		return NULL;
	}

	
	return pszMbs;
#else
	hUnicode2Mb = iconv_open("GB2312", "UNICODE");
	if (hUnicode2Mb == (iconv_t)-1)
	{
		return NULL;
	}

	if (TDR_FALSE == bTestd)
	{
		/*第一次转换iconv会添加0xFF,0xFE两个字符，通过这个方法可以避免后续的字符转换也包含这两个字符*/
		wchar_t szTemp[] = {0xFFFE, 0x2d4e, 0};
		char szMbs[12];

		pszIn = (char *)&szTemp[0];
		pszOut = &szMbs[0];
		iInLeft = 3;
		iOutLeft = sizeof(szMbs);

		iLen = iconv(hUnicode2Mb, &pszIn, &iInLeft, &pszOut, &iOutLeft);
		bTestd = TDR_TRUE;
	}

	
	pszIn = (char *)a_pszWcstr;	
	iInLeft = a_iWcsLen*sizeof(tdr_wchar_t);
	iOutLeft = iInLeft + 1;
	iLen = iOutLeft;
	pszMbs = (char *)malloc( iInLeft + 1);
	if (NULL == pszMbs)
	{
		return NULL;
	}
	pszOut =  pszMbs;

	iconv(hUnicode2Mb, &pszIn, &iInLeft, &pszOut, &iOutLeft);
	iconv_close(hUnicode2Mb);
	if (0 >= (iLen - iOutLeft))
	{
		free(pszMbs);
		return NULL;
	}

	return pszMbs;
#endif
}


/*
tdr_strnmov(dst,src,length) moves length characters, or until end, of src to
dst and appends a closing NUL to dst if src is shorter than length.
The result is a pointer to the first NUL in dst, or is dst+n if dst was
truncated.
*/
char *tdr_strnmov(register char *dst, register const char *src, unsigned int n)
{
	while (n-- != 0) {
		if (!(*dst++ = *src++)) {
			return (char*) dst-1;
		}
	}
	return dst;
}

/*
tdr_strmov(dst, src) moves all the  characters  of  src  (including  the
closing NUL) to dst, and returns a pointer to the new closing NUL in
dst.	 The similar UNIX routine strcpy returns the old value of dst,
which I have never found useful.  strmov(strmov(dst,a),b) moves a//b
into dst, which seems useful.
*/
char *tdr_strmov(register char *dst, register const char *src)
{
	while ((*dst++ = *src++)) ;
	return dst-1;
}

unsigned int tdr_get_hash_code(const char* szKey, int iKeyLen)
{
	int i;
	unsigned int uCode=0;

	for (i = 0; i < iKeyLen; i++)
	{
		uCode += (uCode<<5) + ((unsigned char)szKey[i]);
	}	

	return uCode;
}


/***
*qsort.c - quicksort algorithm; qsort() library function for sorting arrays
*
*   Copyright (c) Microsoft Corporation. All rights reserved.
*
*Purpose:
*   To implement the qsort() routine for sorting arrays.
*
*******************************************************************************/



static void  tdr_shortsort_i(char *lo, char *hi, size_t width,
						int ( *comp)(const void *, const void *));


static void  tdr_swap_i(char *p, char *q, size_t width);

/* this parameter defines the cutoff between using quick sort and
insertion sort for arrays; arrays with lengths shorter or equal to the
below value use insertion sort */

#define CUTOFF_BETWEEN_QSORT_INSERTIONSORT 8            /* testing shows that this is good value */

#define ENTRIES_STKSIZ (8*sizeof(void*) - 2)

/***
*tdr_qsort(base, num, wid, comp) - quicksort function for sorting arrays
*
*Purpose:
*   quicksort the array of elements
*   side effects:  sorts in place
*   maximum array size is number of elements times size of elements,
*   but is limited by the virtual address space of the processor
*
*Entry:
*   char *base = pointer to base of array
*   size_t num  = number of elements in the array
*   size_t width = width in bytes of each array element
*   int (*comp)() = pointer to function returning analog of strcmp for
*           strings, but supplied by user for comparing the array elements.
*           it accepts 2 pointers to elements.
*           Returns neg if 1<2, 0 if 1=2, pos if 1>2.
*
*Exit:
*   returns void
*
*Exceptions:
*   Input parameters are validated. Refer to the validation section of the function.
*
*******************************************************************************/
void  tdr_qsort (		   void *base,
			  size_t num,
			  size_t width,
			  int ( *comp)(const void *, const void *)
			  )
{
	/* Note: the number of stack entries required is no more than
	1 + log2(num), so 30 is sufficient for any array */
	char *lo, *hi;              /* ends of sub-array currently sorting */
	char *mid;                  /* points to middle of subarray */
	char *loguy, *higuy;        /* traveling pointers for partition step */
	size_t size;                /* size of the sub-array */
	char *lostk[ENTRIES_STKSIZ], *histk[ENTRIES_STKSIZ];
	int stkptr;                 /* stack for saving sub-array to be processed */

	/* validation section */
	/*_VALIDATE_RETURN_VOID(base != NULL || num == 0, EINVAL);
	_VALIDATE_RETURN_VOID(width > 0, EINVAL);
	_VALIDATE_RETURN_VOID(comp != NULL, EINVAL);*/

	if (num < 2)
		return;                 /* nothing to do */

	stkptr = 0;                 /* initialize stack */

	lo = (char *)base;
	hi = (char *)base + width * (num-1);        /* initialize limits */

	/* this entry point is for pseudo-recursion calling: setting
	lo and hi and jumping to here is like recursion, but stkptr is
	preserved, locals aren't, so we preserve stuff on the stack */
recurse:

	size = (hi - lo) / width + 1;        /* number of el's to sort */

	/* below a certain size, it is faster to use a O(n^2) sorting method */
	if (size <= CUTOFF_BETWEEN_QSORT_INSERTIONSORT) {
		tdr_shortsort_i(lo, hi, width, comp);
	}
	else {
		/* First we pick a partitioning element.  The efficiency of the
		algorithm demands that we find one that is approximately the median
		of the values, but also that we select one fast.  We choose the
		median of the first, middle, and last elements, to avoid bad
		performance in the face of already sorted data, or data that is made
		up of multiple sorted runs appended together.  Testing shows that a
		median-of-three algorithm provides better performance than simply
		picking the middle element for the latter case. */

		mid = lo + (size / 2) * width;      /* find middle element */

		/* Sort the first, middle, last elements into order */
		if (comp(lo, mid) > 0) {
			tdr_swap_i(lo, mid, width);
		}
		if (comp(lo, hi) > 0) {
			tdr_swap_i(lo, hi, width);
		}
		if (comp(mid, hi) > 0) {
			tdr_swap_i(mid, hi, width);
		}

		/* We now wish to partition the array into three pieces, one consisting
		of elements <= partition element, one of elements equal to the
		partition element, and one of elements > than it.  This is done
		below; comments indicate conditions established at every step. */

		loguy = lo;
		higuy = hi;

		/* Note that higuy decreases and loguy increases on every iteration,
		so loop must terminate. */
		for (;;) {
			/* lo <= loguy < hi, lo < higuy <= hi,
			A[i] <= A[mid] for lo <= i <= loguy,
			A[i] > A[mid] for higuy <= i < hi,
			A[hi] >= A[mid] */

			/* The doubled loop is to avoid calling comp(mid,mid), since some
			existing comparison funcs don't work when passed the same
			value for both pointers. */

			if (mid > loguy) {
				do  {
					loguy += width;
				} while (loguy < mid && comp(loguy, mid) <= 0);
			}
			if (mid <= loguy) {
				do  {
					loguy += width;
				} while (loguy <= hi && comp(loguy, mid) <= 0);
			}

			/* lo < loguy <= hi+1, A[i] <= A[mid] for lo <= i < loguy,
			either loguy > hi or A[loguy] > A[mid] */

			do  {
				higuy -= width;
			} while (higuy > mid && comp(higuy, mid) > 0);

			/* lo <= higuy < hi, A[i] > A[mid] for higuy < i < hi,
			either higuy == lo or A[higuy] <= A[mid] */

			if (higuy < loguy)
				break;

			/* if loguy > hi or higuy == lo, then we would have exited, so
			A[loguy] > A[mid], A[higuy] <= A[mid],
			loguy <= hi, higuy > lo */

			tdr_swap_i(loguy, higuy, width);

			/* If the partition element was moved, follow it.  Only need
			to check for mid == higuy, since before the swap,
			A[loguy] > A[mid] implies loguy != mid. */

			if (mid == higuy)
				mid = loguy;

			/* A[loguy] <= A[mid], A[higuy] > A[mid]; so condition at top
			of loop is re-established */
		}

		/*     A[i] <= A[mid] for lo <= i < loguy,
		A[i] > A[mid] for higuy < i < hi,
		A[hi] >= A[mid]
		higuy < loguy
		implying:
		higuy == loguy-1
		or higuy == hi - 1, loguy == hi + 1, A[hi] == A[mid] */

		/* Find adjacent elements equal to the partition element.  The
		doubled loop is to avoid calling comp(mid,mid), since some
		existing comparison funcs don't work when passed the same value
		for both pointers. */

		higuy += width;
		if (mid < higuy) {
			do  {
				higuy -= width;
			} while (higuy > mid && comp(higuy, mid) == 0);
		}
		if (mid >= higuy) {
			do  {
				higuy -= width;
			} while (higuy > lo && comp(higuy, mid) == 0);
		}

		/* OK, now we have the following:
		higuy < loguy
		lo <= higuy <= hi
		A[i]  <= A[mid] for lo <= i <= higuy
		A[i]  == A[mid] for higuy < i < loguy
		A[i]  >  A[mid] for loguy <= i < hi
		A[hi] >= A[mid] */

		/* We've finished the partition, now we want to sort the subarrays
		[lo, higuy] and [loguy, hi].
		We do the smaller one first to minimize stack usage.
		We only sort arrays of length 2 or more.*/

		if ( higuy - lo >= hi - loguy ) {
			if (lo < higuy) {
				lostk[stkptr] = lo;
				histk[stkptr] = higuy;
				++stkptr;
			}                           /* save big recursion for later */

			if (loguy < hi) {
				lo = loguy;
				goto recurse;           /* do small recursion */
			}
		}
		else {
			if (loguy < hi) {
				lostk[stkptr] = loguy;
				histk[stkptr] = hi;
				++stkptr;               /* save big recursion for later */
			}

			if (lo < higuy) {
				hi = higuy;
				goto recurse;           /* do small recursion */
			}
		}
	}

	/* We have sorted the array, except for any pending sorts on the stack.
	Check if there are any, and do them. */

	--stkptr;
	if (stkptr >= 0) {
		lo = lostk[stkptr];
		hi = histk[stkptr];
		goto recurse;           /* pop subarray from stack */
	}
	else
		return;                 /* all subarrays done */
}


/***
*tdr_tshortsort_i(hi, lo, width, comp) - insertion sort for sorting short arrays
*
*Purpose:
*       sorts the sub-array of elements between lo and hi (inclusive)
*       side effects:  sorts in place
*       assumes that lo < hi
*
*Entry:
*       char *lo = pointer to low element to sort
*       char *hi = pointer to high element to sort
*       size_t width = width in bytes of each array element
*       int (*comp)() = pointer to function returning analog of strcmp for
*               strings, but supplied by user for comparing the array elements.
*               it accepts 2 pointers to elements, together with a pointer to a context
*               (if present). Returns neg if 1<2, 0 if 1=2, pos if 1>2.
*
*Exit:
*       returns void
*
*Exceptions:
*
*******************************************************************************/


static void  tdr_shortsort_i (
						 char *lo,
						 char *hi,
						 size_t width,
						 int ( *comp)(const void *, const void *)
						 )

{
	char *p, *max;

	/* Note: in assertions below, i and j are alway inside original bound of
	array to sort. */

	while (hi > lo) {
		/* A[i] <= A[j] for i <= j, j > hi */
		max = lo;
		for (p = lo+width; p <= hi; p += width) {
			/* A[i] <= A[max] for lo <= i < p */
			if (comp(p, max) > 0) {
				max = p;
			}
			/* A[i] <= A[max] for lo <= i <= p */
		}

		/* A[i] <= A[max] for lo <= i <= hi */

		tdr_swap_i(max, hi, width);

		/* A[i] <= A[hi] for i <= hi, so A[i] <= A[j] for i <= j, j >= hi */

		hi -= width;

		/* A[i] <= A[j] for i <= j, j > hi, loop top condition established */
	}
	/* A[i] <= A[j] for i <= j, j > lo, which implies A[i] <= A[j] for i < j,
	so array is sorted */
}

/***
*tdr_swap_i(a, b, width) - swap two elements
*
*Purpose:
*       swaps the two array elements of size width
*
*Entry:
*       char *a, *b = pointer to two elements to swap
*       size_t width = width in bytes of each array element
*
*Exit:
*       returns void
*
*Exceptions:
*
*******************************************************************************/

static void  tdr_swap_i (
				   char *a,
				   char *b,
				   size_t width
				   )
{
	char tmp;

	if ( a != b )
		/* Do the swap one character at a time to avoid potential alignment
		problems. */
		while ( width-- ) {
			tmp = *a;
			*a++ = *b;
			*b++ = tmp;
		}
}

