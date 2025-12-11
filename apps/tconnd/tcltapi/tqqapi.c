/*
**  @file $RCSfile: tqqapi.c,v $
**  general description of this module
**  $Id: tqqapi.c,v 1.20 2009/04/01 10:18:57 hardway Exp $
**  @author $Author: hardway $
**  @date $Date: 2009/04/01 10:18:57 $
**  @version $Revision: 1.20 $
**  @note Editor: Vim 6.1, Gcc 4.0.1, tab=4
**  @note Platform: Linux
*/

#include "pal/pal.h"
#include "tsec/oi_tea.h"
#include "tsec/tsecbasedef.h"
#include "tsec/md5.h"
#include "apps/tcltapi/tqqdef.h"
#include "apps/tcltapi/tpdudef.h"
#include "apps/tcltapi/tqqapi.h"

extern unsigned char g_szMetalib_tqqapi[];


static char gs_szCharList[]	=	"ABCDEFGHIJKMNPQRSTUVWXYZabcdefghijkmnpqrstuvwxyz23456789";
LPTDRMETA g_pstqqapiPduHead = NULL;
static const char *tqqapi_error_string[]=
                     {
                       "None",
			  "NetWork Failure",
		         "Convert Head Net To Host Failed by TDR",			   	
		         "Not Enough Buff To Recv Msg",
		         "Decrypt Head Info Failed!",
		         "Convert Msg Net to Host Failed by TDR",
		         "Convert Head Host to Net Failed by TDR",
			  "Convert Msg Host to  Net Failed by TDR",
			  "Recv package Timeout",
			  "Decrypt Msg Failed!",
			  "INVALID package Recvied",
			  "Convert Head UserIdent info Host to Net Failed!",
			  "Build Auth Msg Failed",
			  "Build SynAck Msg Failed",
			  "",
                     };


struct tagTQQApiHandle
{
	int s;
	int iErr;
	int iTdrErrorCode;
	int iSelfVersion;
	int iPeerVersion;
	LPTDRMETA pstMetaBase;
	LPTDRMETA pstMetaHead;
	LPTDRMETA pstRecvMeta;
	LPTDRMETA pstSendMeta;
	char* pszRecvBuff;
	int iRecvBuff;
	char* pszRecvBuff2;
	int iRecvBuff2;
	int iOff;
	int iData;
	char* pszSendBuff;
	int iSendBuff;
	char* pszSendBuff2;
	int iSendBuff2;
	int iIsGameKeyValid;
	char szGameKey[TQQ_KEY_LEN];
	int iPos;
	char szIdentity[TQQ_IDENT_LEN];
       TPDUSYNINFO stSynInfo; 
};


struct tagTQQSign
{
    char len[2];
    char stx;
    char ver[2];
    char cmd[2];
    char seq[2];
    char uin[4];
    char k1[16];
    char encrypt[32];
    char etx;
};

typedef struct tagTQQSign		TQQSIGN;
typedef struct tagTQQSign		*LPTQQSIGN;

struct tagSignKeyPair
{
	char szMd5[16];
	char szK1[16];
};

typedef struct tagSignKeyPair	SIGNKEYPAIR;
typedef struct tagSignKeyPair	*LPSIGNKEYPAIR;

static char* gs_aszUri[] =
{
//	"tcpconn.tencent.com",
//	"tcpconn2.tencent.com",
//	"tcpconn3.tencent.com",
//	"tcpconn4.tencent.com",
	"218.17.209.23",
	"218.18.95.153",
	"219.133.38.246",
	"219.133.38.247",
	"219.133.38.248",
	"219.133.38.249",
	"219.133.38.250",
};


extern int tqqapi_build_synack_msg(HTQQAPI a_hQQClt,TPDUHEAD* a_pstHead);



int  tqqapi_create()
{
     LPTDRMETALIB pstMetaLib;
     pstMetaLib	=	(LPTDRMETALIB) tqqapi_get_meta_data();

     g_pstqqapiPduHead = tdr_get_meta_by_name(pstMetaLib, "TPDUHead");

     if( !g_pstqqapiPduHead )
     {	
           return -1;		   
     }

     return 0;
}


int tqqapi_decode(const char* pszBuff, int iBuff, TPDUHEAD* pstHead, int* piHeadLen)
{
       TDRDATA stHost;
	TDRDATA stNet;

	stNet.pszBuff	=	(char*)pszBuff;
	stNet.iBuff		=	iBuff;

	stHost.pszBuff	=	(char*) pstHead;
	stHost.iBuff	=	sizeof(*pstHead);

	if( tdr_ntoh(g_pstqqapiPduHead, &stHost, &stNet, 0)<0 )
	{
		return -1;
	}

	*piHeadLen	=	stNet.iBuff;

	return 0;

}

int tqqapi_encode(char* pszBuff, int* piBuff, TPDUHEAD* pstHead)
{
       TDRDATA stHost;
	TDRDATA stNet;

	stNet.pszBuff	=	(char*)pszBuff;
	stNet.iBuff	=	*piBuff;

	stHost.pszBuff	=	(char*) pstHead;
	stHost.iBuff	=	sizeof(*pstHead);

	if( tdr_hton(g_pstqqapiPduHead, &stNet,&stHost,  0)<0 )
	{
		return -1;
	}

	*piBuff		=	stNet.iBuff;
	((TPDUBASE *)(pszBuff))->bHeadLen=*piBuff	;
	((TPDUBASE *)(pszBuff))->iBodyLen=HTONL(0);
	return 0;     


}

const char* tqqapi_get_meta_data(void)
{
	return (const char*) g_szMetalib_tqqapi;
}

int tqqapi_make_authinfo(TQQAUTHINFO* pstAuthInfo, TQQGAMESIG* pstGameSig, TQQSIGFORS2* pstSigForS2, long uin, char* pszSvrSKey)
{
	LPTDRMETALIB pstMetaLib;
	LPTDRMETA pstMetaGameSig;
	LPTDRMETA pstMetaSigForS2;
	TQQAUTHINFO stTempAuth;
	TDRDATA stHost;
	TDRDATA stNet;
	char* pszEnc;
	int iEnc;
	int iRet=0;

	pstAuthInfo->lUin = uin;

	pstMetaLib	=	(LPTDRMETALIB) g_szMetalib_tqqapi;

	pstMetaGameSig	=	tdr_get_meta_by_name(pstMetaLib, "TQQGameSig");
	pstMetaSigForS2	=	tdr_get_meta_by_name(pstMetaLib, "TQQSigForS2");

	stHost.pszBuff	=	(char*)pstGameSig;
	stHost.iBuff	=	(int)sizeof(*pstGameSig);

	stNet.pszBuff	=	(char*) stTempAuth.szSignData;
	stNet.iBuff	=	(int)sizeof(stTempAuth.szSignData);
	iRet=tdr_hton(pstMetaGameSig, &stNet, &stHost, TPDU_VERSION);

	if( 0 != iRet )
	{
		return iRet;
	}

	pszEnc	=	(char*)pstAuthInfo->szSignData;
	iEnc	=	(int)sizeof(pstAuthInfo->szSignData);

	oi_symmetry_encrypt2(stNet.pszBuff, stNet.iBuff, pszSvrSKey, pszEnc, &iEnc);

	

	pstAuthInfo->bSignLen	=	(unsigned char)iEnc;

	stHost.pszBuff	=	(char*)pstSigForS2;
	stHost.iBuff	=	(int)sizeof(*pstSigForS2);

	stNet.pszBuff	=	(char*) stTempAuth.szSign2Data;
	stNet.iBuff	=	(int)sizeof(stTempAuth.szSign2Data);

	iRet = tdr_hton(pstMetaSigForS2, &stNet, &stHost, TPDU_VERSION);

	if( 0 != iRet )
	{
		return iRet;
	}

	pszEnc	=	(char*) pstAuthInfo->szSign2Data;
	iEnc	=	(int)sizeof(pstAuthInfo->szSign2Data);

	oi_symmetry_encrypt2(stNet.pszBuff, stNet.iBuff, pszSvrSKey, pszEnc, &iEnc);



	pstAuthInfo->bSign2Len	=	(unsigned char)iEnc;

        
	return 0;
}

