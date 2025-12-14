#include "pal/pal.h"
#include "tdr/tdr.h"
#include "tlog/tlogerr.h"
#include "tlog/tlogfile.h"

#define TLOGFILE_PROTO_PREFIX		"file://"
#define TLOGFILE_PROTO_LEN		7

int tlogfile_sync_i(TLOGFILE* a_pstLogFile, time_t a_tNow)
{
	int iRet=0;

	if( a_pstLogFile->stInst.fp && a_pstLogFile->stInst.fDirty )
	{
		iRet	=	fflush(a_pstLogFile->stInst.fp);

		if( 0==iRet )
		{
			a_pstLogFile->stInst.fDirty	=	0;
			a_pstLogFile->stInst.tLastSync	=	a_tNow;
		}
	}

	return iRet;
}

int tlogfile_sync(TLOGFILE* a_pstLogFile)
{
	return tlogfile_sync_i(a_pstLogFile, time(NULL));
}

int tlogfile_make_path_i(TLOGFILE* a_pstLogFile, char* a_pszBuff, int a_iBuff)
{
	char szPath[TLOGFILE_PATH_LEN];
	time_t tNow;
	struct tm stTm;
	int iLen;

	assert( a_pszBuff && a_iBuff>0 );

	tNow	=	a_pstLogFile->stInst.tLastMsg;

	if( a_pstLogFile->pstDev->iPrecision>0 )
	{
		tNow	+=	a_pstLogFile->pstDev->iPrecision - 1;
		tNow	=	tNow / a_pstLogFile->pstDev->iPrecision * a_pstLogFile->pstDev->iPrecision;
	}

	a_pszBuff[0]	=	'\0';
	
	localtime_r(&tNow, &stTm);

	szPath[0]	=	'0';

	if( 0==memcmp(a_pstLogFile->pstDev->szPattern, TLOGFILE_PROTO_PREFIX, TLOGFILE_PROTO_LEN) )
		strftime(szPath, sizeof(szPath), a_pstLogFile->pstDev->szPattern + TLOGFILE_PROTO_LEN, &stTm);
	else
		strftime(szPath, sizeof(szPath), a_pstLogFile->pstDev->szPattern, &stTm);

	iLen	=	strlen(szPath);

	if( strncmp(szPath, a_pstLogFile->stInst.szPath, iLen) )
		a_pstLogFile->stInst.iCurRotate	=	0;

	if( a_pstLogFile->pstDev->iMaxRotate>0 )
	{
		a_pstLogFile->stInst.iCurRotate	%=	a_pstLogFile->pstDev->iMaxRotate;
	}
	else
	{
		a_pstLogFile->stInst.iCurRotate	=	0;
	}

	if( a_pstLogFile->stInst.iCurRotate>0 )
		snprintf(a_pszBuff, a_iBuff, "%s.%d", szPath, a_pstLogFile->stInst.iCurRotate);
	else
		snprintf(a_pszBuff, a_iBuff, "%s", szPath);

	a_pszBuff[a_iBuff-1]	=	'\0';

	return 0;
}

int tlogfile_close_file_i(TLOGFILE* a_pstLogFile)
{
	if( a_pstLogFile->stInst.fp )
	{
		fclose(a_pstLogFile->stInst.fp);
		a_pstLogFile->stInst.fp	=	NULL;
	}

	a_pstLogFile->stInst.iTail	=	0;
	a_pstLogFile->stInst.fDirty	=	0;

	if( a_pstLogFile->stInst.pszBuff )
	{
		free(a_pstLogFile->stInst.pszBuff);
		a_pstLogFile->stInst.pszBuff	=	NULL;
	}

	return 0;
}

int tlogfile_open_file_i(TLOGFILE* a_pstLogFile, const char* a_pszMod)
{
	int iRet;

	if( a_pstLogFile->stInst.fp )
		return -1;

	a_pstLogFile->stInst.fp	=	fopen(a_pstLogFile->stInst.szPath, a_pszMod) ;

	if( !a_pstLogFile->stInst.fp )
	{
		if(ENOENT!=errno)
			return -1;

		iRet	=	tos_mkdir_by_path(a_pstLogFile->stInst.szPath);
		if( iRet<0 )
			return -1;

		a_pstLogFile->stInst.fp		=	fopen(a_pstLogFile->stInst.szPath, a_pszMod);
	}

	if( a_pstLogFile->stInst.fp )
	{
		fseek(a_pstLogFile->stInst.fp, 0, SEEK_END);
		a_pstLogFile->stInst.iTail	=	ftell(a_pstLogFile->stInst.fp);
	}

	if( a_pstLogFile->stInst.fp && a_pstLogFile->pstDev->iBuffSize>0 )
		a_pstLogFile->stInst.pszBuff	=	calloc(1, a_pstLogFile->pstDev->iBuffSize);

	if( a_pstLogFile->stInst.fp && a_pstLogFile->stInst.pszBuff )
		setvbuf(a_pstLogFile->stInst.fp, a_pstLogFile->stInst.pszBuff, _IOFBF, a_pstLogFile->pstDev->iBuffSize);


	return a_pstLogFile->stInst.fp ? 0 : -1;
}

