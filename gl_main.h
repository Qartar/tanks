/*
===============================================================================

Name    :   gl_main.h

Purpose :   OpenGL Window Abstraction Layer

Date    :   10/15/2004

===============================================================================
*/

#pragma once

#include <gl/gl.h>
#include <gl/glu.h>

// additional opengl bindings
typedef void (APIENTRY* PFNGLBINDRENDERBUFFER)(GLenum target, GLuint renderbuffer);
typedef void (APIENTRY* PFNGLDELETERENDERBUFFERS)(GLsizei n, const GLuint* renderbuffers);
typedef void (APIENTRY* PFNGLGENRENDERBUFFERS)(GLsizei n, GLuint* renderbuffers);
typedef void (APIENTRY* PFNGLRENDERBUFFERSTORAGE)(GLenum target, GLenum internalformat, GLsizei width, GLsizei height);
typedef void (APIENTRY* PFNGLBINDFRAMEBUFFER)(GLenum target, GLuint framebuffer);
typedef void (APIENTRY* PFNGLDELETEFRAMEBUFFERS)(GLsizei n, const GLuint* framebuffers);
typedef void (APIENTRY* PFNGLGENFRAMEBUFFERS)(GLsizei n, GLuint* framebuffers);
typedef void (APIENTRY* PFNGLFRAMEBUFFERRENDERBUFFER)(GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer);
typedef void (APIENTRY* PFNGLBLITFRAMEBUFFER)(GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1, GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1, GLbitfield mask, GLenum filter);

static PFNGLBINDRENDERBUFFER glBindRenderbuffer = NULL;
static PFNGLDELETERENDERBUFFERS glDeleteRenderbuffers = NULL;
static PFNGLGENRENDERBUFFERS glGenRenderbuffers = NULL;
static PFNGLRENDERBUFFERSTORAGE glRenderbufferStorage = NULL;
static PFNGLBINDFRAMEBUFFER glBindFramebuffer = NULL;
static PFNGLDELETEFRAMEBUFFERS glDeleteFramebuffers = NULL;
static PFNGLGENFRAMEBUFFERS glGenFramebuffers = NULL;
static PFNGLFRAMEBUFFERRENDERBUFFER glFramebufferRenderbuffer = NULL;
static PFNGLBLITFRAMEBUFFER glBlitFramebuffer = NULL;

#define GL_FRAMEBUFFER                  0x8D40
#define GL_READ_FRAMEBUFFER             0x8CA8
#define GL_DRAW_FRAMEBUFFER             0x8CA9
#define GL_RENDERBUFFER                 0x8D41
#define GL_COLOR_ATTACHMENT0            0x8CE0
#define GL_DEPTH_STENCIL_ATTACHMENT     0x821A
#define GL_DEPTH24_STENCIL8             0x88F0

// default size and position
#define DEFAULT_X   100
#define DEFAULT_Y   100
#define DEFAULT_W   640
#define DEFAULT_H   480
#define DEFAULT_FS  1       // fullscreen
#define DEFAULT_MS  1       // multisampling

/*
=============================

Name    :   sWndParam

Purpose :   stores window parameters

=============================
*/

struct sWndParam
{
    int     nSize[2];
    int     nPos[2];

    bool    bActive;
    bool    bMinimized;
    bool    bFullscreen;
};

/*
===========================================================

Name    :   cOpenGLWnd

Purpose :   Windows Window using OpenGL

===========================================================
*/

class cRender;

class cOpenGLWnd
{
public:
    cOpenGLWnd ();
    ~cOpenGLWnd () {}

    int Init (HINSTANCE hInstance, WNDPROC WndProc);
    int Shutdown ();

    LRESULT Message (UINT uCmd, WPARAM wParam, LPARAM lParam);

    HWND    get_hWnd () { return m_hWnd; }
    HDC     get_hDC () { return m_hDC; }
    sWndParam   &get_WndParams () { return m_WndParams; }
    cRender     *get_Render () { return &m_Render; }

    int EndFrame ();

private:
    int m_CreateWindow (int nSizeX, int nSizeY, int nPosX, int nPosY, bool bFullscreen);
    int m_CreateFramebuffer (int nSizeX, int nSizeY);
    int m_DestroyWindow ();
    int m_DestroyFramebuffer ();

    int m_InitGL ();
    int m_ShutdownGL ();

    int m_Activate (bool bActive, bool bMinimized);

    bool        m_bFullscreen;
    bool        m_bRefreshing;

    sWndParam   m_WndParams;
    cRender     m_Render;

    HINSTANCE   m_hInstance;
    WNDPROC     m_WndProc;

    HWND    m_hWnd;

    // opengl/gdi layer crap
    HDC     m_hDC;
    HGLRC   m_hRC;

    // framebuffer objects
    GLuint  m_fbo;
    GLuint  m_rbo[2];
};
