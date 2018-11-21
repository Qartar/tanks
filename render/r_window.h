// r_window.h
//

#pragma once

#include <gl/gl.h>
#include <gl/glu.h>

// default size and position
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
    bool toggle_fullscreen();

    HWND hwnd() const { return _hwnd; }
    HDC hdc() const { return _hdc; }

    bool active() const { return _active; }
    vec2i position() const { return _position; }
    vec2i size() const { return _physical_size; }
    vec2i framebuffer_size() const { return _framebuffer_size; }
    int width() const { return _physical_size.x; }
    int height() const { return _physical_size.y; }
    bool fullscreen() const { return _fullscreen; }

    render::system* renderer() { return &_renderer; }

private:
    result create(int xpos, int ypos, int width, int height, bool fullscreen);
    result create_framebuffer(int width, int height);
    void destroy_framebuffer();

    result init_opengl();
    void shutdown_opengl();

    result activate(bool active, bool minimized);
    void resize_for_dpi(RECT const* suggested, int dpi);

    int _current_dpi;

    bool _active;
    bool _minimized;
    bool _fullscreen;
    vec2i _position;
    vec2i _logical_size;
    vec2i _physical_size;

    render::system _renderer;

    HINSTANCE _hinst;
    WNDPROC _wndproc;

    HWND _hwnd;
    HDC _hdc;
    HGLRC _hrc;

    // framebuffer objects
    GLuint _fbo;
    GLuint _rbo[2];
    vec2i _framebuffer_size;
};

} // namespace render
