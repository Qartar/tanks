/*
===============================================================================

Name    :   r_model.cpp

Purpose :   Compound rectangle model class

===============================================================================
*/

#include "local.h"
#pragma hdrstop

void cRender::DrawModel (cModel *pModel, vec2 vPos, float flAngle, vec4 vColor)
{
    int         i;
    sRect       *pRect;

    glMatrixMode( GL_MODELVIEW );
    glPushMatrix();

    glTranslatef( vPos.x, vPos.y, 0 );
    glRotatef( flAngle, 0, 0, 1 );  // YAW

    glBegin( GL_QUADS );

    for (i=0 ; i<pModel->m_List.numRects ; i++)
    {
        pRect = &pModel->m_List.lpRects[i];

        glColor4f( vColor.r * pRect->flGamma, vColor.g * pRect->flGamma, vColor.b * pRect->flGamma, 1.0 );
        glVertex2f( pRect->nPosX - pRect->nSizeX / 2, pRect->nPosY - pRect->nSizeY / 2 );
        glVertex2f( pRect->nPosX + pRect->nSizeX / 2, pRect->nPosY - pRect->nSizeY / 2 );
        glVertex2f( pRect->nPosX + pRect->nSizeX / 2, pRect->nPosY + pRect->nSizeY / 2 );
        glVertex2f( pRect->nPosX - pRect->nSizeX / 2, pRect->nPosY + pRect->nSizeY / 2 );
    }

    glEnd( );

    glPopMatrix();
}

cModel::cModel (sRectList *lpList)
{
    int     i;
    sRect   *lpRect;

    m_AbsMax = vec2(0,0);
    m_AbsMin = vec2(0,0);

    m_List.numRects = lpList->numRects;
    m_List.lpRects = lpList->lpRects;       // beware! shallow copy

    // find absolute bounds
    for (i=0 ; i<m_List.numRects ; i++)
    {
        lpRect = &m_List.lpRects[i];

        if (lpRect->nPosX + lpRect->nSizeX / 2 > m_AbsMax.x)        // xmax
            m_AbsMax.x = lpRect->nPosX + lpRect->nSizeX / 2;
        else if (lpRect->nPosX - lpRect->nSizeX / 2 < m_AbsMin.x)
            m_AbsMin.x = lpRect->nPosX - lpRect->nSizeX / 2;

        if (lpRect->nPosY + lpRect->nSizeY / 2 > m_AbsMax.y)
            m_AbsMax.y = lpRect->nPosY + lpRect->nSizeY / 2;
        else if (lpRect->nPosY - lpRect->nSizeY / 2 < m_AbsMin.y)
            m_AbsMin.y = lpRect->nPosY - lpRect->nSizeY / 2;
    }
}

bool cModel::Clip (cModel *lpOther, vec2 vPos, float flAngle)
{
    vec3    Vert[4];
    vec2    AbsMax(0,0), AbsMin(0,0);

    mat3    mat;

    if (!lpOther)
        return ClipPoint( vPos );

    // rotate corners

    mat.rotateyaw( deg2rad( flAngle ) );

    Vert[0] = mat.mult( vec3( lpOther->m_AbsMin.x, lpOther->m_AbsMin.y, 0 ) );
    Vert[1] = mat.mult( vec3( lpOther->m_AbsMax.x, lpOther->m_AbsMin.y, 0 ) );
    Vert[2] = mat.mult( vec3( lpOther->m_AbsMax.x, lpOther->m_AbsMax.y, 0 ) );
    Vert[3] = mat.mult( vec3( lpOther->m_AbsMin.x, lpOther->m_AbsMax.y, 0 ) );

    // find new absolutes

    for (int i = 0 ; i<4 ; i++)
    {
        if (Vert[i].x > AbsMax.x)
            AbsMax.x = Vert[i].x;
        else if (Vert[i].x < AbsMin.x)
            AbsMin.x = Vert[i].x;

        if (Vert[i].y > AbsMax.y)
            AbsMax.y = Vert[i].y;
        else if (Vert[i].y < AbsMin.y)
            AbsMin.y = Vert[i].y;
    }

    // add absolutes to relative position (vPos)

    AbsMax = AbsMax + vPos;
    AbsMin = AbsMin + vPos;

    // compare values

        return ( ! (
        (m_AbsMin.x > AbsMax.x || m_AbsMax.x < AbsMin.x) ||
        (m_AbsMin.y > AbsMax.y || m_AbsMax.y < AbsMin.y) ) );

    // no poly/poly checking - too code intensive and not necessary
}

bool cModel::ClipPoint (vec2 vPos)
{
    if ( !this )
        return false;

    return ( ! (
        (m_AbsMin.x > vPos.x || m_AbsMax.x < vPos.x) ||
        (m_AbsMin.y > vPos.y || m_AbsMax.y < vPos.y) ) );
}
