// r_window.cpp
//

#include "precompiled.h"
#pragma hdrstop

#include "resource.h"

#include <ShellScalingApi.h>

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

typedef BOOL (APIENTRY* PFNWGLSWAPINTERVALEXT)(GLint interval);
typedef int (APIENTRY* PFNWGLGETSWAPINTERVALEXT)();

static PFNGLBINDRENDERBUFFER glBindRenderbuffer = NULL;
static PFNGLDELETERENDERBUFFERS glDeleteRenderbuffers = NULL;
static PFNGLGENRENDERBUFFERS glGenRenderbuffers = NULL;
static PFNGLRENDERBUFFERSTORAGE glRenderbufferStorage = NULL;
static PFNGLBINDFRAMEBUFFER glBindFramebuffer = NULL;
static PFNGLDELETEFRAMEBUFFERS glDeleteFramebuffers = NULL;
static PFNGLGENFRAMEBUFFERS glGenFramebuffers = NULL;
static PFNGLFRAMEBUFFERRENDERBUFFER glFramebufferRenderbuffer = NULL;
static PFNGLBLITFRAMEBUFFER glBlitFramebuffer = NULL;

static PFNWGLSWAPINTERVALEXT wglSwapIntervalEXT = NULL;
static PFNWGLGETSWAPINTERVALEXT wglGetSwapIntervalEXT = NULL;

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

constexpr int windowed_style = WS_OVERLAPPED|WS_BORDER|WS_CAPTION|WS_SYSMENU|WS_MINIMIZEBOX|WS_MAXIMIZEBOX;
constexpr int fullscreen_style = WS_OVERLAPPED;

//------------------------------------------------------------------------------
window::window(HINSTANCE hInstance, WNDPROC WndProc)
    : _hinst(hInstance)
    , _wndproc(WndProc)
    , _hdc(nullptr)
    , _hrc(nullptr)
    , _renderer(this)
    , _current_dpi(USER_DEFAULT_SCREEN_DPI)
{
    _fbo       = 0;
    _rbo[0]    = 0;
    _rbo[1]    = 0;
}

//------------------------------------------------------------------------------
void window::create()
{
    char const* command;
    bool fullscreen = DEFAULT_FS;

    if ( (command = strstr( g_Application->init_string(), "fullscreen" )) )
        fullscreen = ( atoi(command+11) > 0 );

    create(CW_USEDEFAULT, 0, DEFAULT_W, DEFAULT_H, fullscreen);

    _renderer.init();
}

