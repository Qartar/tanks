// cm_keys.h
//

#pragma once

////////////////////////////////////////////////////////////////////////////////
enum keys
{
    KEY_NULL = 0,
    KEY_DOWN = 1,

    K_TAB = 9,
    K_ENTER = 13,
    K_ESCAPE = 27,
    K_SPACE = 32,

// normal keys should be passed as lowercased ascii

    K_BACKSPACE = 127,
    K_UPARROW = 128,
    K_DOWNARROW = 129,
    K_LEFTARROW = 130,
    K_RIGHTARROW = 131,

    K_ALT = 132,
    K_CTRL = 133,
    K_SHIFT = 134,
    K_F1 = 135,
    K_F2 = 136,
    K_F3 = 137,
    K_F4 = 138,
    K_F5 = 139,
    K_F6 = 140,
    K_F7 = 141,
    K_F8 = 142,
    K_F9 = 143,
    K_F10 = 144,
    K_F11 = 145,
    K_F12 = 146,
    K_INS = 147,
    K_DEL = 148,
    K_PGDN = 149,
    K_PGUP = 150,
    K_HOME = 151,
    K_END = 152,

    K_KP_HOME = 160,
    K_KP_UPARROW = 161,
    K_KP_PGUP = 162,
    K_KP_LEFTARROW = 163,
    K_KP_5 = 164,
    K_KP_RIGHTARROW = 165,
    K_KP_END = 166,
    K_KP_DOWNARROW = 167,
    K_KP_PGDN = 168,
    K_KP_ENTER = 169,
    K_KP_INS = 170,
    K_KP_DEL = 171,
    K_KP_SLASH = 172,
    K_KP_MINUS = 173,
    K_KP_PLUS = 174,

    K_PAUSE = 255,

//
// mouse buttons generate virtual keys
//
    K_MOUSE1 = 200,
    K_MOUSE2 = 201,
    K_MOUSE3 = 202,

    K_MWHEELDOWN = 203,
    K_MWHEELUP = 204,
};
