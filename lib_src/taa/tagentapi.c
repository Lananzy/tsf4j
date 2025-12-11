/*
**  @file $RCSfile: tagentapi.c,v $
**  general description of this module
**  $Id: tagentapi.c,v 1.10 2009/03/31 01:20:19 sean Exp $
**  @author $Author: sean $
**  @date $Date: 2009/03/31 01:20:19 $
**  @version $Revision: 1.10 $
**  @note Editor: Vim 6.1, Gcc 4.0.1, tab=4
**  @note Platform: Linux
*/

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <time.h>

#include "pal/pal.h"
#include "pal/tsem.h"
#include "pal/tshm.h"

#include "taa/tagentapi.h"

#ifndef __i386__
#define __i386__
#endif

#if defined(__i386__) && !defined(_WIN64)
#define uint2korr(A)    (*((uint16_t *) (A)))
#define uint4korr(A)    (*((unsigned long *) (A)))

#define int2store(T,A)  *((uint16_t*) (T))= (uint16_t) (A)
#define int4store(T,A)  *((long *) (T))= (long) (A)
#endif /* __i386__ */

#ifdef WIN32
struct sembuf
{
	unsigned short int sem_num;   /* semaphore number */
	short int sem_op;     /* semaphore operation */
	short int sem_flg;        /* operation flag */
};
#endif

void *semcreate  (int, int);
void  semdestroy (void *);
int   semreset   (void *, int);
int   semacquire (void *);
int   semrelease (void *);

static char* initShm(key_t key, int size)
{
	char *pShm=0;
	int iShmid=0;
	//struct shmid_ds stShmStat ;
	
	iShmid = shmget(key, size,  IPC_EXCL | 0666);
	if (iShmid < 0)
	{
		if(errno != ENOENT)
		{
			return NULL;
		}

		iShmid = shmget(key, size, IPC_CREAT | IPC_EXCL | 0666);
		if(iShmid < 0)
		{
			if(errno != EEXIST)
			{
				return NULL;
			}

			iShmid = shmget(key, 0, 0666);
			if( iShmid < 0 )
			{
				return NULL;
			}
			else
			{
				if(shmctl(iShmid, IPC_RMID, NULL))
				{
					return NULL;
				}

				iShmid = shmget(key, size, IPC_CREAT|IPC_EXCL|0666);
				if(iShmid < 0)
				{
					return NULL;
				}
			}
		}
	}
	
	pShm = (char *)shmat(iShmid, NULL, 0);
	/*
	if ( 0 == shmctl( iShmid, IPC_STAT, &stShmStat))
	{
		size_t tSize = (unsigned int)stShmStat.shm_segsz;
	}
	*/
	
	return pShm;
}

static int agent_api_getid(char *pShm, unsigned int *puiAppid, unsigned int *puiBusid)
{
	int iLen = sizeof(unsigned int);
	assert (pShm);
	
	*puiAppid = 0;
	*puiBusid = 0;

	memcpy(puiAppid, pShm, iLen);
	memcpy(puiBusid, pShm+iLen, iLen);
	
	return 0;
}

static void agent_api_setid(char *pShm, unsigned int uiAppid, unsigned int uiBusid)
{
	assert (pShm);
	
	int4store(pShm, uiAppid);
	pShm += sizeof(unsigned int);

	int4store(pShm, uiBusid);
}

static LPEXCHANGEBLOCK exchange_find_ids(LPEXCHANGEMNG pstExcMng, unsigned int uiAppid, unsigned int uiBusid)
{
	unsigned i = 0;
	for (i = 0; i < pstExcMng->used; i++)
	{
		if (pstExcMng->ppstBlock[i]->uiAppid == uiAppid &&
			pstExcMng->ppstBlock[i]->uiBusid == uiBusid)
		{	
			return pstExcMng->ppstBlock[i];
		}
	}

	return NULL;
}

