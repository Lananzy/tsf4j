/**
*
* @file     tdr_XMLTags_i_h.h  
* @brief    获取不同版本的元数据描述XML标签定义
* 
* @author jackyai  
* @version 1.0
* @date 2007-04-05 
*
*
* Copyright (c)  2007, 腾讯科技有限公司互动娱乐研发部
* All rights reserved.
*
*/

#ifndef TDR_AUXTOOLS_H
#define TDR_AUXTOOLS_H

#include <stdlib.h>
#include "tdr/tdr_types.h"
#include "tdr/tdr_ctypes_info.h"

#ifdef WIN32
#define TDR_ATOLL  _atoi64
#else
#define  TDR_ATOLL	atoll
#endif


/**
*剔除pszSrcStr前后的的空白字符，然后将处理后的串拷贝到a_pszDstStr中
*@param[out] a_pszDstStr 保存处理后串的缓冲区
*@param[in] a_iDstStrSize 输出缓冲区的大小
*@param[in] a_pszSrcStr 带处理的串
*@return 成功返回处理后字符串的长度（不包括null字符），否则返回负数
*@note 如果输出缓冲区空间不够，则截断拷贝
*/
int tdr_normalize_string(char *a_pszDstStr, int a_iDstStrSize, const char *a_pszSrcStr);

/**
*剔除a_strInput前后的的空白字符，然后将处理后的串拷贝到a_strInput中
*@param[in,out] a_strInput 保存处理后串的缓冲区
*
*@note 如果输出缓冲区空间不够，则截断拷贝
*/
void tdr_trim_str( char *a_strInput );

/**
*将中文字符串转换成unicode字符串
*@param[out] a_pszWcstr 保存unicode字符串的缓冲区
*@param[in,out] a_piWcsLen 
*	- 输入: 保存unicode字符串的大小，以char为单位计算
*   - 输出：转换后unicode字符串的大小,以char为单位计算
*@param[in] a_pszMbs 需要转换的中文字符串缓冲区指针
*@param[in] a_iMbsLen 需要转换的字节数
*@return 0 成功 非0 失败
*
*@note 如果输出缓冲区空间不够，则截断拷贝
*/
int tdr_chinesembstowcs(OUT char *a_pszWcstr, INOUT int *a_piWcsLen, IN char *a_pszMbs, IN int a_iMbsLen);

/**
*将unicode字符串转换成中文字符串
*@param[out] a_pszMbs 保存中文字符串的缓冲区
*@param[in,out] a_piMbsLen 
*	- 输入: 保存中文字符串的大小，以wchar_t为单位计算
*	- 输出：被转换的中文字符串大小
*@param[in] a_pszWcstr 需要转换的unicode字符串缓冲区指针
*@param[in] a_iWcsLen 需要转换的字节数
*@return 0 成功 非0 失败
*
*@note 如果输出缓冲区空间不够，则截断拷贝
*/
int tdr_wcstochinesembs(OUT char *a_pszMbs, INOUT int *a_piMbsLen, IN tdr_wchar_t *a_pszWcstr, IN int a_iWcsLen);

/**
*将unicode字符串转换成中文字符串
*@param[in] a_pszWcstr 需要转换的unicode字符串缓冲区指针
*@param[in] a_iWcsLen 需要转换的字节数
*@return 成功 返回转换后的中文字符串的指针,否则返回NULL
*
*@note 转换后的中文字符串缓冲区的空间在函数内分配,使用完毕,必须调用free函数进行释放
*/
char *tdr_wcstochinesembs_i(IN tdr_wchar_t *a_pszWcstr, IN int a_iWcsLen);

/**
strnmov(dst,src,length) moves length characters, or until end, of src to
dst and appends a closing NUL to dst if src is shorter than length.
The result is a pointer to the first NUL in dst, or is dst+n if dst was
truncated.
*/
char *tdr_strnmov(register char *dst, register const char *src, unsigned int n);

/**
tdr_strmov(dst, src) moves all the  characters  of  src  (including  the
closing NUL) to dst, and returns a pointer to the new closing NUL in
dst.	 The similar UNIX routine strcpy returns the old value of dst,
which I have never found useful.  tdr_strmov(tdr_strmov(dst,a),b) moves a//b
into dst, which seems useful.
*/
char *tdr_strmov(register char *dst, register const char *src);

unsigned int tdr_get_hash_code(const char* szKey, int iKeyLen);

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
			  );

#endif /*TDR_AUXTOOLS_H*/
