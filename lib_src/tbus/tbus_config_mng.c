#include <errno.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "taa/tagentapi.h"
#include "tbus/tbus_error.h"
#include "tbus_config_mng.h"
#include "tbus_log.h"
#include "tbus_kernel.h"

#ifdef WIN32
#define tbus_stat _stat
#else
#define tbus_stat stat
#endif

static int tbus_gen_shmkey(OUT key_t *ptShmkey, IN const char *a_pszShmkey, 
						   IN int a_iBussId);

/**  check whether the string is decimal-digit string
*@return  nonzero if the string is decimal-digit,otherwise return zero
*/
static int tbus_is_decimal_string(IN const char *a_pszShmkey);





/**Try to attach share memory
*@param[in,out] ppShm To get the address of share memory
*@param[in] a_tKey	Key of share memory
*@param[in] a_iShmSize size of share memory
*@param[in] a_iSHmFlag control flags to attach share memory
*@param[in] a_iTimeout If the share memory do not exist, try to attach again, until time out
*@retval	0 success
*@retval <0 failed
*/
static int tbus_attach_shm(INOUT void **ppShm, IN key_t a_tKey, IN unsigned int *a_piShmSize,
					   IN int a_iSHmFlag, IN int a_iTimeout);

static int tbus_create_shm(INOUT void **ppShm, IN key_t a_tKey, IN unsigned int a_iShmSize,
						  IN int a_iSHmFlag, OUT int *a_piCreate);

static int tbus_create_file(const char *a_pszFile);

static int tbus_check_gcim_conf_i(IN LPTBUSGCIM a_pstGCIM, LPTBUSADDRTEMPLET a_pstAddrTemplet);

static int tbus_is_channel_configured_i(IN LPTBUSGCIM a_pstGCIM, LPTBUSSHMCHANNELCNF pstShmChl);

static int tbus_delete_channel_shm_i(LPTBUSSHMCHANNELCNF a_pstShmChl);



static int tbus_create_channel_shm_i(LPTBUSSHMCHANNELCNF a_pstShmChl, LPTBUSSHMGCIMHEAD a_pstHead);

static int tbus_find_channel_in_gcim_i(IN LPTBUSSHMGCIM a_pstShmGCIM,  LPCHANNELCNF pstChl);

static int tbus_mmap_open(INOUT void **ppShm, IN key_t a_tKey, IN unsigned int *a_iSize, IN int a_iFlag,
	OUT int *a_piCreate);

////////////////////////////////////////////////////////////////////////////////////
int tbus_get_gcimshm(INOUT LPTBUSSHMGCIM *ppstGCIM, IN const char *a_pszShmkey, 
					 IN int a_iBussId, IN unsigned int a_iShmSize, IN int a_iTimeout)
{
	int iRet = 0;
	key_t tShmkey = -1;
	int iFlags = 0;
	LPTBUSSHMGCIM pstGCIM = NULL;
	int iSize;	
	LPTBUSSHMGCIMHEAD pstHead;
#ifdef WIN32
	int iCreate = 0;
#endif

	assert(NULL != ppstGCIM);

	if ((NULL == a_pszShmkey) || ('\0' == a_pszShmkey))
	{

		a_pszShmkey = TBUS_DEFAULT_GCIM_KEY;
		tbus_log(TLOG_PRIORITY_DEBUG,"null shmkey, so use the default shmkey %s", TBUS_DEFAULT_GCIM_KEY);
	}
	iRet = tbus_gen_shmkey(&tShmkey, a_pszShmkey, a_iBussId);
	if (TBUS_SUCCESS != iRet)
	{
		tbus_log(TLOG_PRIORITY_ERROR,"failed to generate shmkey, by shmkey<%s> and bussId<%d>, for %s", 
			a_pszShmkey, a_iBussId, tbus_error_string(iRet));
		return iRet;
	}
	

	iSize = a_iShmSize;
#ifndef WIN32 
	iFlags = TSHM_DFT_ACCESS;
	iRet = tbus_attach_shm((void **)&pstGCIM, tShmkey, (unsigned int *)&iSize, iFlags,
			a_iTimeout);	
#else
	iFlags = TMMAPF_READ | TMMAPF_WRITE|TMMAPF_EXCL;
	iRet = tbus_mmap_open((void **)&pstGCIM, tShmkey, &iSize, iFlags, &iCreate);
#endif /*#ifndef WIN32 */
	if ( 0 > iRet )
	{
		tbus_log(TLOG_PRIORITY_ERROR,"failed attch GCIM by  generate shmkey, by shmkey<%s> and bussId<%d>, for %s", 
			a_pszShmkey, a_iBussId, tbus_error_string(iRet));
		return iRet;
	}

	/* share memory avaiable now */
	pstHead = &pstGCIM->stHead;
	if (iSize < (int)sizeof(TBUSSHMGCIMHEAD))
	{
		tbus_log(TLOG_PRIORITY_FATAL,"share memory size %d is less than the size of TUBSSHMHEADER\n",
			iSize, sizeof(TBUSSHMGCIMHEAD)) ;
		return TBUS_ERR_MAKE_ERROR(TBUS_ERROR_CHECK_GCIMSHM_FAILED);
	}
	if (tbus_check_shmgcimheader_stamp(pstHead) != 0)
	{
		tbus_log(TLOG_PRIORITY_ERROR,"failed to check GCIM  shm stamp, shmkey %d\n", tShmkey);
		return TBUS_ERR_MAKE_ERROR(TBUS_ERROR_CHECK_GCIMSHM_FAILED);
	}
	if ((key_t)pstHead->dwShmKey != tShmkey)
	{
		tbus_log(TLOG_PRIORITY_ERROR,"share memory key %d not match to the settings key:%d\n",
			pstHead->dwShmKey, tShmkey);
		return TBUS_ERR_MAKE_ERROR(TBUS_ERROR_CHECK_GCIMSHM_FAILED);
	}
	if (iSize != (int)pstHead->dwShmSize)
	{
		tbus_log(TLOG_PRIORITY_ERROR,"share memory size not match, real size %d, but need size %d",
			iSize, pstHead->dwShmSize);
		iRet = TBUS_ERR_MAKE_ERROR(TBUS_ERROR_CHECK_GCIMSHM_FAILED);
	}/*if ( 0 != iRet )*/

	*ppstGCIM = pstGCIM;	

	return iRet ;
}