static int exchange_new(LPEXCHANGEMNG pstExcMng, unsigned int uiAppid, unsigned int uiBusid, char *pShm)
{
	LPEXCHANGEBLOCK pstBlock = NULL;
	
	pstBlock = malloc(sizeof(EXCHANGEBLOCK));
	if (pstExcMng->size == 0)
	{
		pstExcMng->used = 0;
		pstExcMng->size = 2;
		pstExcMng->ppstBlock = malloc(pstExcMng->size * sizeof(LPEXCHANGEMNG));
	}
	else if (pstExcMng->size == pstExcMng->used)
	{
		pstExcMng->size += 2;
		pstExcMng->ppstBlock = realloc(pstExcMng->ppstBlock, pstExcMng->size * sizeof(LPEXCHANGEMNG));
	}
	
	assert(pstExcMng->ppstBlock && pstBlock);
	pstBlock -> uiAppid = uiAppid;
	pstBlock -> uiBusid = uiBusid;
	pstBlock -> pBlk = pShm;
	pstExcMng->ppstBlock[pstExcMng->used++] = pstBlock;
	
	return 0;
}

int agent_api_init(LPEXCHANGEMNG *ppstExcMng)
{
	*ppstExcMng = (LPEXCHANGEMNG)malloc(sizeof(EXCHANGEMNG));
	if (NULL == *ppstExcMng)
	{
		fprintf (stderr, "Error:%s %d:malloc Fail:%s\n", 
			__FILE__, __LINE__, strerror(errno));
		return -1;
	}
	
	memset(*ppstExcMng, 0x00, sizeof(EXCHANGEMNG));
	(*ppstExcMng)->aSem = semcreate(EXCHANGEKEY, 1);
	if ((*ppstExcMng)->aSem == 0)
	{
		fprintf (stderr, "Error:%s %d:Init semaphore Fail:%s\n", 
			__FILE__, __LINE__, strerror(errno));
		return -1;
	}
	
	//lock
	semacquire((*ppstExcMng)->aSem);
	(*ppstExcMng)->pShm = initShm(EXCHANGEKEY, EXCHANGESIZE);
	
	//set Exchange Head description
	memset(&((*ppstExcMng)->stExcHead), 0x00, sizeof(EXCHANGEHEAD));
	memcpy(&((*ppstExcMng)->stExcHead), (*ppstExcMng)->pShm, sizeof(EXCHANGEHEAD));
	if (0 == (*ppstExcMng)->stExcHead.iExchangeSize)
	{
		//first time set exchange
		(*ppstExcMng)->stExcHead.iExchangeSize	= EXCHANGESIZE;
		(*ppstExcMng)->stExcHead.iBlockSize 		= EXCBLOCKSIZE;
		(*ppstExcMng)->stExcHead.iCreateTime 		= time(NULL);
		strcpy((*ppstExcMng)->stExcHead.szDescription, "Create by TAGENT_API");
		
		memcpy((*ppstExcMng)->pShm, &((*ppstExcMng)->stExcHead), sizeof(EXCHANGEHEAD));
	}
	
	//unlock
	semrelease((*ppstExcMng)->aSem);
	
	if ((*ppstExcMng)->pShm == NULL)
	{
		fprintf (stderr, "Error:%s %d:initShm Fail:%s\n", 
			__FILE__, __LINE__, strerror(errno));
		return -1;
	}
	
	return 0;
}

void agent_api_destroy(LPEXCHANGEMNG pstExcMng)
{
	unsigned i = 0;
	if (pstExcMng == NULL)
	{
		return;
	}
	
	/*
	 Realease shm
	 */
	if (pstExcMng->pShm)
	{
		shmdt(pstExcMng->pShm);
		pstExcMng->pShm = NULL;
	}
	
	/*
	 Realease sem
	 */
	if (pstExcMng->aSem)
	{
		semdestroy(pstExcMng->aSem);
		pstExcMng->aSem = NULL;
	}
	
	for (i = 0; i < pstExcMng->used; i++)
	{
		free(pstExcMng->ppstBlock[i]);
		pstExcMng->ppstBlock[i]= NULL;
	}
	
	free(pstExcMng->ppstBlock);
	pstExcMng->ppstBlock = NULL;
	/*
	 free LPEXCHANGEMNG
	 */
	free(pstExcMng);
	pstExcMng = NULL;
}

