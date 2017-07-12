// r_window.cpp
//

#include "local.h"
#pragma hdrstop

#include "resource.h"

namespace {

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

} // anonymous namespace

////////////////////////////////////////////////////////////////////////////////
namespace render {

//------------------------------------------------------------------------------
window::window(HINSTANCE hInstance, WNDPROC WndProc)
    : _hinst(hInstance)
    , _wndproc(WndProc)
    , _renderer(this)
{
    _fbo       = 0;
    _rbo[0]    = 0;
    _rbo[1]    = 0;
}

//------------------------------------------------------------------------------
void window::create()
{
    char* command;
    bool fullscreen = DEFAULT_FS;

    if ( (command = strstr( g_Application->InitString(), "fullscreen" )) )
        fullscreen = ( atoi(command+11) > 0 );

    create(
        DEFAULT_W,
        DEFAULT_H,
        DEFAULT_X,
        DEFAULT_Y,
        fullscreen );

    _renderer.init();
}

void window::end_frame ()
{
    glBindFramebuffer(GL_READ_FRAMEBUFFER, _fbo);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

    glDrawBuffer(GL_BACK);

    glBlitFramebuffer(
        0, 0, DEFAULT_W, DEFAULT_H,
        0, 0, _size.x, _size.y,
        GL_COLOR_BUFFER_BIT, GL_NEAREST
    );

    SwapBuffers(_hdc);

    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, _fbo);

    glDrawBuffer(GL_COLOR_ATTACHMENT0);
}

//------------------------------------------------------------------------------
window::~window()
{
    if (_hwnd)
    {
        _renderer.shutdown();
        destroy();
    }
}

//------------------------------------------------------------------------------
int window::create(int width, int height, int xpos, int ypos, bool fullscreen)
{
    WNDCLASSA   wc;
    RECT        rect;

    int         style;

    style = WS_OVERLAPPED|WS_SYSMENU|WS_BORDER|WS_CAPTION|WS_MINIMIZEBOX;

    rect.top = 0;
    rect.left = 0;
    rect.right = width;
    rect.bottom = height;

    AdjustWindowRect( &rect, style, FALSE );

    width = rect.right - rect.left;
    height = rect.bottom - rect.top;

    // Setup struct for RegisterClass

    wc.style            = 0;
    wc.lpfnWndProc      = _wndproc;
    wc.cbClsExtra       = 0;
    wc.cbWndExtra       = 0;
    wc.hInstance        = _hinst;
    wc.hIcon            = LoadIconA(_hinst, MAKEINTRESOURCEA(IDI_ICON1));
    wc.hCursor          = LoadCursorA(NULL, IDC_ARROW);
    wc.hbrBackground    = 0;
    wc.lpszMenuName     = 0;
    wc.lpszClassName    = APP_CLASSNAME;

    if (!RegisterClassA(&wc)) {
        g_Application->Error( "Tanks! Error", "window::m_CreateWindow | RegisterClass failed\n" );

        return ERROR_FAIL;
    }

    // Create the Window

    _hwnd = CreateWindowA(
        APP_CLASSNAME,
        "Tanks!",
        style,
        xpos, ypos, width, height,
        NULL, NULL,
        _hinst,
        NULL );

    if (fullscreen) {
        MONITORINFO info = { sizeof(info) };
        HMONITOR hMonitor = MonitorFromWindow( _hwnd, MONITOR_DEFAULTTOPRIMARY );

        GetMonitorInfoA( hMonitor, &info );

        _position.x = info.rcMonitor.left;
        _position.y = info.rcMonitor.top;
        _size.x = info.rcMonitor.right - info.rcMonitor.left;
        _size.y = info.rcMonitor.bottom - info.rcMonitor.top;

        SetWindowLongPtrA( _hwnd, GWL_STYLE, WS_OVERLAPPED );

        MoveWindow(
            _hwnd,
            _position.x,
            _position.y, 
            _size.x,
            _size.y,
            FALSE
        );

        _active = true;
        _minimized = false;
        _fullscreen = true;
    } else {
        _size.x = width;
        _size.y = height;
        _position.x = xpos - rect.left;
        _position.y = ypos - rect.top;

        _active = true;
        _minimized = false;
        _fullscreen = false;
    }

    if (_hwnd == NULL) {
        g_Application->Error( "Tanks! Error", "window::m_CreateWindow | CreateWindow failed\n" );

        return ERROR_FAIL;
    }

    ShowWindow(_hwnd, SW_SHOW);
    UpdateWindow(_hwnd);

    // initialize OpenGL
    if (init_opengl() != ERROR_NONE) {
        return ERROR_FAIL;
    }

    // initialize framebuffer
    if (create_framebuffer(width, height) != ERROR_NONE) {
        return ERROR_FAIL;
    }

    // show the window
    SetForegroundWindow(_hwnd);
    SetFocus(_hwnd);

    return ERROR_NONE;
}