int tqqapi_extract_authinfo(TQQGAMESIG* pstGameSig, TQQSIGFORS2* pstSigForS2, long uin, TQQAUTHINFO* pstAuthInfo, char* pszSvrSKey)
{
	LPTDRMETALIB pstMetaLib;
	LPTDRMETA pstMetaGameSig;
	LPTDRMETA pstMetaSigForS2;
	TQQAUTHINFO stTempAuth;
	TDRDATA stHost;
	TDRDATA stNet;
	char* pszDec;
	int iDec;
	int iRet;
	

	uin = pstAuthInfo->lUin;
		
	pstMetaLib	=	(LPTDRMETALIB) g_szMetalib_tqqapi;

	pstMetaGameSig	=	tdr_get_meta_by_name(pstMetaLib, "TQQGameSig");
	pstMetaSigForS2	=	tdr_get_meta_by_name(pstMetaLib, "TQQSigForS2");

	pszDec	=	(char*) stTempAuth.szSignData;
	iDec	=	(int)sizeof(stTempAuth.szSignData);

	if( !oi_symmetry_decrypt2((char*)pstAuthInfo->szSignData, pstAuthInfo->bSignLen, pszSvrSKey, pszDec, &iDec) )
	{
		return -2;
	}

	stHost.pszBuff	=	(char*)pstGameSig;
	stHost.iBuff	=	(int)sizeof(*pstGameSig);

	stNet.pszBuff	=	(char*) stTempAuth.szSignData;
	stNet.iBuff	=	iDec;
	iRet=tdr_ntoh(pstMetaGameSig, &stHost, &stNet, TPDU_VERSION);

	if( 0 != iRet )
	{
		return iRet;
	}


	pszDec	=	(char*)stTempAuth.szSign2Data;
	iDec	=	(int)sizeof(stTempAuth.szSign2Data);

	if( !oi_symmetry_decrypt2((char*)pstAuthInfo->szSign2Data, pstAuthInfo->bSign2Len, pszSvrSKey, pszDec, &iDec) )
	{
		return -2;
	}

	stHost.pszBuff	=	(char*)pstSigForS2;
	stHost.iBuff	=	(int)sizeof(*pstSigForS2);

	stNet.pszBuff	=	(char*) stTempAuth.szSign2Data;
	stNet.iBuff	=	iDec;
	iRet=tdr_ntoh(pstMetaSigForS2, &stHost, &stNet, TPDU_VERSION);
	
	if( 0 != iRet  )
	{
		return iRet;
	}
	
	return 0;
}

int tqqapi_make_QQUnify_Authinfo(TQQUNIFIEDAUTHINFO *pstUnifyAuthInfo,long uin,char* pszSvrSKey,char *pszSessionkey,unsigned int uCltIP)
{
      TQQUNIFIEDSIG stUinSig;
      TQQUNIFIEDENCRYSIG stEncSig;
      char* pszEnc;
	int iEnc;
	TDRDATA stHost;
	TDRDATA stNet;
	char szTemp[TQQ_MAX_SIGN_LEN]={0};
	LPTDRMETALIB pstMetaLib = NULL;
	LPTDRMETA pstMetaEncUniSig = NULL;
	LPTDRMETA pstMetaUniSig = NULL;
	
	int iRet =0;

	pstUnifyAuthInfo->lUin= uin;
	
       pstMetaLib	=	(LPTDRMETALIB) tqqapi_get_meta_data();;
	pstMetaEncUniSig	=	tdr_get_meta_by_name(pstMetaLib, "TQQUnifiedEncrySig");
	pstMetaUniSig        =     tdr_get_meta_by_name(pstMetaLib,"TQQUnifiedSig");
	

       memset((void *)&stEncSig,0,sizeof(stEncSig));

	stEncSig.nVersion=1;
	stEncSig.dwUin= uin;
	stEncSig.iTime= time(NULL);
	stEncSig.iClientIP=uCltIP;
	memcpy((char *)stEncSig.szSessionKey,pszSessionkey,(int)sizeof(stEncSig.szSessionKey));

	stHost.pszBuff = (char *)&stEncSig;
	stHost.iBuff = (int )sizeof(stEncSig);
       stNet.pszBuff = (char *)szTemp;
	 stNet.iBuff =(int)sizeof(szTemp);
	 iRet = tdr_hton( pstMetaEncUniSig, &stNet, &stHost, TPDU_VERSION);
        if( 0 != iRet )
        {
              return iRet;
	 }
       pszEnc = (char *)stUinSig.szEncryptSignData;
	iEnc  =sizeof(stUinSig.nEncryptSignLen);
	oi_symmetry_encrypt2(stNet.pszBuff,stNet.iBuff, pszSvrSKey, pszEnc, &iEnc);
	
	stUinSig.nVersion=1;
	stUinSig.iTime= time(NULL);
	stUinSig.nEncryptSignLen = iEnc;

	stHost.pszBuff = (char *)&stUinSig;
	stHost.iBuff = (int )sizeof(stUinSig);
       stNet.pszBuff = (char *)pstUnifyAuthInfo->szSigInfo;
	 stNet.iBuff =(int)sizeof(pstUnifyAuthInfo->szSigInfo);

	 iRet =  tdr_hton( pstMetaUniSig, &stNet, &stHost, TPDU_VERSION);
	 if(  0 != iRet   )
	 {
              return iRet;
	 }

	pstUnifyAuthInfo->bLen = stNet.iBuff;
        return 0;
}
int tqqapi_extract_QQUnify_Authinfo(TQQUNIFIEDAUTHINFO *pstUnifyAuthInfo,char* pszSvrSKey)
{
      TQQUNIFIEDSIG stUinSig;
      TQQUNIFIEDENCRYSIG stEncSig;
      char* pszDec;
	int iDec;
	TDRDATA stHost;
	TDRDATA stNet;
	char szTemp[TQQ_MAX_SIGN_LEN]={0};
	LPTDRMETALIB pstMetaLib;
	LPTDRMETA pstMetaEncUniSig;
	LPTDRMETA pstMetaUniSig;
	long uin;
	int iRet =0;
	
       pstMetaLib	=	(LPTDRMETALIB) g_szMetalib_tqqapi;

	pstMetaEncUniSig	=	tdr_get_meta_by_name(pstMetaLib, "TQQUnifiedEncrySig");  
	pstMetaUniSig	=	tdr_get_meta_by_name(pstMetaLib, "TQQUnifiedSig");  
	

	uin = pstUnifyAuthInfo->lUin;

	stNet.pszBuff     =    pstUnifyAuthInfo->szSigInfo;
	stNet.iBuff         =    pstUnifyAuthInfo->bLen;
	stHost.pszBuff    =  (char *)&stUinSig;
	stHost.iBuff       =  (int)sizeof(stUinSig);

	iRet =tdr_ntoh(pstMetaUniSig, &stHost, &stNet, TPDU_VERSION);

	if(  0 != iRet  )
	{
             return iRet;
	}
	
	pszDec = (char *)szTemp;
	iDec = (int)sizeof(szTemp);
	
	if( !oi_symmetry_decrypt2((char*)stUinSig.szEncryptSignData, stUinSig.nEncryptSignLen, pszSvrSKey, pszDec, &iDec) )
	{
		return -2;
	}

	stNet.pszBuff=(char*) pszDec;
	stNet.iBuff=	(int)iDec;

	stHost.pszBuff	=	(char*)&stEncSig;
	stHost.iBuff	=	(int)sizeof(stEncSig);

       iRet = tdr_ntoh(pstMetaEncUniSig, &stHost, &stNet, TPDU_VERSION);
	if( 0 != iRet  )
	{
		return iRet;
	}
	
	return 0;


}
char * tqqapi_randstr(char* pszBuff, int iLen)
{
	char *pszChars=gs_szCharList;

	int	i, iChars;

	iChars	=	sizeof(gs_szCharList) - 1;

	for ( i=0; i<iLen;i++)
	{	
		pszBuff[i]	=	pszChars[ (int)( (float)iChars*rand()/(RAND_MAX+1.0) ) ];
	}

	return pszBuff;
}


