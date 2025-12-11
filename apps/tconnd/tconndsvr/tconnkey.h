/*
**  @file $RCSfile: tconnkey.h,v $
**  general description of this module
**  $Id: tconnkey.h,v 1.4 2009/01/22 10:02:49 sean Exp $
**  @author $Author: sean $
**  @date $Date: 2009/01/22 10:02:49 $
**  @version $Revision: 1.4 $
**  @note Editor: Vim 6.1, Gcc 4.0.1, tab=4
**  @note Platform: Linux
*/

#ifndef TCONNKEY_H
#define TCONNKEY_H

#include <time.h>
#include <sys/file.h>

#include "pal/pal.h"
#include "taa/tagentapi.h"
#include "tconnddef.h"

#ifdef __cplusplus
extern "C"
{
#endif

#define TCONNDSEMKEY 	0xccddeeaa
#define TCONNDSHMKEY 	0xccddeeaa

extern void *semcreate  (int, int);
extern void  semdestroy (void *);
extern int semacquire (void *);
extern int semrelease (void *);

#ifdef __cplusplus
}
#endif

#endif /* TCONNKEY_H */
