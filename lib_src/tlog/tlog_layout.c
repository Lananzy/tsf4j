/*
**  @file $RCSfile: tlog_layout.c,v $
**  general description of this module
**  $Id: tlog_layout.c,v 1.9 2009/03/27 06:17:02 kent Exp $
**  @author $Author: kent $
**  @date $Date: 2009/03/27 06:17:02 $
**  @version $Revision: 1.9 $
**  @note Editor: Vim 6.1, Gcc 4.0.1, tab=4
**  @note Platform: Linux
*/

#include "pal/pal.h"
#include "tlog/tlog_priority.h"
#include "tlog/tlog_layout.h"

int tlog_layout_format(tlog_event_t* a_pstEvt, const char* a_pszFmt)
{
	tlog_buffer_t * pstBuffer = &a_pstEvt->evt_buffer;
	const char* pszFmt;

	unsigned int uiBuffOff = 0;
	int iSubLen = 0;
	
	if( !a_pszFmt || '\0'==a_pszFmt[0] )
		return -1;

	pszFmt	=	a_pszFmt;

	while(*pszFmt && uiBuffOff < pstBuffer->buf_maxsize)
	{
		if('%' != *pszFmt)
		{ 		
			pstBuffer->buf_data[uiBuffOff++] = *pszFmt++;	
			continue;
		}		

		switch(*(++pszFmt))
		{
		case 'c' :
			{
				iSubLen = snprintf(pstBuffer->buf_data + uiBuffOff, 
					pstBuffer->buf_maxsize -uiBuffOff, "%s", a_pstEvt->evt_category);

				if(iSubLen > 0 && iSubLen <= (int)(pstBuffer->buf_maxsize -uiBuffOff))
				{
					uiBuffOff += iSubLen;
				}					
				break;
			}
		case 'd' :
			{
				struct tm	tm;

				localtime_r(&a_pstEvt->evt_timestamp.tv_sec, &tm);

				iSubLen = snprintf(pstBuffer->buf_data + uiBuffOff,
					pstBuffer->buf_maxsize -uiBuffOff,  "%04d%02d%02d %02d:%02d:%02d",
					tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday,
					tm.tm_hour, tm.tm_min, tm.tm_sec);
				if(iSubLen > 0 && iSubLen <= (int)(pstBuffer->buf_maxsize -uiBuffOff))
				{
					uiBuffOff += iSubLen;
				}				
				break;
			}

		case 'u' :
			{
				iSubLen = snprintf(pstBuffer->buf_data + uiBuffOff,
					pstBuffer->buf_maxsize -uiBuffOff,  "%06d", (int) a_pstEvt->evt_timestamp.tv_usec);
				if(iSubLen > 0 && iSubLen <= (int)(pstBuffer->buf_maxsize -uiBuffOff))
				{
					uiBuffOff += iSubLen;
				}				
				break;
			}

		case 'm' :
			{
				iSubLen	=	 pstBuffer->buf_maxsize - uiBuffOff;

				if( iSubLen > a_pstEvt->evt_msg_len )
					iSubLen	=	a_pstEvt->evt_msg_len;

				memcpy(pstBuffer->buf_data + uiBuffOff, a_pstEvt->evt_msg, iSubLen);

				if(iSubLen > 0)
				{
					uiBuffOff += iSubLen;
				}						

				break;
			}
		case 'P' :
			{
				iSubLen = snprintf(pstBuffer->buf_data + uiBuffOff, 
					pstBuffer->buf_maxsize -uiBuffOff, "%d", getpid());
				if(iSubLen > 0 && iSubLen <= (int)(pstBuffer->buf_maxsize -uiBuffOff))
				{
					uiBuffOff += iSubLen;
				}		
				break;
			}
		case 'p' :
			{
				iSubLen = snprintf(pstBuffer->buf_data + uiBuffOff, 
					pstBuffer->buf_maxsize -uiBuffOff, "%-8s", 
					tlog_priority_to_string(a_pstEvt->evt_priority));
				if(iSubLen > 0 && iSubLen <= (int)(pstBuffer->buf_maxsize -uiBuffOff))
				{
					uiBuffOff += iSubLen;
				}		
				
				break;
			}
#ifndef WIN32
			/*TODO:在windows环境pthread_self的实现不支持将返回值转换成整数*/
		case 't' :
			{
				iSubLen = snprintf(pstBuffer->buf_data + uiBuffOff, 
					pstBuffer->buf_maxsize -uiBuffOff, "%lu", (unsigned long)pthread_self());
				if(iSubLen > 0 && iSubLen <= (int)(pstBuffer->buf_maxsize -uiBuffOff))
				{
					uiBuffOff += iSubLen;
				}

				break;
			}
#endif
		case 'f' :
			{
				iSubLen = snprintf(pstBuffer->buf_data + uiBuffOff, 
					pstBuffer->buf_maxsize -uiBuffOff, "%s", 
					(a_pstEvt->evt_loc ? a_pstEvt->evt_loc->loc_file : "(nil)"));
				if(iSubLen > 0 && iSubLen <= (int)(pstBuffer->buf_maxsize -uiBuffOff))
				{
					uiBuffOff += iSubLen;
				}						
				break;
			}
		case 'F' :
			{
				iSubLen = snprintf(pstBuffer->buf_data + uiBuffOff, 
					pstBuffer->buf_maxsize -uiBuffOff, "%s", 
					(a_pstEvt->evt_loc ? a_pstEvt->evt_loc->loc_function: "(nil)"));
				if(iSubLen > 0 && iSubLen <= (int)(pstBuffer->buf_maxsize -uiBuffOff))
				{
					uiBuffOff += iSubLen;
				}						
				break;
			}
		case 'l' :
			{
				iSubLen = snprintf(pstBuffer->buf_data + uiBuffOff, 
					pstBuffer->buf_maxsize -uiBuffOff, "%d", 
					(a_pstEvt->evt_loc ? a_pstEvt->evt_loc->loc_line: 0));
				if(iSubLen > 0 && iSubLen <= (int)(pstBuffer->buf_maxsize -uiBuffOff))
				{
					uiBuffOff += iSubLen;
				}						
				break;
			}
		case 'n' : 
			{
				iSubLen = snprintf(pstBuffer->buf_data + uiBuffOff, 
					pstBuffer->buf_maxsize -uiBuffOff, "\n");
				if(iSubLen > 0 && iSubLen <= (int)(pstBuffer->buf_maxsize -uiBuffOff))
				{
					uiBuffOff += iSubLen;
				}		
			}
		default:
			{
				/* unknown */
			}
		}	
		if(*pszFmt)
		{
			++pszFmt;
		}	
	}

	if((unsigned int)uiBuffOff >= pstBuffer->buf_maxsize) 
	{
		pstBuffer->buf_data[pstBuffer->buf_maxsize - 1] = '\0';
		pstBuffer->buf_data[pstBuffer->buf_maxsize - 2] = '.';
		pstBuffer->buf_data[pstBuffer->buf_maxsize - 3] = '.';
		pstBuffer->buf_data[pstBuffer->buf_maxsize - 4] = '.';
		pstBuffer->buf_size		=	uiBuffOff - 1;
	}
	else
	{
		pstBuffer->buf_data[uiBuffOff] = '\0';
		pstBuffer->buf_size		=	uiBuffOff;
	}


	a_pstEvt->evt_rendered_msg		=	pstBuffer->buf_data;
	a_pstEvt->evt_rendered_msg_len	=	pstBuffer->buf_size;

	return (int)(pstBuffer->buf_size);
}

