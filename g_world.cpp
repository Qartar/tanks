/*
===============================================================================

Name	:	g_world.cpp

Purpose	:	World Object

Date	:	10/21/2004

===============================================================================
*/

#include "local.h"

cWorld *g_World;

/*
===========================================================

Name	:	cWorld::Init

Purpose	:	Initializes world

===========================================================
*/

void cWorld::Init ()
{
	memset( m_Objects, 0, sizeof( m_Objects) );
	m_ClearParticles ();
	g_World = this;
}

void cWorld::Shutdown ()
{
}

void cWorld::Reset ()
{
}

/*
===========================================================

Name	:	cWorld::AddObject / ::DelObject

Purpose	:	adds and removes objects from the object list

===========================================================
*/

void cWorld::AddObject (cObject *newObject)
{
	int			i;

	for (i=0 ; i<MAX_OBJECTS ; i++)
	{
		if (!m_Objects[i])
		{
			m_Objects[i] = newObject;
			break;
		}
	}
}

void cWorld::DelObject (cObject *oldObject)
{
	int			i;

	for (i=0 ; i<MAX_OBJECTS ; i++)
	{
		if (m_Objects[i] == oldObject)
		{
			m_Objects[i] = NULL;
			break;
		}
	}
}

/*
===========================================================

Name	:	cWorld::Draw

Purpose	:	Renders the world to the screen

===========================================================
*/

void cWorld::Draw ()
{
	int			i;

	for (i=0 ; i<MAX_OBJECTS ; i++)
	{
		if (!m_Objects[i])
			continue;

		m_Objects[i]->Draw( );
	}

	m_DrawParticles( );
}

/*
===========================================================

Name	:	cWorld::RunFrame

Purpose	:	Runs the world one frame, runs think funcs and movement

===========================================================
*/

void cWorld::RunFrame ()
{
	int			i, j;

	for (i=0 ; i<MAX_OBJECTS ; i++)
	{
		if (!m_Objects[i])
			continue;

		m_Objects[i]->Think( );
	}

	for (i=0 ; i<MAX_OBJECTS ; i++)
	{
		if (!m_Objects[i])
			continue;

		MoveObject( m_Objects[i] );
	}
}

/*
===========================================================

Name	:	cWorld::MoveObject

Purpose	:	moves an object in the world according to its velocity

===========================================================
*/

#define NUM_STEPS	8
#define STEP_SIZE	0.125

void cWorld::MoveObject (cObject *pObject)
{
	vec2	vOldPos;
	float	flOldAngle;

	vec2	vDelta;
	float	flEpsilon;

	int			i;

	vOldPos = pObject->vPos;
	flOldAngle = pObject->flAngle;

	pObject->vPos = pObject->vPos + pObject->vVel * FRAMETIME;
	pObject->flAngle += pObject->flAVel * FRAMETIME;

	for (i=0 ; i<MAX_OBJECTS ; i++)
	{
		if (!m_Objects[i])
			continue;

		if (m_Objects[i] == pObject)
			continue;

		vDelta = m_Objects[i]->vPos - pObject->vPos;

		vDelta = vDelta.rot( - pObject->flAngle );

		if (!pObject->pModel && m_Objects[i]->pModel)
		{
			vec2	vStep;
			pObject->vPos = vOldPos;

			vStep = pObject->vVel * FRAMETIME * STEP_SIZE;
			for (int j=0 ; j<NUM_STEPS ; j++)
			{
				pObject->vPos = pObject->vPos + vStep;

				vDelta = pObject->vPos - m_Objects[i]->vPos;
				vDelta = vDelta.rot( -m_Objects[i]->flAngle );

				if (m_Objects[i]->pModel->ClipPoint( vDelta ))
				{
					pObject->Touch( m_Objects[i] );
					return;
				}
			}
		}
		else if (!pObject->pModel && !m_Objects[i]->pModel)
			continue;
		else if (pObject->pModel->Clip( m_Objects[i]->pModel, vDelta, m_Objects[i]->flAngle ) )
		{
			// ghetto action : simply remove all movement, add sparks

			AddEffect( pObject->vPos + vDelta * 0.5, effect_sparks );	// add spark effect

			pObject->vPos = vOldPos;
			pObject->vVel = vec2(0,0);

			vDelta = vDelta.rot( pObject->flAngle );	// rotate angle back

			pObject->Touch( m_Objects[i] );

			return;
		}
	}

	// check for collision with window bounds
	if (pObject->vPos.x < 0)
	{
		pObject->vPos.x = 0;
		pObject->vVel.x = 0;

		pObject->Touch( NULL );
	}
	else if (pObject->vPos.x > g_Application->get_glWnd()->get_WndParams().nSize[0])
	{
		pObject->vPos.x = g_Application->get_glWnd()->get_WndParams().nSize[0];
		pObject->vVel.x = 0;

		pObject->Touch( NULL );
	}

	if (pObject->vPos.y < 0)
	{
		pObject->vPos.y = 0;
		pObject->vVel.y = 0;

		pObject->Touch( NULL );
	}
	else if (pObject->vPos.y > g_Application->get_glWnd()->get_WndParams().nSize[1])
	{
		pObject->vPos.y = g_Application->get_glWnd()->get_WndParams().nSize[1];
		pObject->vVel.y = 0;

		pObject->Touch( NULL );
	}
}

