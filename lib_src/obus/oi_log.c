
#include "oi_log.h"
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdarg.h>
#include <sys/time.h>

static int ShiftFiles(LogFile* pstLogFile)
{
   struct stat stStat;
	char sLogFileName[300];
	char sNewLogFileName[300];
	int i;
	struct tm stLogTm, stShiftTm;

   sprintf(sLogFileName,"%s.log", pstLogFile->sBaseFileName);

	if(stat(sLogFileName, &stStat) < 0) return -1;
	switch (pstLogFile->iShiftType) {
		case 0:
			if (stStat.st_size < pstLogFile->lMaxSize) return 0;
			break;
		case 2:
			if (stStat.st_mtime - pstLogFile->lLastShiftTime < pstLogFile->lMaxCount) return 0;
			break;
		case 3:
			if (pstLogFile->lLastShiftTime - stStat.st_mtime > 86400) break;
			memcpy(&stLogTm, localtime(&stStat.st_mtime), sizeof(stLogTm));
			memcpy(&stShiftTm, localtime(&pstLogFile->lLastShiftTime), sizeof(stShiftTm));
			if (stLogTm.tm_mday == stShiftTm.tm_mday) return 0;
			break;
		case 4:
			if (pstLogFile->lLastShiftTime - stStat.st_mtime > 3600) break;
			memcpy(&stLogTm, localtime(&stStat.st_mtime), sizeof(stLogTm));
			memcpy(&stShiftTm, localtime(&pstLogFile->lLastShiftTime), sizeof(stShiftTm));
			if (stLogTm.tm_hour == stShiftTm.tm_hour) return 0;
			break;
		case 5:
			if (pstLogFile->lLastShiftTime - stStat.st_mtime > 60) break;
			memcpy(&stLogTm, localtime(&stStat.st_mtime), sizeof(stLogTm));
			memcpy(&stShiftTm, localtime(&pstLogFile->lLastShiftTime), sizeof(stShiftTm));
			if (stLogTm.tm_min == stShiftTm.tm_min) return 0;
			break;
		default:
			if (pstLogFile->lLogCount < pstLogFile->lMaxCount) return 0;
			pstLogFile->lLogCount = 0;
	}

	// fclose(pstLogFile->pLogFile);

	for(i = pstLogFile->iMaxLogNum-2; i >= 0; i--)
	{
		if (i == 0)
			sprintf(sLogFileName,"%s.log", pstLogFile->sBaseFileName);
		else
			sprintf(sLogFileName,"%s%d.log", pstLogFile->sBaseFileName, i);
			
		if (access(sLogFileName, F_OK) == 0)
		{
			sprintf(sNewLogFileName,"%s%d.log", pstLogFile->sBaseFileName, i+1);
			if (rename(sLogFileName,sNewLogFileName) < 0 )
			{
				return -1;
			}
		}
	}
	// if ((pstLogFile->pLogFile = fopen(sLogFileName, "a+")) == NULL) return -1;
	time(&pstLogFile->lLastShiftTime);
	return 0;
}

long InitLogFile(LogFile* pstLogFile, char* sLogBaseName, long iShiftType, long iMaxLogNum, long iMAX)
{
char sLogFileName[300] = "";

	memset(pstLogFile, 0, sizeof(LogFile));
	strncat(pstLogFile->sLogFileName, sLogBaseName, sizeof(pstLogFile->sLogFileName) - 10);
	strcat(pstLogFile->sLogFileName, ".log");

	// if ((pstLogFile->pLogFile = fopen(sLogFileName, "a+")) == NULL) return -1;
	strncpy(pstLogFile->sBaseFileName, sLogBaseName, sizeof(pstLogFile->sBaseFileName) - 1);
	pstLogFile->iShiftType = iShiftType;
	pstLogFile->iMaxLogNum = iMaxLogNum;
	pstLogFile->lMaxSize = iMAX;
	pstLogFile->lMaxCount = iMAX;
	pstLogFile->lLogCount = iMAX;
	time(&pstLogFile->lLastShiftTime);

	return ShiftFiles(pstLogFile);
}

int Log(LogFile* pstLogFile, int iLogTime, char* sFormat, ...)
{
va_list ap;
struct timeval stLogTv;

	if ((pstLogFile->pLogFile = fopen(pstLogFile->sLogFileName, "a+")) == NULL) return -1;
	va_start(ap, sFormat);
	if (iLogTime == 1) {
		fprintf(pstLogFile->pLogFile, "[%s] ", CurrDateTimeStr());
	}
	else if (iLogTime == 2) {
		gettimeofday(&stLogTv, NULL);
		fprintf(pstLogFile->pLogFile, "[%s.%.6d] ", DateTimeStr(&(stLogTv.tv_sec)), stLogTv.tv_usec);
	}
	vfprintf(pstLogFile->pLogFile, sFormat, ap);
	fprintf(pstLogFile->pLogFile, "\n");
	va_end(ap);
	pstLogFile->lLogCount++;
	fclose(pstLogFile->pLogFile);
	return ShiftFiles(pstLogFile);
}

int Logv(LogFile* pstLogFile, char* sFormat, va_list ap) 
{
struct timeval stLogTv;

	if ((pstLogFile->pLogFile = fopen(pstLogFile->sLogFileName, "a+")) == NULL) return -1;

	gettimeofday(&stLogTv, NULL);
	fprintf(pstLogFile->pLogFile, "[%s.%.6d] ", DateTimeStr(&(stLogTv.tv_sec)), stLogTv.tv_usec);

	vfprintf(pstLogFile->pLogFile, sFormat, ap);
	fprintf(pstLogFile->pLogFile, "\n");
	va_end(ap);
	pstLogFile->lLogCount++;
	fclose(pstLogFile->pLogFile);
	return ShiftFiles(pstLogFile);
}

int Logv2(LogFile* pstLogFile, int iLogTime, char* sFormat, va_list ap) 
{
struct timeval stLogTv;

	if ((pstLogFile->pLogFile = fopen(pstLogFile->sLogFileName, "a+")) == NULL) return -1;

	if (iLogTime == 1) {
		fprintf(pstLogFile->pLogFile, "[%s] ", CurrDateTimeStr());
	}
	else if (iLogTime == 2) {
		gettimeofday(&stLogTv, NULL);
		fprintf(pstLogFile->pLogFile, "[%s.%.6d] ", DateTimeStr(&(stLogTv.tv_sec)), stLogTv.tv_usec);
	}

	vfprintf(pstLogFile->pLogFile, sFormat, ap);
	fprintf(pstLogFile->pLogFile, "\n");
	va_end(ap);
	pstLogFile->lLogCount++;
	fclose(pstLogFile->pLogFile);
	return ShiftFiles(pstLogFile);
}