int tbus_create_gcimshm(INOUT LPTBUSSHMGCIM *a_ppstGCIM, IN const char *a_pszShmkey, 
						IN int a_iBussId, IN unsigned int a_iShmSize)
{

	int	iRet = TBUS_SUCCESS ;
	int iCreate = 0;
	LPTBUSSHMGCIM pstGCIM = NULL;
	key_t tShmkey = -1;
	int iFlags;

	assert(NULL != a_ppstGCIM);


	if ((NULL == a_pszShmkey) || ('\0' == a_pszShmkey))
	{
		a_pszShmkey = TBUS_DEFAULT_GCIM_KEY;
		tbus_log(TLOG_PRIORITY_DEBUG,"null shmkey, so use the default shmkey %s", TBUS_DEFAULT_GCIM_KEY);
	}
	iRet = tbus_gen_shmkey(&tShmkey, a_pszShmkey, a_iBussId);
	if (TBUS_SUCCESS != iRet)
	{
		tbus_log(TLOG_PRIORITY_ERROR,"failed to generate shmkey, by shmkey<%s> and bussId<%d>, for %s", 
			a_pszShmkey, a_iBussId, tbus_error_string(iRet));
		return iRet;
	}

#ifndef WIN32
	iFlags = TSHM_DFT_ACCESS |IPC_CREAT|IPC_EXCL ;
	iRet = tbus_create_shm((void **)&pstGCIM, tShmkey, a_iShmSize, iFlags, &iCreate);
#else
	iFlags = TMMAPF_READ | TMMAPF_WRITE|TMMAPF_CREATE;
	iRet = tbus_mmap_open((void **)&pstGCIM, tShmkey, &a_iShmSize, iFlags, &iCreate);
#endif /*#ifndef WIN32*/
	if (TBUS_SUCCESS != iRet)
	{
		tbus_log(TLOG_PRIORITY_ERROR,"failed to create GCIM share memory, iRet :%x", iRet); 
		return iRet;
	}

	if (iCreate)
	{
		LPTBUSSHMGCIMHEAD pstHead = &pstGCIM->stHead;
		pthread_rwlockattr_t rwlattr;

		pthread_rwlockattr_init(&rwlattr);
		pthread_rwlockattr_setpshared(&rwlattr, PTHREAD_PROCESS_SHARED);
		pthread_rwlock_init(&pstHead->stRWLock, &rwlattr);
		
		tbus_wrlock(&pstHead->stRWLock);		
		pstHead->dwBusiID = a_iBussId;
		pstHead->dwCreateTime = (unsigned int)time(NULL);
		pstHead->dwMaxCnt = TBUS_MAX_CHANNEL_NUM_PREHOST;
		pstHead->dwShmKey = tShmkey;
		pstHead->dwShmSize = a_iShmSize;
		pstHead->dwUsedCnt = 0;
		pstHead->dwVersion = 0;
		pstHead->iAlign = TBUS_DEFAULT_CHANNEL_DATA_ALIGN;
		TBUS_CALC_ALIGN_LEVEL(pstHead->iAlignLevel, pstHead->iAlign);
		memset(&pstHead->stAddrTemplet, 0, sizeof(pstHead->stAddrTemplet));
		memset(&pstHead->reserve, 0, sizeof(pstHead->reserve));
		tbus_set_shmgcimheader_stamp(pstHead);
		tbus_unlock(&pstHead->stRWLock);
	}

	*a_ppstGCIM = pstGCIM;

	return iRet ;
}

/** 根据字符串信息，生成
*@param[in] a_pszShmkey	保存生产GCIM共享内存key的信息串，此信息串中的信息只能为十进制数字
*	串或为一个文件的路径（当此文件不存在时，tbus系统会尝试生成此文件），根据此信息串生成GCIM共享内存key的算法是:
*	- 十进制数字串	则将此数字串转换整数，此整数作为GCIM的共享内存的key
*	- 文件路径	将此文件路径和a_iBussId作为参数，调用ftok接口生产GCIM共享内存的key
*/
int tbus_gen_shmkey(OUT key_t *ptShmkey, IN const char *a_pszShmkey, 
						   IN int a_iBussId)
{
	int iRet = TBUS_SUCCESS;
	key_t tkey;

	assert(NULL != ptShmkey);
	assert(NULL != a_pszShmkey);

	if ('\0' == a_pszShmkey)
	{
		tbus_log(TLOG_PRIORITY_ERROR,"a_pszShmkey %s is empty string, cannot generate shm key", a_pszShmkey);
		return TBUS_ERR_MAKE_ERROR(TBUS_ERROR_GEN_SHMKEY_FAILED);
	}

	if (tbus_is_decimal_string(a_pszShmkey))
	{
		tkey = (key_t)atoi(a_pszShmkey);
		if (0 > tkey)
		{
			tbus_log(TLOG_PRIORITY_ERROR,"invalid shmkey<%s>, decimal value trasformed by shmkey string is less than zero",
				a_pszShmkey);
			iRet = TBUS_ERR_MAKE_ERROR(TBUS_ERROR_GEN_SHMKEY_FAILED);
		}else
		{
			tbus_log(TLOG_PRIORITY_DEBUG,"gen shmkey<%d> by  info shmkey<%s> and bussid<%d>",
				tkey, a_pszShmkey, a_iBussId);
		}/*if (0 > tkey)*/
	}else
	{
		TFSTAT stFileStat;

		iRet = tbus_stat(a_pszShmkey, &stFileStat);
		if ((0 != iRet) && (errno == ENOENT))
		{
			/*file is not exist, so create */
			iRet = tbus_create_file(a_pszShmkey);			
			if (0 != iRet)
			{
				tbus_log(TLOG_PRIORITY_ERROR,"failed to create file<%s>",	a_pszShmkey);
				return TBUS_ERR_MAKE_ERROR(TBUS_ERROR_GEN_SHMKEY_FAILED);
			}			
		}/*if ((0 != iRet) && (errno == ENOENT))*/		

		tkey = ftok(a_pszShmkey, a_iBussId);
		if (0 > tkey)
		{
			tbus_log(TLOG_PRIORITY_ERROR,"ftok failed by shmkey<%s> and bussid<%d>, for %s",
				a_pszShmkey, a_iBussId, strerror(errno));
			iRet = TBUS_ERR_MAKE_ERROR(TBUS_ERROR_GEN_SHMKEY_FAILED);
		}else
		{
			tbus_log(TLOG_PRIORITY_DEBUG,"gen shmkey<%d> by  info shmkey<%s> and bussid<%d>",
				tkey, a_pszShmkey, a_iBussId);
		}/*if (0 > tkey)*/
	}/*if (tbus_is_decimal_string(a_pszShmkey))*/	

	*ptShmkey = tkey;
	return iRet;
}

int tbus_create_file(const char *a_pszFile)
{
	int iRet = TBUS_SUCCESS;
	FILE *fp;

	assert(NULL != a_pszFile);
	
	iRet = tos_mkdir_by_path(a_pszFile);
	if (0 != iRet)
	{
		tbus_log(TLOG_PRIORITY_ERROR,"failed to mkdir by %s, %s",	a_pszFile, strerror(errno));
		return iRet;
	}

	fp = fopen(a_pszFile, "w");
	if (NULL == fp)
	{
		tbus_log(TLOG_PRIORITY_ERROR,"failed to open file %s to read %s, %s",	a_pszFile, strerror(errno));
		return iRet;
	}
	fclose(fp);

	return iRet;
}

/**  check whether the string is decimal-digit string
*@return  nonzero if the string is decimal-digit,otherwise return zero
*/
int tbus_is_decimal_string(IN const char *a_pszShmkey)
{
	const char *pch;

	assert(NULL != a_pszShmkey);
	for (pch = a_pszShmkey; *pch != '\0'; pch++)
	{
		if (isspace(*pch))
		{
			continue;
		}
		if (!isdigit(*pch))
		{
			return 0;
		}
	}

	return 1;
}

void tbus_set_shmgcimheader_stamp(LPTBUSSHMGCIMHEAD pstHeader)
{
	assert(NULL != pstHeader);


	pstHeader->dwLastStamp = (unsigned int)time(NULL);
	pstHeader->dwCRC = pstHeader->dwVersion ^ pstHeader->dwCreateTime ^ pstHeader->dwLastStamp;
	pstHeader->dwCRC ^= pstHeader->dwShmKey;
	pstHeader->dwCRC ^= pstHeader->dwShmSize;
	pstHeader->dwCRC ^= pstHeader->dwBusiID;
	pstHeader->dwCRC ^= pstHeader->iAlign;
}


int tbus_check_shmgcimheader_stamp(LPTBUSSHMGCIMHEAD pstHeader)
{
	unsigned int dwTempInt = 0;
	assert(NULL != pstHeader);

	dwTempInt = pstHeader->dwVersion ^ pstHeader->dwCreateTime ^ pstHeader->dwLastStamp;
	dwTempInt ^= pstHeader->dwShmKey;
	dwTempInt ^= pstHeader->dwShmSize;
	dwTempInt ^= pstHeader->dwBusiID;
	dwTempInt ^= pstHeader->dwCRC;
	dwTempInt ^= pstHeader->iAlign;

	return (int)dwTempInt;
}


