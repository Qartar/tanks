/*
===============================================================================

Name	:	gl_main.h

Purpose	:	OpenGL Window Abstraction Layer

Date	:	10/15/2004

===============================================================================
*/

#include "local.h"
#include "resource.h"

/*
===========================================================

Name	:	cOpenGLWnd::Init

Purpose	:	Initialized member data and creates a default window

===========================================================
*/

int cOpenGLWnd::Init (HINSTANCE hInstance, WNDPROC WndProc)
{
	int		res;

	char	*command;
	bool	bFullscreen = DEFAULT_FS;

	if (hInstance == NULL || WndProc == NULL)
	{
		g_Application->Error( "Tanks! Error", "cOpenGLWnd::Init | bad parameters\n" );

		return ERROR_FAIL;
	}

	m_hInstance = hInstance;
	m_WndProc = WndProc;

	if ( (command = strstr( g_Application->InitString(), "fullscreen=" )) )
		bFullscreen = ( atoi(command+11) > 0 );

	res = m_CreateWindow( DEFAULT_W, DEFAULT_H, DEFAULT_X, DEFAULT_Y, bFullscreen );

	m_Render.Init( );

	return res;
}

int cOpenGLWnd::Recreate (int nSizeX, int nSizeY, bool bFullscreen)
{
	m_bRefreshing = true;

	Shutdown( );

	m_CreateWindow( nSizeX, nSizeY, DEFAULT_X, DEFAULT_Y, bFullscreen );

	return m_Render.Init( );
}

int cOpenGLWnd::Refresh ()
{
	if ( m_bFullscreen != m_WndParams.bFullscreen )
		return Recreate( m_WndParams.nSize[0], m_WndParams.nSize[1], m_bFullscreen );

	return ERROR_NONE;
}

/*
===========================================================

Name	:	cOpenGLWnd::Shutdown

Purpose	:	Shuts down window and opengl

===========================================================
*/

int cOpenGLWnd::Shutdown ()
{
	if (!m_hWnd)
	{
		// already shut down
		return ERROR_NONE;
	}

	m_Render.Shutdown( );

	return m_DestroyWindow( );
}

