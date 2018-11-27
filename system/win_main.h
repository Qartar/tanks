// win_main.h
//

#pragma once

#ifndef WINAPI
#define WINAPI __stdcall
#endif

//------------------------------------------------------------------------------
class application
{
public:
    application(HINSTANCE hInstance);

    int main(LPSTR szCmdLine, int nCmdShow);

    void error(char const *title, char const *message);

    void quit(int exit_code);

    std::string clipboard() const;
    char const* init_string() { return _init_string; }

    time_value time() const;

    HINSTANCE hinstance() { return _hinstance; }
    config::system* config() { return &_config; }
    render::window* window() { return &_window; }

protected:
    HINSTANCE _hinstance;
    int _exit_code;

    WPARAM _mouse_state;

    char const* _init_string;

    LARGE_INTEGER _timer_frequency;
    LARGE_INTEGER _timer_base;

    render::window _window;
    game::session _game;
    config::system _config;

protected:
    result init(HINSTANCE hInstance, LPSTR szCmdLine);
    int shutdown();

    void key_event(LPARAM param, bool down);
    void mouse_event(WPARAM mouse_state, vec2i position);

    void generate_gamepad_events();

    static LRESULT WINAPI wndproc(HWND hWnd, UINT nCmd, WPARAM wParam, LPARAM lParam);
};

extern application *g_Application;