void tbus_set_shmgrmheader_stamp(LPTBUSSHMGRMHEAD pstHeader)
{
	assert(NULL != pstHeader);


	pstHeader->dwLastStamp = (unsigned int)time(NULL);
	pstHeader->dwCRC = pstHeader->dwVersion ^ pstHeader->dwCreateTime ^ pstHeader->dwLastStamp;
	pstHeader->dwCRC ^= pstHeader->dwShmKey;
	pstHeader->dwCRC ^= pstHeader->dwShmSize;
	pstHeader->dwCRC ^= pstHeader->dwBusiID;

}

int tbus_check_shmgrmheader_stamp(LPTBUSSHMGRMHEAD pstHeader)
{
	unsigned int dwTempInt = 0;
	assert(NULL != pstHeader);

	dwTempInt = pstHeader->dwVersion ^ pstHeader->dwCreateTime ^ pstHeader->dwLastStamp;
	dwTempInt ^= pstHeader->dwShmKey;
	dwTempInt ^= pstHeader->dwShmSize;
	dwTempInt ^= pstHeader->dwBusiID;
	dwTempInt ^= pstHeader->dwCRC;


	return (int)dwTempInt;
}

int tbus_attach_shm(INOUT void **a_ppShm, IN key_t a_tKey, IN unsigned int *a_piShmSize,
						   IN int a_iSHmFlag, IN int a_iTimeout)
{
	int iShmID = -1,
		iRet = TBUS_SUCCESS ;
	int iTimeCount;
	struct shmid_ds stShmStat ;

	assert(NULL != a_ppShm);
	assert(!(a_iSHmFlag & IPC_CREAT));

	iTimeCount = 0;
	while (1)
	{
		iShmID = shmget ( a_tKey, *a_piShmSize, a_iSHmFlag) ;
		if ( 0 <= iShmID )
		{
			break;
		}

		/*如果共享内存不存在则尝试再次挂载*/
		if (errno != ENOENT)
		{
			tbus_log(TLOG_PRIORITY_ERROR,"shmget failed by key %d, for %s\n", a_tKey, strerror(errno));
			break;
		}
		if (iTimeCount >= a_iTimeout)
		{
			break;
		}
		tbus_log(TLOG_PRIORITY_DEBUG,"shmget failed by key %d, for %s, so try to attach again\n", a_tKey, strerror(errno));
		tos_usleep(TBUS_SLEEP_PRELOOP*1000);
		iTimeCount += TBUS_SLEEP_PRELOOP;
				
	}/*while (1)*/
	
	if (0 > iShmID)
	{
		tbus_log(TLOG_PRIORITY_ERROR,"shmget failed by key %d, for %s, timeout value is %d\n", 
			a_tKey, strerror(errno), a_iTimeout);
		return TBUS_ERR_MAKE_ERROR(TBUS_ERROR_SHMGET_FAILED);
	}

	*a_ppShm =	shmat ( iShmID, NULL, 0 ) ;
	if (NULL == *a_ppShm)
	{
		tbus_log(TLOG_PRIORITY_ERROR,"shmat failed by id %d, for %s\n", iShmID, strerror(errno));
		return TBUS_ERR_MAKE_ERROR(TBUS_ERROR_SHMAT_FAILED);
	}
	if ( 0 == shmctl( iShmID, IPC_STAT, &stShmStat))
	{
		*a_piShmSize = (int)stShmStat.shm_segsz;
	}
	
	return iRet;
}

int tbus_register_bussid(IN int a_iBussId)
{
	int iRet = TBUS_SUCCESS;

	LPEXCHANGEMNG pstMng = NULL;
#ifndef WIN32
	iRet = agent_api_init(&pstMng);
	if (0 != iRet)
	{
		tbus_log(TLOG_PRIORITY_ERROR,"agent_api_init failed, iRet=%d", iRet);
		iRet = TBUS_ERR_MAKE_ERROR(TBUS_ERROR_REGISTER_BUSSID);
	}

	iRet = agent_api_register(pstMng, ID_APPID_BUSCONFIG, a_iBussId);
	if (0 != iRet)
	{
		tbus_log(TLOG_PRIORITY_ERROR,"agent_api_init failed, iRet=%d", iRet);
		iRet = TBUS_ERR_MAKE_ERROR(TBUS_ERROR_REGISTER_BUSSID);
	}
#else
	//TODO tagentapi在windows环境下的接口实现还需要调整
#endif
	return iRet;
}

int tbus_create_shm(INOUT void **ppShm, IN key_t a_tKey, IN unsigned int a_iShmSize,
						   IN int a_iSHmFlag, OUT int *a_piCreate)
{
	int iShmID;

	assert(NULL != ppShm);
	assert(NULL != a_piCreate);


	a_iSHmFlag &= ~IPC_CREAT;
	a_iSHmFlag &= ~IPC_EXCL;
	*a_piCreate = 0;
	iShmID = shmget(a_tKey, 0, a_iSHmFlag) ;
	if (0 <= iShmID)
	{
		/*share memory is exist*/
		struct shmid_ds stShmStat ;
		tbus_log(TLOG_PRIORITY_DEBUG,"shmget shmid %d by  shmkey<%d>", iShmID, a_tKey);
		if ( 0 != shmctl( iShmID, IPC_STAT, &stShmStat))
		{
			tbus_log(TLOG_PRIORITY_ERROR,"shmctl IPC_STAT failed, by shmid %d, %s", iShmID, strerror(errno));
			return TBUS_ERR_MAKE_ERROR(TBUS_ERROR_SHMGET_FAILED); 
		}
		if (a_iShmSize != (unsigned int)stShmStat.shm_segsz)
		{
			tbus_log(TLOG_PRIORITY_ERROR,"shm size %d is not equal the size %d expected, so delete it and then create",
				(int)stShmStat.shm_segsz, a_iShmSize);
			if( shmctl(iShmID, IPC_RMID, NULL) )
			{
				tbus_log(TLOG_PRIORITY_ERROR,"Remove share memory(id:%d) failed, %s.\n", iShmID, strerror(errno));
				return TBUS_ERR_MAKE_ERROR(TBUS_ERROR_SHMGET_FAILED); 
			}
			iShmID = -1;
		}/*if (a_iShmSize != (int)stShmStat.shm_segsz)*/
	}else
	{
		tbus_log(TLOG_PRIORITY_DEBUG,"shmget failed  by shmkey<%d> size:%u flag:0x%x, %s", a_tKey,0, 
			a_iSHmFlag, strerror(errno));
	}/*if (0 <= iShmID)*/

	/*if not exist, try to create*/
	if (0 > iShmID)
	{
		a_iSHmFlag |= IPC_CREAT;
		a_iSHmFlag |= IPC_EXCL;
		iShmID = shmget(a_tKey, a_iShmSize, a_iSHmFlag) ;
		if (0 > iShmID)
		{
			tbus_log(TLOG_PRIORITY_ERROR,"failed to create share memory(key:%d) %s.\n", a_tKey, strerror(errno));
			return TBUS_ERR_MAKE_ERROR(TBUS_ERROR_SHMGET_FAILED); 
		}
		*a_piCreate = 1;
	}/*if (0 > iShmID)*/

	

	*ppShm = (LPTBUSSHMGCIM)shmat ( iShmID, NULL, 0 ) ;
	if (NULL == *ppShm)
	{
		tbus_log(TLOG_PRIORITY_ERROR,"create gcim failed by <key:%d, id:%d>, for %s\n",
			a_tKey, iShmID, strerror(errno));
		return TBUS_ERR_MAKE_ERROR(TBUS_ERROR_SHMAT_FAILED);
	}	
	tbus_log(TLOG_PRIORITY_DEBUG,"create/attach GCIM, shmkey:%d, size:%d, flag:0x%x",
		a_tKey, a_iShmSize, a_iSHmFlag ) ;

	return TBUS_SUCCESS;
}

