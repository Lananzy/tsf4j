/**
*
* @file     tdr_error_i.c  
* @brief    Internal error functions
* 
* @author jackyai  
* @version 1.0
* @date 2007-03-28 
*
*
* Copyright (c)  2007, 腾讯科技有限公司互动娱乐研发部
* All rights reserved.
*
*/



#include "tdr_error_i.h"

#ifdef WIN32
#pragma warning(disable:4996)
#endif

#ifndef _TDR_MUTIL_THREAD
/* single-threaded version */

static int last_error = TDR_SUCCESS;

void tdr_set_last_error(int code)
{
    last_error = code;
}

int tdr_get_last_error()
{
    return last_error;
}

#else /* _TDR_MUTIL_THREAD */
/* multi-threaded versions */

#ifdef _MSC_VER
/* Microsoft Visual C++ multi-thread version */


#define WIN32_LEAN_AND_MEAN

#include <windows.h>

static DWORD last_error_key = TLS_OUT_OF_INDEXES;

void tdr_set_last_error(int code)
{
    if (last_error_key == TLS_OUT_OF_INDEXES)
    {
        last_error_key = TlsAlloc();
    }
    TlsSetValue(last_error_key, (LPVOID) code);
}

int tdr_get_last_error()
{
    if (last_error_key == TLS_OUT_OF_INDEXES)
    {
        last_error_key = TlsAlloc();
        TlsSetValue(last_error_key, (LPVOID) scew_error_none);
    }
    return (int) TlsGetValue(last_error_key);
}

#else /* _MSC_VER */
/* pthread multi-threaded version */

#include <pthread.h>

static pthread_key_t key_error;
static pthread_once_t key_once = PTHREAD_ONCE_INIT;

void create_keys()
{
    int* code = NULL;

    pthread_key_create(&key_error, free);

    code = (scew_error*) malloc(sizeof(scew_error));
    *code = scew_error_none;
    pthread_setspecific(key_error, code);
}

void tdr_set_last_error(int code)
{
    scew_error* old_code = NULL;
    scew_error* new_code = NULL;

    /* Initialize error code per thread */
    pthread_once(&key_once, create_keys);

    old_code = (scew_error*) pthread_getspecific(key_error);
    new_code = (scew_error*) malloc(sizeof(scew_error));
    *new_code = code;
    free(old_code);
    pthread_setspecific(key_error, new_code);
}

int tdr_get_last_error()
{
    int* code = NULL;

    /* Initialize error code per thread */
    pthread_once(&key_once, create_keys);

    code = (int*) pthread_getspecific(key_error);
    if (code == NULL)
    {
        return scew_error_none;
    }
    return *code;
}

#endif /* _MSC_VER */

#endif /* _TDR_MUTIL_THREAD */
