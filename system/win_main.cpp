// win_main.cpp
//

#include "precompiled.h"
#pragma hdrstop

#include "cm_keys.h"

#include <WS2tcpip.h>
#include <XInput.h>

application *g_Application; // global instance, extern declaration in "win_main.h"

//------------------------------------------------------------------------------
int WINAPI WinMain (HINSTANCE hInstance, HINSTANCE /*hPrevInstance*/, LPSTR szCmdLine, int nCmdShow)
{
    return application(hInstance).main(szCmdLine, nCmdShow);
}

//------------------------------------------------------------------------------
application::application(HINSTANCE hInstance)
    : _hinstance(hInstance)
    , _window(hInstance, wndproc)
    , _exit_code(0)
    , _mouse_state(0)
{
    g_Application = this;
}

//------------------------------------------------------------------------------
int application::main(LPSTR szCmdLine, int /*nCmdShow*/)
{
    time_value previous_time, current_time;
    MSG msg;

    init(_hinstance, szCmdLine);

    previous_time = time();

    while (true) {

        // inactive/idle loop
        if (!_window.active()) {
            Sleep(1);
        }

        // message loop (pump)
        while (PeekMessageA(&msg, NULL, 0, 0, PM_NOREMOVE)) {
            if (!GetMessageA(&msg, NULL, 0, 0 )) {
                return shutdown();
            }

            TranslateMessage(&msg);
            DispatchMessageA(&msg);
        }

        // gamepad events are polled from the device
        generate_gamepad_events();

        current_time = time();
        _game.run_frame(current_time - previous_time);
        previous_time = current_time;
    }
}

//------------------------------------------------------------------------------
result application::init(HINSTANCE hInstance, LPSTR szCmdLine)
{
    // set instance
    _hinstance = hInstance;

    // set init string
    _init_string = szCmdLine;

    // init timer
    QueryPerformanceFrequency(&_timer_frequency);
    QueryPerformanceCounter(&_timer_base);

    srand(static_cast<unsigned int>(_timer_base.QuadPart));

    _config.init();

    // initialize networking
    {
        WSADATA wsadata = {};
        if (WSAStartup(MAKEWORD(2, 2), &wsadata)) {
        }
    }

    // create sound class
    sound::system::create();

    // init opengl
    _window.create();

    // init game
    _game.init( szCmdLine );

    return result::success;
}

//------------------------------------------------------------------------------
int application::shutdown()
{
    // shutdown opengl
    _window.destroy();

    // shutdown game
    _game.shutdown( );

    // shutdown sound
    sound::system::destroy();

    // shutdown networking
    WSACleanup();

    _config.shutdown();

#ifdef DEBUG_MEM    
    _CrtDumpMemoryLeaks();
#endif // DEBUG_MEM

    return _exit_code;
}

//------------------------------------------------------------------------------
void application::error(char const *title, char const *message)
{
    MessageBoxA(NULL, message, title, MB_OK);

    quit(-1);
}

//------------------------------------------------------------------------------
void application::quit(int exit_code)
{
    // save the exit code for shutdown
    _exit_code = exit_code;

    // tell windows we dont want to play anymore
    PostQuitMessage(exit_code);
}

//------------------------------------------------------------------------------
time_value application::time() const
{
    LARGE_INTEGER counter;
    QueryPerformanceCounter(&counter);

    double num = static_cast<double>(counter.QuadPart - _timer_base.QuadPart);
    double den = 1e-6 * static_cast<double>(_timer_frequency.QuadPart); 
    return time_value::from_microseconds(static_cast<uint64_t>(num / den));
}

