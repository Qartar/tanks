/*
===============================================================================

Name    :   win_main.h

Purpose :   Windows API Interface

Date    :   10/15/2004

===============================================================================
*/

#include "local.h"
#pragma hdrstop

#include "keys.h"

cWinApp *g_Application; // global instance, extern declaration in "win_main.h"

filectrl_c  g_filectrl_c, *s_filectrl_c = &g_filectrl_c;
memctrl_c   g_memctrl_c, *s_memctrl_c = &g_memctrl_c;

/*
===========================================================

Name    :   WinMain

Purpose :   Program Entry ; routes to cWinApp::Main

===========================================================
*/

int WINAPI WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR szCmdLine, int nCmdShow)
{
    cWinApp Application(hInstance);

    g_Application = &Application;   // set up global object

    return Application.Main( szCmdLine, nCmdShow );
}

cWinApp::cWinApp(HINSTANCE hInstance)
    : m_hInstance(hInstance)
    , _window(hInstance, (WNDPROC )m_WndProc)
    , m_nExitCode(0)
{}

/*
===========================================================

Name    :   cWinApp::Main

Purpose :   internal replacement for WinMain (or main in console)

===========================================================
*/

int cWinApp::Main (LPSTR szCmdLine, int nCmdShow)
{
    float   flTime, flNewTime, flOldTime;
    MSG     msgMessage;

    Init( m_hInstance, szCmdLine );

    flOldTime = get_time( );

    while( true )
    {
        // message loop (pump)

        Sleep( 1 );

        while ( PeekMessage( &msgMessage, NULL, 0, 0, PM_NOREMOVE ) )
        {
            if ( !GetMessage( &msgMessage, NULL, 0, 0 ) )
                return Shutdown( );

            TranslateMessage( &msgMessage );
            DispatchMessage( &msgMessage );
        }

        // inactive/idle loop

        if (!_window.active())
            Sleep( 1 );

        // wait loop

        flNewTime = get_time( );
        flTime = flNewTime - flOldTime;
        flOldTime = flNewTime;

        _game.run_frame( flTime );
    }

    // abnormal termination
    return ERROR_FAIL;
}

/*
===========================================================

Name    :   cWinApp::Init

Purpose :   initialize timer and data members

===========================================================
*/
int cWinApp::Init (HINSTANCE hInstance, LPSTR szCmdLine)
{
    // set instance
    m_hInstance = hInstance;

    // set init string
    m_szInitString = szCmdLine;

    // init timer
    QueryPerformanceFrequency( &m_timerFrequency );
    QueryPerformanceCounter( &m_timerBase );

    srand( m_timerBase.QuadPart );

    // NETWORKING OMGWTFLOL

    pNet = &_network;
    _network.init( );

    vVariable::Create( );
    
    // create sound class
    sound::system::create();

    // init opengl
    _window.create();

    // init game
    _game.init( szCmdLine );

    return ERROR_NONE;
}

/*
===========================================================

Name    :   cWinApp::Shutdown

Purpose :   shuts down the application completely
            the program ends right after calling this

===========================================================
*/

int cWinApp::Shutdown ()
{
    // shutdown opengl
    _window.destroy();

    // shutdown game
    _game.shutdown( );

    // shutdown sound
    sound::system::destroy();

    vVariable::Destroy( );

    _network.shutdown( );

#ifdef DEBUG_MEM    
    _CrtDumpMemoryLeaks();
#endif // DEBUG_MEM


    return m_nExitCode;
}

/*
===========================================================

Name    :   cWinApp::Error

Purpose :   outputs an error using a message box
            for use before graphics have been successfully
            initialized

===========================================================
*/

void cWinApp::Error (char *szTitle, char *szMessage)
{
    MessageBox( NULL, szMessage, szTitle, MB_OK );

    Quit( ERROR_FAIL );
}

/*
===========================================================

Name    :   cWinApp::Quit

Purpose :   causes the program to begin the exit sequence

===========================================================
*/

void cWinApp::Quit (int nExitCode)
{
    // save the exit code for shutdown
    m_nExitCode = nExitCode;

    // tell windows we dont want to play anymore
    PostQuitMessage( nExitCode );
}

/*
===========================================================

Name    :   cWinApp::m_WndProc

Purpose :   internal static function for windows message handling

===========================================================
*/

LRESULT cWinApp::m_WndProc (HWND hWnd, UINT nCmd, WPARAM wParam, LPARAM lParam)
{
    char    *command;

    switch (nCmd)
    {
    case WM_CREATE:
        if ( !(command = strstr( g_Application->InitString(), "sound=" )) || ( atoi(command+6) > 0 ))
            pSound->on_create( hWnd );
        return DefWindowProc( hWnd, nCmd, wParam, lParam );

    case WM_CLOSE:
        g_Application->Quit( 0 );
        return 0;

    // Game Messages

    case WM_LBUTTONDOWN:
    case WM_LBUTTONUP:
    case WM_RBUTTONDOWN:
    case WM_RBUTTONUP:
    case WM_MBUTTONDOWN:
    case WM_MBUTTONUP:
    case WM_MOUSEMOVE:
        g_Application->m_MouseEvent ( wParam );
        break;

    case WM_KEYDOWN:
        g_Application->m_KeyEvent( lParam, true );
        break;
    case WM_KEYUP:
        g_Application->m_KeyEvent( lParam, false );
        break;

    // glWnd Messages

    case WM_ACTIVATE:
    case WM_SIZE:
    case WM_MOVE:
    case WM_SYSKEYDOWN:
    case WM_DESTROY:
        return g_Application->_window.message( nCmd, wParam, lParam );

    default:
        return DefWindowProc( hWnd, nCmd, wParam, lParam );
    }

    return 0;
}

/*
===========================================================

Name    :   cWinApp::m_KeyEvent

Purpose :   translates a win message into a key code

===========================================================
*/

void cWinApp::m_KeyEvent (int Param, bool Down)
{
    int result;
    int modified = ( Param >> 16 ) & 255;
    bool    is_extended = false;

    if ( modified > 127)
        return;

    if ( Param & ( 1 << 24 ) )
        is_extended = true;

    result = keymap[modified];

    if ( !is_extended )
    {
        switch ( result )
        {
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
    }
    else
    {
        switch ( result )
        {
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

    _game.key_event( result, Down );
}

/*
===========================================================

Name    :   cWinApp::m_MouseEvent

Purpose :   translates mouse event messages into key codes

===========================================================
*/

void cWinApp::m_MouseEvent (int mstate)
{
    int     i;
    static int  oldstate;

// perform button actions
    for (i=0 ; i<3 ; i++)
    {
        if ( (mstate & (1<<i)) &&
            !(oldstate & (1<<i)) )
        {
            _game.key_event (K_MOUSE1 + i, true);
        }

        if ( !(mstate & (1<<i)) &&
            (oldstate & (1<<i)) )
        {
            _game.key_event (K_MOUSE1 + i, false);
        }
    }   
        
    oldstate = mstate;
}

char *cWinApp::ClipboardData( void )
{
    char *data = NULL;
    char *cliptext;

    if ( OpenClipboard( NULL ) != 0 )
    {
        HANDLE hClipboardData;

        if ( ( hClipboardData = GetClipboardData( CF_TEXT ) ) != 0 )
        {
            if ( ( cliptext = (char *)GlobalLock( hClipboardData ) ) != 0 ) 
            {
                data = (char *)malloc( GlobalSize( hClipboardData ) + 1 );
                strcpy( data, cliptext );
                GlobalUnlock( hClipboardData );
            }
        }
        CloseClipboard();
    }
    return data;
}
