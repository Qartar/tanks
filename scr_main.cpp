/*
===============================================================================

Name	:	scr_main.h

Purpose	:	script parsing object

Date	:	12/10/2004

===============================================================================
*/

#include "scr_main.h"

#include <iostream>
#include <fstream>
#include <stdarg.h>

// PROTOTYPES

int FUNC_LOCAL (void *iter);
int	FUNC_IF (void *iter);
int FUNC_EXIT (void *iter);
int FUNC_SET (void *iter);
int FUNC_WAIT (void *iter);

char *wsex (char *arg);

/*
===========================================================

Name	:	cScript

===========================================================
*/

int cScript::Load (string filename)
{
	ifstream	ifs;
	string		str;

	m_fileName = filename;

	ifs.open( filename.c_str(), ios::in );

	if (ifs.is_open() == false)
		return 1;	// fail

	while ( getline( ifs, str ) )
		m_vLines.push_back( str );

	ifs.close( );

	return 0;	// success
}

int cScript::Unload ()
{
	m_vLines.clear( );

	return 0;
}

/*
===========================================================

Name	:	cScript::iterator

===========================================================
*/

cScript::iterator::iterator ()
{
}

void cScript::iterator::begin(cScript *pScript)
{
	m_fileName = pScript->m_fileName;

	m_strBegin = &pScript->m_vLines[0];
	m_strEnd = &pScript->m_vLines.back();

	m_strCursor = m_strBegin;

	// default variables and commands HERE
	addcmd( FUNC_LOCAL, string("local") );
	addcmd( FUNC_IF, string("if") );
	addcmd( FUNC_EXIT, string("exit") );
	addcmd( FUNC_SET, string("set") );
	addcmd( FUNC_WAIT, string("wait") );
}

int cScript::iterator::operator++ (int)
{
	int			i, size, ret;
#if 0
	if ( strlen(wsex((char *)m_strCursor->c_str())) < 1 )
	{
		if (m_strCursor < m_strEnd)
			m_strCursor++;
		else
		{
			printf( "end of script reached without exit\n" );
			return SCRIPT_ERROR;
		}
		return SCRIPT_CONTINUE;
	}
#endif

	size = m_Cmds.size( );
	for (i=0 ; i<size ; i++)
		if ( strnicmp( m_strCursor->c_str(), m_Cmds[i].strCmd.c_str(), m_Cmds[i].strCmd.length() ) == 0 )
		{

			ret = m_Cmds[i].pfFunc( (void *)this );

			if (m_strCursor < m_strEnd)
				m_strCursor++;
			else
			{
				printf( "end of script reached without exit\n" );
				return SCRIPT_ERROR;
			}

			return ret;
		}

	return SCRIPT_CONTINUE;	
}

void cScript::iterator::run ()
{
	int	last;
	while ( ( (last = (*this)++) > 0 ) ) {}

	// KILL ME NOW
	if (last < 0) 
		return;
}

sScriptVar *cScript::iterator::getval (string str)
{
	int		i, size;
	
	size = m_Vars.size( );
	for (i=0 ; i<size ; i++)
		if ( strnicmp( str.c_str(), m_Vars[i].strVar.c_str(), m_Vars[i].strVar.length() ) == 0 )
			return &m_Vars[i];

	return NULL;
}

int cScript::iterator::addvar (magic_t magic, string name, eType type)
{
	sScriptVar	pVar;

	pVar.value = magic;
	pVar.strVar = name;

	m_Vars.push_back( pVar );

	return 0;
}

int cScript::iterator::addcmd (func *pfunc, string name)
{
	sScriptCmd	pCmd;

	pCmd.pfFunc = pfunc;
	pCmd.strCmd = name;

	m_Cmds.push_back( pCmd );

	return 0;
}

int cScript::iterator::addlocal (void *value)
{
	m_Locals.push_back( *((int *)value) );

	return 0;
}

/*
===============================================================================

	COMMANDS

===============================================================================
*/

//#define min(a,b) (a < b ? a : b)

void parse (const char *in, const char *fmt, ...)
{
	va_list	vptr;
	char	*fcrs = (char *)fmt;
	char	*fend = (char *)fmt;
	char	*icrs = (char *)in;
	char	*iend = (char *)in;
	char	*varg;
	char	tempbuf[32];
	int		buflen = 0;
	int		len, i, vlen;

	va_start( vptr, fmt );

	while (*fcrs)
	{
		if (*fcrs == '%')
		{
			// find the size of buffer to be written
			fend = fcrs;
			while (*fend++ != 's')
				;
			if (fend - fcrs > 2)
			{
				len = fend - fcrs - 2;
				memset(tempbuf,0,32);
				for (i=0 ; i<len ; i++)
					tempbuf[i] = *(fcrs+i+1);
				vlen = atoi( tempbuf );
			}
			else
				vlen = 1024;

			// get buffer ptr
			varg = va_arg( vptr, char* );

			// find escape sequence
			fcrs = fend;
			while (*fend++ != '%' && *fend)
				;
			if (fend - fcrs > 1)
			{
				len = fend - fcrs - 1;
 				memset(tempbuf,0,32);
           		for (i=0 ; i<len ; i++)
					tempbuf[i] = *(fcrs+i);
				buflen = strlen(tempbuf);
			}
			else
			{
				memset(tempbuf,0,32);
				tempbuf[0] = *fcrs;
				buflen = 1;
			}

			// copy in into buf until we hit escape sequence

			while (strncmp(iend,tempbuf,buflen))
			{
				if (!*iend)
				{
					printf( "inproper format\n" );
					return;
				}
				else
					iend++;
			}

			strncpy( varg, icrs, min(vlen,iend-icrs) );
			icrs = iend + buflen;
			iend = icrs;

			fcrs = fend - 1;
		}
		else
			fcrs++;
	}

	va_end( vptr );
}