int tbus_check_gcim_channels(IN LPTBUSSHMGCIM a_pstShmGCIM, IN LPTBUSGCIM a_pstGCIM)
{
	int iRet = TBUS_SUCCESS;
	LPTBUSSHMGCIMHEAD pstHead;
	unsigned int i;
	LPTBUSSHMCHANNELCNF pstShmChl;
	time_t tNow;

	assert(NULL != a_pstShmGCIM);
	assert(NULL != a_pstGCIM);

	pstHead = &a_pstShmGCIM->stHead;
	tNow = time(NULL);

	if ((0 > a_pstGCIM->iChannelDataAlign) ||(TBUS_MAX_CHANNEL_DATA_ALIGN < a_pstGCIM->iChannelDataAlign))
	{
		a_pstGCIM->iChannelDataAlign = TBUS_DEFAULT_CHANNEL_DATA_ALIGN;
	}

	for (i = 0; i < pstHead->dwUsedCnt; i++)
	{
		pstShmChl = &a_pstShmGCIM->astChannels[i];
		if (tbus_is_channel_configured_i(a_pstGCIM, pstShmChl))
		{
			/*check the share memory for channel*/
			TBUS_GCIM_CHANNEL_SET_ENABLE(pstShmChl);
			iRet = tbus_check_channel_shm_i(pstShmChl, pstHead);
			continue;
		}

		/*the channel is not in configure, so delete it*/
		tbus_log(TLOG_PRIORITY_FATAL,"channel(0x%08x <--> 0x%08x) is not in config list, so disable it",
			pstShmChl->astAddrs[0], pstShmChl->astAddrs[1]);
		if (TBUS_GCIM_CHANNEL_IS_ENABLE(pstShmChl))
		{
			TBUS_GCIM_CHANNEL_CLR_ENABLE(pstShmChl);
			pstShmChl->dwInvalidTime = tNow;
		}else if ((tNow - pstShmChl->dwInvalidTime) >= TBUS_DISABLE_CHANNEL_CLEAR_DISABLE_TIMEOUTGAP)
		{
			tbus_log(TLOG_PRIORITY_FATAL,"the time which the channel(0x%08x <--> 0x%08x) is not in config list reach %d seconds, so delete it",
				pstShmChl->astAddrs[0], pstShmChl->astAddrs[1], TBUS_DISABLE_CHANNEL_CLEAR_DISABLE_TIMEOUTGAP);
			tbus_delete_channel_shm_i(pstShmChl);
			if (i < (pstHead->dwUsedCnt -1))
			{
				memcpy(&a_pstShmGCIM->astChannels[i], &a_pstShmGCIM->astChannels[pstHead->dwUsedCnt -1],
					sizeof(TBUSSHMCHANNELCNF));
			}				
			pstHead->dwUsedCnt--;
			i--;	
		}/*if (pstShmChl->dwFlag & TBUS_ROUTE_VALID)*/			
	}/*for (i = 0; i < pstHead->dwUsedCnt; i++)*/

	return iRet;
}

int tbus_set_gcim(IN LPTBUSSHMGCIM a_pstShmGCIM, IN LPTBUSGCIM a_pstGCIM)
{
	int iRet = TBUS_SUCCESS;
	LPTBUSSHMGCIMHEAD pstHead;
	unsigned int i;
	LPTBUSSHMCHANNELCNF pstShmChl;

	assert(NULL != a_pstShmGCIM);
	assert(NULL != a_pstGCIM);

	
	pstHead = &a_pstShmGCIM->stHead;
	tbus_wrlock(&pstHead->stRWLock);


	/*分析地址模板*/
	iRet = tbus_parse_addrtemplet(&pstHead->stAddrTemplet, a_pstGCIM->szAddrTemplet);
	
	if (TBUS_SUCCESS == iRet)
	{
		iRet = tbus_check_gcim_conf_i(a_pstGCIM, &pstHead->stAddrTemplet);
	}

	/*设置数据对齐方式*/
	if (pstHead->iAlign != a_pstGCIM->iChannelDataAlign)
	{
		pstHead->iAlign = a_pstGCIM->iChannelDataAlign;
		TBUS_CALC_ALIGN_LEVEL(pstHead->iAlignLevel, pstHead->iAlign);
	}
	
	/*先检测SHM 已经配置的通道项，如果这些通道项不在配置列表中，则在共享内存中清除此配置项*/
	if (TBUS_SUCCESS == iRet)
	{
		tbus_check_gcim_channels(a_pstShmGCIM, a_pstGCIM);		
	}/*if (TBUS_SUCCESS == iRet)*/

	

	/*添加新的配置项*/
	if (TBUS_SUCCESS == iRet)
	{
		for (i = 0; i < a_pstGCIM->dwCount; i++)
		{
			LPCHANNELCNF pstChl = &a_pstGCIM->astChannels[i];
			int idx = tbus_find_channel_in_gcim_i(a_pstShmGCIM, pstChl);
			if (0 <= idx)
			{	
				/*指定地址的通道已经存在则更新优先级*/
				pstShmChl = &a_pstShmGCIM->astChannels[idx];
				pstShmChl->dwPriority = pstChl->dwPriority;
				continue;
			}
				
			if (pstHead->dwUsedCnt >= pstHead->dwMaxCnt)
			{
				tbus_log(TLOG_PRIORITY_ERROR,"channel num reach the max limit(%d) of GCIM, so cannot add channel",
					pstHead->dwMaxCnt);
				iRet = TBUS_ERR_MAKE_ERROR(TBUS_ERROR_CHANNEL_NUM_LIMITED);
				break;
			}
			pstShmChl = &a_pstShmGCIM->astChannels[pstHead->dwUsedCnt];
			memset(pstShmChl, 0, sizeof(TBUSSHMCHANNELCNF));
			pstShmChl->astAddrs[0] = pstChl->interAddr[0];
			pstShmChl->astAddrs[1] = pstChl->interAddr[1];
			pstShmChl->dwCTime = (unsigned int)time(NULL);
			pstShmChl->dwPriority = pstChl->dwPriority;
			pstShmChl->dwSendSize = pstChl->dwSendSize;
			TBUS_CALC_ALIGN_VALUE(pstShmChl->dwSendSize, pstHead->iAlign);
			pstShmChl->dwRecvSize = pstChl->dwRecvSize;
			TBUS_CALC_ALIGN_VALUE(pstShmChl->dwRecvSize, pstHead->iAlign);
			pstShmChl->dwFlag = TBUS_CHANNEL_FLAG_ENABLE;
			pstShmChl->iShmID = -1;
			iRet = tbus_check_channel_shm_i(pstShmChl, pstHead);
			if (TBUS_SUCCESS != iRet)
			{
				tbus_log(TLOG_PRIORITY_ERROR,"failed to create share memory for channel(address %d <--> address %d), iRet %x",
					pstShmChl->astAddrs[0], pstShmChl->astAddrs[1], iRet);
				break;
			}
			pstHead->dwUsedCnt++;
		}/*for (i = 0; i < a_pstGCIM->dwCount; i++)*/
	}

	pstHead->dwVersion++;
	tbus_set_shmgcimheader_stamp(pstHead);
	tbus_unlock(&pstHead->stRWLock);


	return iRet;
}

