/*
**  @file $RCSfile: tini.c,v $
**  general description of this module
**  $Id: tini.c,v 1.2 2008/07/02 10:38:11 steve Exp $
**  @author $Author: steve $
**  @date $Date: 2008/07/02 10:38:11 $
**  @version $Revision: 1.2 $
**  @note Editor: Vim 6.1, Gcc 4.0.1, tab=4
**  @note Platform: Linux
*/

#include <assert.h>
#include "pal/pal.h"
#include "comm/tini.h"


#ifdef WIN32
#pragma warning(disable:4996)
#endif /* WIN32 */

#define B_TRUE			1
#define B_FALSE			0

#define STX				0x02
#define ETX				0x03
#define RS				0x1e
#define US				0x1f



int     g_iTiniCaseRestrict = 0;
/*
        0: No Restrict: Group | Ident all UPPER/LOWER to fit.
        1:    Restrict: Group | Ident all Restrict case fit.
*/


/************************************************************
 *    desc:    Left Trim the String inputed.
 ************************************************************/
void    vCfgsLTrim_i( char * sOprt )
{
    int nHead, nCurr;

    if( NULL == sOprt )
        return ;

    nHead = nCurr = 0;
    while( 0 == ( 0x80 & (unsigned int)(unsigned char)(*(sOprt+nCurr)) ) &&
        0 != isspace((int)*(sOprt+nCurr)) )
        nCurr++;
    while( '\0' != (*(sOprt+nCurr)) )
    {
        *(sOprt+nHead) = *(sOprt+nCurr);
        nHead++, nCurr++;
    }
    *(sOprt+nHead) = *(sOprt+nCurr);

    return ;
}


/************************************************************
 *    desc:    Right Trim the String inputed.
 ************************************************************/
void    vCfgsRTrim_i( char * sOprt )
{
    char *    sTail;

    if( NULL == sOprt )
        return ;

    if( 0 == (int)strlen(sOprt) )
        return ;
    sTail = sOprt+strlen(sOprt)-1;
    while( 0 == ( 0x80 & (unsigned int)(unsigned char)(*sTail) ) &&
            0 != isspace((int)*sTail) && sTail >= sOprt )
    {
        *sTail = '\0';
        sTail--;
    }

    return ;
}


/************************************************************
 *    desc:    Trim both end of the String inputed.
 ************************************************************/
void    vCfgsATrim( char * sOprt )
{
    if( NULL == sOprt )
        return ;
    vCfgsRTrim_i( sOprt );
    vCfgsLTrim_i( sOprt );
    return ;
}


/************************************************************
 *    desc:    Replace the Char Escaped by '\\' with SpecialChar.
 *        \b,\t,\n,\r,\TINI_CHARDIVIDE,\\,...
 *    rett:    void.
 ************************************************************/
void    vCfgsEscCh( char * sOprt, int bDivide )
{
    char    * sSrc, * sDst;
    int        nModified;

    if( NULL == sOprt )
        return ;

    sSrc = sDst = sOprt;
    nModified = 0;

    while( '\0' != *sSrc )
    {
        if( bDivide && TINI_CHARDIVIDE == *sSrc )
        {
            *sDst = '\0';
            return ;
        }
        else if( '\\' != *sSrc && 0 == nModified )
        {
            sSrc++;
            sDst++;
        }
        else if( '\\' != *sSrc && 0 != nModified )
        {
            *sDst = *sSrc;
            sSrc++;
            sDst++;
        }
        else
        {
            switch( *(sSrc+1) )
            {
            case    'b':
                *sDst = '\b';
                sSrc += 2;
                sDst += 1;
                break;
            case    'n':
                *sDst = '\n';
                sSrc += 2;
                sDst += 1;
                break;
            case    'r':
                *sDst = '\r';
                sSrc += 2;
                sDst += 1;
                break;
            case    't':
                *sDst = '\t';
                sSrc += 2;
                sDst += 1;
                break;
            case    TINI_CHARDIVIDE:
                *sDst = TINI_CHARDIVIDE;
                sSrc += 2;
                sDst += 1;
                break;
            case    '\0':
                sSrc += 1;
                break;
            default:
                sSrc += 1;
                break;
            }
            nModified += 1;
        }
    }    /*    end of while()_Loop */
    *sDst = '\0';

    return ;
}
/*************** end of vCfgsEscCh() *************************/