int tqqapi_init_qqsign_i(TQQSIGN* pstSign, long lUin, char* pszPass, SIGNKEYPAIR* pstPair)
{
	int iLen;
	char szBuf[1024];
	char szK1[20], szMd5[17];

	pstSign->stx = 2;
	pstSign->etx = 3;
	pstSign->ver[0] = 9;
	*(short *)pstSign->len = htons(sizeof(*pstSign));
	*(short *)pstSign->cmd = htons(0x37);
	*(long *)pstSign->uin = htonl(lUin);

	tqqapi_randstr(szK1, 16);

	/* MD5 hash the password twice. */
	Md5HashBuffer(szMd5, pszPass, strlen(pszPass));
	Md5HashBuffer(szMd5, szMd5, 16);

	oi_symmetry_encrypt2("", 0, szMd5, szBuf + 1, &iLen);

	szBuf[0] = 0x12;
	oi_symmetry_encrypt2(szBuf, 17, szK1, pstSign->encrypt, &iLen);
	memcpy(pstSign->k1, szK1, 16);

	if( pstPair )
	{
		memcpy(pstPair->szMd5, szMd5, 16);
		memcpy(pstPair->szK1, szK1, 16);
	}

	return 0;
}


int tqqapi_get_siginfo_by_socket(TQQSIGINFO* pstSigInfo, TQQUSERTOKEN* pstToken, int s, char* pszErr, int iErrLen, int iTimeout)
{
	int iLen;
	int iDecLen;
	int iNickLen;
	TQQBASEGAMESIG* pstBaseGameSig;
	TQQUSERINFO* pstQQUserInfo;
	TQQSIGN stSign;
	SIGNKEYPAIR stPair;
	char szBuf[1024];
	char szDecrypt[256];
	char* pszUserInfo;

	if( pszErr && iErrLen>0 )
		pszErr[0]	=	'\0';

	pstBaseGameSig	=	&pstSigInfo->stBaseGameSig;
	pstQQUserInfo	=	&pstSigInfo->stUserInfo;

	memset(pstBaseGameSig, 0, sizeof(pstSigInfo->stBaseGameSig));
	memset(pstQQUserInfo, 0, sizeof(pstSigInfo->stUserInfo));

	tqqapi_init_qqsign_i(&stSign, pstToken->lUin, pstToken->szPass, &stPair);

	if( tnet_sendall(s, (char*) &stSign, sizeof(stSign), iTimeout)<0 )
	{
		return TQQE_CONN;
	}

	iLen	=	tnet_recv(s, szBuf, sizeof(szBuf), iTimeout);

	if( iLen<0 )
	{
		return TQQE_CONN;
	}

	iDecLen = sizeof(szDecrypt);

	if( !oi_symmetry_decrypt2(szBuf+9, iLen-10, stPair.szMd5, szDecrypt, &iDecLen) &&
	    !oi_symmetry_decrypt2(szBuf+9, iLen-10, stPair.szK1, szDecrypt, &iDecLen) )
	{
		return TQQE_DECRYPT;
	}

	if(szDecrypt[1] == 1)
	{
		szDecrypt[szDecrypt[2] + 3] = 0;

		if( szDecrypt[2]<iErrLen )
			strcpy(pszErr, szDecrypt + 3);

		return TQQE_PASSWD;
	}
	else if (szDecrypt[1] == 2)
	{
		szDecrypt[szDecrypt[2] + 3] = 0;

		if( szDecrypt[2]<iErrLen )
			strcpy(pszErr, szDecrypt + 3);

		return TQQE_ERROR;
	}
	else if (szDecrypt[1] != 0)
	{
		return TQQE_ERROR;
	}

	memcpy(pstBaseGameSig->szGameKey, szDecrypt + 2, 16);
	memcpy(pstBaseGameSig->szSvcBitmap, szDecrypt + 18, 12);
	pstBaseGameSig->ulUinFlag	=	ntohl( *(unsigned long*)(szDecrypt + 30) );

	pstSigInfo->bSignLen = (unsigned char) szDecrypt[34];
	memcpy(pstSigInfo->szSignData, szDecrypt + 35, pstSigInfo->bSignLen);

	pszUserInfo	=	szDecrypt + pstSigInfo->bSignLen + 35;

	iNickLen = pszUserInfo[4];
	pstQQUserInfo->lUin	=	pstToken->lUin;
	pstQQUserInfo->chNickLen	=	(char) iNickLen;
	pstQQUserInfo->wFace	=	ntohs(*(unsigned short*)pszUserInfo);
	pstQQUserInfo->chGender	=	pszUserInfo[2];
	pstQQUserInfo->chAge	=	pszUserInfo[3];
	memcpy(pstQQUserInfo->szNick, pszUserInfo + 5, iNickLen);

	return 0;

}

int tqqapi_get_siginfo_by_uri(TQQSIGINFO* pstSigInfo, TQQUSERTOKEN* pstToken, const char* pszUri, char* pszErr, int iErrLen, int iTimeout)
{
	int s;
	int iRet;

	if( pszErr && iErrLen>0 )
		pszErr[0]	=	'\0';

	s	=	tnet_connect(pszUri, iTimeout);

	if( -1==s )
		return TQQE_CONN;

	iRet	=	tqqapi_get_siginfo_by_socket(pstSigInfo, pstToken, s, pszErr, iErrLen, iTimeout);

	tnet_close(s);

	return iRet;
}


int tqqapi_get_siginfo(TQQSIGINFO* pstSigInfo, TQQUSERTOKEN* pstToken, char* pszErr, int iErrLen, int iTimeout)
{
	static int s_fInited = 0;
	static int s_iOrd = 0;
	int iRet;
	const char* pszDNS;
	char szUri[128];
	struct hostent* pstEnt;

	if( pszErr && iErrLen>0 )
		pszErr[0]	=	'\0';

	if( !s_fInited )
	{
		s_iOrd	=	rand() % (sizeof(gs_aszUri)/sizeof(char*));
		s_fInited	=	1;
	}
	else
	{
		s_iOrd	=	(s_iOrd + 1) % (sizeof(gs_aszUri)/sizeof(char*));
	}

	pszDNS	=	gs_aszUri[s_iOrd];

	if( pszDNS[0]>'9' || pszDNS[0]<'0' )
	{
		pstEnt  =  gethostbyname(pszDNS);

		if( !pstEnt )
			return -1;

		pszDNS	=	(const char*) inet_ntoa(*(struct in_addr*)pstEnt->h_addr);
	}

	sprintf(szUri, "tcp://%s:80", pszDNS);
	iRet	= tqqapi_get_siginfo_by_uri(pstSigInfo, pstToken, szUri, pszErr, iErrLen, iTimeout);
	if( iRet>=0 || TQQE_PASSWD==iRet )
		return iRet;

	sprintf(szUri, "tcp://%s:8000", pszDNS);
	iRet = tqqapi_get_siginfo_by_uri(pstSigInfo, pstToken, szUri, pszErr, iErrLen, iTimeout);
	if( iRet>=0 || TQQE_PASSWD==iRet )
		return iRet;

	sprintf(szUri, "tcp://%s:443", pszDNS);
	return tqqapi_get_siginfo_by_uri(pstSigInfo, pstToken, szUri, pszErr, iErrLen, iTimeout);
}


