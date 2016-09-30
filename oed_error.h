/*
===========================================================

Name	:	oed_error.h

Purpose	:	error checking class

Modified:	11/03/2006

===========================================================
*/

#ifndef __OED_ERROR__
#define __OED_ERROR__

#include "oed_shared.h"
#include <string.h> 	// strncpy

#define ERROR_NONE				0x00
#define ERROR_FAIL				0x01
#define ERROR_BADPTR			0x02
#define ERROR_BADINDEX			0x04
#define ERROR_DUPLICATE			0x08
#define ERROR_NOTFOUND			0x10

class errorobj_c
{
protected:
	errorobj_c () {}

	char	m_type[SHORT_STRING];
	char	m_message[MAX_STRING];
	int		m_code;
	bool	m_fatal;

public:
	errorobj_c (char *msg, bool fatal) { strcpy( m_message, msg ); strcpy( m_type, "generic" ); m_code = ERROR_FAIL; m_fatal = fatal; }
	errorobj_c (char *msg, int code, bool fatal) { strcpy( m_message, msg ); strcpy( m_type, "generic" ); m_code = code; m_fatal = fatal; }

	char	*message () { return m_message; }
	char	*type () { return m_type; }
	int		code () { return m_code; }
	bool	is_fatal () { return m_fatal; }
};

#endif // __OED_ERROR__