//------------------------------------------------------------------------------
void window::end_frame ()
{
    glBindFramebuffer(GL_READ_FRAMEBUFFER, _fbo);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

    glDrawBuffer(GL_BACK);

    glBlitFramebuffer(
        0, 0, _framebuffer_size.x, _framebuffer_size.y,
        0, 0, _physical_size.x, _physical_size.y,
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
int window::create(int xpos, int ypos, int width, int height, bool fullscreen)
{
    SetProcessDpiAwareness(PROCESS_PER_MONITOR_DPI_AWARE);

    // Setup struct for RegisterClass
    WNDCLASSA   wc;

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
        g_Application->error( "Tanks! Error", "window::m_CreateWindow | RegisterClass failed\n" );
        return ERROR_FAIL;
    }

    // Create the Window

    int style = windowed_style;

    RECT rect = { 0, 0, width, height };
    AdjustWindowRect(&rect, style, FALSE);

    _hwnd = CreateWindowExA(
        0,
        APP_CLASSNAME,
        "Tanks!",
        style,
        xpos,
        ypos,
        rect.right - rect.left,
        rect.bottom - rect.top,
        NULL, NULL,
        _hinst,
        NULL);

    // Adjust for DPI
    {
        int dpi = GetDpiForWindow(_hwnd);

        rect.top = 0;
        rect.left = 0;
        rect.right = MulDiv(width, dpi, USER_DEFAULT_SCREEN_DPI);
        rect.bottom = MulDiv(height, dpi, USER_DEFAULT_SCREEN_DPI);

        AdjustWindowRectExForDpi(&rect, style, FALSE, 0, dpi);

        _physical_size.x = rect.right;
        _physical_size.y = rect.bottom;

        SetWindowPos(
            _hwnd,
            NULL,
            rect.left,
            rect.top,
            rect.right - rect.left,
            rect.bottom - rect.top,
            SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
    }

    _logical_size.x = width;
    _logical_size.y = height;

    if (fullscreen) {
        _active = true;
        _minimized = false;
        _fullscreen = true;
    } else {
        _active = true;
        _minimized = false;
        _fullscreen = false;
    }

    if (_hwnd == NULL) {
        g_Application->error( "Tanks! Error", "window::m_CreateWindow | CreateWindow failed\n" );

        return ERROR_FAIL;
    }

    if (fullscreen) {
        SetWindowLongPtrA(_hwnd, GWL_STYLE, fullscreen_style);
        ShowWindow(_hwnd, SW_SHOWMAXIMIZED);
    } else {
        ShowWindow(_hwnd, SW_SHOWNORMAL);
    }
    UpdateWindow(_hwnd);

    // initialize OpenGL
    if (init_opengl() != ERROR_NONE) {
        return ERROR_FAIL;
    }

    // initialize framebuffer
    if (create_framebuffer(_logical_size.x, _logical_size.y) != ERROR_NONE) {
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
        g_Application->error( "Tanks! Error", "window::m_InitGL | GetDC failed");
        return ERROR_FAIL;
    }

    // select pixel format
    if ((pixelformat = ChoosePixelFormat(_hdc, &pfd)) == 0) {
        g_Application->error( "Tanks! Error", "window::m_InitGL | ChoosePixelFormat failed");
        return ERROR_FAIL;
    }
    if ((SetPixelFormat(_hdc, pixelformat, &pfd)) == FALSE) {
        g_Application->error( "Tanks! Error", "window::m_InitGL | SetPixelFormat failed");
        return ERROR_FAIL;
    }

    // set up context
    if ((_hrc = wglCreateContext(_hdc)) == 0) {
        g_Application->error ( "Tanks! Error", "window::m_InitGL: wglCreateContext failed" );
        shutdown_opengl();
        return ERROR_FAIL;
    }
    if (!wglMakeCurrent(_hdc, _hrc)) {
        g_Application->error ( "Tanks! Error", "window::m_InitGL: wglMakeCurrent failed" );
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

    wglSwapIntervalEXT = (PFNWGLSWAPINTERVALEXT )wglGetProcAddress("wglSwapIntervalEXT");
    wglGetSwapIntervalEXT = (PFNWGLGETSWAPINTERVALEXT )wglGetProcAddress("wglGetSwapIntervalEXT");

    if (wglSwapIntervalEXT) {
        wglSwapIntervalEXT(0);
    }

    return ERROR_NONE;
}

//------------------------------------------------------------------------------
int window::create_framebuffer(int width, int height)
{
    if (!_hrc) {
        return ERROR_FAIL;
    }

    if (_fbo) {
        destroy_framebuffer();
    }

    glGenFramebuffers(1, &_fbo);
    glGenRenderbuffers(2, _rbo);

    glBindFramebuffer(GL_FRAMEBUFFER, _fbo);

    glBindRenderbuffer(GL_RENDERBUFFER, _rbo[0]);
    glRenderbufferStorage( GL_RENDERBUFFER, GL_RGBA, width, height);
    glFramebufferRenderbuffer( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, _rbo[0]);

    glBindRenderbuffer(GL_RENDERBUFFER, _rbo[1]);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, _rbo[1]);

    _framebuffer_size.x = width;
    _framebuffer_size.y = height;

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
    switch (nCmd) {
        case WM_ACTIVATE:
            activate( (LOWORD(wParam) != WA_INACTIVE), ( HIWORD(wParam) > 0 ) );
            break;

        case WM_SIZE:
            _physical_size.x = LOWORD(lParam);
            _physical_size.y = HIWORD(lParam);
            _renderer.resize(); // uses params in WndParams
            break;

        case WM_MOVE:
            _position.x = (short)LOWORD(lParam); // horizontal position
            _position.y = (short)HIWORD(lParam); // vertical position
            break;

        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;

        case WM_DPICHANGED: {
            resize_for_dpi(reinterpret_cast<RECT*>(lParam), HIWORD(wParam));
            break;
        }

        default:
            break;
    }

    return DefWindowProcA( _hwnd, nCmd, wParam, lParam );
}

//------------------------------------------------------------------------------
void window::resize_for_dpi(RECT const* suggested, int dpi)
{
    // By default Windows adjusts the window to preserve scale for the entire
    // window including the non-client area. This causes the dimensions of the
    // client area to change which we want to avoid.

    RECT new_border = {0, 0, 0, 0};
    RECT old_border = {0, 0, 0, 0};
    AdjustWindowRectExForDpi(&new_border, windowed_style, FALSE, 0, dpi);
    AdjustWindowRectExForDpi(&old_border, windowed_style, FALSE, 0, _current_dpi);

    _physical_size.x = MulDiv(_logical_size.x, dpi, USER_DEFAULT_SCREEN_DPI);
    _physical_size.y = MulDiv(_logical_size.y, dpi, USER_DEFAULT_SCREEN_DPI);

    SetWindowPos(
        _hwnd,
        NULL,
        suggested->left - old_border.left + new_border.left,
        suggested->top,
        _physical_size.x + new_border.right - new_border.left,
        _physical_size.y + new_border.bottom - new_border.top,
        SWP_NOZORDER | SWP_NOACTIVATE);

    _current_dpi = dpi;

    end_frame();
}

//------------------------------------------------------------------------------
bool window::toggle_fullscreen()
{
    if (_fullscreen) {
        ShowWindow(_hwnd, SW_RESTORE);
        SetWindowLongA(_hwnd, GWL_STYLE, windowed_style | WS_VISIBLE);
        _fullscreen = false;

        // reset the window dimensions. the resize message from above still
        // uses the fullscreen style and doesn't have the right dimensions
        {
            RECT rect, border = {}; GetWindowRect(_hwnd, &rect);
            AdjustWindowRectExForDpi(&border, windowed_style, FALSE, 0, GetDpiForWindow(_hwnd));
            _physical_size.x = rect.right - rect.left - (border.right - border.left);
            _physical_size.y = rect.bottom - rect.top - (border.bottom - border.top);
            _renderer.resize(); // uses params in WndParams
        }
    } else {
        SetWindowLongA(_hwnd, GWL_STYLE, fullscreen_style | WS_VISIBLE);
        ShowWindow(_hwnd, SW_MAXIMIZE);
        _fullscreen = true;
    }
    return true;
}

//------------------------------------------------------------------------------
int window::activate(bool active, bool minimized)
{
    if (active && !minimized) {
        if (!_active || _minimized) {
            SetForegroundWindow(_hwnd);
            if (_minimized) {
                ShowWindow(_hwnd, SW_RESTORE);
            }
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