int tqqapi_init(HTQQAPI a_hQQClt)
{
	LPTDRMETALIB pstMetaLib;

	if( !a_hQQClt )
	{
		return -1;
	}

	a_hQQClt->s	=	-1;

	pstMetaLib	=	(LPTDRMETALIB) tqqapi_get_meta_data();

	a_hQQClt->pstMetaBase	=	tdr_get_meta_by_name(pstMetaLib, "TPDUBase");
	a_hQQClt->pstMetaHead	=	tdr_get_meta_by_name(pstMetaLib, "TPDUHead");

	return 0;
}


int tqqapi_new(HTQQAPI* a_phQQClt)
{
	HTQQAPI hQQClt;

	hQQClt	=	(HTQQAPI) calloc(1, sizeof(*hQQClt));

	if( !hQQClt )
	{
		return -1;
	}

	tqqapi_init(hQQClt);

	if( a_phQQClt )
	{
		*a_phQQClt	=	hQQClt;
	}

	return 0;
}


int tqqapi_free(HTQQAPI* a_phQQClt)
{
	if( !a_phQQClt )
	{
		return -1;
	}

	if( (*a_phQQClt)->pszSendBuff )
	{
             free( (*a_phQQClt)->pszSendBuff  );
	      (*a_phQQClt)->pszSendBuff  = NULL;     
	}

	if( (*a_phQQClt)->pszSendBuff2 )
	{
             free( (*a_phQQClt)->pszSendBuff2  );
	      (*a_phQQClt)->pszSendBuff2  = NULL;     
	}

	if( (*a_phQQClt)->pszRecvBuff)
	{
             free( (*a_phQQClt)->pszRecvBuff  );
	      (*a_phQQClt)->pszSendBuff  = NULL;     
	}

	if( (*a_phQQClt)->pszRecvBuff2)
	{
             free( (*a_phQQClt)->pszRecvBuff2  );
	      (*a_phQQClt)->pszRecvBuff2  = NULL;     
	}

	if( *a_phQQClt )
	{
		if( -1!=(*a_phQQClt)->s )
		{
			tnet_close((*a_phQQClt)->s);
			(*a_phQQClt)->s	=	-1;
		}
		
		free(*a_phQQClt);

		*a_phQQClt	=	NULL;
	}

	return 0;
}


int tqqapi_detach(HTQQAPI a_hQQClt)
{
	int s=-1;

	if( -1!=a_hQQClt->s )
	{
		s	=	a_hQQClt->s;

		a_hQQClt->s	=	-1;
	}

	return s;
}


void tqqapi_attach(HTQQAPI a_hQQClt, int a_s)
{
	if( -1!=a_hQQClt->s )
	{
		tnet_close(a_hQQClt->s);
		a_hQQClt->s	=	-1;
	}

	a_hQQClt->s	=	a_s;
}


int tqqapi_open(const char* a_pszUri, int a_iTimeout, HTQQAPI* a_phQQClt)
{
	int s=-1;

	s	=	tnet_connect(a_pszUri, a_iTimeout);

	if( -1==s )
	{
		return -1;
	}

	if( -1==tqqapi_new(a_phQQClt) )
	{
		tnet_close(s);

		return -2;
	}

	tqqapi_attach(*a_phQQClt, s);
	
	return s;

}

int tqqapi_set_pdu_meta(HTQQAPI a_hQQClt, LPTDRMETA a_pstSendMeta, LPTDRMETA a_pstRecvMeta)
{
	int iSendBuff;
	int iRecvBuff;

	if( NULL == a_pstRecvMeta ||  NULL==a_pstSendMeta)
	{
           return -1;
	}

	a_hQQClt->pstRecvMeta	=	a_pstRecvMeta;
	a_hQQClt->pstSendMeta	=	a_pstSendMeta;

	iSendBuff	=	tdr_get_meta_size(a_pstSendMeta) + tdr_get_meta_size(a_hQQClt->pstMetaHead) + 32;
	iRecvBuff	=	tdr_get_meta_size(a_pstRecvMeta) + tdr_get_meta_size(a_hQQClt->pstMetaHead) + 32;

	if( iSendBuff>a_hQQClt->iSendBuff )
	{
		if( a_hQQClt->pszSendBuff )
		{
			free(a_hQQClt->pszSendBuff);
			a_hQQClt->pszSendBuff	=	NULL;
			a_hQQClt->iSendBuff	=	0;
		}

		a_hQQClt->pszSendBuff	=	(char*)calloc(1, iSendBuff);

		if( a_hQQClt->pszSendBuff )
			a_hQQClt->iSendBuff	=	iSendBuff;
	}

	if( iSendBuff>a_hQQClt->iSendBuff2 )
	{
		if( a_hQQClt->pszSendBuff2 )
		{
			free(a_hQQClt->pszSendBuff2);
			a_hQQClt->pszSendBuff2	=	NULL;
			a_hQQClt->iSendBuff2	=	0;
		}

		a_hQQClt->pszSendBuff2	=	(char*)calloc(1, iSendBuff);

		if( a_hQQClt->pszSendBuff2 )
			a_hQQClt->iSendBuff2	=	iSendBuff;
	}

	if( iRecvBuff>a_hQQClt->iRecvBuff )
	{
		if( a_hQQClt->pszRecvBuff )
		{
			free(a_hQQClt->pszRecvBuff);
			a_hQQClt->pszRecvBuff	=	NULL;
			a_hQQClt->iRecvBuff	=	0;
		}

		a_hQQClt->pszRecvBuff	=	(char*)calloc(1, iRecvBuff);

		if( a_hQQClt->pszRecvBuff )
			a_hQQClt->iRecvBuff	=	iRecvBuff;
	}

	if( iRecvBuff>a_hQQClt->iRecvBuff2 )
	{
		if( a_hQQClt->pszRecvBuff2 )
		{
			free(a_hQQClt->pszRecvBuff2);
			a_hQQClt->pszRecvBuff2	=	NULL;
			a_hQQClt->iRecvBuff2	=	0;
		}

		a_hQQClt->pszRecvBuff2	=	(char*)calloc(1, iRecvBuff);

		if( a_hQQClt->pszRecvBuff2 )
			a_hQQClt->iRecvBuff2	=	iRecvBuff;
	}


	if( a_hQQClt->pszSendBuff && a_hQQClt->pszSendBuff2 && a_hQQClt->pszRecvBuff && a_hQQClt->pszRecvBuff2 )
	{
		return 0;
	}
	else
	{
		return -1;
	}
}

