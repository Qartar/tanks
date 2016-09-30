/*
===============================================================================

Name	:	scr_main.h

Purpose	:	script parsing object

Date	:	12/10/2004

===============================================================================
*/

#include <string>
#include <vector>
#include <stack>

using namespace std;

typedef string *pstring;
typedef int (func)(void *iter);

extern void parse (const char *in, const char *fmt, ...);
extern char *wsex (char *arg);

#define SCRIPT_EXIT		-2
#define SCRIPT_ERROR	-1
#define SCRIPT_PAUSE	0
#define SCRIPT_CONTINUE	1

struct sScriptCmd
{
	func	*pfFunc;
	string	strCmd;
};

enum eType
{
	type_int,
	type_float,
	type_object
};

typedef class magic_s
{
public:
	magic_s () {}
	magic_s (double fl) {type = type_float ; fvalue = fl ; }
	magic_s (float fl) {type = type_float ; fvalue = fl ; }
	magic_s (int val) {type = type_int ; value = val ; }

	union
	{
		int		value;
		float	fvalue;
	};
	eType	type;

	magic_s operator + (magic_s &o)
	{
		if ((type == type_float) && (o.type == type_int))
			return ( magic_s( (float)(fvalue + (float)o.value) ) );
		if ((type == type_int) && (o.type == type_float))
			return ( magic_s( (float)(o.fvalue + (float)value) ) );
		if ((type == type_int) && (o.type == type_int))
			return ( magic_s( value + o.value ) );
		if ((type == type_float) && (o.type == type_float))
			return ( magic_s( fvalue + o.fvalue ) );
	}
	
	magic_s operator - (magic_s &o)
	{
		if ((type == type_float) && (o.type == type_int))
			return ( magic_s( (float)(fvalue - (float)o.value) ) );
		if ((type == type_int) && (o.type == type_float))
			return ( magic_s( (float)(o.fvalue - (float)value) ) );
		if ((type == type_int) && (o.type == type_int))
			return ( magic_s( value - o.value ) );
		if ((type == type_float) && (o.type == type_float))
			return ( magic_s( fvalue - o.fvalue ) );
	}

	magic_s operator * (magic_s &o)
	{
		if ((type == type_float) && (o.type == type_int))
			return ( magic_s( (float)(fvalue * (float)o.value) ) );
		if ((type == type_int) && (o.type == type_float))
			return ( magic_s( (float)(o.fvalue * (float)value) ) );
		if ((type == type_int) && (o.type == type_int))
			return ( magic_s( value * o.value ) );
		if ((type == type_float) && (o.type == type_float))
			return ( magic_s( fvalue * o.fvalue ) );
	}

	magic_s operator / (magic_s &o)
	{
		if ((type == type_float) && (o.type == type_int))
			return ( magic_s( (float)(fvalue / (float)o.value) ) );
		if ((type == type_int) && (o.type == type_float))
			return ( magic_s( (float)(o.fvalue / (float)value) ) );
		if ((type == type_int) && (o.type == type_int))
			return ( magic_s( value / o.value ) );
		if ((type == type_float) && (o.type == type_float))
			return ( magic_s( fvalue / o.fvalue ) );
	}

} magic_t;

struct sScriptVar
{
	string	strVar;
	magic_t	value;
};

class cScript
{
public:
	cScript () {}

	int	Load	(string filename);
	int	Unload	();

	class iterator
	{
	public:
		iterator ();

		void begin (cScript *pScript);
		int	operator++ (int);
		void run ();

		sScriptVar *getval (string str);
		const char	*getstr () { return m_strCursor->c_str(); }

		int	addvar (magic_t magic, string name, eType type);
		int addcmd (func *pfunc, string name);
		int	addlocal (void *value);

	private:
		string			m_fileName;

		string			*m_strBegin;
		string			*m_strEnd;
		stack<pstring>	m_vFrames;
		string			*m_strCursor;

		vector<sScriptCmd>	m_Cmds;
		vector<sScriptVar>	m_Vars;
		vector<int>			m_Locals;
	};

private:
	vector<string>	m_vLines;
	string			m_fileName;
};