char *wsex (char *arg)
{
	int	i, j, len = strlen(arg);
	static char	buffer[1024];
	
	if (len > 1024)
		len = 1024;

	memset( buffer, 0, strlen(buffer) );

	for (i=0,j=0 ; i<len ; i++)
	{
		if (arg[i] > 32)
			buffer[j++] = arg[i];
	}
	strncpy( arg, buffer, len );

	return arg;
}

magic_s logic (char *exp, cScript::iterator *iter)
{
	char	*crs, *end;
	char	tempbuf[64];
	stack<magic_t>	stmagic;
	magic_t			val1, val2;
	magic_t			ret;
	sScriptVar		*var;

	crs = end = exp;
	while(*end)
	{
		end++;

		if ((*end == 32) || (!*end))
		{
			if ((*crs == '+') || (*crs == '-') || (*crs == '*') || (*crs == '/'))
			{
				val1 = stmagic.top( );
				stmagic.pop( );
				val2 = stmagic.top( );
				stmagic.pop( );

				switch (*crs)
				{
				case '+':
					stmagic.push( val1 + val2 );
					break;
				case '-':
					stmagic.push( val1 - val2 );
					break;
				case '*':
					stmagic.push( val1 * val2 );
					break;
				case '/':
					stmagic.push( val1 / val2 );
					break;
				default:
					break;
				}
			}
			else
			{
				memset( tempbuf, 0, 64 );
				strncpy( tempbuf, crs, end - crs - 1 );
				if (*(end-1) == 'f')
				{
					val1 = magic_s(atof(tempbuf));
					stmagic.push( val1 );
				}
				else if (atoi(tempbuf) != 0)
				{
					val1 = magic_s(atoi(tempbuf));
					stmagic.push( val1 );
				}
				else
				{
					var = iter->getval( string(tempbuf) );
					val1 = var->value;
					stmagic.push( val1 );
				}
			}

			crs = end + 1;
			end = crs;
		}
	}

	return stmagic.top( );
}

/*
===========================================================

Name	:	FUNC_LOCAL

Purpose	:	Adds a localized variable to the script

===========================================================
*/

int FUNC_LOCAL (void *iter)
{
	cScript::iterator	*piter;
	char	szCmd[64];
	char	szName[64];
	char	szType[64];
	char	szValue[64];

	eType	type;
	magic_t	magic;

	memset( szCmd, 0, 64 );
	memset( szName, 0, 64 );
	memset( szType, 0, 64 );
	memset( szValue, 0, 64 );

	piter = (cScript::iterator *)iter;

//	sscanf( piter->getstr(), "%63s(%63s,%63s,%63s);", szCmd, szName, szType, szValue );
	parse( piter->getstr(), "%63s(%63s,%63s,%63s);", szCmd, szName, szType, szValue );

	wsex( szCmd );
	wsex( szName );
	wsex( szType );
	wsex( szValue );

	printf( "%s\n%s\n%s\n%s\n%s\n", piter->getstr(), szCmd, szName, szType, szValue );


	if ( stricmp( szType, "int" ) == 0 )
		magic = magic_s(atoi( szValue ));
	else if ( stricmp( szType, "float" ) == 0 )
		magic = magic_s(atof( szValue ));
	else if ( stricmp( szType, "object" ) == 0 )
		magic = magic_s( 0 );
	else
	{
		printf( "script error: %s: unknown type %s\n", szCmd, szType );
		return SCRIPT_ERROR;
	}

	piter->addvar( magic, string(szName), type );

	return SCRIPT_CONTINUE;
}

/*
===========================================================

Name	:	FUNC_IF

Purpose	:	logical if statements

===========================================================
*/

int FUNC_IF (void *iter)
{
	cScript::iterator	*piter = (cScript::iterator *)iter;
	char	szCmd[64];
	char	szExp[128];

	memset( szCmd, 0, 64 );
	memset( szExp, 0, 128 );

	parse( piter->getstr(), "%63s(%127s)", szCmd, szExp );

//	wsex( szExp );

	printf( "%s\n%s\n%s\n", piter->getstr(), szCmd, szExp );

	return SCRIPT_CONTINUE;
}

/*
===========================================================

Name	:	FUNC_EXIT

Purpose	:	exits the script

===========================================================
*/

int FUNC_EXIT (void *iter)
{
	return SCRIPT_EXIT;
}

/*
===========================================================

Name	:	FUNC_SET

Purpose	:	sets variable to value

===========================================================
*/

int FUNC_SET (void *iter)
{
	cScript::iterator	*piter = (cScript::iterator *)iter;
	char	szCmd[64];
	char	szVar[64];
	char	szExp[128];
	sScriptVar	*pVar;

	memset( szCmd, 0, 64 );
	memset( szVar, 0, 64 );
	memset( szExp, 0, 128 );

	parse( piter->getstr(), "%63s(%63s,%127s)", szCmd, szVar, szExp );

	wsex( szVar );

	pVar = piter->getval( string(szVar) );
	pVar->value = logic( szExp, piter );

	return SCRIPT_CONTINUE;
}

/*
===========================================================

Name	:	FUNC_WAIT

Purpose	:	halts script

===========================================================
*/

int FUNC_WAIT (void *iter)
{
	return SCRIPT_PAUSE;
}