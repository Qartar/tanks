// keys.h

//
// these are the key numbers that should be passed to Key_Event
//
#define KEY_NULL		0
#define KEY_DOWN		1

#define	K_TAB			9
#define	K_ENTER			13
#define	K_ESCAPE		27
#define	K_SPACE			32

// normal keys should be passed as lowercased ascii

#define	K_BACKSPACE		127
#define	K_UPARROW		128
#define	K_DOWNARROW		129
#define	K_LEFTARROW		130
#define	K_RIGHTARROW	131

#define	K_ALT			132
#define	K_CTRL			133
#define	K_SHIFT			134
#define	K_F1			135
#define	K_F2			136
#define	K_F3			137
#define	K_F4			138
#define	K_F5			139
#define	K_F6			140
#define	K_F7			141
#define	K_F8			142
#define	K_F9			143
#define	K_F10			144
#define	K_F11			145
#define	K_F12			146
#define	K_INS			147
#define	K_DEL			148
#define	K_PGDN			149
#define	K_PGUP			150
#define	K_HOME			151
#define	K_END			152

#define K_KP_HOME		160
#define K_KP_UPARROW	161
#define K_KP_PGUP		162
#define	K_KP_LEFTARROW	163
#define K_KP_5			164
#define K_KP_RIGHTARROW	165
#define K_KP_END		166
#define K_KP_DOWNARROW	167
#define K_KP_PGDN		168
#define	K_KP_ENTER		169
#define K_KP_INS   		170
#define	K_KP_DEL		171
#define K_KP_SLASH		172
#define K_KP_MINUS		173
#define K_KP_PLUS		174

#define K_PAUSE			255

//
// mouse buttons generate virtual keys
//
#define	K_MOUSE1		200
#define	K_MOUSE2		201
#define	K_MOUSE3		202

#define K_MWHEELDOWN	203
#define K_MWHEELUP		204

//
//	map from wnd params to ascii
//

static unsigned char	keymap[128] = { 
	0  ,    27,     '1',    '2',    '3',    '4',    '5',	'6', 
	'7',    '8',    '9',    '0',    '-',    '=',    K_BACKSPACE, 9, // 0 
	'q',    'w',    'e',    'r',    't',    'y',    'u',	'i', 
	'o',    'p',    '[',    ']',    13 ,    K_CTRL,	'a',	's',      // 1 
	'd',    'f',    'g',    'h',    'j',    'k',    'l',	';', 
	'\'' ,	'`',    K_SHIFT,'\\',	'z',	'x',    'c',	'v',      // 2 
	'b',    'n',    'm',    ',',    '.',    '/',    K_SHIFT,'*', 
	K_ALT,	' ',	0  ,	K_F1,	K_F2,	K_F3,	K_F4,	K_F5,   // 3 
	K_F6,	K_F7,	K_F8,	K_F9,	K_F10,	K_PAUSE,	0  ,K_HOME, 
	K_UPARROW,	K_PGUP,	K_KP_MINUS,	K_LEFTARROW,K_KP_5,	K_RIGHTARROW,	K_KP_PLUS,	K_END, //4 
	K_DOWNARROW,K_PGDN,	K_INS,		K_DEL,	0,		0,		0,	K_F11, 
	K_F12,	0  ,    0  ,    0  ,    0  ,    0  ,    0  ,    0,        // 5
	0  ,    0  ,    0  ,    0  ,    0  ,    0  ,    0  ,    0, 
	0  ,    0  ,    0  ,    0  ,    0  ,    0  ,    0  ,    0,        // 6 
	0  ,    0  ,    0  ,    0  ,    0  ,    0  ,    0  ,    0, 
	0  ,    0  ,    0  ,    0  ,    0  ,    0  ,    0  ,    0         // 7 
}; 
