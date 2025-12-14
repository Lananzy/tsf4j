#include "pal/pal.h"
#include "comm/terr.h"
#include "comm/terrimpl.h"

static pthread_key_t 	s_key	=	0;
static pthread_once_t	s_once	=	PTHREAD_ONCE_INIT;

static void terr_free_buff(void* pvBuff)
{
	free(pvBuff);
}

static void* terr_alloc_buff(void)
{
	return calloc(1, sizeof(HERR));
}

static void terr_create_key(void)
{
	pthread_key_create(&s_key, terr_free_buff);
}

static void terr_check_key(void)
{
	pthread_once(&s_once, terr_create_key);
}

HERR* terr_get_location(void)
{
	HERR* phErr;

	terr_check_key();

	phErr	=	(HERR*) pthread_getspecific(s_key);

	if( phErr )
		return phErr; 

	phErr	=	(HERR*) terr_alloc_buff();

	pthread_setspecific(s_key, phErr);

	return phErr;
}
