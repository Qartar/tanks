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

#define MAX_BUTTONS		8
#define MAX_SUBMENUS	4

typedef void (*func_t)();

class cBaseButton
{
public:
	cBaseButton () {}
	cBaseButton (char *szTitle, vec2 vPos, vec2 vSize, func_t op_click);
	~cBaseButton () {}

	virtual bool	Click (vec2 vCursorPos, bool bDown);
	virtual void	Draw (vec2 vCursorPos);
protected:
	bool	m_Over (vec2 vCursorPos);

	bool	m_bOver, m_bClicked;

	char	m_szTitle[64];
	vec2	m_vPos, m_vSize;
	func_t	m_op_click;
};

class cCondButton : public cBaseButton
{
public:
	cCondButton () {}
	cCondButton (char *szTitle, vec2 vPos, vec2 vSize, func_t op_click, bool *bCond);
	~cCondButton () {}

	virtual bool	Click (vec2 vCursorPos, bool bDown);
	virtual void	Draw (vec2 vCursorPos);

protected:
	bool	*m_bCond;
};

class cTank;

class cInputObject;

class cTankButton : public cBaseButton, public cInputObject
{
public:
	cTankButton () {}
	cTankButton (char *szTitle, vec2 vPos, vec2 vSize, cTank *pTank);
	~cTankButton () {}

	virtual bool	Click (vec2 vCursorPos, bool bDown);
	virtual void	Draw (vec2 vCursorPos);

	virtual void	Key_Event (int nKey, bool bDown);

protected:
	cTank	*m_pTank;
	int		m_nTankColor;

	vec2	m_vBoxSize;
	vec2	m_vBoxPos;

	vec2	m_vTextSize;
	vec2	m_vTextPos;

//	bool	m_bComputer;
//	char	m_szScript[64];
	int		m_scrCursor;
	bool	m_bWriting;
};

class cRangeButton : public cBaseButton
{
public:
	cRangeButton () {}
	cRangeButton (char *szTitle, vec2 vPos, vec2 vSize, int nMax, int *nVal);
	~cRangeButton () {}

	virtual bool	Click (vec2 vCursorPos, bool bDown);
	virtual void	Draw (vec2 vCursorPos);

protected:
	int		m_nMax;
	int		*m_nVal;
};

class cMenu;

class cMenuButton : public cBaseButton
{
public:
	cMenuButton () {}
	cMenuButton (char *szTitle, vec2 vPos, vec2 vSize, cMenu *pParent, cMenu *pMenu);
	~cMenuButton () {}

	virtual bool	Click (vec2 vCursorPos, bool bDown);
	virtual void	Draw (vec2 vCursorPos);

	void	Deactivate () { m_bActive = false; }

protected:
	cMenu	*m_pParent, *m_pMenu;
	bool	m_bActive;
};

/*
===========================================================

Name	:	cMenu

Purpose	:	controller class for a menu set

===========================================================
*/

class cMenu
{
public:
	cMenu () { 
		memset(m_Buttons, 0, sizeof(m_Buttons));
		memset(m_SubMenus,0,sizeof(m_SubMenus));
		m_ActiveMenu = NULL;
		m_ActiveButton = NULL; }
	~cMenu () {}

	void	Init ();
	void	Shutdown ();

	void	AddButton (cBaseButton *pButton);

	void	Draw (vec2 vCursorPos);
	void	Click (vec2 vCursorPos, bool bDown);

	bool	ActivateMenu (cMenuButton *pButton, cMenu *pActiveMenu);

private:
	cBaseButton	*m_Buttons[MAX_BUTTONS];
	cMenu		*m_SubMenus[MAX_SUBMENUS];

	cMenuButton	*m_ActiveButton;
	cMenu		*m_ActiveMenu;
};
