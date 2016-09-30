/*
===============================================================================

Name	:	r_main.h

Purpose	:	Rendering controller

Date	:	10/20/2004

===============================================================================
*/

#include "local.h"

/*
===========================================================

Name	:	cRender::Init

Purpose	:	Object Initialization

===========================================================
*/

int cRender::Init ()
{
	SetViewOrigin( vec2(0,0) );

	m_InitFonts( );

	for (int i = 0; i < 360 ; i++)
	{
		sintbl[i] = sin(DEG2RAD(i));
		costbl[i] = cos(DEG2RAD(i));
	}

	return ERROR_NONE;
}

/*
===========================================================

Name	:	cRender::Shutdown

Purpose	:	Shuts down object before removal

===========================================================
*/

int cRender::Shutdown ()
{
	m_ClearFonts( );

	return ERROR_NONE;
}

/*
===========================================================

Name	:	cRender::BeginFrame

Purpose	:	Preps the renderer for a new frame

===========================================================
*/

void cRender::BeginFrame ()
{
	glClear( GL_COLOR_BUFFER_BIT );
}

/*
===========================================================

Name	:	cRender::EndFrame

Purpose	:	End of Drawing, Swap to screen

===========================================================
*/

void cRender::EndFrame ()
{
	SwapBuffers( g_Application->get_glWnd()->get_hDC() );
}

/*
===========================================================

Name	:	cRender::m_setDefaultState

Purpose	:	Sets default state based on window size

===========================================================
*/

void cRender::m_setDefaultState ()
{
	glDisable( GL_TEXTURE_2D );

	glClearColor( 0, 0, 0, 0.1f );

	glEnable( GL_BLEND );
	glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

	glEnable( GL_ALPHA_TEST );
	glAlphaFunc( GL_GREATER, 0.0 );

	glEnable( GL_POINT_SMOOTH );
	glHint( GL_POINT_SMOOTH_HINT, GL_NICEST );
	glPointSize( 2.0f );

	glEnable( GL_LINE_SMOOTH );
	glHint( GL_LINE_SMOOTH_HINT, GL_NICEST );

	glDisable( GL_DEPTH_TEST );
	glDisable( GL_CULL_FACE );

	glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );

	glDrawBuffer( GL_BACK );

	glViewport(
		0, 0,
		g_Application->get_glWnd()->get_WndParams().nSize[0],
		g_Application->get_glWnd()->get_WndParams().nSize[1] );

	glMatrixMode( GL_PROJECTION );
	glLoadIdentity( );
/*
	glOrtho(
		m_viewOrigin[0],
		g_Application->get_glWnd()->get_WndParams().nSize[0] + m_viewOrigin[0],
		g_Application->get_glWnd()->get_WndParams().nSize[1] + m_viewOrigin[1],
		m_viewOrigin[1],
		-99999,
		99999 );
*/

	glOrtho( 0, 640, 480, 0, -99999, 99999 );

	glMatrixMode( GL_MODELVIEW );
	glLoadIdentity( );
}