int tbus_find_channel_in_gcim_i(IN LPTBUSSHMGCIM a_pstShmGCIM,  LPCHANNELCNF pstChl)
{
	unsigned int i = -1;

	assert(NULL != a_pstShmGCIM);
	assert(NULL != pstChl);

	for (i = 0; i < a_pstShmGCIM->stHead.dwUsedCnt; i++)
	{
		LPTBUSSHMCHANNELCNF pstChannels = &a_pstShmGCIM->astChannels[i];
		if ((pstChl->interAddr[0] == pstChannels->astAddrs[0]) && 
			(pstChl->interAddr[1] == pstChannels->astAddrs[1]))
		{
			break;
		}
	}/*for (i = 0; i < a_pstShmGCIM->stHead.dwUsedCnt; i++)*/
	if (i >= a_pstShmGCIM->stHead.dwUsedCnt)
	{
		return -1;
	}

	return i;
}

/**检查共享内存中配置的某通道是否在配置列表中
*return 在配置列表中返回非零值，否则返回0
*/
int tbus_is_channel_configured_i(IN LPTBUSGCIM a_pstGCIM, LPTBUSSHMCHANNELCNF pstShmChl)
{
	int iIsConfigured = 0;
	unsigned int i = 0;

	assert(NULL!= a_pstGCIM);
	assert(NULL != pstShmChl);

	for (i = 0; i < a_pstGCIM->dwCount; i++)
	{
		LPCHANNELCNF pstChlCnf = &a_pstGCIM->astChannels[i];
		if ((pstChlCnf->interAddr[0] == pstShmChl->astAddrs[0]) && 
			(pstChlCnf->interAddr[1] == pstShmChl->astAddrs[1]))
		{
			iIsConfigured = 1;
			break;
		}
	}/*for (i = 0; i < a_pstGCIM->dwCount; i++)*/

	return iIsConfigured;
}

int tbus_check_channel_shm_i(LPTBUSSHMCHANNELCNF a_pstShmChl, LPTBUSSHMGCIMHEAD a_pstHead)
{
	
	int iRet = TBUS_SUCCESS;

	assert(NULL != a_pstShmChl);
	assert(NULL != a_pstHead);
	
	/*检查通道是否存在*/
#ifdef WIN32
	{
		//TODO windows下shmget不支持IPC_PRIVATE,目前仅仅是临时实现
		char szKey[TIPC_MAX_NAME];
		int iShmID;

		snprintf(szKey, sizeof(szKey), "%u_%u", (unsigned int)a_pstShmChl->astAddrs[0], 
			(unsigned int)a_pstShmChl->astAddrs[1]);

		iShmID = (int) OpenFileMapping(FILE_MAP_ALL_ACCESS, 0, szKey );
		if (0 == iShmID)
		{
			iRet = -1;
		}/*if (0 == iShmID)*/
		CloseHandle((HANDLE)iShmID);
	}	
#else
	{
		struct shmid_ds stShmStat ;
		iRet = shmctl( a_pstShmChl->iShmID, IPC_STAT, &stShmStat);
	}	
#endif/*#ifdef WIN32*/
	if (0 == iRet)
	{
		return TBUS_SUCCESS;
	}

	/*如果不存在则创建*/
	tbus_log(TLOG_PRIORITY_FATAL,"chmctl IPC_STAT failed, for %s, channel shm(id: %d) ,so recreate",
		strerror(errno), a_pstShmChl->iShmID);
	//tbus_delete_channel_shm_i(a_pstShmChl);
	iRet = tbus_create_channel_shm_i(a_pstShmChl, a_pstHead);


	return iRet;
}

int tbus_create_channel_shm_i(LPTBUSSHMCHANNELCNF a_pstShmChl, LPTBUSSHMGCIMHEAD a_pstHead)
{
	int iRet = TBUS_SUCCESS;
	int	iSize;
	char *sShm;
	CHANNELHEAD *pstHead ;
	int i;
	int iFlags;
	int iChannelHeadLen;

	assert(NULL != a_pstShmChl);
	assert(NULL != a_pstHead);

	iChannelHeadLen = sizeof(CHANNELHEAD);
	TBUS_CALC_ALIGN_VALUE(iChannelHeadLen, a_pstHead->iAlign);
	iSize = iChannelHeadLen + a_pstShmChl->dwRecvSize + a_pstShmChl->dwSendSize;
#ifdef WIN32
	{
		//TODO windows下shmget不支持IPC_PRIVATE,目前仅仅是临时实现
		char szKey[TIPC_MAX_NAME];
		SECURITY_ATTRIBUTES sa;

		snprintf(szKey, sizeof(szKey), "%u_%u", (unsigned int)a_pstShmChl->astAddrs[0], 
			(unsigned int)a_pstShmChl->astAddrs[1]);
#ifdef WINNT
		sa.lpSecurityDescriptor	=	tos_get_sd();
#else
		sa.lpSecurityDescriptor	=	NULL;
#endif #ifdef WINNT
		a_pstShmChl->iShmID	=	(int) CreateFileMapping(INVALID_HANDLE_VALUE, &sa, PAGE_READWRITE, 0, 
			(DWORD)iSize, szKey );
		if( (0 < a_pstShmChl->iShmID) &&  ERROR_ALREADY_EXISTS==GetLastError() )
		{
			CloseHandle((HANDLE)a_pstShmChl->iShmID);
			a_pstShmChl->iShmID	=	-1;
		}

#ifdef WINNT
		tos_free_sd(sa.lpSecurityDescriptor);
#endif /*#ifdef WINNT*/
		tbus_log(TLOG_PRIORITY_TRACE,"create shm channel, shmid %d by key %s", a_pstShmChl->iShmID, szKey);
	}	
#else
	iFlags = TSHM_DFT_ACCESS | IPC_CREAT;
	a_pstShmChl->iShmID = shmget(IPC_PRIVATE, iSize, iFlags);
#endif	/*#ifdef WIN32*/
	if (0 > a_pstShmChl->iShmID)
	{
		tbus_log(TLOG_PRIORITY_ERROR,"shmget IPC_PRIVATE failed to create share momory(size:%d), for %s",
			iSize, strerror(errno));
		return TBUS_ERR_MAKE_ERROR(TBUS_ERROR_SHMCTL_FAILED);
	}

	
	sShm = shmat(a_pstShmChl->iShmID, NULL , 0);
	if (NULL == sShm)
	{
		tbus_log(TLOG_PRIORITY_ERROR,"shmat  failed to attach share momory(id:%d), for %s",
			a_pstShmChl->iShmID, strerror(errno));
		shmctl(a_pstShmChl->iShmID, IPC_RMID, NULL);
		a_pstShmChl->iShmID = -1;
		return TBUS_ERR_MAKE_ERROR(TBUS_ERROR_SHMAT_FAILED);
	}


	pstHead = (CHANNELHEAD *)sShm;
	memset(pstHead, 0, sizeof(CHANNELHEAD));
	for (i = 0; i < TBUS_CHANNEL_SIDE_NUM; i++)
	{
		pstHead->astAddr[i] = a_pstShmChl->astAddrs[i];		
	}	
	pstHead->dwAlignLevel = a_pstHead->iAlignLevel;
	pstHead->astQueueVar[0].dwSize =  a_pstShmChl->dwRecvSize;
	pstHead->astQueueVar[1].dwSize =  a_pstShmChl->dwSendSize;
	shmdt(sShm);
		
	return iRet;
}