int agent_api_get_bussid(LPEXCHANGEMNG pstExcMng, unsigned int uiAppid, unsigned int astuiBusid[], int *piAstSize)
{
	int i=0;
	char *pShm = 0;
	unsigned int uiTmpAppid=0, uiTmpBussid=0;
	int iCount;
	
	if  ((NULL == pstExcMng) && (NULL == astuiBusid) && (0 >= *piAstSize))
	{
		return -1;
	}
	else
	{
		pShm = pstExcMng->pShm;
		if (NULL == pShm)
		{
			return -1;
		}
	}
	
	iCount = 0;
	pShm += EXCBLOCKSIZE;
	semacquire(pstExcMng->aSem);
	
	for (i = 1; i < EXCHANGESIZE / EXCBLOCKSIZE; i++)
	{
		agent_api_getid(pShm, &uiTmpAppid, &uiTmpBussid);
		if (uiTmpAppid == uiAppid)
		{
			astuiBusid[iCount] = uiTmpBussid;
			iCount++;
			exchange_new(pstExcMng, uiTmpAppid, uiTmpBussid, pShm);
			if (iCount  >= *piAstSize)
			{
				break;
			}
		}
		
		pShm += EXCBLOCKSIZE;
	}
	*piAstSize = iCount;
	
	semrelease(pstExcMng->aSem);
	return 0;
}

int agent_api_get_blocks(LPEXCHANGEMNG pstExcMng, LPEXCHANGEBLOCK **pppstBlock, int *piBlockSize)
{
	char *pShm = 0;
	
	int i = 0;
	int iExcCnt = 0;
	unsigned int uiAppid = 0, uiBusid = 0;
	
	iExcCnt = EXCHANGESIZE / EXCBLOCKSIZE;
	*pppstBlock = (LPEXCHANGEBLOCK *)malloc(sizeof(LPEXCHANGEBLOCK)*iExcCnt);
	
	if (0 == *pppstBlock)
	{
		fprintf (stderr, "Error:%s %d:malloc Fail:%s\n", 
			__FILE__, __LINE__, strerror(errno));
		return -1;
	}
	
	*piBlockSize = 0;
	pShm = pstExcMng->pShm;
	pShm += EXCBLOCKSIZE;
	
	for (i = 1; i < iExcCnt; i++)
	{
		agent_api_getid(pShm, &uiAppid, &uiBusid);
		if (uiAppid != 0 /*&& uiBusid != 0*/)
		{
			(*pppstBlock)[*piBlockSize] = (LPEXCHANGEBLOCK)malloc(sizeof(EXCHANGEBLOCK));
			if (0 == (*pppstBlock)[*piBlockSize])
			{
				fprintf (stderr, "Error:%s %d:malloc Fail:%s\n", 
					__FILE__, __LINE__, strerror(errno));
				return -1;
			}
			
			(*pppstBlock)[*piBlockSize]->uiAppid = uiAppid;
			(*pppstBlock)[*piBlockSize]->uiBusid = uiBusid;
			(*pppstBlock)[*piBlockSize]->pBlk = pShm;
			
			(*piBlockSize) ++;
		}
		
		pShm += EXCBLOCKSIZE;
	}
	
	return 0;
}

char *agent_api_get_freebufptr(LPEXCHANGEMNG pstExcMng, unsigned int uiAppid, unsigned int uiBusid)
{
	LPEXCHANGEBLOCK pstBlock = 0;
	pstBlock = exchange_find_ids(pstExcMng, uiAppid, uiBusid);
	
	if (!pstBlock)
	{
		return NULL;
	}
	
	return pstBlock->pBlk + EXCHEADSIZE;
}

