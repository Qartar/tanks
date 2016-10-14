/*
===============================================================================

Name    :   gl_main.h

Purpose :   OpenGL Window Abstraction Layer

Date    :   10/15/2004

===============================================================================
*/

#include "local.h"
#pragma hdrstop

#include "resource.h"

#define STATIC_SIZE

#ifndef STATIC_SIZE
cvar_t  *vid_width;
cvar_t  *vid_height;
#endif //!STATIC_SIZE

/*
===========================================================

Name    :   cOpenGLWnd::cOpenGLWnd

Purpose :   Initialized member data and creates a default window

===========================================================
*/

cOpenGLWnd::cOpenGLWnd ()
{
    m_fbo       = 0;
    m_rbo[0]    = 0;
    m_rbo[1]    = 0;
}

/*
===========================================================

Name    :   cOpenGLWnd::Init

Purpose :   Initialized member data and creates a default window

===========================================================
*/

int cOpenGLWnd::Init (HINSTANCE hInstance, WNDPROC WndProc)
{
    int     res;

    char    *command;
    bool    bFullscreen = DEFAULT_FS;

#ifndef STATIC_SIZE
    vid_width   = pVariable->Get( "vid_width", "640", "int", CVAR_ARCHIVE, "width of display" );
    vid_height  = pVariable->Get( "vid_height", "480", "int", CVAR_ARCHIVE, "height of display" );
#endif //!STATIC_SIZE

    if (hInstance == NULL || WndProc == NULL)
    {
        g_Application->Error( "Tanks! Error", "cOpenGLWnd::Init | bad parameters\n" );

        return ERROR_FAIL;
    }

    m_hInstance = hInstance;
    m_WndProc = WndProc;

    if ( (command = strstr( g_Application->InitString(), "fullscreen" )) )
        bFullscreen = ( atoi(command+11) > 0 );

    res = m_CreateWindow(
#ifndef STATIC_SIZE
        vid_width->getInt( ),
        vid_height->getInt( ),
#else
        DEFAULT_W,
        DEFAULT_H,
#endif //!STATIC_SIZE
        DEFAULT_X,
        DEFAULT_Y,
        bFullscreen );

    m_Render.Init( );

    return res;
}

int cOpenGLWnd::EndFrame ()
{
    glBindFramebuffer( GL_READ_FRAMEBUFFER, m_fbo );
    glBindFramebuffer( GL_DRAW_FRAMEBUFFER, 0 );

    glDrawBuffer( GL_BACK );

    glBlitFramebuffer(
        0, 0, DEFAULT_W, DEFAULT_H,
        0, 0, m_WndParams.nSize[0], m_WndParams.nSize[1],
        GL_COLOR_BUFFER_BIT, GL_NEAREST
    );

    SwapBuffers( m_hDC );

    glBindFramebuffer( GL_DRAW_FRAMEBUFFER, m_fbo );

    glDrawBuffer( GL_COLOR_ATTACHMENT0 );

    return ERROR_NONE;
}

/*
===========================================================

Name    :   cOpenGLWnd::Shutdown

Purpose :   Shuts down window and opengl

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

Name    :   cOpenGLWnd::m_CreateWindow

Purpose :   Creates a window ; calls m_InitGL

===========================================================
*/
int cOpenGLWnd::m_CreateWindow (int nSizeX, int nSizeY, int nPosX, int nPosY, bool bFullscreen)
{
    WNDCLASS    wc;
    RECT        rect;

    int         style;
    int         width, height;

    style = WS_OVERLAPPED|WS_SYSMENU|WS_BORDER|WS_CAPTION|WS_MINIMIZEBOX;

    rect.top = 0;
    rect.left = 0;
    rect.right = nSizeX;
    rect.bottom = nSizeY;

    AdjustWindowRect( &rect, style, FALSE );

    width = rect.right - rect.left;
    height = rect.bottom - rect.top;

    // Setup struct for RegisterClass

    wc.style            = 0;
    wc.lpfnWndProc      = m_WndProc;
    wc.cbClsExtra       = 0;
    wc.cbWndExtra       = 0;
    wc.hInstance        = m_hInstance;
    wc.hIcon            = LoadIcon( m_hInstance, MAKEINTRESOURCE(IDI_ICON1) );
    wc.hCursor          = LoadCursor (NULL,IDC_ARROW);
    wc.hbrBackground    = 0;
    wc.lpszMenuName     = 0;
    wc.lpszClassName    = APP_CLASSNAME;

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

    if ( bFullscreen )
    {
        MONITORINFO info = { sizeof(info) };
        HMONITOR hMonitor = MonitorFromWindow( m_hWnd, MONITOR_DEFAULTTOPRIMARY );

        GetMonitorInfo( hMonitor, &info );

        m_WndParams.nPos[0] = info.rcMonitor.left;
        m_WndParams.nPos[1] = info.rcMonitor.top;
        m_WndParams.nSize[0] = info.rcMonitor.right - info.rcMonitor.left;
        m_WndParams.nSize[1] = info.rcMonitor.bottom - info.rcMonitor.top;

        SetWindowLongPtr( m_hWnd, GWL_STYLE, WS_OVERLAPPED );

        MoveWindow(
            m_hWnd,
            m_WndParams.nPos[0],
            m_WndParams.nPos[1], 
            m_WndParams.nSize[0],
            m_WndParams.nSize[1],
            FALSE
        );

        m_WndParams.bActive = true;
        m_WndParams.bMinimized = false;
        m_WndParams.bFullscreen = true;
        m_bFullscreen = true;
    }
    else
    {
        m_WndParams.nSize[0] = nSizeX ; m_WndParams.nSize[1] = nSizeY ;
        m_WndParams.nPos[0] = nPosX - rect.left ; m_WndParams.nPos[1] = nPosY - rect.top ;

        m_WndParams.bActive = true;
        m_WndParams.bMinimized = false;
        m_WndParams.bFullscreen = false;
        m_bFullscreen = false;
    }

    if (m_hWnd == NULL)
    {
        g_Application->Error( "Tanks! Error", "cOpenGLWnd::m_CreateWindow | CreateWindow failed\n" );

        return ERROR_FAIL;
    }

    ShowWindow( m_hWnd, SW_SHOW );
    UpdateWindow( m_hWnd );

    // initialize OpenGL
    if ( m_InitGL() != ERROR_NONE )
    {
        return ERROR_FAIL;
    }

    // initialize framebuffer
    if ( m_CreateFramebuffer( nSizeX, nSizeY ) != ERROR_NONE )
    {
        return ERROR_FAIL;
    }

    // show the window
    SetForegroundWindow( m_hWnd );
    SetFocus( m_hWnd );

    return ERROR_NONE;
}