int tbus_delete_channel_shm_i(LPTBUSSHMCHANNELCNF a_pstShmChl)
{
	struct shmid_ds stShmStat ;

	assert(NULL != a_pstShmChl);
	
	if ( 0 != shmctl( a_pstShmChl->iShmID, IPC_STAT, &stShmStat))
	{
		tbus_log(TLOG_PRIORITY_ERROR,"shmctl IPC_STAT failed, shmid %d, %s",a_pstShmChl->iShmID,
			strerror(errno));
		return TBUS_ERR_MAKE_ERROR(TBUS_ERROR_SHMCTL_FAILED);
	}
	if (0 < (int)stShmStat.shm_nattch)
	{
		return TBUS_SUCCESS;
	}

	tbus_log(TLOG_PRIORITY_ERROR,"delete share memory(id %d)", a_pstShmChl->iShmID);
	if( shmctl(a_pstShmChl->iShmID, IPC_RMID, NULL) )
	{
		tbus_log(TLOG_PRIORITY_ERROR,"Remove share memory(id:%d) failed, %s.\n", a_pstShmChl->iShmID, strerror(errno));
		return TBUS_ERR_MAKE_ERROR(TBUS_ERROR_SHMCTL_FAILED); 
	}
	a_pstShmChl->iShmID = -1;

	return TBUS_SUCCESS;
}

int tbus_check_gcim_conf_i(IN LPTBUSGCIM a_pstGCIM, LPTBUSADDRTEMPLET a_pstAddrTemplet)
{
	int iRet = TBUS_SUCCESS;
	unsigned int i = 0;

	/*检查配置信息*/
	if (TBUS_MAX_CHANNEL_NUM_PREHOST < a_pstGCIM->dwCount)
	{
		tbus_log(TLOG_PRIORITY_ERROR,"channel count(%d) is invalid, it must be 0~%d\n",a_pstGCIM->dwCount,
			TBUS_MAX_CHANNEL_NUM_PREHOST);
		return TBUS_ERR_MAKE_ERROR(TBUS_ERROR_INVALID_GCIM_CONF);
	}
	for (i = 0; i < a_pstGCIM->dwCount; i++)
	{
		LPCHANNELCNF pstChlCnf = &a_pstGCIM->astChannels[i];
		if (TBUS_CHANNEL_SIDE_NUM > pstChlCnf->dwAddressCount)
		{
			tbus_log(TLOG_PRIORITY_ERROR,"%dth channel configure is invalid, the address count is less than %d",
				i+1, pstChlCnf->dwAddressCount);
			iRet = TBUS_ERR_MAKE_ERROR(TBUS_ERROR_INVALID_GCIM_CONF);
			break;
		}

		iRet = tbus_addr_aton_by_addrtemplet(a_pstAddrTemplet, pstChlCnf->aszAddress[0], 
			(LPTBUSADDR)&pstChlCnf->interAddr[0]);
		if (TBUS_SUCCESS != iRet)
		{
			tbus_log(TLOG_PRIORITY_ERROR,"failed to convert %s to tbusd address, please check %dth channel configure",
				pstChlCnf->aszAddress[0], i+1);
			iRet = TBUS_ERR_MAKE_ERROR(TBUS_ERROR_INVALID_GCIM_CONF);
			break;
		}
		iRet = tbus_addr_aton_by_addrtemplet(a_pstAddrTemplet, pstChlCnf->aszAddress[1], 
			(LPTBUSADDR)&pstChlCnf->interAddr[1]);
		if (TBUS_SUCCESS != iRet)
		{
			tbus_log(TLOG_PRIORITY_ERROR,"failed to convert %s to tbusd address, please check %dth channel configure",
				pstChlCnf->aszAddress[1], i+1);
			iRet = TBUS_ERR_MAKE_ERROR(TBUS_ERROR_INVALID_GCIM_CONF);
			break;
		}
		if (pstChlCnf->interAddr[0] == pstChlCnf->interAddr[1])
		{
			tbus_log(TLOG_PRIORITY_ERROR,"the two address of channel must be different, but %dth channel was configured the same address %s",
				i+1, tbus_addr_nota_by_addrtemplet(a_pstAddrTemplet, pstChlCnf->interAddr[0]));
			iRet = TBUS_ERR_MAKE_ERROR(TBUS_ERROR_INVALID_GCIM_CONF);
			break;
		}

		/*adjust the order of address, first set the litter*/
		if (pstChlCnf->interAddr[0] > pstChlCnf->interAddr[1])
		{
			unsigned int dwSize = pstChlCnf->dwRecvSize;
			TBUSADDR tTmp = pstChlCnf->interAddr[0];
			pstChlCnf->dwRecvSize = pstChlCnf->dwSendSize;
			pstChlCnf->dwSendSize = dwSize;
			pstChlCnf->interAddr[0] = pstChlCnf->interAddr[1];
			pstChlCnf->interAddr[1] = tTmp;
		}
	}/*for (i = 0; i < a_pstGCIM->dwCount; i++)*/

	TBUS_NORMALIZE_ALIGN(a_pstGCIM->iChannelDataAlign);
	return iRet;
}

int tbus_delete_channel_by_index(IN LPTBUSSHMGCIM a_pstShmGCIM, IN int a_idx)
{
	int iRet = TBUS_SUCCESS;
	LPTBUSSHMGCIMHEAD pstHead ;
	LPTBUSSHMCHANNELCNF pstShmChl;

	assert(NULL != a_pstShmGCIM);

	pstHead = &a_pstShmGCIM->stHead;
	if ((0 > a_idx) || (a_idx >= (int)pstHead->dwUsedCnt))
	{
		tbus_log(TLOG_PRIORITY_ERROR,"invalid channel index(%d), it must be in 0~%d",
			a_idx, pstHead->dwUsedCnt);
		return TBUS_ERR_ARG;
	}

	tbus_wrlock(&pstHead->stRWLock);		

	
	pstShmChl = &a_pstShmGCIM->astChannels[a_idx];
	tbus_delete_channel_shm_i(pstShmChl);
	if (a_idx < (int)(pstHead->dwUsedCnt -1))
	{
		memcpy(&a_pstShmGCIM->astChannels[a_idx], &a_pstShmGCIM->astChannels[pstHead->dwUsedCnt -1],
			sizeof(TBUSSHMCHANNELCNF));
	}				
	pstHead->dwUsedCnt--;
	
	pstHead->dwVersion++;
	tbus_set_shmgcimheader_stamp(pstHead);
	tbus_unlock(&pstHead->stRWLock);

	return iRet;
}
int tbus_create_grmshm(INOUT LPTBUSSHMGRM *a_ppstGRM, IN const char *a_pszShmkey, 
					   IN int a_iBussId, IN unsigned int a_iShmSize)
{

	int	iRet = TBUS_SUCCESS ;
	int iCreate = 0;
	LPTBUSSHMGRM pstGRM = NULL;
	key_t tShmkey = -1;
	int iFlags;

	assert(NULL != a_ppstGRM);


	if ((NULL == a_pszShmkey) || ('\0' == a_pszShmkey))
	{

		a_pszShmkey = TBUS_DEFAULT_GRM_KEY;
		tbus_log(TLOG_PRIORITY_DEBUG,"null shmkey, so use the default shmkey %s", TBUS_DEFAULT_GRM_KEY);
	}
	iRet = tbus_gen_shmkey(&tShmkey, a_pszShmkey, a_iBussId);
	if (TBUS_SUCCESS != iRet)
	{
		tbus_log(TLOG_PRIORITY_ERROR,"failed to generate shmkey, by shmkey<%s> and bussId<%d>, for %s", 
			a_pszShmkey, a_iBussId, tbus_error_string(iRet));
		return iRet;
	}

#ifndef WIN32
	iFlags = TSHM_DFT_ACCESS |IPC_CREAT|IPC_EXCL ;
	iRet = tbus_create_shm((void **)&pstGRM, tShmkey, a_iShmSize, iFlags, &iCreate);
#else
	iFlags = TMMAPF_READ | TMMAPF_WRITE|TMMAPF_CREATE;
	iRet = tbus_mmap_open((void **)&pstGRM, tShmkey, &a_iShmSize, iFlags, &iCreate);
#endif /*#ifndef WIN32*/
	if (TBUS_SUCCESS != iRet)
	{
		tbus_log(TLOG_PRIORITY_ERROR,"failed to create GCIM share memory, iRet :%x", iRet); 
		return iRet;
	}

	if (iCreate)
	{
		LPTBUSSHMGRMHEAD pstHead = &pstGRM->stHead;
		pthread_rwlockattr_t rwlattr;

		pthread_rwlockattr_init(&rwlattr);
		pthread_rwlockattr_setpshared(&rwlattr, PTHREAD_PROCESS_SHARED);
		pthread_rwlock_init(&pstHead->stRWLock, &rwlattr);

		tbus_wrlock(&pstHead->stRWLock);		
		pstHead->dwBusiID = a_iBussId;
		pstHead->dwCreateTime = (unsigned int)time(NULL);
		pstHead->dwMaxCnt = TBUS_MAX_RELAY_NUM_PREHOST;
		pstHead->dwShmKey = tShmkey;
		pstHead->dwShmSize = a_iShmSize;
		pstHead->dwUsedCnt = 0;
		pstHead->dwVersion = 0;
		memset(&pstHead->stAddrTemplet, 0, sizeof(pstHead->stAddrTemplet));
		memset(&pstHead->reserve, 0, sizeof(pstHead->reserve));
		tbus_set_shmgrmheader_stamp(pstHead);
		tbus_unlock(&pstHead->stRWLock);
	}

	*a_ppstGRM = pstGRM;

	return iRet ;
}

