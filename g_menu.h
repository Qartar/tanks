/*
===============================================================================

Name	:	g_menu.h

Purpose	:	handles menu functions

Date	:	10/20/2004

===============================================================================
*/

static vec4 menu_colors[] = {
	vec4(0.000,0.000,0.000,1),
	vec4(0.125,0.125,0.125,1),
	vec4(0.250,0.250,0.250,1),
	vec4(0.375,0.375,0.375,1),
	vec4(0.500,0.500,0.500,1),
	vec4(0.625,0.625,0.625,1),
	vec4(0.750,0.750,0.750,1),
	vec4(0.875,0.875,0.875,1),
	vec4(1.000,1.000,1.000,1) };

#define NUM_BUTTONS	4

typedef void (*func_t)();

class cMenuButton
{
public:
	cMenuButton () {}
	~cMenuButton () {}

	void	Init (char *szTitle, vec2 vPos, vec2 vSize, func_t op_click, bool *bCond);

	bool	Click (vec2 vCursorPos, bool bDown);
	void	Draw (vec2 vCursorPos);

private:
	bool	m_Over (vec2 vCursorPos);

	bool	m_bOver, m_bClicked, *m_bCond;

	char	m_szTitle[64];
	vec2	m_vPos, m_vSize;
	func_t	m_op_click;
};

class cMenu
{
public:
	cMenu () {}
	~cMenu () {}

	void	Init ();
	void	Shutdown ();

	void	Draw (vec2 vCursorPos);
	void	Click (vec2 vCursorPos, bool bDown);

private:
	cMenuButton	m_Buttons[NUM_BUTTONS];
};