#include "tsec/oi_crypt.h"
#include "tsec/oi_tea.h"

#include <stdlib.h>
#include <string.h>

#ifdef WIN32
#include <winsock2.h>
#else
#include <netinet/in.h>
#endif

#include <time.h>

typedef char BYTE;
typedef char BOOL;
typedef unsigned long DWORD;


#define CLUB_TEA_KEY	"0000000000000000"  /* IT'S A FAKE KEY. */

void OicqEncrypt(unsigned short shVersion, const BYTE* pInBuf, int nInBufLen, const BYTE* pKey, BYTE* pOutBuf, int *pOutBufLen) 
{
	return oi_symmetry_encrypt2(pInBuf, nInBufLen, pKey, pOutBuf, pOutBufLen);
}

BOOL OicqDecrypt(unsigned short shVersion, const BYTE* pInBuf, int nInBufLen, const BYTE* pKey, BYTE* pOutBuf, int *pOutBufLen)
{
	return oi_symmetry_decrypt2(pInBuf, nInBufLen, pKey, pOutBuf, pOutBufLen);
}

BOOL OicqDecrypt3(unsigned long lUin, unsigned short shVersion, const BYTE* pInBuf, int nInBufLen, const BYTE* pKey, BYTE* pOutBuf, int *pOutBufLen)
{
	int iRet, iOutBufLen;
	char cMainVer, cSubVer;
	
	cMainVer = shVersion / 100;
	cSubVer = shVersion % 100;
	
	iOutBufLen = *pOutBufLen;
	
	iRet = oi_symmetry_decrypt2(pInBuf, nInBufLen, pKey, pOutBuf, pOutBufLen);
	if (iRet != 0 || shVersion < 750) return iRet;
	
	*pOutBufLen = iOutBufLen;
	return qq_symmetry_decrypt3(pInBuf, nInBufLen, cMainVer, cSubVer, lUin, pKey, pOutBuf, pOutBufLen);
}

int MakeSigniture(short shVer, unsigned long lUin, short shFlag, const BYTE* pKey, BYTE* pOutBuf, int *pOutBufLen)
{
	char sInBuf[10];
	time_t lCurTime;
	int iInLen;
	
	time(&lCurTime);
	
	*(time_t *)sInBuf = htonl(lCurTime);
	*(long *)(sInBuf + 4) = htonl(lUin);
	
	if (shVer <= 700) 
	{
		*(sInBuf + 8) = shFlag;
		iInLen = 9;
	}	
	else 
	{
		//*(short *)(sInBuf + 8) = htons(shFlag);
		*(sInBuf + 8) = shFlag;
		*(sInBuf + 9) = shFlag;
		iInLen = 10;
	}
	
	OicqEncrypt(C2C_ENCRYPT_VERSION, sInBuf, iInLen, CLUB_TEA_KEY, pOutBuf, pOutBufLen);
	
	return 0;
}

int MakeValueAddedSigniture(unsigned long lUin, char *sValueAddedFlag, BYTE* pOutBuf)
{
	char sInBuf[20];
	time_t lCurTime;
	int iInLen, iOutBufLen=32;
	
	time(&lCurTime);
	
	*(long *)sInBuf = htonl(lUin);
	*(time_t *)(sInBuf + 4) = htonl(lCurTime);
	memcpy(sInBuf + 8, sValueAddedFlag, 12);
	
	iInLen = 20;
	
	OicqEncrypt(C2C_ENCRYPT_VERSION, sInBuf, iInLen, CLUB_TEA_KEY, pOutBuf, &iOutBufLen);
	
	return 0;
}

int MakeValueAddedSignitureNew(short shVer, unsigned long lUin, char *sValueAddedFlag, const BYTE* pKey, BYTE* pOutBuf, int *pOutBufLen)
{
	char sInBuf[20];
	time_t lCurTime;
	int iInLen;
	
	time(&lCurTime);
	
	*(long *)sInBuf = htonl(lUin);
	*(time_t *)(sInBuf + 4) = htonl(lCurTime);
	memcpy(sInBuf + 8, sValueAddedFlag, 12);
	
	iInLen = 20;
	
	OicqEncrypt(C2C_ENCRYPT_VERSION, sInBuf, iInLen, pKey, pOutBuf, pOutBufLen);
	
	return 0;
}