/*
===========================================================

Name	:	cWorld::AddEffect

Purpose	:	adds particle effects

===========================================================
*/

void cWorld::AddEffect (vec2 vPos, eEffects eType)
{
	if (eType == effect_sparks)
	{
		int		i;
		cParticle	*p;

		for (i=0 ; i<2 ; i++)
		{
			if ( (p = AddParticle()) == NULL )
				return;

			p->vPos = vPos + vec2(crand()*2,crand()*2);
			p->vVel = vec2(crand()*128,crand()*128);

			p->vColor = vec4(1,0.5+frand()*0.5,0,1);
			p->vColorVel = vec4(0,0,0,-2);
			p->flSize = 2.0f;
			p->flSizeVel = 0.0f;
		}
	}
	else if (eType == effect_explosion)
	{
		int		i;
		cParticle	*p;

		// smoke

		for (i=0 ; i<96 ; i++)
		{
			if ( (p = AddParticle()) == NULL )
				return;

			p->vPos = vPos + vec2(crand()*8,crand()*8);
			p->vVel = vec2(crand()*128,crand()*128);

			p->vColor = vec4(1.0,frand(),0.0,0.1);
			p->vColorVel = vec4(0,0,0,-p->vColor.a/(0.2+frand()*frand()*2.0f));
			p->flSize = 8.0 + frand()*16.0f;
			p->flSizeVel = 1.0f;

			p->flDrag = 0.95 - frand()*0.03;
		}

		// debris

		for (i=0 ; i<16 ; i++)
		{
			if ( (p = AddParticle()) == NULL )
				return;

			p->vPos = vPos + vec2(crand()*2,crand()*2);
			p->vVel = vec2(crand()*128,crand()*128);

			p->vColor = vec4(1,0.5+frand()*0.5,0,1);
			p->vColorVel = vec4(0,0,0,-2);
			p->flSize = 2.0f;
			p->flSizeVel = 0.0f;
		}
	}
}

void cWorld::AddSmokeEffect (vec2 vPos, vec2 vVel, int nCount)
{
	int			i;
	cParticle	*p;

	for (i=0 ; i<nCount ; i++)
	{
		if ( (p = AddParticle()) == NULL )
			return;

		p->vPos = vPos + vec2(crand(),crand());
		p->vVel = vVel * (0.25 + frand()*0.75) + vec2(crand()*24,crand()*24);

		p->flSize = 4.0f + frand()*8.0f;
		p->flSizeVel = 2.0;

		p->vColor = vec4(0.5,0.5,0.5,0.1+frand()*0.1f);
		p->vColorVel = vec4(0,0,0,-p->vColor.a / (1+frand()*0.5f));

		p->flDrag = 0.98f - frand()*0.05;
	}
}

void cWorld::m_ClearParticles ()
{
	int		i;
	
	memset (&m_Particles, 0, sizeof(m_Particles));

	pFreeParticles = &m_Particles[0];
	pActiveParticles = NULL;

	for (i=0 ;i<MAX_PARTICLES ; i++)
		m_Particles[i].pNext = &m_Particles[i+1];
	m_Particles[MAX_PARTICLES-1].pNext = NULL;
}

