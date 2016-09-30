/*
===============================================================================

Name	:	scr_tank.h

Purpose	:	function definitions for tank objects

Date	:	12/11/2004

===============================================================================
*/

#include "local.h"
//#include "scr_main.h"

int tankfunc_move (void *iter)
{
	cScript::iterator	*piter = (cScript::iterator *)iter;
	cTank *pTank;

	char	szCmd[64];
	char	szVar[64];
	sScriptVar	*pVar;

	memset( szCmd, 0, 64 );
	memset( szVar, 0, 64 );

	parse( piter->getstr(), "%63s(%63s)", szCmd, szVar );

	pVar = piter->getval( string("this") );
	pTank = (cTank *)pVar->value.value;

	wsex( szVar );

	if (stricmp(szVar,"forward") == 0)
		pTank->m_Keys[KEY_FORWARD] = 1;
	else if (stricmp(szVar,"back") == 0)
		pTank->m_Keys[KEY_BACK] = 1;
	else if (stricmp(szVar,"left") == 0)
		pTank->m_Keys[KEY_LEFT] = 1;
	else if (stricmp(szVar,"right") == 0)
		pTank->m_Keys[KEY_RIGHT] = 1;
	else if (stricmp(szVar,"turret_left") == 0)
		pTank->m_Keys[KEY_TLEFT] = 1;
	else if (stricmp(szVar,"turret_right") == 0)
		pTank->m_Keys[KEY_TRIGHT] = 1;
	else if (stricmp(szVar,"fire") == 0)
		pTank->m_Keys[KEY_FIRE] = 1;

	return SCRIPT_CONTINUE;
}

int tank_begin (void *tank, void *iter)
{
	cTank	*pTank = (cTank *)tank;
	cScript::iterator *pIter = (cScript::iterator *)iter;

	pIter->addvar( magic_s((int )pTank), "this", type_object );

	pIter->addcmd( tankfunc_move, string("move") );

	return 0;
}