//------------------------------------------------------------------------------
LRESULT application::wndproc(HWND hWnd, UINT nCmd, WPARAM wParam, LPARAM lParam)
{
    char const* command;

    switch (nCmd)
    {
    case WM_NCCREATE:
        EnableNonClientDpiScaling(hWnd);
        return DefWindowProc( hWnd, nCmd, wParam, lParam );

    case WM_CREATE:
        if ( !(command = strstr( g_Application->init_string(), "sound=" )) || ( atoi(command+6) > 0 ))
            pSound->on_create( hWnd );
        return DefWindowProc( hWnd, nCmd, wParam, lParam );

    case WM_CLOSE:
        g_Application->quit( 0 );
        return 0;

    // Game Messages

    case WM_LBUTTONDOWN:
    case WM_LBUTTONUP:
    case WM_RBUTTONDOWN:
    case WM_RBUTTONUP:
    case WM_MBUTTONDOWN:
    case WM_MBUTTONUP:
    case WM_MOUSEMOVE: {
        POINT P{(int)(short)LOWORD(lParam), (int)(short)HIWORD(lParam)};
        g_Application->mouse_event(wParam, vec2i(P.x, P.y));
        break;
    }

    case WM_MOUSEWHEEL: {
        POINT P{(int)(short)LOWORD(lParam), (int)(short)HIWORD(lParam)};
        ScreenToClient(hWnd, &P);
        g_Application->mouse_event(wParam, vec2i(P.x, P.y));
        break;
    }

    case WM_SYSKEYDOWN:
    case WM_KEYDOWN:
        if (wParam == VK_RETURN && (HIWORD(lParam) & KF_ALTDOWN)) {
            return 0;
        }
        g_Application->key_event( lParam, true );
        break;

    case WM_SYSKEYUP:
    case WM_KEYUP:
        g_Application->key_event( lParam, false );
        break;

    case WM_SYSCHAR:
    case WM_CHAR:
        if (wParam == VK_RETURN && (HIWORD(lParam) & KF_ALTDOWN)) {
            g_Application->_window.toggle_fullscreen();
            return 0;
        }
        g_Application->char_event(wParam, lParam);
        break;

    // glWnd Messages

    case WM_ACTIVATE:
    case WM_SIZE:
    case WM_MOVE:
    case WM_DESTROY:
    case WM_DPICHANGED:
        return g_Application->_window.message( nCmd, wParam, lParam );

    default:
        break;
    }

    return DefWindowProcA(hWnd, nCmd, wParam, lParam);
}

//------------------------------------------------------------------------------
void application::char_event(WPARAM key, LPARAM /*state*/)
{
    _game.char_event(narrow_cast<int>(key));
}

//------------------------------------------------------------------------------
constexpr unsigned char keymap[128] = {
    0  ,    27,     '1',    '2',    '3',    '4',    '5',    '6',
    '7',    '8',    '9',    '0',    '-',    '=',    K_BACKSPACE, 9, // 0
    'q',    'w',    'e',    'r',    't',    'y',    'u',    'i',
    'o',    'p',    '[',    ']',    13 ,    K_CTRL, 'a',    's',      // 1
    'd',    'f',    'g',    'h',    'j',    'k',    'l',    ';',
    '\'' ,  '`',    K_SHIFT,'\\',   'z',    'x',    'c',    'v',      // 2
    'b',    'n',    'm',    ',',    '.',    '/',    K_SHIFT,'*',
    K_ALT,  ' ',    0  ,    K_F1,   K_F2,   K_F3,   K_F4,   K_F5,   // 3
    K_F6,   K_F7,   K_F8,   K_F9,   K_F10,  K_PAUSE,    0  ,K_HOME,
    K_UPARROW,  K_PGUP, K_KP_MINUS, K_LEFTARROW,K_KP_5, K_RIGHTARROW,   K_KP_PLUS,  K_END, //4
    K_DOWNARROW,K_PGDN, K_INS,      K_DEL,  0,      0,      0,  K_F11,
    K_F12,  0  ,    0  ,    0  ,    0  ,    0  ,    0  ,    0,        // 5
    0  ,    0  ,    0  ,    0  ,    0  ,    0  ,    0  ,    0,
    0  ,    0  ,    0  ,    0  ,    0  ,    0  ,    0  ,    0,        // 6
    0  ,    0  ,    0  ,    0  ,    0  ,    0  ,    0  ,    0,
    0  ,    0  ,    0  ,    0  ,    0  ,    0  ,    0  ,    0         // 7
};