void tqqapi_set_version(HTQQAPI a_hQQClt, int a_iSelfVersion, int a_iPeerVersion)
{
	a_hQQClt->iSelfVersion	=	a_iSelfVersion;
	a_hQQClt->iPeerVersion	=	a_iPeerVersion;
}

void tqqapi_set_identity(HTQQAPI a_hQQClt, int a_iPos, const char * a_pszIdentity)
{
      a_hQQClt->iPos = a_iPos;
      memcpy(a_hQQClt->szIdentity, a_pszIdentity, TQQ_IDENT_LEN);
}

void tqqapi_get_identity(HTQQAPI a_hQQClt, int *a_iPos, char * a_pszIdentity)
{
       *a_iPos = a_hQQClt->iPos;
	 memcpy(a_pszIdentity,a_hQQClt->szIdentity,TQQ_IDENT_LEN);
}

void tqqapi_init_base(TPDUBASE* a_pstBase)
{
	a_pstBase->bMagic	=	TPDU_MAGIC;
	a_pstBase->bVersion	=	TPDU_VERSION;
	a_pstBase->bCmd		=	TPDU_CMD_NONE;
	a_pstBase->bHeadLen	=	(unsigned char)sizeof(TPDUBASE);
	a_pstBase->iBodyLen	=	0;
}


int tqqapi_send_msg(HTQQAPI a_hQQClt, TPDUHEAD* a_pstHead, const char* a_pszMsg, int a_iMsg, int a_iTimeout)
{
	TDRDATA stHost;
	TDRDATA stNet;
	TPDUHEAD stHead;
	char* pszEnc=NULL;
	int iEnc=0;
	int iHead=0;
	int iPkg=0;
	int iRet=0;
	int iSend=0;

	if( NULL==a_pstHead )
	{
		tqqapi_init_base(&stHead.stBase);
		a_pstHead	=	&stHead;
	}

	stNet.pszBuff	=	a_hQQClt->pszSendBuff;
	stNet.iBuff	=	a_hQQClt->iSendBuff;

	stHost.pszBuff	=	(char*)a_pstHead;
	stHost.iBuff	=	(int)sizeof(*a_pstHead);

	iRet = tdr_hton(a_hQQClt->pstMetaHead, &stNet, &stHost, TPDU_VERSION);
	if( iRet<0 )
	{
              a_hQQClt->iErr=TQQAPI_ERR_TDR_HTON_HEAD;
		a_hQQClt->iTdrErrorCode = iRet;
		return -2;
	}

	iHead		=	stNet.iBuff;
	((TPDUBASE*)a_hQQClt->pszSendBuff)->bHeadLen	=	(unsigned char)iHead;

	if( a_iMsg>0 )
	{
		if( a_hQQClt->iIsGameKeyValid )
		{
			stNet.pszBuff	=	a_hQQClt->pszSendBuff2;
			stNet.iBuff	=	a_hQQClt->iSendBuff2 - iHead - 32;
		}
		else
		{
			stNet.pszBuff	=	a_hQQClt->pszSendBuff + iHead;
			stNet.iBuff	=	a_hQQClt->iSendBuff - iHead;
		}

		stHost.pszBuff	=	(char*)a_pszMsg;
		stHost.iBuff	=	a_iMsg;
		iRet = tdr_hton(a_hQQClt->pstSendMeta, &stNet, &stHost, a_hQQClt->iPeerVersion);
		if( iRet <0 )
		{
			a_hQQClt->iErr=TQQAPI_ERR_TDR_HTON_BODY;
			a_hQQClt->iTdrErrorCode = iRet;
			return iRet;
		}

		if( a_hQQClt->iIsGameKeyValid )
		{
			pszEnc	=	a_hQQClt->pszSendBuff + iHead;
			iEnc	=	a_hQQClt->iSendBuff - iHead;
			oi_symmetry_encrypt2(a_hQQClt->pszSendBuff2, stNet.iBuff, a_hQQClt->szGameKey, pszEnc, &iEnc);
			stNet.iBuff	=	iEnc;
		}
	}
	else
	{
		stNet.iBuff	=	0;
	}

	((TPDUBASE*)a_hQQClt->pszSendBuff)->iBodyLen	=	(int)HTONL((long)stNet.iBuff);

	iPkg	=	iHead + stNet.iBuff;

	
       iSend=tnet_sendall(a_hQQClt->s, a_hQQClt->pszSendBuff, iPkg, a_iTimeout);
	if( iPkg!= iSend)
	{
		a_hQQClt->iErr	=	TQQAPI_ERR_NETWORK_FAILURE;
		return -3;
	}
	else
	{
		return 0;
	}
}

int tqqapi_get_err(HTQQAPI a_hQQClt)
{
	return a_hQQClt->iErr;
}

void tqqapi_set_gamekey(HTQQAPI a_hQQClt, const char* pszGameKey)
{
	memcpy(a_hQQClt->szGameKey, pszGameKey, sizeof(a_hQQClt->szGameKey));
	a_hQQClt->iIsGameKeyValid	=	1;
}

int   tqqapi_get_gamekey(HTQQAPI a_hQQClt, char* pszGameKey)
{
       assert(pszGameKey!=NULL);
	if( a_hQQClt->iIsGameKeyValid	!=	1 )
	{
            return -1;
	}
	memcpy(pszGameKey, a_hQQClt->szGameKey,sizeof(a_hQQClt->szGameKey));
	return 0;
}


int tqqapi_get_msg(HTQQAPI a_hQQClt, TPDUHEAD* a_pstHead, char* a_pszMsg, int* a_piMsg)
{
	TDRDATA stHost;
	TDRDATA stNet;
	int iHead;
	int iBody;
	char* pszEnc;
	int iEnc;
	char* pszDec;
	int iDec;
	//TPDUIDENTINFO  stIdentInfo;
	//char szSessionKey[TQQ_KEY_LEN] = {0};
	int iRet=0;

	if( a_hQQClt->iData<(int)sizeof(TPDUBASE) )
	{
		iHead	=	(int) sizeof(TPDUBASE);
		iBody	=	0;
	}
	else
	{
		iHead	=	((TPDUBASE*)(a_hQQClt->pszRecvBuff + a_hQQClt->iOff))->bHeadLen;
		iBody	=	((TPDUBASE*)(a_hQQClt->pszRecvBuff + a_hQQClt->iOff))->iBodyLen;

		iBody	=	NTOHL((long)iBody);
	}

	if( a_hQQClt->iData<iHead+iBody )
	{
		if( a_hQQClt->iOff )
		{
			if( a_hQQClt->iData )
				memmove(a_hQQClt->pszRecvBuff, a_hQQClt->pszRecvBuff+a_hQQClt->iOff, a_hQQClt->iData);
		
			a_hQQClt->iOff	=	0;
		}
             return 0;
		//return -1;
	}


	stHost.pszBuff	=	(char*)a_pstHead;
	stHost.iBuff	=	(int)sizeof(*a_pstHead);

	stNet.pszBuff	=	a_hQQClt->pszRecvBuff + a_hQQClt->iOff;
	stNet.iBuff	=	iHead;

	iRet=tdr_ntoh(a_hQQClt->pstMetaHead, &stHost, &stNet, TPDU_VERSION);

	if( iRet<0 )
	{
		a_hQQClt->iErr	=	TQQAPI_ERR_TDR_NTOH_HEAD;
		a_hQQClt->iTdrErrorCode = iRet;	
		return iRet;
	}



	if( iBody )
	{

		stHost.pszBuff	=	a_pszMsg;
		stHost.iBuff	=	*(a_piMsg);

		if( a_hQQClt->iIsGameKeyValid && TPDU_CMD_PLAIN!=a_pstHead->stBase.bCmd )
		{
			pszEnc	=	a_hQQClt->pszRecvBuff + a_hQQClt->iOff + iHead;	
			iEnc	=	iBody;
			pszDec	=	a_hQQClt->pszRecvBuff2;
			iDec	=	a_hQQClt->iRecvBuff2;

			if( oi_symmetry_decrypt2(pszEnc, iEnc, a_hQQClt->szGameKey, pszDec, &iDec) )
			{
				stNet.pszBuff	=	pszDec;
				stNet.iBuff	=	iDec;
			}
			else
			{
				a_hQQClt->iErr	=	TQQAPI_ERR_DECRYPTION_BODY;
				return -1;
			}
		}
		else
		{
			stNet.pszBuff	=	a_hQQClt->pszRecvBuff + a_hQQClt->iOff + iHead;
			stNet.iBuff	=	iBody;
		}

		iRet = tdr_ntoh(a_hQQClt->pstRecvMeta, &stHost, &stNet, a_hQQClt->iPeerVersion);
		if (iRet < 0)
		{
			a_hQQClt->iErr	=	TQQAPI_ERR_TDR_NTOH_BODY;
			a_hQQClt->iTdrErrorCode = iRet;
			return iRet;
		}
	}
	else
	{
		stHost.iBuff	=	0;
	}

	*a_piMsg	=	stHost.iBuff;

	a_hQQClt->iOff	+=	(iHead + iBody);
	a_hQQClt->iData -=	(iHead + iBody);

	//return 0;
	return 1;
}