/*
===========================================================

Name	:	cOpenGLWnd::m_CreateWindow

Purpose	:	Creates a window ; calls m_InitGL

===========================================================
*/
int cOpenGLWnd::m_CreateWindow (int nSizeX, int nSizeY, int nPosX, int nPosY, bool bFullscreen)
{
	WNDCLASS	wc;
	RECT		rect;

	int			style;
	int			width, height;

	if ( bFullscreen )
	{
		DEVMODE	dm;

		memset( &dm, 0, sizeof(DEVMODE) );

		dm.dmSize = sizeof(DEVMODE);
		dm.dmPelsWidth = nSizeX;
		dm.dmPelsHeight = nSizeY;
		dm.dmFields = DM_PELSWIDTH|DM_PELSHEIGHT;

		rect.top = 0;
		rect.left = 0;
		rect.right = 0;
		rect.bottom = 0;

		if ( ChangeDisplaySettings(&dm,CDS_FULLSCREEN) != DISP_CHANGE_SUCCESSFUL )
		{
			bFullscreen = false;
			style = WS_OVERLAPPED;
		}
		else
		{
			style = WS_POPUP|WS_VISIBLE;

			nPosX = 0;
			nPosY = 0;
			width = nSizeX;
			height = nSizeY;
		}
	}
	else if ( m_WndParams.bFullscreen )
		ChangeDisplaySettings(NULL,NULL);

	if ( !bFullscreen )
	{
		style = WS_OVERLAPPED|WS_SYSMENU|WS_BORDER|WS_CAPTION|WS_MINIMIZEBOX|WS_VISIBLE;

		rect.top = 0;
		rect.left = 0;
		rect.right = nSizeX;
		rect.bottom = nSizeY;

		AdjustWindowRect( &rect, style, FALSE );

		width = rect.right - rect.left;
		height = rect.bottom - rect.top;
	}
	else
		nPosX = nPosY = 0;

	// Setup struct for RegisterClass

	wc.style			= 0;
	wc.lpfnWndProc		= m_WndProc;
	wc.cbClsExtra		= 0;
	wc.cbWndExtra		= 0;
	wc.hInstance		= m_hInstance;
	wc.hIcon			= LoadIcon( m_hInstance, MAKEINTRESOURCE(102) );
	wc.hCursor			= LoadCursor (NULL,IDC_ARROW);
	wc.hbrBackground	= 0;
    wc.lpszMenuName		= 0;
    wc.lpszClassName	= APP_CLASSNAME;

	if ( !RegisterClass( &wc ) )
	{
		g_Application->Error( "Tanks! Error", "cOpenGLWnd::m_CreateWindow | RegisterClass failed\n" );

		return ERROR_FAIL;
	}

	// Create the Window

	m_hWnd = CreateWindow(
		APP_CLASSNAME,
		"Tanks!",
		style,
		nPosX, nPosY, width, height,
		NULL, NULL,
		m_hInstance,
		NULL );

	m_WndParams.nSize[0] = nSizeX ; m_WndParams.nSize[1] = nSizeY ;
	m_WndParams.nPos[0] = nPosX - rect.left ; m_WndParams.nPos[1] = nPosY - rect.top ;

	m_WndParams.bActive = true;
	m_WndParams.bMinimized = false;
	m_WndParams.bFullscreen = bFullscreen;
	m_bFullscreen = bFullscreen;
	
	if (m_hWnd == NULL)
	{
		g_Application->Error( "Tanks! Error", "cOpenGLWnd::m_CreateWindow | CreateWindow failed\n" );

		return ERROR_FAIL;
	}

	ShowWindow( m_hWnd, SW_SHOW );
	UpdateWindow( m_hWnd );

	// initialize OpenGL

	return m_InitGL ();
}

/*
===========================================================

Name	:	cOpenGLWnd::m_InitGL

Purpose	:	Initializes OpenGL

===========================================================
*/

int cOpenGLWnd::m_InitGL ()
{
	int		pixelformat;
	PIXELFORMATDESCRIPTOR pfd = 
	{
		sizeof(PIXELFORMATDESCRIPTOR),	// size of this pfd
		1,								// version number
		PFD_DRAW_TO_WINDOW |			// support window
		PFD_SUPPORT_OPENGL |			// support OpenGL
		PFD_DOUBLEBUFFER,				// double buffered
		PFD_TYPE_RGBA,					// RGBA type
		24,								// 24-bit color depth
		0, 0, 0, 0, 0, 0,				// color bits ignored
		0,								// no alpha buffer
		0,								// shift bit ignored
		0,								// no accumulation buffer
		0, 0, 0, 0, 					// accum bits ignored
		24,								// 24-bit z-buffer	
		8,								// 8-bit stencil buffer
		0,								// no auxiliary buffer
		PFD_MAIN_PLANE,					// main layer
		0,								// reserved
		0, 0, 0							// layer masks ignored
    };

	// get DC
	if ( (m_hDC = GetDC( m_hWnd ) ) == NULL)
	{
		g_Application->Error( "Tanks! Error", "cOpenGLWnd::m_InitGL | GetDC failed");
		return ERROR_FAIL;
	}

	// select pixel format
	if ( (pixelformat = ChoosePixelFormat( m_hDC, &pfd ) ) == 0)
	{
		g_Application->Error( "Tanks! Error", "cOpenGLWnd::m_InitGL | ChoosePixelFormat failed");
		return ERROR_FAIL;
	}
	if ( (SetPixelFormat( m_hDC, pixelformat, &pfd ) ) == FALSE)
	{
		g_Application->Error( "Tanks! Error", "cOpenGLWnd::m_InitGL | SetPixelFormat failed");
		return ERROR_FAIL;
	}

	// set up context
	if ( (m_hRC = wglCreateContext( m_hDC ) ) == 0 )
	{
		g_Application->Error ( "Tanks! Error", "cOpenGLWnd::m_InitGL: wglCreateContext failed" );

		m_ShutdownGL( );

		return ERROR_FAIL;
	}
	if ( !wglMakeCurrent( m_hDC, m_hRC ) )
	{
		g_Application->Error ( "Tanks! Error", "cOpenGLWnd::m_InitGL: wglMakeCurrent failed" );

		m_ShutdownGL( );

		return ERROR_FAIL;
	}

	// initialize Renderer

	// show the window

	SetForegroundWindow( m_hWnd );
	SetFocus( m_hWnd );

	return ERROR_NONE;
}

