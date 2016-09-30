/*
===============================================================================

Name	:	g_particles.cpp

Purpose	:	handles cParticle and cWorld particle handling

===============================================================================
*/

#include "local.h"

cParticle *cWorld::AddParticle ()
{
	cParticle	*pRet = pFreeParticles;

	if (pRet == NULL)
		return NULL;

	pRet->AddToActive ();
	pRet->flTime = g_Game->m_flTime;
	return pRet;
}

void cWorld::FreeParticle (cParticle *pParticle)
{
	pParticle->AddToFree( );
}

void cParticle::AddToActive ()
{
	g_World->pFreeParticles = pNext;
	pNext = g_World->pActiveParticles;
	g_World->pActiveParticles = this;
}

void cParticle::AddToFree ()
{
	memset( this, 0, sizeof(cParticle) );
	pNext = g_World->pFreeParticles;
	g_World->pFreeParticles = this;
}

void cWorld::m_DrawParticles ()
{
	cParticle	*p, *next;
	cParticle	*active, *tail;
	float		time;

	active = NULL;
	tail = NULL;

	next = pActiveParticles;
	for (p = pActiveParticles ; p ; p=next )
	{
		next = p->pNext;

		if (g_Game->m_flTime < p->flTime)
		{
			p->pNext = NULL;
			if (!tail)
				active = tail = p;
			else
			{
				tail->pNext = p;
				tail = p;
			}
			continue;
		}

		time = (g_Game->m_flTime - p->flTime)*0.001f;
		p->vColor.a += p->vColorVel.a * time;
		if (p->vColor.a <= 0.0f)
		{
			FreeParticle( p );
			continue;
		}

		p->pNext = NULL;
		if (!tail)
			active = tail = p;
		else
		{
			tail->pNext = p;
			tail = p;
		}

		p->vColor.r += p->vColorVel.r * time;
		p->vColor.g += p->vColorVel.g * time;
		p->vColor.b += p->vColorVel.b * time;

		p->vPos.x += p->vVel.x * time + p->vAccel.x * time * time;
		p->vPos.y += p->vVel.y * time + p->vAccel.y * time * time;

		if (p->flDrag)
			p->vVel = p->vVel * p->flDrag * (1.0f - time);

		p->flSize += p->flSizeVel * time;

		p->flTime = g_Game->m_flTime;
	}

	pActiveParticles = active;

	g_Application->get_glWnd()->get_Render()->DrawParticles( pActiveParticles );
}
	