int tqqapi_recv_msg(HTQQAPI a_hQQClt, TPDUHEAD* a_pstHead, char* a_pszMsg, int* a_piMsg, int a_iTimeout)
{
	TPDUHEAD stHead;
	char* pszRecvBuff;
	int iRecvBuff;
	int iRecv;
	int iRet=0;


	if( NULL==a_pstHead )
	{
		a_pstHead	=	&stHead;
	}

	iRet=tqqapi_get_msg(a_hQQClt, a_pstHead, a_pszMsg, a_piMsg);

	//iRet<0     means recv package failed!
	//iRet==0   means doesn't recv a full package!
	//iRet==1   means recv a package!
	if( 0!=iRet )
	{
		return iRet;
	}
	

	pszRecvBuff	=	a_hQQClt->pszRecvBuff + a_hQQClt->iOff + a_hQQClt->iData;
	iRecvBuff	=	a_hQQClt->iRecvBuff - a_hQQClt->iOff - a_hQQClt->iData;

	if( iRecvBuff<=0 )
	{
		a_hQQClt->iErr	=	TQQAPI_ERR_NOT_ENOUGHBUF;
		return -1;
	}

	iRecv	=	tnet_recv(a_hQQClt->s, pszRecvBuff, iRecvBuff, a_iTimeout);

	if( iRecv<0 )		
	{
		a_hQQClt->iErr	=	TQQAPI_ERR_NETWORK_FAILURE;
		return -1;
	}
	else if( 0==iRecv )
	{
		return 0;
	}

	a_hQQClt->iData	+=	iRecv;
	
	iRet=tqqapi_get_msg(a_hQQClt, a_pstHead, a_pszMsg, a_piMsg);

	return iRet;
}


int tqqapi_extract_identinfo(HTQQAPI a_hQQClt, TPDUHEAD* a_pstHead, TPDUIDENTINFO* pstIdentInfo)
{
	LPTDRMETALIB pstMetaLib;
	LPTDRMETA pstMetaIdentInfo;
	TPDUHEAD stTempHead;
	TDRDATA stHost;
	TDRDATA stNet;
	char* pszDec;
	int iDec;
	int iRet=0;

	if( TPDU_CMD_IDENT!=a_pstHead->stBase.bCmd )		
	{
              a_hQQClt->iErr=TQQAPI_ERR_INVALID_CMD;
		return -1;
	}
	pstMetaLib	=	(LPTDRMETALIB) g_szMetalib_tqqapi;

	pstMetaIdentInfo=	tdr_get_meta_by_name(pstMetaLib, "TPDUIdentInfo");

	pszDec	=	(char*) stTempHead.stExt.stIdent.szEncryptIdent;
	iDec	=	(int)sizeof(stTempHead.stExt.stIdent.szEncryptIdent);

	if( !oi_symmetry_decrypt2((char*)a_pstHead->stExt.stIdent.szEncryptIdent, a_pstHead->stExt.stIdent.iLen, a_hQQClt->szGameKey, pszDec, &iDec) )
	{
              a_hQQClt->iErr =TQQAPI_ERR_DECRYPTION_HEAD;
		return -1;
	}

	stHost.pszBuff	=	(char*)pstIdentInfo;
	stHost.iBuff	=	(int)sizeof(*pstIdentInfo);

	stNet.pszBuff	=	(char*) pszDec;
	stNet.iBuff	=	iDec;
	
       iRet=tdr_ntoh(pstMetaIdentInfo, &stHost, &stNet, TPDU_VERSION);
	if( iRet<0 )
	{
              a_hQQClt->iErr =TQQAPI_ERR_TDR_NTOH_HEAD;
	       return iRet;
	}

	return 0;
}

int tqqapi_extract_Sessionkey(HTQQAPI a_hQQClt, TPDUHEAD* a_pstHead,char *pszSessionkey)
{
    
	TPDUHEAD stTempHead;
	char* pszDec;
	int iDec;

	if( TPDU_CMD_CHGSKEY!=a_pstHead->stBase.bCmd )
	{
              a_hQQClt->iErr=TQQAPI_ERR_INVALID_CMD;
		return -1;
	}


	pszDec	=	(char*) stTempHead.stExt.stChgSkey.szEncryptSkey;
	iDec	=	(int)sizeof(stTempHead.stExt.stChgSkey.szEncryptSkey);

	if( !oi_symmetry_decrypt2((char*)a_pstHead->stExt.stChgSkey.szEncryptSkey, a_pstHead->stExt.stChgSkey.nLen, a_hQQClt->szGameKey, pszDec, &iDec) )
	{
              a_hQQClt->iErr =TQQAPI_ERR_DECRYPTION_HEAD;
		return -2;
	}

	memcpy(pszSessionkey,pszDec,TQQ_KEY_LEN);

	return 0;  


}

int  tqqapi_extract_syninfo(HTQQAPI a_hQQClt,TPDUHEAD* a_pstHead)
{
      LPTDRMETALIB pstMetaLib;
      LPTDRMETA pstMetaSynInfo;
      TPDUEXTSYN stTempSyn;
      TDRDATA stHost;
      TDRDATA stNet;
      char *pszDec=NULL;
      int     iDec=0;
      int     iRet=0;

      if( TPDU_CMD_SYN!=a_pstHead->stBase.bCmd)
      	{
             a_hQQClt->iErr=TQQAPI_ERR_INVALID_CMD;
	      return -1;

	}

	pszDec=(char *)stTempSyn.szEncryptSynInfo;
	iDec=(int)sizeof(stTempSyn.szEncryptSynInfo);

	if(!oi_symmetry_decrypt2(a_pstHead->stExt.stSyn.szEncryptSynInfo,a_pstHead->stExt.stSyn.bLen, a_hQQClt->szGameKey, pszDec, &iDec))
	{
             a_hQQClt->iErr=TQQAPI_ERR_DECRYPTION_HEAD;
	      return -2;
	}

	pstMetaLib	=	(LPTDRMETALIB) g_szMetalib_tqqapi;

	pstMetaSynInfo=	tdr_get_meta_by_name(pstMetaLib, "TPDUSynInfo");

	stNet.pszBuff=pszDec;
	stNet.iBuff=iDec;

	stHost.pszBuff=(char *)&a_hQQClt->stSynInfo;
	stHost.iBuff=(int)sizeof(a_hQQClt->stSynInfo);

	iRet=tdr_ntoh(pstMetaSynInfo,&stHost,&stNet,TPDU_VERSION);

	if(iRet<0)
	{
	     return iRet;
	}

	return 0;

}

