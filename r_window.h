// r_window.h
//

#pragma once

#include <gl/gl.h>
#include <gl/glu.h>

// default size and position
#define DEFAULT_X   100
#define DEFAULT_Y   100
#define DEFAULT_W   640
#define DEFAULT_H   480
#define DEFAULT_FS  1       // fullscreen
#define DEFAULT_MS  1       // multisampling

////////////////////////////////////////////////////////////////////////////////
namespace render {

//------------------------------------------------------------------------------
class window
{
public:
    window(HINSTANCE hInstance, WNDPROC WndProc);
    ~window();

    void create();
    void destroy();
    void end_frame();

    LRESULT message(UINT uCmd, WPARAM wParam, LPARAM lParam);

    HWND hwnd() const { return _hwnd; }
    HDC hdc() const { return _hdc; }

    bool active() const { return _active; }
    vec2 position() const { return _position; }
    vec2 size() const { return _size; }
    int width() const { return _size.x; }
    int height() const { return _size.y; }

    render::system* renderer() { return &_renderer; }

private:
    int create(int width, int height, int xpos, int ypos, bool fullscreen);
    int create_framebuffer(int width, int height);
    void destroy_framebuffer();

    int init_opengl();
    void shutdown_opengl();

    int activate(bool active, bool minimized);

    bool _active;
    bool _minimized;
    bool _fullscreen;
    vec2 _position;
    vec2 _size;

    render::system _renderer;

    HINSTANCE _hinst;
    WNDPROC _wndproc;

    HWND _hwnd;
    HDC _hdc;
    HGLRC _hrc;

    // framebuffer objects
    GLuint _fbo;
    GLuint _rbo[2];
};

} // namespace render
