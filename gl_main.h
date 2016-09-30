/*
===============================================================================

Name	:	gl_main.h

Purpose	:	OpenGL Window Abstraction Layer

Date	:	10/15/2004

===============================================================================
*/

#include <gl/gl.h>
#include <gl/glu.h>
#include "r_main.h"

// default size and position
#define DEFAULT_X	100
#define DEFAULT_Y	100
#define DEFAULT_W	640
#define DEFAULT_H	480

/*
=============================

Name	:	sWndParam

Purpose	:	stores window parameters

=============================
*/

struct sWndParam
{
	int		nSize[2];
	int		nPos[2];

	bool	bActive;
	bool	bMinimized;
};

/*
===========================================================

Name	:	cOpenGLWnd

Purpose	:	Windows Window using OpenGL

===========================================================
*/

class cOpenGLWnd
{
public:
	cOpenGLWnd () {}
	~cOpenGLWnd () {}

	int	Init (HINSTANCE hInstance, WNDPROC WndProc);
	int	Shutdown ();

	LRESULT	Message (UINT uCmd, WPARAM wParam, LPARAM lParam);

	HWND	get_hWnd () { return m_hWnd; }
	HDC		get_hDC () { return m_hDC; }
	sWndParam	&get_WndParams () { return m_WndParams; }
	cRender		*get_Render () { return &m_Render; }

private:
	int	m_CreateWindow (int nSizeX, int nSizeY, int nPosX, int nPosY, bool bFullscreen);
	int m_DestroyWindow ();

	int m_InitGL ();
	int m_ShutdownGL ();

	int	m_Activate (bool bActive, bool bMinimized);

	sWndParam	m_WndParams;
	cRender		m_Render;

	HINSTANCE	m_hInstance;
	WNDPROC		m_WndProc;

	HWND	m_hWnd;

	// opengl/gdi layer crap
	HDC		m_hDC;
	HGLRC	m_hRC;
};
