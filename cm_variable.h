// variable.h

#include "shared.h"

#ifndef __CM_VARIABLE_H__
#define __CM_VARIABLE_H__

#define CVAR_GET(a)			pVariable->Get(a)

#define	CVAR_LOCAL		0x00000001
#define CVAR_ARCHIVE	0x00000002
#define CVAR_NOSET		0x00000004
#define CVAR_FORCESET	0x00000008	// initialization only

typedef enum cVarType { cvar_undefined, cvar_string, cvar_float, cvar_int, cvar_bool, cvar_num_types } cvar_type_t;
static char *cvar_type_strings[] = { "undefined", "string", "float", "int", "bool" };

typedef class vVar
{
public:
	virtual void	setString (char *szString) = 0;
	virtual void	setFloat (float f) = 0;
	virtual void	setBool (bool b) = 0;
	virtual void	setInt (int i) = 0;

	virtual char	*getString () = 0;
	virtual float	getFloat () = 0;
	virtual bool	getBool () = 0;
	virtual int		getInt () = 0;

	virtual char		*getName () = 0;
	virtual char		*getDesc () = 0;
	virtual int			getFlags () = 0;
	virtual cvar_type_t	getType () = 0;
} cvar_t;

class vVariable
{
public:
	static void		Create ();
	static void		Destroy ();

	virtual int	Load () = 0;
	virtual int	Unload () = 0;

	virtual cvar_t *Get (char *szName) = 0;
	virtual cvar_t *Get	(char *szName, char *szValue) = 0;
	virtual cvar_t *Get	(char *szName, char *szValue, char *szType, int bitFlags, char *szDesc) = 0;

	virtual int	List () = 0;
};

extern vVariable	*pVariable;

#endif //__CM_VARIABLE_H__