int tqqapi_build_auth_msg(TPDUHEAD *a_pstHead, long uin, const char *pszSvrkey,int iAuthType,char *pszGameKey,unsigned int uCltIP)
{
	
    int iRet=0;
    char szSessionKey[TQQ_KEY_LEN] = {0};
    char szSvrkey[TQQ_KEY_LEN]={0};
    TQQGAMESIG stGameSig;
    TQQGAMESIG stTempSig;	
    TQQSIGFORS2 stSigForS2;
    TQQSIGFORS2 stTempSig2;

    assert(NULL!=pszSvrkey);
    assert(NULL!=a_pstHead);
    assert(NULL!=pszGameKey); 
    if( (NULL == pszSvrkey)
        || (NULL == pszSvrkey)	
        || (NULL == pszSvrkey))
    {
             return -1;
    }

    tqqapi_init_base(&a_pstHead->stBase);

    a_pstHead->stBase.bCmd=TPDU_CMD_AUTH;
    a_pstHead->stExt.stAuthInfo.iAuthType=iAuthType;
    tqqapi_randstr(szSessionKey, (int)TQQ_KEY_LEN);
    memcpy(pszGameKey,szSessionKey,TQQ_KEY_LEN);
    memcpy(szSvrkey,pszSvrkey,TQQ_KEY_LEN);
  
    switch( iAuthType)
    {
        case TSEC_AUTH_NONE:
	      break;
	 case TSEC_AUTH_QQV1:
	 case TSEC_AUTH_QQV2:
	 	//sig1
	 	memset(&stGameSig, 0, sizeof(stGameSig));
	       stGameSig.lUin	=	uin;
	       stGameSig.ulTime=	(unsigned long)time(NULL);
	       memcpy(stGameSig.szGameKey, szSessionKey, TQQ_KEY_LEN);

		//sig2
	       memset(&stSigForS2, 0, sizeof(stSigForS2));
	       stSigForS2.lUin	=	uin;
	       stSigForS2.ulTime =    (unsigned long)time(NULL);
		stSigForS2.ulCltIP =uCltIP;
	 	iRet=tqqapi_make_authinfo(&a_pstHead->stExt.stAuthInfo.stAuthData.stAuthQQV2, &stGameSig, &stSigForS2, uin,szSvrkey);
		if( 0 != iRet )
		{
                   return iRet;
		}

		//checkout signature
		iRet=tqqapi_extract_authinfo(&stTempSig,&stTempSig2, uin, &a_pstHead->stExt.stAuthInfo.stAuthData.stAuthQQV2,szSvrkey );
		if( 0 != iRet )
		{
                   return iRet;
		}	
	      break;
	 case TSEC_AUTH_QQUNIFIED:
	 	iRet = tqqapi_make_QQUnify_Authinfo(&a_pstHead->stExt.stAuthInfo.stAuthData.stAuthQQUnified, uin, szSvrkey,szSessionKey,uCltIP);
		if( 0 != iRet )
		{
                  return iRet;
		}
		//checkout signature 
		iRet = tqqapi_extract_QQUnify_Authinfo(&a_pstHead->stExt.stAuthInfo.stAuthData.stAuthQQUnified,szSvrkey);
		if( 0 != iRet )
		{
                  return iRet;
		}		
	      break;
	 default: 	
	      return -3;

    }

    return 0;

}

int tqqapi_build_relay_msg(TPDUHEAD* a_pstHead,long uin,int nPos,const char * szConnectKey,const char *szIdentity,int iRelayType)
{
  
   LPTDRMETALIB pstMetaLib;
   LPTDRMETA pstMetaIdentInfo;
   TQQUSERIDENT stIdent;
   TQQUSERIDENT stTempIdent;
   char *pszEnc=NULL;
   int iEnc;
   int iRet=0;
   TDRDATA stHost;
   TDRDATA stNet;
   
   assert(NULL!=szConnectKey);
   assert(NULL!=szIdentity);
   assert(NULL!=a_pstHead);

   tqqapi_init_base(&a_pstHead->stBase);
   a_pstHead->stBase.bCmd=TPDU_CMD_RELAY;
   a_pstHead->stExt.stRelay.iOldPos = nPos;
   
   stIdent.iPos=nPos;
   stIdent.lUin=uin;
   memcpy(stIdent.szIdent,szIdentity,TQQ_IDENT_LEN);

   stHost.pszBuff=(char *)&stIdent;
   stHost.iBuff=(int)sizeof(stIdent);

   stNet.pszBuff=(char *)&stTempIdent;
   stNet.iBuff=(int)sizeof(stTempIdent);

   pstMetaLib	=	(LPTDRMETALIB) g_szMetalib_tqqapi;

   pstMetaIdentInfo=	tdr_get_meta_by_name(pstMetaLib, "TQQUserIdent");

   iRet = tdr_hton(pstMetaIdentInfo, &stNet,&stHost,0);

   if(iRet <0 )
   {
         return iRet;
   }

   pszEnc=a_pstHead->stExt.stRelay.szEncryptIdent;
   iEnc=sizeof(a_pstHead->stExt.stRelay.szEncryptIdent);

   oi_symmetry_encrypt2(stNet.pszBuff, stNet.iBuff, szConnectKey, pszEnc, &iEnc);
   
   a_pstHead->stExt.stRelay.iLen=iEnc;

   a_pstHead->stExt.stRelay.iOldPos=nPos;
   
   a_pstHead->stExt.stRelay.iRelayType=iRelayType;
   return 0;
   
}