int tbus_get_grmshm(INOUT LPTBUSSHMGRM *a_ppstGRM, IN const char *a_pszShmkey, 
					 IN int a_iBussId, IN unsigned int a_iShmSize)
{
	int iRet = 0;
	key_t tShmkey = -1;
	int iFlags;
	LPTBUSSHMGRM pstGRM = NULL;
	int iSize;	
	LPTBUSSHMGRMHEAD pstHead;
#ifdef WIN32
	int iCreate = 0;
#endif

	assert(NULL != a_ppstGRM);

	if ((NULL == a_pszShmkey) || ('\0' == a_pszShmkey))
	{

		a_pszShmkey =TBUS_DEFAULT_GRM_KEY;
		tbus_log(TLOG_PRIORITY_DEBUG,"null shmkey, so use the default shmkey %s", TBUS_DEFAULT_GRM_KEY);
	}
	iRet = tbus_gen_shmkey(&tShmkey, a_pszShmkey, a_iBussId);
	if (TBUS_SUCCESS != iRet)
	{
		tbus_log(TLOG_PRIORITY_ERROR,"failed to generate shmkey, by shmkey<%s> and bussId<%d>, for %s", 
			a_pszShmkey, a_iBussId, tbus_error_string(iRet));
		return iRet;
	}


	iSize = a_iShmSize;
#ifndef WIN32 
	iFlags = TSHM_DFT_ACCESS;
	iRet = tbus_attach_shm((void **)&pstGRM, tShmkey, (unsigned int *)&iSize, iFlags,
		0);	
#else
	iFlags = TMMAPF_READ | TMMAPF_WRITE|TMMAPF_EXCL;
	iRet = tbus_mmap_open((void **)&pstGRM, tShmkey, &iSize, iFlags, &iCreate);
#endif /*#ifndef WIN32 */
	if ( 0 > iRet )
	{
		tbus_log(TLOG_PRIORITY_ERROR,"failed attch GRM by  generate shmkey, by shmkey<%s> and bussId<%d>, for %s", 
			a_pszShmkey, a_iBussId, tbus_error_string(iRet));
		return iRet;
	}


	/* share memory avaiable now */
	pstHead = &pstGRM->stHead;
	if (iSize < (int)sizeof(TBUSSHMGRMHEAD))
	{
		tbus_log(TLOG_PRIORITY_FATAL,"share memory size %d is less than the size of TUBSSHMHEADER\n",
			iSize, sizeof(TBUSSHMGRMHEAD)) ;
		return TBUS_ERR_MAKE_ERROR(TBUS_ERROR_CHECK_GCIMSHM_FAILED);
	}
	if (tbus_check_shmgrmheader_stamp(pstHead) != 0)
	{
		tbus_log(TLOG_PRIORITY_ERROR,"failed to check GCIM  shm stamp, shmkey %d\n", tShmkey);
		return TBUS_ERR_MAKE_ERROR(TBUS_ERROR_CHECK_GCIMSHM_FAILED);
	}
	if ((key_t)pstHead->dwShmKey != tShmkey)
	{
		tbus_log(TLOG_PRIORITY_ERROR,"share memory key %d not match to the settings key:%d\n",
			pstHead->dwShmKey, tShmkey);
		return TBUS_ERR_MAKE_ERROR(TBUS_ERROR_CHECK_GCIMSHM_FAILED);
	}
	if (iSize != (int)pstHead->dwShmSize)
	{
		tbus_log(TLOG_PRIORITY_ERROR,"share memory size not match, real size %d, but need size %d",
			iSize, pstHead->dwShmSize);
		iRet = TBUS_ERR_MAKE_ERROR(TBUS_ERROR_CHECK_GCIMSHM_FAILED);
	}/*if ( 0 != iRet )*/
	
	*a_ppstGRM = pstGRM;

	return iRet ;
}

int tbus_set_grm(IN LPTBUSSHMGRM a_pstShmGRM, IN LPRELAYMNGER a_pstRelayConf)
{
	int iRet = TBUS_SUCCESS;
	LPTBUSSHMGRMHEAD pstHead;
	unsigned int i,j;
	LPTBUSSHMRELAYCNF pstShmRelay;

	assert(NULL != a_pstShmGRM);
	assert(NULL != a_pstRelayConf);

	pstHead = &a_pstShmGRM->stHead;
	tbus_wrlock(&pstHead->stRWLock);		

	/*分析地址模板*/
	iRet = tbus_parse_addrtemplet(&pstHead->stAddrTemplet, a_pstRelayConf->szAddrTemplet);
	if (TBUS_SUCCESS == iRet)
	{

		pstHead->dwUsedCnt = 0;
		for (i = 0; i < a_pstRelayConf->dwUsedCnt; i++)
		{
			LPSHMRELAY pstRelay = &a_pstRelayConf->astRelays[i];
			TBUSADDR iAddr;
			if (TBUS_SUCCESS != tbus_addr_aton_by_addrtemplet(&pstHead->stAddrTemplet,
				pstRelay->szAddr, &iAddr))
			{
				tbus_log(TLOG_PRIORITY_ERROR,"failed to convert %s to tbus address", pstRelay->szAddr);
				iRet = TBUS_ERR_MAKE_ERROR(TBUS_ERROR_INVALID_ADDR_STRING);
				break;
			}
			for (j = 0; j < pstHead->dwUsedCnt; j ++)
			{
				if (iAddr == a_pstShmGRM->astRelays[j].dwAddr)
				{
					break;
				}
			}
			if (j < pstHead->dwUsedCnt)
			{
				continue;
			}

			if (pstHead->dwUsedCnt >= pstHead->dwMaxCnt)
			{
				tbus_log(TLOG_PRIORITY_ERROR,"relay route num reach the max limit(%d) of GCIM, so cannot add channel",
					pstHead->dwMaxCnt);
				iRet = TBUS_ERR_MAKE_ERROR(TBUS_ERROR_RELAY_NUM_LIMITED);
				break;
			}
			pstShmRelay = &a_pstShmGRM->astRelays[pstHead->dwUsedCnt];
			memset(pstShmRelay, 0, sizeof(TBUSSHMRELAYCNF));
			pstShmRelay->dwAddr = iAddr;
			pstShmRelay->dwFlag = pstRelay->dwEnable;
			pstShmRelay->dwPriority = pstRelay->dwPriority;
			pstShmRelay->dwStrategy = pstRelay->dwStrategy;
			STRNCPY(pstShmRelay->szMConn, pstRelay->szMConn, sizeof(pstShmRelay->szMConn));
			STRNCPY(pstShmRelay->szSConn, pstRelay->szSConn, sizeof(pstShmRelay->szSConn));
			
			pstHead->dwUsedCnt++;
		}/*for (i = 0; i < a_pstGCIM->dwCount; i++)*/
	}/*if (TBUS_SUCCESS == iRet)*/
	


	pstHead->dwVersion++;
	tbus_set_shmgrmheader_stamp(pstHead);
	tbus_unlock(&pstHead->stRWLock);

	return iRet;
}

