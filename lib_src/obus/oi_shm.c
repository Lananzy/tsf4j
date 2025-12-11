
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "oi_shm.h"

char* GetShm(int iKey, int iSize, int iFlag)
{
key_t key;
int iShmID;
char* sShm;
char sErrMsg[50];

	if ((iShmID = shmget(iKey, iSize, iFlag)) < 0) {
		sprintf(sErrMsg, "shmget %d %d", iKey, iSize);
		perror(sErrMsg);
		return NULL;
	}
	if ((sShm = shmat(iShmID, NULL ,0)) == (char *) -1) {
		perror("shmat");
		return NULL;
	}
	return sShm;
}

int GetShm2(void **pstShm, int iShmID, int iSize, int iFlag)
{
char* sShm;

	if (!(sShm = GetShm(iShmID, iSize, iFlag & (~IPC_CREAT)))) {
		if (!(iFlag & IPC_CREAT)) return -1;
		if (!(sShm = GetShm(iShmID, iSize, iFlag))) return -1;
		
		memset(sShm, 0, iSize);
	}
	*pstShm = sShm;
	return 0;
}

int GetShm3(void **pstShm, int iShmID, int iSize, int iFlag)
{
char* sShm;

	if (!(sShm = GetShm(iShmID, iSize, iFlag & (~IPC_CREAT)))) {
		if (!(iFlag & IPC_CREAT)) return -1;
		if (!(sShm = GetShm(iShmID, iSize, iFlag))) return -1;
		
		*pstShm = sShm;
		return 1;
	}
	*pstShm = sShm;
	return 0;
}

int GetShmAttachByKey(int iKey, int iSize, int iFlag)
{
	int iShmID;
	int iAttach;

	iShmID = shmget(iKey, iSize, iFlag);
	if( iShmID<0 )
		return -1;

	return GetShmAttach(iShmID);
}

int GetShmAttach(int iShmID)
{
	struct shmid_ds stDs;

	if( shmctl(iShmID, IPC_STAT, &stDs)<0 )
		return -1;

	return stDs.shm_nattch;
}

