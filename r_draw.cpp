/*
===============================================================================

Name    :   r_draw.cpp

Purpose :   drawing functions for cRender (r_main.h)

===============================================================================
*/

#include "local.h"
#pragma hdrstop

/*
===========================================================

Name    :   DrawString

Purpose :   draws a string to the screen

===========================================================
*/

void cRender::DrawString (char *szString, vec2 vPos, vec4 vColor)
{
    m_Fonts[m_activeFont].Draw( szString, vPos, vColor );
}

/*
===========================================================

Name    :   DrawBox

Purpose :   draws a rotated box to the screen

===========================================================
*/

void cRender::DrawBox (vec2 vSize, vec2 vPos, float flAngle, vec4 vColor)
{
    float   xl, xh, yl, yh;

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

Name    :   cRender::DrawParticles

Purpose :   draws a list of particles

===========================================================
*/

#define ANTI    1

void cRender::DrawParticles (cParticle *pHead)
{
    cParticle   *p;
    float       flSize = 0.0f;

    int     i, frac;
    float   offx, offy;
    float   a_in, a_out;

    p = pHead;
    while ( p )
    {
        flSize = p->flSize * 0.5;

        glBegin( GL_TRIANGLE_FAN );

        if ( p->bitFlags & PF_INVERT )
        {
            a_in = p->vColor.a * 0.25;
            a_out= p->vColor.a;
        }
        else
        {
            a_in = p->vColor.a;
            a_out = p->vColor.a * 0.25;
        }
    
        glColor4f( p->vColor.r, p->vColor.g, p->vColor.b, a_in );
        glVertex2f( p->vPos.x, p->vPos.y );

        glColor4f( p->vColor.r, p->vColor.g, p->vColor.b, a_out );
        for (i=0 ; i<flSize ; i++)
        {
            frac = (int)floor((float)i/flSize*360);
            offx = costbl[frac] * flSize;
            offy = sintbl[frac] * flSize;
            glVertex2f( p->vPos.x + offx, p->vPos.y + offy );
        }
        glVertex2f( p->vPos.x + costbl[0] * flSize, p->vPos.y + sintbl[0] * flSize );

        glEnd( );

        p = p->pNext;
    }
}

/*
===========================================================

Name    :   cRender::DrawLine

Purpose :   draws a line

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
