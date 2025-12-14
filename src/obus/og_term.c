
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include "og_term.h"

int iTerminal=-1;


void SetTerminal(int iSignal)
{
	iTerminal = 1;
}

void CheckTerm(const char *sDesc)
{
	if (iTerminal == 0) return;
	else if (iTerminal < 0) {
		signal(SIGTERM, SetTerminal);
		iTerminal = 0;
		return;
	}
	printf("Get SIGTERM, %s exit.\n", sDesc);
	exit(0);
}

void CheckTermWithClean(const char *sDesc, PFNTERMCLEAN pfnClean)
{
	if (iTerminal == 0) return;
	else if (iTerminal < 0) {
		signal(SIGTERM, SetTerminal);
		iTerminal = 0;
		return;
	}
	(*pfnClean)();
	printf("Get SIGTERM, %s exit.\n", sDesc);
	exit(0);
}

void CheckTermWithExit(const char *sDesc, PFNTERMEXIT pfnExit, void* pvArg)
{
	if (iTerminal == 0) return;
	else if (iTerminal < 0) {
		signal(SIGTERM, SetTerminal);
		iTerminal = 0;
		return;
	}
	(*pfnExit)(pvArg);
	printf("Get SIGTERM, %s exit.\n", sDesc);
	exit(0);
}
