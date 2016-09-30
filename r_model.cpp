/*
===============================================================================

Name	:	r_model.cpp

Purpose	:	Compound rectangle model class

===============================================================================
*/

#include "local.h"

void cRender::DrawModel (cModel *pModel, vec2 vPos, float flAngle, vec4 vColor)
{
	int			i;
	sRect		*pRect;

	glPushMatrix();

	glTranslatef( vPos.x, vPos.y, 0 );
	glRotatef( flAngle, 0, 0, 1 );	// YAW

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
	int		i;
	sRect	*lpRect;

	m_AbsMax = vec2(0,0);
	m_AbsMin = vec2(0,0);

	m_List.numRects = lpList->numRects;
	m_List.lpRects = lpList->lpRects;		// beware! shallow copy

	// find absolute bounds
	for (i=0 ; i<m_List.numRects ; i++)
	{
		lpRect = &m_List.lpRects[i];

		if (lpRect->nPosX + lpRect->nSizeX / 2 > m_AbsMax.x)		// xmax
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
	vec2	Vert[4];
	vec2	AbsMax(0,0), AbsMin(0,0);

	if (!lpOther)
		return ClipPoint( vPos );

	// rotate corners

	Vert[0] = vec2(lpOther->m_AbsMin.x, lpOther->m_AbsMin.y);
	Vert[0] = Vert[0].rot( flAngle );

	Vert[1] = vec2(lpOther->m_AbsMax.x, lpOther->m_AbsMin.y);
	Vert[1] = Vert[1].rot( flAngle );

	Vert[2] = vec2(lpOther->m_AbsMax.x, lpOther->m_AbsMax.y);
	Vert[2] = Vert[2].rot( flAngle );

	Vert[3] = vec2(lpOther->m_AbsMin.x, lpOther->m_AbsMax.y);
	Vert[3] = Vert[3].rot( flAngle );

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