/************************************************************
 *    desc:    read the Item configed in the IniFile with String Value.
 *    rett:    char *.
 *    retv:    !=NULL:    success.
 *            NULL:    failed.
 *    memo:    No Trim at Content.
 ************************************************************/
int    tini_read( IN  char    *a_pszFileName,
                  IN  char    *a_pszGroupName,
                  IN  char    *a_pszIdentName, 
                  OUT char    *a_pszResult,
                  IN  int     a_iBufSize )
{
    FILE    *fp;
    char    szGName[ TINI_LINELENGTH ], szIName[ TINI_LINELENGTH ];
    char    szIdent[ TINI_LINELENGTH ];
    char    szGroup[ TINI_LINELENGTH ];
    char    *pszTmp;
    int        iTmp, iLen;


    if( NULL == a_pszFileName ||
        NULL == a_pszGroupName || 
        NULL == a_pszIdentName ||
        NULL == a_pszResult )
    {
        return -1;
    }

    fp = fopen( a_pszFileName, "r" );
    if( NULL == fp )
    {
        return -1;
    }

    strcpy( a_pszResult, "" );
    pszTmp = NULL;
    snprintf( szIName, sizeof(szIName), "%s", a_pszGroupName );
    vCfgsATrim( szIName );
    snprintf( szGName, sizeof(szGName), "[%s]", szIName );
    snprintf( szIName, sizeof(szIName), "%s", a_pszIdentName );
    vCfgsATrim( szIName );
    if( 0 == strlen(szGName) || 0 == strlen(szIName) )
    {
        fclose( fp );
        return -1;
    }

    if( 0 == g_iTiniCaseRestrict )
    {
        iLen = (int)strlen(szGName);
        for( iTmp=0; iTmp<iLen; iTmp++ )
        {
            szGName[iTmp] = (char)toupper(szGName[iTmp]);
        }
        iLen = (int)strlen(szIName);
        for( iTmp=0; iTmp<iLen; iTmp++ )
        {
            szIName[iTmp] = (char)toupper(szIName[iTmp]);
        }
    }

    /*    Find Group    */
    iLen = (int)strlen(szGName);   
    while( 1 )
    {
        if( NULL == fgets(szGroup,sizeof(szGroup),fp) )
        {
            fclose( fp );
            return -1;
        }
        vCfgsEscCh( szGroup, B_TRUE );
        vCfgsATrim( szGroup );
        if( (int)strlen(szGroup) != iLen )
            continue;
        for( iTmp=0; iTmp<iLen; iTmp++ )
            szGroup[iTmp] = (char)toupper(szGroup[iTmp]);
        if( 0 == strcmp(szGroup,szGName) )
            break;    /*    Group found */
    }

    /*   Find Ident */
    iLen = (int)strlen(szIName);    
    while( 1 )
    {
        if( NULL == fgets(szIdent,sizeof(szIdent),fp) )
        {
            fclose( fp );
            return -1;
        }
        if( '\n' == szIdent[strlen(szIdent)-1] )
            szIdent[strlen(szIdent)-1] = '\0';

        vCfgsEscCh( szIdent, B_TRUE );
        vCfgsLTrim_i( szIdent );
        if( TINI_LEFTSQUOTE == szIdent[0] &&
            NULL != strchr(szIdent,TINI_RIGHTQUOTE) )
        {
            fclose( fp );
            return -1;
        }
        for( iTmp=0; iTmp<iLen; iTmp++ )
            szIdent[iTmp] = (char)toupper(szIdent[iTmp]);
        if( 0 == strncmp(szIdent,szIName,iLen) )
        {
            pszTmp = szIdent+iLen;
            while( isspace(*pszTmp) )
                pszTmp++;
            if( TINI_CHARASSIGN != *pszTmp )
                continue;
            else
            {
                pszTmp++;    //    goto Content
                break;    
            }
        }
    }

    fclose( fp );

    vCfgsATrim( pszTmp );
    snprintf( a_pszResult, a_iBufSize-1, "%s", pszTmp );

    return 0;
}

static char* SkipSpace(register char* sStr)
{
	while(*sStr == ' ' || *sStr == '\t' || *sStr == '\r' || *sStr == '\n') sStr++;
	return sStr;
}


static char* TrimSpace(register char* sStr)
{
	int i;

	i = strlen(sStr) - 1;
	while (sStr[i] == ' ' || sStr[i] == '\t' || sStr[i] == '\r' || sStr[i] == '\n') sStr[i--] = 0;
	return sStr;
}