int agent_api_register(LPEXCHANGEMNG pstExcMng, unsigned int uiAppid, unsigned int uiBusid)
{
	int i=0;
	char *pShm = 0;
	
	unsigned int uiTmpAppid=0,uiTmpBussid=0;
	
	if (pstExcMng == NULL)
	{
		return -1;
	}
	else
	{
		pShm = pstExcMng->pShm;
		if (NULL == pShm)
		{
			return -1;
		}
	}
	
	pShm = &pstExcMng->pShm[EXCBLOCKSIZE];
	semacquire(pstExcMng->aSem);
	for (i = 1; i < EXCHANGESIZE / EXCBLOCKSIZE; i++)
	{
		agent_api_getid(pShm, &uiTmpAppid, &uiTmpBussid);
		if (uiAppid == uiTmpAppid && uiBusid == uiTmpBussid)
		{
			//已经注册过
			if (!exchange_find_ids(pstExcMng, uiAppid, uiBusid))
			{
				exchange_new(pstExcMng, uiAppid, uiBusid, pShm);
			}
			
			semrelease(pstExcMng->aSem);
			return 0;
		}
		else if (0 == uiTmpAppid /*&& 0 == uiTmpBussid*/)
		{
			//新注册
			agent_api_setid(pShm, uiAppid, uiBusid);
			exchange_new(pstExcMng, uiAppid, uiBusid, pShm);
			
			semrelease(pstExcMng->aSem);
			return 0;
		}
		
		pShm += EXCBLOCKSIZE;
	}
	
	semrelease(pstExcMng->aSem);
	return -2;
}

int agent_api_update_exchange(
	LPEXCHANGEMNG pstExcMng, 
	unsigned int uiAppid, 
	unsigned int uiBusid, 
	char *szbuf, 
	unsigned int off, 
	size_t size)
{
	LPEXCHANGEBLOCK pstBlock = 0;
	if (NULL == pstExcMng)
	{
		return -1;
	}
	
	if ((off+size) > EXCBLOCKSIZE-EXCHEADSIZE)
	{
		//缓冲益处
		return -3;
	}
	
	if ((pstBlock = exchange_find_ids(pstExcMng,uiAppid,uiBusid)))
	{
		memcpy(pstBlock->pBlk + EXCHEADSIZE, szbuf, size);
		return 0;
	}
	
	//没有对应
    return -2;
}

int semacquire(void *sem) 
{
	struct sembuf sb = { 0, -1, SEM_UNDO };
	if (sem && semop(*(int *)sem, &sb, 1) < 0)
	{
		return -1;
	}

	return 0;
}

int semrelease(void *sem)
{
	struct sembuf sb = { 0, 1, SEM_UNDO };
	if (sem && semop(*(int *)sem, &sb, 1) < 0)
	{
		return -1;
	}

	return 0;
}

int semreset(void *sem, int val)
{
	/* I think semaphores with UNDO won't need resetting... */
	(void)sem;
	(void)val;

	return 0;
}

void semdestroy(void *sem)
{
	union semun u;
	u.val = 1;

	semctl(*(int *)sem, 0, IPC_RMID, u);
}

void *semcreate(key_t key, int val)
{
	int *semid = (int *)malloc(sizeof(int));
	union semun u;

	u.val = val;

	if (semid == NULL)
	{
		return NULL;
	}

	if ((*semid = semget(key, 1, 0666)) < 0)
	{
		*semid = semget(key, 1, IPC_CREAT | IPC_EXCL | 0666);
		if (*semid < 0)
		{
			return NULL;
		}
	}
	
	if (semctl(*semid, 0, SETVAL, u) < 0)
	{
		return NULL;
	}

	return (void *)semid;
}