int tlogfile_find_latest_i(TLOGFILE* a_pstLogFile)
{
	char szPath[TLOGFILE_PATH_LEN];
	TFSTAT stStat;
	int iOldRotate;
	time_t tLastModify;
	int iLatest;
	int iRet;

	if( a_pstLogFile->pstDev->iMaxRotate<=0 )
		return -1;

	iLatest		=	-1;

	iOldRotate	=	a_pstLogFile->stInst.iCurRotate;

	while( a_pstLogFile->stInst.iCurRotate<a_pstLogFile->pstDev->iMaxRotate )
	{
		tlogfile_make_path_i(a_pstLogFile, szPath, (int)sizeof(szPath));
		STRNCPY(a_pstLogFile->stInst.szPath, szPath, sizeof(a_pstLogFile->stInst.szPath));

		if( tflstat(szPath, &stStat)<0 )
		{
			a_pstLogFile->stInst.iCurRotate++;
			continue;
		}

		if( -1==iLatest )
		{
			iLatest		=	a_pstLogFile->stInst.iCurRotate;
			tLastModify	=	stStat.st_mtime;
		}
		else if( tLastModify<=stStat.st_mtime )
		{
			iLatest		=	a_pstLogFile->stInst.iCurRotate;
			tLastModify	=	stStat.st_mtime;
		}

		a_pstLogFile->stInst.iCurRotate++;
	}

	if( -1==iLatest )
	{
		iRet	=	-1;
		a_pstLogFile->stInst.iCurRotate	=	iOldRotate;
		tlogfile_make_path_i(a_pstLogFile, szPath, (int)sizeof(szPath));
		STRNCPY(a_pstLogFile->stInst.szPath, szPath, sizeof(a_pstLogFile->stInst.szPath));
	}
	else
	{
		iRet	=	0;
		a_pstLogFile->stInst.iCurRotate	=	iLatest;
		tlogfile_make_path_i(a_pstLogFile, szPath, (int)sizeof(szPath));
		STRNCPY(a_pstLogFile->stInst.szPath, szPath, sizeof(a_pstLogFile->stInst.szPath));
	}

	return iRet;
}

int tlogfile_stick_path_i(TLOGFILE* a_pstLogFile)
{
	
	char szPath[TLOGFILE_PATH_LEN];
	char szPathNext[TLOGFILE_PATH_LEN];
	int iTmpRotate;
	char *ptr;
		
	if( a_pstLogFile->pstDev->iMaxRotate<=1 )
		return 0;

	ptr = strrchr(a_pstLogFile->stInst.szPath, '.');
	if (NULL == ptr ||a_pstLogFile->stInst.iCurRotate != atoi(ptr+1))
		return -1;
	*ptr = 0;
	
	iTmpRotate = a_pstLogFile->pstDev->iMaxRotate - 2;
	while( 0 <= iTmpRotate )
	{
		if (0 == iTmpRotate)
		{
			snprintf(szPath, sizeof(szPath), "%s", a_pstLogFile->stInst.szPath);
		}
		else
		{
			snprintf(szPath, sizeof(szPath), "%s.%d", a_pstLogFile->stInst.szPath, iTmpRotate);
		}
		szPath[sizeof(szPath) - 1] = 0;

		if (0 != tfexist(szPath))
		{
			iTmpRotate--;
			continue;
		}

		snprintf(szPathNext, sizeof(szPathNext), "%s.%d", a_pstLogFile->stInst.szPath, iTmpRotate+1);
		szPathNext[sizeof(szPathNext) - 1] = 0;

		tfdelete(szPathNext);
		rename(szPath, szPathNext);
		
		iTmpRotate--;
	}

	a_pstLogFile->stInst.iCurRotate = 0;
		
	return 0;
}

