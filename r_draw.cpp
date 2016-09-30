/*
===============================================================================

Name	:	r_draw.cpp

Purpose	:	drawing functions for cRender (r_main.h)

===============================================================================
*/

#include "local.h"

/*
===========================================================

Name	:	DrawString

Purpose	:	draws a string to the screen

===========================================================
*/

void cRender::DrawString (char *szString, vec2 vPos, vec4 vColor)
{
	glColor4f( vColor.r, vColor.g, vColor.b, vColor.a );

	glRasterPos2f( vPos.x, vPos.y );

	glCallLists( strlen(szString), GL_UNSIGNED_BYTE, szString );
}

/*
===========================================================

Name	:	DrawBox

Purpose	:	draws a rotated box to the screen

===========================================================
*/

void cRender::DrawBox (vec2 vSize, vec2 vPos, float flAngle, vec4 vColor)
{
	float	xl, xh, yl, yh;

	glColor4f( vColor.r, vColor.g, vColor.b, vColor.a );

	xl = vPos.x - vSize.x / 2;
	xh = vPos.x + vSize.x / 2;
	yl = vPos.y - vSize.y / 2;
	yh = vPos.y + vSize.y / 2;

	// ROTATE POINTS AROUND ORIGIN

	glBegin( GL_QUADS );

	glVertex2f( xl, yl );
	glVertex2f( xh, yl );
	glVertex2f( xh, yh );
	glVertex2f( xl, yh );

	glEnd( );
}

/*
===========================================================

Name	:	cRender::DrawParticles

Purpose	:	draws a list of particles

===========================================================
*/

void cRender::DrawParticles (cParticle *pHead)
{
	cParticle	*p;
	float		flSize = 0.0f;

	p = pHead;
	while (p)
	{
		glPointSize( (flSize = p->flSize) );

		glBegin( GL_POINTS );
		while (p && p->flSize == flSize)
		{
			glColor4f( p->vColor.r, p->vColor.g, p->vColor.b, p->vColor.a );
			glVertex2f( p->vPos.x, p->vPos.y );

			p = p->pNext;
		}
		glEnd( );
	}
}

/*
===========================================================

Name	:	cRender::DrawLine

Purpose	:	draws a line

===========================================================
*/

void cRender::DrawLine (vec2 vOrg, vec2 vEnd, vec4 vColorO, vec4 vColorE)
{
	glBegin( GL_LINES );

	glColor4f( vColorO.r, vColorO.g, vColorO.b, vColorO.a );

	glVertex2f( vOrg.x, vOrg.y );

	glColor4f( vColorE.r, vColorE.g, vColorE.b, vColorE.a );

	glVertex2f( vEnd.x, vEnd.y );

	glEnd( );
}