static char* nGetVal(char* desc, char* src, int iLen)
{
char *descp=desc, *srcp=src;
int mtime=0, space=0;
int iCurrLen = 0;

	while (mtime!=2 && *srcp != '\0' && iCurrLen < iLen - 1) {
		switch(*srcp) {
			case ' ':
			case '\t':
			case '\0':
			case '\n':
			case US:
				space=1;
				srcp++;
				break;
			default:
				if (space||srcp==src) mtime++;
				space=0;
				if ( mtime==2 ) break;
				*descp=*srcp;
				descp++;
				iCurrLen++;
				srcp++;
		}
	}
	*descp='\0';
	strcpy(src, srcp);
	return desc;
}



static int CfgSetVal(void *pMember, TINITYPEDESC *pstDesc, char *sVal)
{
char *p, *p1;
int i, iArrayNum = 0;

	// if (!sVal[0]) return 0;
	if( (p = strchr(sVal, '\n')) ) *p = 0;
	switch (pstDesc->iType) {
		case TINI_INT:
			*(int *)pMember = atoi(sVal);
			break;
		case TINI_STRING:
			nGetVal((char *)pMember, sVal, 256);
			break;
		case TINI_LINE:
			strcpy(pMember, sVal);
			break;
		case TINI_IP:
			inet_aton(sVal, (struct in_addr *)pMember);
			break;
		case TINI_ADDR:
			p = strchr(sVal, ':');
			if (!p) break;
			*p = 0;
			if (!inet_aton(sVal, &((struct sockaddr_in *)pMember)->sin_addr)) break;
			((struct sockaddr_in *)pMember)->sin_port = htons((unsigned short)atoi(p + 1));
			break;
		case TINI_INT_ARRAY:
			while (1) {
				if (iArrayNum >= pstDesc->iArrayMax) break;
				if ( (p = strchr(sVal, pstDesc->chSeparator) ) ) {
					*p = 0;
					*(int *)((char *)pMember + sizeof(int) * iArrayNum) = atoi(sVal);
					sVal = p + 1;
					iArrayNum++;
				} else {
					if (*sVal == 0) break;
					*(int *)((char *)pMember + sizeof(int) * iArrayNum) = atoi(sVal);
					iArrayNum++;
					break;
				}
			}
			for (i = iArrayNum; i < pstDesc->iArrayMax; i++) 
				*(int *)((char *)pMember + sizeof(int) * i) = 0;
			break;
		case TINI_IP_ARRAY:
			while (1) {
				if (iArrayNum >= pstDesc->iArrayMax) break;
				sVal = SkipSpace(sVal);
				if ( (p = strchr(sVal, pstDesc->chSeparator) ) ) {
					*p = 0;
					if (inet_aton(sVal, (struct in_addr *)((char *)pMember + sizeof(long) * iArrayNum))) iArrayNum++;
					sVal = p + 1;
				} else {
					if (*sVal == 0) break;
					if (inet_aton(sVal, (struct in_addr *)((char *)pMember + sizeof(long) * iArrayNum))) iArrayNum++;
					break;
				}
			}
			for (i = iArrayNum; i < pstDesc->iArrayMax; i++) 
				*(long *)((char *)pMember + sizeof(int) * i) = 0;
			break;
		case TINI_ADDR_ARRAY:
			while (1) {
				if (iArrayNum >= pstDesc->iArrayMax) break;
				sVal = SkipSpace(sVal);
				if ( (p = strchr(sVal, pstDesc->chSeparator) ) ) {
					*p = 0;
					if ( (p1 = strchr(sVal, ':') ) ) {
						*p1 = 0;
						if (inet_aton(sVal, &((struct sockaddr_in *)((char *)pMember + sizeof(struct sockaddr_in) * iArrayNum))->sin_addr)) {
							((struct sockaddr_in *)((char *)pMember + sizeof(struct sockaddr_in) * iArrayNum))->sin_port = htons((unsigned short)atoi(p1 + 1));
							iArrayNum++;
						}
					}
					sVal = p + 1;
				} else {
					if (*sVal == 0) break;
					if ( (p1 = strchr(sVal, ':') ) ) {
						*p1 = 0;
						if (inet_aton(sVal, &((struct sockaddr_in *)((char *)pMember + sizeof(struct sockaddr_in) * iArrayNum))->sin_addr)) {
							((struct sockaddr_in *)((char *)pMember + sizeof(struct sockaddr_in) * iArrayNum))->sin_port = htons((unsigned short)atoi(p1 + 1));
							iArrayNum++;
						}
					}
					break;
				}
			}
			for (i = iArrayNum; i < pstDesc->iArrayMax; i++) 
				((struct sockaddr_in *)((char *)pMember + sizeof(struct sockaddr_in) * i))->sin_addr.s_addr = 0;
			break;

		case TINI_STRING_ARRAY:
			while (1){
				if (iArrayNum >= pstDesc->iArrayMax) break;
				sVal = SkipSpace(sVal);
				if ( (p = strchr(sVal, pstDesc->chSeparator) ) ) {
					*p = 0;
					nGetVal((char *)pMember + TINI_STR_LEN * iArrayNum, sVal, TINI_STR_LEN);
					sVal = p + 1;
					iArrayNum++;
				} else {
					if (*sVal == 0) break;
					nGetVal((char *)pMember + TINI_STR_LEN * iArrayNum, sVal, TINI_STR_LEN);
					iArrayNum++;
					break;
				}
			}
			for (i = iArrayNum; i < pstDesc->iArrayMax; i++) {
				*((char *)pMember + TINI_STR_LEN * i) = 0;
			}
			break;
	}
	return 0;
}