int tbus_wrlock(pthread_rwlock_t *a_pstRWLock)
{
	int iRet = TBUS_SUCCESS;
	int iTimeCount = 0;

	assert(NULL != a_pstRWLock);
#ifdef WIN32
	//TODO 目前pal的pthread_rwlock系列函数在windows平台下不支持PTHREAD_PROCESS_SHARED
#else
	/*很粗陋的设计,这样设计的前提是，正常情况读写锁的释放时间不会超过
	预定时间，如果超过了预定时间还没有获取到锁，则强制将锁夺过来*/
	for (iTimeCount = 0; iTimeCount < TBUS_RWLOCK_TIMEOUT;)
	{
		iRet = pthread_rwlock_trywrlock(a_pstRWLock);
		if (0 == iRet)
		{
			break;
		}
		tos_usleep(TBUS_SLEEP_PRELOOP*1000);
		iTimeCount += TBUS_SLEEP_PRELOOP;
	}

	if (iTimeCount >= TBUS_RWLOCK_TIMEOUT)
	{
		pthread_rwlockattr_t rwlattr;
		tbus_log(TLOG_PRIORITY_FATAL,"timeout(%d milliseconds) to acqure lock, so force to acqure it", iTimeCount);		

		pthread_rwlockattr_init(&rwlattr);
		pthread_rwlockattr_setpshared(&rwlattr, PTHREAD_PROCESS_SHARED);
		pthread_rwlock_init(a_pstRWLock, &rwlattr);
		pthread_rwlock_trywrlock(a_pstRWLock);
	}
#endif /*#ifdef WIN32*/
	return iRet;
}

int tbus_rdlock(pthread_rwlock_t *a_pstRWLock)
{
	int iRet = TBUS_SUCCESS;
	int iTimeCount;

	assert(NULL != a_pstRWLock);
#ifdef WIN32
	//TODO 目前pal的pthread_rwlock系列函数在windows平台下不支持PTHREAD_PROCESS_SHARED
#else
	/*很粗陋的设计,这样设计的前提是，正常情况读写锁的释放时间不会超过
	预定时间，如果超过了预定时间还没有获取到锁，则强制将锁夺过来*/
	for (iTimeCount = 0; iTimeCount < TBUS_RWLOCK_TIMEOUT;)
	{
		iRet = pthread_rwlock_tryrdlock(a_pstRWLock);
		if (0 == iRet)
		{
			break;
		}
		tos_usleep(TBUS_SLEEP_PRELOOP*1000);
		iTimeCount += TBUS_SLEEP_PRELOOP;
	}

	if (iTimeCount >= TBUS_RWLOCK_TIMEOUT)
	{
		pthread_rwlockattr_t rwlattr;
		tbus_log(TLOG_PRIORITY_FATAL,"timeout(%d milliseconds) to acqure lock, so force to acqure it", iTimeCount);		

		pthread_rwlockattr_init(&rwlattr);
		pthread_rwlockattr_setpshared(&rwlattr, PTHREAD_PROCESS_SHARED);
		pthread_rwlock_init(a_pstRWLock, &rwlattr);
		pthread_rwlock_tryrdlock(a_pstRWLock);
	}
#endif

	return iRet;
}

void tbus_unlock(pthread_rwlock_t *a_pstRWLock)
{
	assert(NULL != a_pstRWLock);
	pthread_rwlock_unlock(a_pstRWLock);	
}

int tbus_mmap_open(INOUT void **ppShm, IN key_t a_tKey, IN unsigned int *a_iSize, IN int a_iFlag,
	OUT int *a_piCreate)
{
	char szPath[TBUS_MAX_PATH_LEN];
	char szAppPath[TBUS_MAX_PATH_LEN];
	char *pszAppDataPath = NULL;
	int iRet = 0 ;
	int iMapid = -1;
	struct shmid_ds stShmStat ;

	assert(NULL != ppShm);
	assert(NULL != a_piCreate);
	
#ifdef WIN32
	iRet = GetEnvironmentVariable(TEXT(TBUS_APPDATA_PATH), &szAppPath[0], sizeof(szAppPath));
	if(!iRet)
	{
		tbus_log(TLOG_PRIORITY_ERROR,"GetEnvironmentVariable failed (%d) by name %s\n", GetLastError(), TBUS_APPDATA_PATH);
		pszAppDataPath = TBUS_DEFAULT_MMAP_FILE_DIR;
	}else
	{
		pszAppDataPath = &szAppPath[0];
	}
#else
	pszAppDataPath = getenv(TBUS_APPDATA_PATH);
	if (NULL == pszAppDataPath)
	{
		pszAppDataPath = TBUS_DEFAULT_MMAP_FILE_DIR;
	}
#endif
	iRet = snprintf(szPath, sizeof(szPath),  "%s%ctbus_%d", pszAppDataPath,TOS_DIRSEP, a_tKey);
	if ((0 > iRet) || (iRet >= (int)sizeof(szPath)))
	{
		tbus_log(TLOG_PRIORITY_ERROR,"failed to write mmap filepath,  iwrite(%d) sizeofBuff:%d", iRet, sizeof(szPath));
		return -1;
	}

	a_iFlag  &= ~TMMAPF_CREATE;
	a_iFlag  |= TMMAPF_EXCL;
	iMapid = tmmapopen(&szPath[0], *a_iSize, a_iFlag);
	if (0 > iMapid)
	{
		a_iFlag |= TMMAPF_CREATE;
		iMapid = tmmapopen(&szPath[0], *a_iSize, a_iFlag);	
		if (0 < iMapid)
		{
			*a_piCreate = 1;
		}
	}/*if (a_iFlag & TMMAPF_CREATE)*/
	if (0 > iMapid)
	{
		tbus_log(TLOG_PRIORITY_ERROR,"failed to tmmapopen by path(%s),  isize:%d", szPath, *a_iSize);
		return -1;	
	}

	*ppShm = tmmap(iMapid, 0, *a_iSize, TMMAPF_PROT_WRITE|TMMAPF_PROT_READ);
	if (NULL == *ppShm)
	{
		tbus_log(TLOG_PRIORITY_ERROR,"failed to tmmap by id(%d),  isize:%d", iMapid, *a_iSize);
		return -1;	
	}

	if ( 0 == shmctl( iMapid, IPC_STAT, &stShmStat))
	{
		*a_iSize = (int)stShmStat.shm_segsz;
	}

	return 0;
}