/*
===========================================================

Name    :   cOpenGLWnd::m_InitGL

Purpose :   Initializes OpenGL

===========================================================
*/

int cOpenGLWnd::m_InitGL ()
{
    int     pixelformat;
    PIXELFORMATDESCRIPTOR pfd = 
    {
        sizeof(PIXELFORMATDESCRIPTOR),  // size of this pfd
        1,                              // version number
        PFD_DRAW_TO_WINDOW |            // support window
        PFD_SUPPORT_OPENGL |            // support OpenGL
        PFD_DOUBLEBUFFER,               // double buffered
        PFD_TYPE_RGBA,                  // RGBA type
        24,                             // 24-bit color depth
        0, 0, 0, 0, 0, 0,               // color bits ignored
        0,                              // no alpha buffer
        0,                              // shift bit ignored
        0,                              // no accumulation buffer
        0, 0, 0, 0,                     // accum bits ignored
        24,                             // 24-bit z-buffer  
        8,                              // 8-bit stencil buffer
        0,                              // no auxiliary buffer
        PFD_MAIN_PLANE,                 // main layer
        0,                              // reserved
        0, 0, 0                         // layer masks ignored
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

    glBindRenderbuffer = (PFNGLBINDRENDERBUFFER )wglGetProcAddress("glBindRenderbuffer");
    glDeleteRenderbuffers = (PFNGLDELETERENDERBUFFERS )wglGetProcAddress("glDeleteRenderbuffers");
    glGenRenderbuffers = (PFNGLGENRENDERBUFFERS )wglGetProcAddress("glGenRenderbuffers");
    glRenderbufferStorage = (PFNGLRENDERBUFFERSTORAGE )wglGetProcAddress("glRenderbufferStorage");
    glBindFramebuffer = (PFNGLBINDFRAMEBUFFER )wglGetProcAddress("glBindFramebuffer");
    glDeleteFramebuffers = (PFNGLDELETEFRAMEBUFFERS )wglGetProcAddress("glDeleteFramebuffers");
    glGenFramebuffers = (PFNGLGENFRAMEBUFFERS )wglGetProcAddress("glGenFramebuffers");
    glFramebufferRenderbuffer = (PFNGLFRAMEBUFFERRENDERBUFFER )wglGetProcAddress("glFramebufferRenderbuffer");
    glBlitFramebuffer = (PFNGLBLITFRAMEBUFFER )wglGetProcAddress("glBlitFramebuffer");

    return ERROR_NONE;
}

/*
===========================================================

Name    :   cOpenGLWnd::m_CreateFramebuffer

Purpose :   

===========================================================
*/

int cOpenGLWnd::m_CreateFramebuffer (int, int)
{
    if (m_fbo)
    {
        m_DestroyFramebuffer( );
    }

    glGenFramebuffers( 1, &m_fbo );
    glGenRenderbuffers( 2, m_rbo );

    glBindFramebuffer( GL_FRAMEBUFFER, m_fbo );

    glBindRenderbuffer( GL_RENDERBUFFER, m_rbo[0] );
    glRenderbufferStorage( GL_RENDERBUFFER, GL_RGBA, DEFAULT_W, DEFAULT_H );
    glFramebufferRenderbuffer( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, m_rbo[0] );

    glBindRenderbuffer( GL_RENDERBUFFER, m_rbo[1] );
    glRenderbufferStorage( GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, DEFAULT_W, DEFAULT_H );
    glFramebufferRenderbuffer( GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_rbo[1] );

    return ERROR_NONE;
}

/*
===========================================================

Name    :   cOpenGLWnd::m_DestroyFramebuffer

Purpose :   

===========================================================
*/

int cOpenGLWnd::m_DestroyFramebuffer ()
{
    glDeleteRenderbuffers( 2, m_rbo );
    glDeleteFramebuffers( 1, &m_fbo );

    return ERROR_NONE;
}

/*
===========================================================

Name    :   cOpenGLWnd::m_DestroyWindow

Purpose :   Destroys the window and calls m_ShutdownGL

===========================================================
*/

int cOpenGLWnd::m_DestroyWindow ()
{
    if (m_hWnd)
    {
        m_DestroyFramebuffer( );

        m_ShutdownGL( );

        DestroyWindow( m_hWnd );
        m_hWnd = NULL;
    }

    UnregisterClass( APP_CLASSNAME, m_hInstance );

    return ERROR_NONE;
}

/*
===========================================================

Name    :   cOpenGLWnd::m_ShutdownGL

Purpose :   Shuts down OpenGL

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

Name    :   cOpenGLWnd::Message

Purpose :   Message routing from Windows

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

Name    :   cOpenGLWnd::m_Activate

Purpose :   handles WM_ACTIVATE messages

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