static int InitMember(void *pstSec, TINITYPEDESC *pstDesc)
{
int iTotal = 0;

	while (pstDesc->szID[0]) {
		CfgSetVal((char *)pstSec + pstDesc->iOffset, pstDesc, pstDesc->szInitVal);
		// printf("init %s: %s\n", pstDesc->szID, pstDesc->szInitVal);
		iTotal++;
		pstDesc++;
	}
	return iTotal;
}

static int CfgGetLine(FILE *f, char *sBuf, char *sSecID)
{
static char sLine[4096];
char *p, *pEnd;

	if (!fgets(sLine, sizeof(sLine), f)) {
		sBuf[0] = 0;
		return -1;
	}
	p = sLine;
	p = SkipSpace(p);
	strcpy(sBuf, p);
	if (!sSecID) return 0;
	if (*p != '[') return 0;
	pEnd = strchr(p, ']');
	if (pEnd) *pEnd = 0;
	strncpy(sSecID, p + 1, TINI_MAX_ID_LEN);
	return 0;
}

static int ReadMember(FILE *f, void *pstSec, TINITYPEDESC *pstTypeDesc, int iTotal, char *sNext)
{
int i;
char *p, *pEq, sBuf[4096];
TINITYPEDESC *pstDesc;

	while (!feof(f)) {
		if (CfgGetLine(f, sBuf, sNext) < 0) break;
		p = sBuf;
		if (*p == 0 || *p == ';' || *p == '#') continue;
		else if (*p == '[') break;
		pEq = strchr(p, '=');
		if (!pEq) continue;
		*pEq = 0;
		TrimSpace(p);
		pstDesc = pstTypeDesc;
		for (i = 0; i < iTotal; i++) {
			if (!pstDesc->szID[0] || !strncasecmp(p, pstDesc->szID, strlen(p))) break;
			pstDesc++;
		}
		if (i >= iTotal || !pstDesc->szID[0]) continue;
		p = pEq + 1;
		p = SkipSpace(p);
		TrimSpace(p);
		CfgSetVal((char *)pstSec + pstDesc->iOffset, pstDesc, p);
	}
	return 0;
}

static int ReadCfg(char *sFile, void *pstCfg, TINITYPEDESC *pstTypeDesc)
{
FILE *f;
int iTotal;

	iTotal = InitMember(pstCfg, pstTypeDesc);

	if (!(f = fopen(sFile, "r"))) {
		printf("Can not open config file %s.\n", sFile);
		return -1;
	}
	ReadMember(f, pstCfg, pstTypeDesc, iTotal, NULL);
	fclose(f);
	return 0;
}

static int InitSection(void *pstIni, TINISECTIONDESC *pstDesc)
{
int i;

	while (pstDesc->szID[0]) {
		switch (pstDesc->iType) {
			case TINI_SECTION:
				InitMember((char *)pstIni + pstDesc->iOffset, pstDesc->astMember);
				break;
			case TINI_SECTION_ARRAY:
				*(int *)((char *)pstIni + pstDesc->iArrayNumOffset) = 0;
				for (i = 0; i < pstDesc->iArrayMax; i++) 
					InitMember((char *)pstIni + pstDesc->iOffset + pstDesc->iArrayMemberSize * i, pstDesc->astMember);
				break;
		}
		pstDesc++;
	}
	return 0;
}