/*
===========================================================

Name	:	cOpenGLWnd::m_DestroyWindow

Purpose	:	Destroys the window and calls m_ShutdownGL

===========================================================
*/

int cOpenGLWnd::m_DestroyWindow ()
{
	if (m_hWnd)
	{
		m_ShutdownGL( );

		DestroyWindow( m_hWnd );
		m_hWnd = NULL;
	}

	UnregisterClass( APP_CLASSNAME, m_hInstance );

	return ERROR_NONE;
}

/*
===========================================================

Name	:	cOpenGLWnd::m_ShutdownGL

Purpose	:	Shuts down OpenGL

===========================================================
*/

int cOpenGLWnd::m_ShutdownGL ()
{
	// shutdown Renderer
	wglMakeCurrent( NULL, NULL );

	if (m_hRC)
	{
		wglDeleteContext( m_hRC );
		m_hRC = NULL;
	}
	if (m_hDC)
	{
		ReleaseDC( m_hWnd, m_hDC );
		m_hDC = NULL;
	}

	return ERROR_NONE;
}

/*
===========================================================

Name	:	cOpenGLWnd::Message

Purpose	:	Message routing from Windows

===========================================================
*/

LRESULT cOpenGLWnd::Message (UINT nCmd, WPARAM wParam, LPARAM lParam)
{
	switch ( nCmd )
	{
	case WM_ACTIVATE:
		m_Activate( (LOWORD(wParam) != WA_INACTIVE), ( HIWORD(wParam) > 0 ) );
		return DefWindowProc( m_hWnd, nCmd, wParam, lParam );

	case WM_SIZE:
		m_WndParams.nSize[0] = LOWORD(lParam);
		m_WndParams.nSize[1] = HIWORD(lParam);
		m_Render.Resize( ); // uses params in WndParams
		return DefWindowProc( m_hWnd, nCmd, wParam, lParam );

	case WM_MOVE:
		m_WndParams.nPos[0] = LOWORD(lParam);    // horizontal position 
		m_WndParams.nPos[1] = HIWORD(lParam);    // vertical position 
        return DefWindowProc( m_hWnd, nCmd, wParam, lParam );

	case WM_SYSKEYDOWN:
		if ( wParam == 13 )
			m_bFullscreen = !m_WndParams.bFullscreen;
		return 0;

	case WM_DESTROY:
		if ( m_bRefreshing )
			m_bRefreshing = false;
		else
			PostQuitMessage(0);
		return 0;

	default:
		return DefWindowProc( m_hWnd, nCmd, wParam, lParam );
	}
}

/*
===========================================================

Name	:	cOpenGLWnd::m_Activate

Purpose	:	handles WM_ACTIVATE messages

===========================================================
*/

int cOpenGLWnd::m_Activate (bool bActive, bool bMinimized)
{
	if (bActive && !bMinimized)
	{
		if (!m_WndParams.bActive || m_WndParams.bMinimized)
		{
			SetForegroundWindow( m_hWnd );
			ShowWindow( m_hWnd, SW_RESTORE );
			m_WndParams.bActive = true;
			m_WndParams.bMinimized = false;
		}
	}
	else
	{
		if ( bMinimized )
		{
			m_WndParams.bMinimized = true;
			ShowWindow( m_hWnd, SW_MINIMIZE );
		}

		m_WndParams.bActive = false;
	}

	return ERROR_NONE;
}