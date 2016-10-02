/*
===============================================================================

Name	:	gl_main.h

Purpose	:	OpenGL Window Abstraction Layer

Date	:	10/15/2004

===============================================================================
*/

#ifndef __GL_MAIN_H__
#define __GL_MAIN_H__

#include <gl/gl.h>
#include <gl/glu.h>

// default size and position
#define DEFAULT_X	100
#define DEFAULT_Y	100
#define DEFAULT_W	640
#define DEFAULT_H	480
#define DEFAULT_FS	0		// fullscreen
#define DEFAULT_MS	1		// multisampling

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
	bool	bFullscreen;
};

/*
===========================================================

Name	:	cOpenGLWnd

Purpose	:	Windows Window using OpenGL

===========================================================
*/

class cRender;

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

	int	Refresh ();

	int	Recreate (int nSizeX, int nSizeY, bool bFullscreen);

private:
	int	m_CreateWindow (int nSizeX, int nSizeY, int nPosX, int nPosY, bool bFullscreen);
	int m_DestroyWindow ();

	int m_InitGL ();
	int m_ShutdownGL ();

	int	m_Activate (bool bActive, bool bMinimized);

	bool		m_bFullscreen;
	bool		m_bRefreshing;

	sWndParam	m_WndParams;
	cRender		m_Render;

	HINSTANCE	m_hInstance;
	WNDPROC		m_WndProc;

	HWND	m_hWnd;

	// opengl/gdi layer crap
	HDC		m_hDC;
	HGLRC	m_hRC;
};

#endif //__GL_MAIN_H__