static int SkipSection(FILE *f, char *sSecID)
{
char sBuf[4096];

	while (!feof(f)) {
		if (CfgGetLine(f, sBuf, sSecID)) break;
		if (sBuf[0] == '[') return 0;
	}
	return 0;
}

static char *GetSectionID(FILE *f, char *sSecID, char *sNext)
{
char sBuf[4096];

	if (sNext[0]) {
		strncpy(sSecID, sNext, TINI_MAX_ID_LEN);
		sNext[0] = 0;
		return sSecID;
	}

	sSecID[0] = 0;
	while (!feof(f)) {
		if (CfgGetLine(f, sBuf, sSecID) < 0) break;
		if (sSecID[0]) break;
	}
	if (feof(f)) return NULL;
	if (sSecID[0]) return sSecID;
	return NULL;
}

static int ReadSection(FILE *f, void *pstIni, TINISECTIONDESC *pstDesc, char *sSecID, char *sNext)
{
int *piArrayNum;

	if (!GetSectionID(f, sSecID, sNext)) return -1;

	while (pstDesc->szID[0]) {
		if (!strncasecmp(sSecID, pstDesc->szID, strlen(pstDesc->szID))) break;
		pstDesc++;
	}
	if (!pstDesc->szID[0]) {
		SkipSection(f, sNext);
		return -2;
	}

	switch (pstDesc->iType) {
		case TINI_SECTION:
			ReadMember(f, (char *)pstIni + pstDesc->iOffset, pstDesc->astMember, TINI_MAX_SECTION_MEMBER, sNext);
			break;
		case TINI_SECTION_ARRAY:
			piArrayNum = (int *)((char *)pstIni + pstDesc->iArrayNumOffset);
			if (*piArrayNum >= pstDesc->iArrayMax) {
				SkipSection(f, sSecID);
				break;
			}
			ReadMember(f, (char *)pstIni + pstDesc->iOffset + pstDesc->iArrayMemberSize * (*piArrayNum), pstDesc->astMember, TINI_MAX_SECTION_MEMBER, sNext);
			// printf("size %d num: %d\n", pstDesc->iArrayMemberSize, *piArrayNum);
			(*piArrayNum)++;
			break;
	}
	return 0;
}

static int ReadIni(char *sFile, void *pstIni, TINISECTIONDESC *pstSecDesc)
{
FILE *f;
char szID[TINI_MAX_ID_LEN], sNext[TINI_MAX_ID_LEN];
int iRet;

	if (!(f = fopen(sFile, "r"))) {
		printf("Can not open INI file %s.\n", sFile);
		return -1;
	}

	InitSection(pstIni, pstSecDesc);

	sNext[0] = 0;
	while (!feof(f)) {
		iRet = ReadSection(f, pstIni, pstSecDesc, szID, sNext);
		// printf("get sec: %s  %d\n", szID, iRet);
	}
	fclose(f);
	return 0;
}

int tini_get_type(IN char* a_pszFile,  OUT void* a_pvType, IN TINITYPEDESC *a_pstTypeDesc)
{
	assert(NULL != a_pszFile);
	assert(NULL != a_pvType);
	assert(NULL != a_pstTypeDesc);

	return ReadCfg(a_pszFile, a_pvType, a_pstTypeDesc);
}

int tini_get_ini(IN char* a_pszFile, OUT void* pvIni,	IN TINISECTIONDESC *a_pstSecDesc)
{
	assert(NULL != a_pszFile);
	assert(NULL != pvIni);
	assert(NULL != a_pstSecDesc);

	return ReadIni(a_pszFile, pvIni, a_pstSecDesc);
}

int tini_read_sec(IN FILE* a_fp, OUT void* a_pvSec, IN TINISECTIONDESC *a_pstSecDesc, IN  char* a_pszSecID, IN char* a_pszNext)
{
	assert(NULL != a_fp);
	assert(NULL != a_pvSec);
	assert(NULL != a_pstSecDesc);
	assert(NULL != a_pszSecID);
	assert(NULL != a_pszNext);


	return ReadSection(a_fp, a_pvSec, a_pstSecDesc, a_pszSecID, a_pszNext);
}