int tqqapi_create_initial_connection(HTQQAPI a_phQQClt,long uin, const char *pszSvrkey,int iAuthType,int a_iTimeOut)
{
     TPDUHEAD stPduHead;
     char szGamekey[TQQ_KEY_LEN]={0};
     int iRet=0;
     int iMsg=0;
     TPDUIDENTINFO  stIdentInfo;
     char szSessionKey[TQQ_KEY_LEN] = {0};
     struct sockaddr_in addr;
     int size=sizeof(addr);
	 
     memset((char *)&stPduHead,0,sizeof(stPduHead));

     getsockname( a_phQQClt->s,(struct sockaddr *)&addr,&size);
	 
     iRet = tqqapi_build_auth_msg(&stPduHead, uin, pszSvrkey, iAuthType, szGamekey,addr.sin_addr.s_addr);
      if( iRet<0)
      {
           a_phQQClt->iErr=TQQAPI_ERR_BUILD_AUTH;
           return iRet;
      }

     tqqapi_set_gamekey(a_phQQClt,(const char *) szGamekey);

      //send Auth Msg
     iRet=tqqapi_send_msg(a_phQQClt,&stPduHead, NULL, 0,a_iTimeOut );
     if( iRet<0 )
     {
           return iRet; 
     }

     //recv syn msg
     iRet=tqqapi_recv_msg(a_phQQClt, &stPduHead, NULL,&iMsg,a_iTimeOut);
     if( ( iRet<0 ) )
     {
           return iRet;
     }
     else if( 0==iRet)
     {
           a_phQQClt->iErr=TQQAPI_ERR_RECV_TIMEOUT;
	    return -1;
     }
	 
     iRet=tqqapi_extract_syninfo(a_phQQClt,&stPduHead);
     if(iRet<0)
     {
           return iRet;

     }

    //send synack Msg
    iRet=tqqapi_build_synack_msg(a_phQQClt, &stPduHead);
    if(iRet<0)
    {
          a_phQQClt->iErr=TQQAPI_ERR_BUILD_SYNACK;
          return iRet;
    }

     iRet=tqqapi_send_msg(a_phQQClt, &stPduHead, NULL, 0, a_iTimeOut);
     if(iRet<0)
     {
           return iRet;
     }

  
     while(1)
     {
	  iRet=tqqapi_recv_msg(a_phQQClt,&stPduHead,NULL,&iMsg,a_iTimeOut);
	  if( iRet<0 )
	  {
	             return iRet;
	  }
	  else if(0 == iRet)
	  {
	              a_phQQClt->iErr=TQQAPI_ERR_RECV_TIMEOUT;
	              return -1;
	  }
	  
	  //recv one package
	  if( TPDU_CMD_QUEINFO == stPduHead.stBase.bCmd )
	  {
	             continue;   
	  }
	  else if( TPDU_CMD_IDENT== stPduHead.stBase.bCmd )
	  {
	             break; 
	  }
	  else
	  {
	             a_phQQClt->iErr=TQQAPI_ERR_INVALID_CMD;
		      return -2;
	  }
    }
     
    //recv identity msg
    iRet=tqqapi_extract_identinfo(a_phQQClt, &stPduHead, &stIdentInfo);
    if( iRet<0 )
    {
          return iRet; 
    }

     tqqapi_set_identity(a_phQQClt, stIdentInfo.iPos, stIdentInfo.szIdent);

     iRet=tqqapi_recv_msg(a_phQQClt, &stPduHead, NULL, &iMsg,a_iTimeOut );
     if( iRet<0 )
     {
           return iRet;
     }
     else if(0==iRet)
     {
           a_phQQClt->iErr=TQQAPI_ERR_RECV_TIMEOUT;
	    return -3;
     }

    /*change session key */	 
      iRet=tqqapi_extract_Sessionkey(a_phQQClt,&stPduHead, szSessionKey);
      if( iRet<0 )
      {
             return iRet;
      }
      tqqapi_set_gamekey(a_phQQClt, szSessionKey);

       return 0;
}

int tqqapi_create_relay_connection(HTQQAPI a_phQQClt,long uin,int nPos,const char * szConnectKey,const char *szIdentity, int a_iTimeOut,int iRelayType)
{
     TPDUHEAD stPduHead;
     int iRet=0;
     int iMsg=0;
     TPDUIDENTINFO  stIdentInfo;

	 
     memset((char *)&stPduHead,0,sizeof(stPduHead));
     iRet=tqqapi_build_relay_msg(&stPduHead, uin, nPos,szConnectKey,szIdentity,iRelayType);
     if( iRet<0 )
     {
          a_phQQClt->iErr=TQQAPI_ERR_TDR_HTON_USERIDENT;
	   return iRet;
     }
     //set gamekey
     tqqapi_set_gamekey(a_phQQClt,(const char *) szConnectKey);

     //send relay msg
     iRet=tqqapi_send_msg(a_phQQClt, &stPduHead, NULL, 0,a_iTimeOut);
     if( iRet<0 )
     {
          return iRet;
     }   
    
	 
     iRet=tqqapi_recv_msg(a_phQQClt, &stPduHead, NULL, &iMsg,  a_iTimeOut);

     if(iRet<0)
     {
          return iRet;
	 
     }
     else if( 0==iRet)
     {
          //recv timeout
	   a_phQQClt->iErr=TQQAPI_ERR_RECV_TIMEOUT;
	   return -1;   
     }

	 
     //recv one msg
     iRet=tqqapi_extract_identinfo(a_phQQClt,&stPduHead, &stIdentInfo);
     if( (iRet<0)  )
     {
          return iRet;
     }
     tqqapi_set_identity(a_phQQClt, stIdentInfo.iPos, stIdentInfo.szIdent);
     return 0;

}

int tqqapi_get_sock(HTQQAPI a_phQQClt)
{
	return a_phQQClt->s;
}


int tqqapi_set_recvbuff(HTQQAPI a_phQQClt, int a_iSize)
{
	return tnet_set_recvbuff(a_phQQClt->s, a_iSize);
}

int tqqapi_set_sendbuff(HTQQAPI a_phQQClt, int a_iSize)
{
	return tnet_set_sendbuff(a_phQQClt->s, a_iSize);
}


int tqqapi_IsRelay_Enable(HTQQAPI a_phQQClt)
{
      if( (TQQAPI_ERR_NETWORK_FAILURE== a_phQQClt->iErr) )
      {
           return 0;

      }
      
      return -1;
}

const char * tqqapi_get_errstring(HTQQAPI a_hQQClt)
{

   if( (a_hQQClt->iErr>0) && (a_hQQClt->iErr < TQQAPI_ERR_COUNT))
   {

       if( ( a_hQQClt->iErr == TQQAPI_ERR_TDR_NTOH_HEAD)
	    || ( a_hQQClt->iErr == TQQAPI_ERR_TDR_HTON_HEAD ) 
	    || ( a_hQQClt->iErr == TQQAPI_ERR_TDR_NTOH_BODY ) 
	    || ( a_hQQClt->iErr == TQQAPI_ERR_TDR_HTON_BODY ) )
       {
          return tdr_error_string(a_hQQClt->iTdrErrorCode);
	}
	else
	{
           return tqqapi_error_string[a_hQQClt->iErr];
	}
	
   }
   else
   {

        return NULL;
   }
}

int tqqapi_build_synack_msg(HTQQAPI a_hQQClt,TPDUHEAD* a_pstHead)
{
     int iRet=0;
     TDRDATA stHost;
     TDRDATA stNet;
     char *pszEnc;
     int    iEnc;
     TPDUEXTSYNACK stTemp;
     LPTDRMETALIB pstMetaLib;	
     LPTDRMETA pstMetaSynInfo;
     

    /* build Head*/	 
     memset((char *)a_pstHead,0,sizeof(TPDUHEAD));
     tqqapi_init_base(&a_pstHead->stBase);
     a_pstHead->stBase.bCmd=TPDU_CMD_SYNACK;

     pstMetaLib	=	(LPTDRMETALIB) g_szMetalib_tqqapi;

     pstMetaSynInfo=	tdr_get_meta_by_name(pstMetaLib, "TPDUSynInfo");

     stNet.pszBuff=(char *)stTemp.szEncryptSynInfo;
     stNet.iBuff=sizeof(stTemp.szEncryptSynInfo);
     stHost.pszBuff=(char *)&a_hQQClt->stSynInfo;
     stHost.iBuff=sizeof(a_hQQClt->stSynInfo);

     iRet=tdr_hton(pstMetaSynInfo, &stNet, &stHost, TPDU_VERSION);
     if( 0 != iRet)
     {
          return iRet;
     }

     pszEnc = (char *)a_pstHead->stExt.stSynAck.szEncryptSynInfo;
     iEnc=sizeof(a_pstHead->stExt.stSynAck.szEncryptSynInfo);

     oi_symmetry_encrypt2(stNet.pszBuff, stNet.iBuff,a_hQQClt->szGameKey,pszEnc,&iEnc);

     a_pstHead->stExt.stSynAck.bLen = iEnc;

     return 0;
}


