/*
===============================================================================

Name    :   g_menu.h

Purpose :   handles menu functions

Date    :   10/20/2004

===============================================================================
*/

#pragma once

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

#define CHAR_WIDTH  2.5
#define over(a,b,c) ( (clamp(a.x,b.x-c.x/2,b.x+c.x/2) == a.x ) && (clamp(a.y,b.y-c.y/2,b.y+c.y/2) == a.y ) )

#define NUM_BUTTONS 4

#define MAX_BUTTONS     8
#define MAX_SUBMENUS    4

typedef void (*func_t)();

class cBaseButton
{
public:
    cBaseButton () {}
    cBaseButton (char *szTitle, vec2 vPos, vec2 vSize, func_t op_click);
    ~cBaseButton () {}

    virtual bool    Click (vec2 vCursorPos, bool bDown);
    virtual void    Draw (vec2 vCursorPos);
protected:
    bool    m_Over (vec2 vCursorPos);

    bool    m_bOver, m_bClicked;

    char    m_szTitle[64];
    vec2    m_vPos, m_vSize;
    func_t  m_op_click;
};

class cServerButton : public cBaseButton
{
public:
    cServerButton () {}
    cServerButton (vec2 vPos, vec2 vSize, char *szServer, float *flPing, func_t op_click);
    ~cServerButton () {}

    virtual bool    Click (vec2 vCursorPos, bool bDown);
    virtual void    Draw (vec2 vCursorPos);
    
protected:
    char    *m_szServer;
    float   *m_flPing;

    func_t  m_op_click;
};

class cHostButton : public cBaseButton
{
public:
    cHostButton () {}
    cHostButton (vec2 vPos, vec2 vSize, func_t op_click);
    ~cHostButton () {}

    virtual bool    Click (vec2 vCursorPos, bool bDown);
    virtual void    Draw (vec2 vCursorPos);

protected:
    func_t  m_op_click;
};

class cCondButton : public cBaseButton
{
public:
    cCondButton () {}
    cCondButton (char *szTitle, vec2 vPos, vec2 vSize, func_t op_click, bool *bCond);
    ~cCondButton () {}

    virtual bool    Click (vec2 vCursorPos, bool bDown);
    virtual void    Draw (vec2 vCursorPos);

protected:
    bool    *m_bCond;
};

class cTank;

class cTankButton : public cBaseButton
{
public:
    cTankButton () {}
    cTankButton (char *szTitle, vec2 vPos, vec2 vSize, cTank *pTank);
    ~cTankButton () {}

    virtual bool    Click (vec2 vCursorPos, bool bDown);
    virtual void    Draw (vec2 vCursorPos);

protected:
    cTank   *m_pTank;
    int     m_nTankColor;
};

class cClientButton : public cBaseButton
{
public:
    cClientButton () {}
    cClientButton (char *szTitle, vec2 vPos, vec2 vSize, vec4 *pColor);
    ~cClientButton () {}

    virtual bool    Click (vec2 vCursorPos, bool bDown);
    virtual void    Draw (vec2 vCursorPos);

protected:
    vec4    *m_pColor;
    int     m_nColor;

    bool    m_bTextOver;
    bool    m_bTextDown;

    vec2    m_vTextBoxPos;
    vec2    m_vTextBoxSize;
};

class cMenu;

class cMenuButton : public cBaseButton
{
public:
    cMenuButton () {}
    cMenuButton (char *szTitle, vec2 vPos, vec2 vSize, cMenu *pParent, cMenu *pMenu);
    ~cMenuButton () {}

    virtual bool    Click (vec2 vCursorPos, bool bDown);
    virtual void    Draw (vec2 vCursorPos);

    void    Deactivate () { m_bActive = false; }

protected:
    cMenu   *m_pParent, *m_pMenu;
    bool    m_bActive;
};

class cCheckButton : public cBaseButton
{
public:
    cCheckButton () {}
    cCheckButton (char *szTitle, vec2 vPos, bool *pValue);
    ~cCheckButton () {}

    virtual bool    Click (vec2 vCursorPos, bool bDown);
    virtual void    Draw (vec2 vCursorPos);

protected:
    char    m_szTitle[64];
    vec2    m_vPos;
    bool    *m_pValue;
};

/*
===========================================================

Name    :   cMenu

Purpose :   controller class for a menu set

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

    void    Init ();
    void    Shutdown ();

    void    AddButton (cBaseButton *pButton);

    virtual void    Draw (vec2 vCursorPos);
    virtual void    Click (vec2 vCursorPos, bool bDown);

    bool    ActivateMenu (cMenuButton *pButton, cMenu *pActiveMenu);

protected:
    cBaseButton *m_Buttons[MAX_BUTTONS];
    cMenu       *m_SubMenus[MAX_SUBMENUS];

    cMenuButton *m_ActiveButton;
    cMenu       *m_ActiveMenu;
};

class cOptionsMenu : public cMenu
{
public:
    cOptionsMenu () {}
    cOptionsMenu (char *szTitle, vec2 vPos, vec2 vSize);
    ~cOptionsMenu () {}

    virtual void    Draw (vec2 vCursorPos);
    virtual void    Click (vec2 vCursorPos, bool bDown);

private:
    char    m_szTitle[64];
    vec2    m_vPos, m_vSize;
};
