/*
===============================================================================

Name	:	r_font.cpp

Purpose	:	OpenGL Font Encapsulation

Date	:	10/19/2004

===============================================================================
*/

#include "local.h"

/*
===========================================================

Name	:	cRender::m_InitFonts

Purpose	:	Creates default font

===========================================================
*/

void cRender::m_InitFonts ()
{
	memset( &m_Fonts, 0, sizeof( m_Fonts ) );

	m_Fonts[0].Init( "Tahoma", 12, 0 );

	m_sysFont = m_Fonts[0].Activate( );

	m_activeFont = 0;
}

/*
===========================================================

Name	:	cRender::m_ClearFonts

Purpose	:	Deactivates all the fonts and restores system font

===========================================================
*/

void cRender::m_ClearFonts ()
{
	int			i;

	SelectObject( g_Application->get_glWnd()->get_hDC(), m_sysFont );

	for ( i=0 ; i<MAX_FONTS ; i++ )
		m_Fonts[i].Shutdown ();
}

/*
===========================================================

Name	:	cRender::AddFont

Purpose	:	Adds a font to m_Fonts

===========================================================
*/

rfont_t cRender::AddFont (char *szName, int nSize, unsigned int bitFlags)
{
	int			i;

	for ( i=0 ; i<MAX_FONTS ; i++ )
		if ( m_Fonts[i].Compare( szName, nSize, bitFlags ) )
			return i;

	for ( i=0 ; i<MAX_FONTS ; i++ )
		if ( ! m_Fonts[i].is_inuse() )
			m_Fonts[i].Init( szName, nSize, bitFlags );
}

/*
===========================================================

Name	:	cRender::RemoveFont

Purpose	:	removes a font from m_Fonts

===========================================================
*/

int cRender::RemoveFont (rfont_t hFont)
{
	SelectObject( g_Application->get_glWnd()->get_hDC(), m_sysFont );

	return m_Fonts[hFont].Shutdown( );
}

/*
===========================================================

Name	:	cRender::UseFont

Purpose	:	makes a font active for rendering

===========================================================
*/

rfont_t cRender::UseFont (rfont_t hFont)
{
	rfont_t	oldFont = m_activeFont;

	m_Fonts[hFont].Activate( );

	m_activeFont = hFont;

	return oldFont;
}

/*
===========================================================

Name	:	cFont::Init

Purpose	:	Initializes a new usable font

===========================================================
*/

int cFont::Init (char *szName, int nSize, unsigned int bitFlags)
{
	HFONT		oldFont;

	// copy member data

	strncpy( m_szName, szName, 64 );
	m_nSize = nSize;
	m_bitFlags = bitFlags;

	// allocate lists

	m_listBase = glGenLists( NUM_CHARS );

	// create font

	m_hFont = CreateFont(
		nSize,
		0,
		0,
		0,
		FW_NORMAL,
		FALSE,
		FALSE,
		FALSE,
		RUSSIAN_CHARSET,
		OUT_TT_PRECIS,
		CLIP_DEFAULT_PRECIS,
		ANTIALIASED_QUALITY,
		FF_DONTCARE|DEFAULT_PITCH,
		m_szName );

	// set our new font to the system

	oldFont = (HFONT )SelectObject( g_Application->get_glWnd()->get_hDC(), m_hFont );

	// generate font bitmaps with selected HFONT

	wglUseFontBitmaps( g_Application->get_glWnd()->get_hDC(), 0, NUM_CHARS-1, m_listBase );

	// restore previous font

	SelectObject( g_Application->get_glWnd()->get_hDC(), oldFont );

	return ERROR_NONE;
}

/*
===========================================================

Name	:	cFont::Shutdown

Purpose	:	Deletes font object

===========================================================
*/

int cFont::Shutdown ()
{
	if ( m_hFont )
	{
	// delete from opengl

		glDeleteLists( m_listBase, NUM_CHARS );

	// delete font from gdi, assume that it has been already removed from the DC

		DeleteObject( m_hFont );
		m_hFont = NULL;
	}

	memset( m_szName, 0, 64 );

	return ERROR_NONE;
}

/*
===========================================================

Name	:	cFont::Compare

Purpose	:	equality comparison

===========================================================
*/

bool cFont::Compare (char *szName, int nSize, unsigned int bitFlags)
{
	if (( strcmp( szName, m_szName ) == 0 )
		&& (nSize == m_nSize)
		&& (bitFlags == m_bitFlags) )
		return true;
	return false;
}

/*
===========================================================

Name	:	cFont::Activate

Purpose	:	Makes font active for drawing

===========================================================
*/

HFONT cFont::Activate ()
{
	glListBase( m_listBase );

	return (HFONT )SelectObject( g_Application->get_glWnd()->get_hDC(), m_hFont );
}