//------------------------------------------------------------------------------
void application::key_event(LPARAM param, bool down)
{
    int result;
    LPARAM modified = ( param >> 16 ) & 255;
    bool is_extended = false;

    if ( modified > 127) {
        return;
    }

    if ( param & ( 1 << 24 ) ) {
        is_extended = true;
    }

    result = keymap[modified];

    if (!is_extended) {
        switch (result) {
            case K_HOME:
                result = K_KP_HOME;
                break;
            case K_UPARROW:
                result = K_KP_UPARROW;
                break;
            case K_PGUP:
                result = K_KP_PGUP;
                break;
            case K_LEFTARROW:
                result = K_KP_LEFTARROW;
                break;
            case K_RIGHTARROW:
                result = K_KP_RIGHTARROW;
                break;
            case K_END:
                result = K_KP_END;
                break;
            case K_DOWNARROW:
                result = K_KP_DOWNARROW;
                break;
            case K_PGDN:
                result = K_KP_PGDN;
                break;
            case K_INS:
                result = K_KP_INS;
                break;
            case K_DEL:
                result = K_KP_DEL;
                break;
            default:
                break;
        }
    } else {
        switch (result) {
            case 0x0D:
                result = K_KP_ENTER;
                break;
            case 0x2F:
                result = K_KP_SLASH;
                break;
            case 0xAF:
                result = K_KP_PLUS;
                break;
        }
    }

    _game.key_event(result, down);
}

//------------------------------------------------------------------------------
void application::mouse_event(WPARAM mouse_state, vec2i position)
{
    for (int ii = 0; ii < 3; ++ii) {
        if ((mouse_state & (1LL << ii)) && !(_mouse_state & (1LL << ii))) {
            _game.key_event (K_MOUSE1 + ii, true);
        }

        if (!(mouse_state & (1LL << ii)) && (_mouse_state & (1LL << ii))) {
            _game.key_event (K_MOUSE1 + ii, false);
        }
    }

    if ((int16_t)HIWORD(mouse_state) < 0) {
        _game.key_event(K_MWHEELDOWN, true);
        _game.key_event(K_MWHEELDOWN, false);
    } else if ((int16_t)HIWORD(mouse_state) > 0) {
        _game.key_event(K_MWHEELUP, true);
        _game.key_event(K_MWHEELUP, false);
    }

    _mouse_state = mouse_state;
    _game.cursor_event(vec2(position));
}

//------------------------------------------------------------------------------
void application::generate_gamepad_events()
{
    for (DWORD ii = 0; ii < XUSER_MAX_COUNT; ++ii) {
        XINPUT_STATE state{};

        DWORD dwResult = XInputGetState(ii, &state);
        if (dwResult != ERROR_SUCCESS) {
            continue;
        }

        constexpr DWORD thumb_maximum = 32767;
        constexpr DWORD thumb_deadzone =
            (XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE + XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE) / 2;

        game::gamepad pad{};

        pad.thumbstick[game::gamepad::left] = {
            (float)state.Gamepad.sThumbLX, (float)state.Gamepad.sThumbLY
        };

        pad.thumbstick[game::gamepad::right] = {
            (float)state.Gamepad.sThumbRX, (float)state.Gamepad.sThumbRY
        };

        pad.trigger[game::gamepad::left] = state.Gamepad.bLeftTrigger * (1.0f / 255.0f);
        pad.trigger[game::gamepad::right] = state.Gamepad.bRightTrigger * (1.0f / 255.0f);

        for (int side = 0; side < 2; ++side) {
            float thumb_magnitude = pad.thumbstick[side].length();
            if (thumb_magnitude < thumb_deadzone) {
                pad.thumbstick[side] = vec2_zero;
            } else {
                pad.thumbstick[side].normalize_self();
                if (thumb_magnitude < thumb_maximum) {
                    pad.thumbstick[side] *= (thumb_magnitude - thumb_deadzone) / (thumb_maximum - thumb_deadzone);
                }
            }
        }

        // emit gamepad event
        _game.gamepad_event(ii, pad);

        // process buttons
        XINPUT_KEYSTROKE keystroke{};

        while (XInputGetKeystroke(ii, 0, &keystroke) == ERROR_SUCCESS) {
            _game.key_event(keystroke.VirtualKey, !(keystroke.Flags & XINPUT_KEYSTROKE_KEYUP));
        }
    }
}

//------------------------------------------------------------------------------
std::string application::clipboard() const
{
    std::string s;

    if (OpenClipboard(NULL) != 0) {
        HANDLE hClipboardData = GetClipboardData(CF_TEXT);

        if (hClipboardData != NULL) {
            char* cliptext = (char *)GlobalLock(hClipboardData);

            if (cliptext != nullptr) {
                s.assign(cliptext, GlobalSize(hClipboardData));
                GlobalUnlock(hClipboardData);
            }
        }
        CloseClipboard();
    }

    return s;
}