//------------------------------------------------------------------------------
int window::init_opengl()
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
    if ((_hdc = GetDC(_hwnd)) == NULL) {
        g_Application->Error( "Tanks! Error", "window::m_InitGL | GetDC failed");
        return ERROR_FAIL;
    }

    // select pixel format
    if ((pixelformat = ChoosePixelFormat(_hdc, &pfd)) == 0) {
        g_Application->Error( "Tanks! Error", "window::m_InitGL | ChoosePixelFormat failed");
        return ERROR_FAIL;
    }
    if ((SetPixelFormat(_hdc, pixelformat, &pfd)) == FALSE) {
        g_Application->Error( "Tanks! Error", "window::m_InitGL | SetPixelFormat failed");
        return ERROR_FAIL;
    }

    // set up context
    if ((_hrc = wglCreateContext(_hdc)) == 0) {
        g_Application->Error ( "Tanks! Error", "window::m_InitGL: wglCreateContext failed" );
        shutdown_opengl();
        return ERROR_FAIL;
    }
    if (!wglMakeCurrent(_hdc, _hrc)) {
        g_Application->Error ( "Tanks! Error", "window::m_InitGL: wglMakeCurrent failed" );
        shutdown_opengl();
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

//------------------------------------------------------------------------------
int window::create_framebuffer(int, int)
{
    if (_fbo) {
        destroy_framebuffer();
    }

    glGenFramebuffers(1, &_fbo);
    glGenRenderbuffers(2, _rbo);

    glBindFramebuffer(GL_FRAMEBUFFER, _fbo);

    glBindRenderbuffer(GL_RENDERBUFFER, _rbo[0]);
    glRenderbufferStorage( GL_RENDERBUFFER, GL_RGBA, DEFAULT_W, DEFAULT_H);
    glFramebufferRenderbuffer( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, _rbo[0]);

    glBindRenderbuffer(GL_RENDERBUFFER, _rbo[1]);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, DEFAULT_W, DEFAULT_H);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, _rbo[1]);

    return ERROR_NONE;
}

//------------------------------------------------------------------------------
void window::destroy_framebuffer()
{
    glDeleteRenderbuffers(2, _rbo);
    glDeleteFramebuffers(1, &_fbo);
}

//------------------------------------------------------------------------------
void window::destroy()
{
    if (_hwnd) {
        destroy_framebuffer();
        shutdown_opengl( );

        DestroyWindow(_hwnd);
        _hwnd = NULL;
    }
}

//------------------------------------------------------------------------------
void window::shutdown_opengl()
{
    wglMakeCurrent(NULL, NULL);

    if (_hrc) {
        wglDeleteContext(_hrc);
        _hrc = NULL;
    }

    if (_hdc) {
        ReleaseDC(_hwnd, _hdc);
        _hdc = NULL;
    }
}

//------------------------------------------------------------------------------
LRESULT window::message (UINT nCmd, WPARAM wParam, LPARAM lParam)
{
    switch ( nCmd )
    {
    case WM_ACTIVATE:
        activate( (LOWORD(wParam) != WA_INACTIVE), ( HIWORD(wParam) > 0 ) );
        break;

    case WM_SIZE:
        _size.x = LOWORD(lParam);
        _size.y = HIWORD(lParam);
        _renderer.resize(); // uses params in WndParams
        break;

    case WM_MOVE:
        _position.x = LOWORD(lParam);    // horizontal position 
        _position.y = HIWORD(lParam);    // vertical position 
        break;

    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;

    default:
        break;
    }
    return DefWindowProcA( _hwnd, nCmd, wParam, lParam );
}

//------------------------------------------------------------------------------
int window::activate(bool active, bool minimized)
{
    if (active && !minimized) {
        if (!_active || _minimized) {
            SetForegroundWindow(_hwnd);
            ShowWindow(_hwnd, SW_RESTORE);
            _active = true;
            _minimized = false;
        }
    } else {
        if (minimized) {
            _minimized = true;
            ShowWindow(_hwnd, SW_MINIMIZE);
        }

        _active = false;
    }

    return ERROR_NONE;
}

} // namespace render