int tlogfile_check_file_i(TLOGFILE* a_pstLogFile, int a_iMsgSize, time_t a_tNow)
{
	char szPath[TLOGFILE_PATH_LEN];
	int iSizeLimit;
	//int fLimit=0;
	int iRet;
	int iPrecision;
	time_t tLastMsg;

	if( a_iMsgSize<0 || a_iMsgSize>TLOGFILE_MAX_SIZE )
		return -1;

	tLastMsg			=	a_pstLogFile->stInst.tLastMsg;
	a_pstLogFile->stInst.tLastMsg	=	a_tNow;

	iPrecision			=	a_pstLogFile->pstDev->iPrecision;

	iSizeLimit			=	a_pstLogFile->pstDev->iSizeLimit;

	if( iSizeLimit<=0 )
		iSizeLimit		=	TLOGFILE_MAX_SIZE;

	if( a_pstLogFile->stInst.fp )
	{
		if( a_iMsgSize > iSizeLimit - a_pstLogFile->stInst.iTail )
		{
			//fLimit	=	1;
			a_pstLogFile->stInst.iCurRotate++;

			if( a_pstLogFile->pstDev->iMaxRotate>0 )
				a_pstLogFile->stInst.iCurRotate	%=	a_pstLogFile->pstDev->iMaxRotate;
			
			tlogfile_close_file_i(a_pstLogFile);

			a_pstLogFile->stInst.tLastSync	=	a_tNow;
		}
	}

	if( a_pstLogFile->stInst.fp && (0==iPrecision || a_tNow/iPrecision==tLastMsg/iPrecision ) )
	{
		return 0;
	}

	iRet	=	tlogfile_make_path_i(a_pstLogFile, szPath, (int)sizeof(szPath));
	if( iRet<0 )
		return -1;

	if( strcasecmp(szPath, a_pstLogFile->stInst.szPath) )
	{
		tlogfile_close_file_i(a_pstLogFile);

		a_pstLogFile->stInst.tLastSync	=	a_tNow;

		//change file path
		if (a_pstLogFile->pstDev->iRotateStick && 0 < a_pstLogFile->stInst.iCurRotate)
		{
			STRNCPY(a_pstLogFile->stInst.szPath, szPath, sizeof(a_pstLogFile->stInst.szPath));
			tlogfile_stick_path_i(a_pstLogFile);
		}
		else
		{
			STRNCPY(a_pstLogFile->stInst.szPath, szPath, sizeof(a_pstLogFile->stInst.szPath));
		}
	}

	if( !a_pstLogFile->stInst.fp )
	{
		tlogfile_open_file_i(a_pstLogFile, "wb+");
	}

	if( a_pstLogFile->stInst.fp )
		return 0;
	else
		return -1;
}

int tlogfile_init(TLOGFILE* a_pstLogFile, TLOGDEVFILE* a_pstDev)
{
	int iRet;
	time_t tNow;
	char szPath[TLOGFILE_PATH_LEN];

	assert( a_pstLogFile && a_pstDev);

	memset(a_pstLogFile, 0, sizeof(*a_pstLogFile));

	a_pstLogFile->pstDev	=	a_pstDev;

	time(&tNow);

	a_pstLogFile->stInst.tLastMsg	=	tNow;
	a_pstLogFile->stInst.tLastSync	=	tNow;

	tlogfile_make_path_i(a_pstLogFile, szPath, (int)sizeof(szPath));
	STRNCPY(a_pstLogFile->stInst.szPath, szPath, sizeof(a_pstLogFile->stInst.szPath));
	
	if( 0==a_pstLogFile->pstDev->iNoFindLatest && 0 == a_pstLogFile->pstDev->iRotateStick)
	{
		tlogfile_find_latest_i(a_pstLogFile);
	}

	iRet	=	tlogfile_open_file_i(a_pstLogFile, "ab+");

	if( iRet<0 )
	{
		printf("open log file %s fail\n", a_pstLogFile->stInst.szPath);
		tlogfile_fini(a_pstLogFile);

		return TLOG_ERR_MAKE_ERROR(TLOG_ERROR_FILE_OPEN);  
	}
	else
		return 0;

}

int tlogfile_writev(TLOGFILE* a_pstLogFile, const TLOGIOVEC* a_pstIOVec, int a_iCount)
{
	time_t tNow;
	int iRet;
	int i;
	int iSize;

	time(&tNow);

	iSize	=	0;

	for(i=0; i<a_iCount; i++)
	{
		iSize	+=	a_pstIOVec[0].iov_len;
	}

	iRet	=	tlogfile_check_file_i(a_pstLogFile, iSize, tNow);

	if( iRet<0 )
		return -1;

	if( iSize>0 )
	{
		a_pstLogFile->stInst.fDirty	=	1;
		a_pstLogFile->stInst.tLastMsg	=	tNow;

		for(i=0; i<a_iCount; i++)
		{
			if( a_pstIOVec[i].iov_len==fwrite((char*)a_pstIOVec[i].iov_base, 1, a_pstIOVec[i].iov_len, a_pstLogFile->stInst.fp) )
			{
				a_pstLogFile->stInst.iTail	+=	(int)a_pstIOVec[i].iov_len;
			}
			else
			{
				iRet	=	-1;
				break;
			}
		}
	}

	if( tNow - a_pstLogFile->stInst.tLastSync >= a_pstLogFile->pstDev->iSyncTime )
	{
		tlogfile_sync_i(a_pstLogFile, tNow);
	}

	return iRet;
}

int tlogfile_write(TLOGFILE* a_pstLogFile, const char* a_pszBuff, int a_iSize)
{
	TLOGIOVEC stIOVec;

	stIOVec.iov_base	=	(void*) a_pszBuff;
	stIOVec.iov_len		=	(size_t) a_iSize;

	return tlogfile_writev(a_pstLogFile, &stIOVec, 1);
}

int tlogfile_fini(TLOGFILE* a_pstLogFile)
{
	if( a_pstLogFile->stInst.fp )
	{
		fflush( a_pstLogFile->stInst.fp );
		fclose( a_pstLogFile->stInst.fp );
		a_pstLogFile->stInst.fp	=	NULL;
		a_pstLogFile->stInst.iTail	=	0;
	}

	if( a_pstLogFile->stInst.pszBuff )
	{
		free( a_pstLogFile->stInst.pszBuff );
		a_pstLogFile->stInst.pszBuff	=	NULL;
	}

	return 0;
}



