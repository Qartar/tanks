/*
===============================================================================

Name	:	g_world.cpp

Purpose	:	World Object

Date	:	10/21/2004

===============================================================================
*/

#include "local.h"
#pragma hdrstop

cWorld *g_World;

/*
===========================================================

Name	:	cWorld::Init

Purpose	:	Initializes world

===========================================================
*/

extern cvar_t	*g_arenaWidth;
extern cvar_t	*g_arenaHeight;

void cWorld::Init ()
{
	char	*command;

	memset( m_Objects, 0, sizeof( m_Objects) );
	ClearParticles ();
	g_World = this;

	m_vWorldMins = vec2(0,0);
	m_vWorldMaxs = vec2(
		g_arenaWidth->getInt( ),
		g_arenaHeight->getInt( ) );

	if ( (command = strstr( g_Application->InitString(), "particles=" )) )
		m_bParticles = ( atoi(command+10) > 0 );
	else
		m_bParticles = true;

	m_bWeakFX = false;	// obsolete
}

void cWorld::Shutdown ()
{
}

void cWorld::Reset ()
{
	m_vWorldMins = vec2(0,0);
	m_vWorldMaxs = vec2(
		g_arenaWidth->getInt( ),
		g_arenaHeight->getInt( ) );

	for ( int i=0 ; i<MAX_OBJECTS ; i++ )
		if ( m_Objects[i] )
			DelObject( m_Objects[i] );
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
	int			i;

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

int	segs[4][2] = {
	{ 0, 1 },
	{ 1, 2 },
	{ 2, 3 },
	{ 3, 0 } };

bool clipModelToModel (cObject *a, cObject *b)
{
	vec3	av[5];
	vec3	bv[5];

	vec2	amin, amax;
	vec2	bmin, bmax;

	cLine2	line[2];

	mat3	mat;

	if ( (b->vPos.x - a->vPos.x)*(b->vPos.x - a->vPos.x) + (b->vPos.y - a->vPos.y)*(b->vPos.y - a->vPos.y) > 32*32 )
		return false;

	mat.rotateyaw( deg2rad( a->flAngle ) );

	av[4] = vec3(a->vPos.x,a->vPos.y,0);
	av[0] = mat.mult( vec3( a->pModel->m_AbsMin.x, a->pModel->m_AbsMin.y, 0 ) ) + av[4];
	av[1] = mat.mult( vec3( a->pModel->m_AbsMax.x, a->pModel->m_AbsMin.y, 0 ) ) + av[4];
	av[2] = mat.mult( vec3( a->pModel->m_AbsMax.x, a->pModel->m_AbsMax.y, 0 ) ) + av[4];
	av[3] = mat.mult( vec3( a->pModel->m_AbsMin.x, a->pModel->m_AbsMax.y, 0 ) ) + av[4];

	bv[4] = vec3(b->vPos.x,b->vPos.y,0);
	bv[0] = mat.mult( vec3( b->pModel->m_AbsMin.x, b->pModel->m_AbsMin.y, 0 ) ) + bv[4];
	bv[1] = mat.mult( vec3( b->pModel->m_AbsMax.x, b->pModel->m_AbsMin.y, 0 ) ) + bv[4];
	bv[2] = mat.mult( vec3( b->pModel->m_AbsMax.x, b->pModel->m_AbsMax.y, 0 ) ) + bv[4];
	bv[3] = mat.mult( vec3( b->pModel->m_AbsMin.x, b->pModel->m_AbsMax.y, 0 ) ) + bv[4];

	for ( int i=0 ; i<4 ; i++ )
	{
		for ( int j=0 ; j<4 ; j++ )
		{
			line[0] = cLine2( vec2(av[segs[i][0]].x, av[segs[i][0]].y),
				vec2(av[segs[i][1]].x,av[segs[i][1]].y), true );
			line[1] = cLine2( vec2(bv[segs[j][0]].x, bv[segs[j][0]].y),
				vec2(bv[segs[j][1]].x,bv[segs[j][1]].y), true );

			if ( line[0].intersect( line[1] ) )
				return true;
		}
	}

	return false;
}

bool clipModelToSegment(cObject *a, vec2 b, vec2 c, vec2 *out)
{
	vec3	av[5];

	float	d, dist = 999999;
	vec2	final;

	cLine2	line[2];

	mat3	mat;

	bool	hit = false;

	mat.rotateyaw( deg2rad( a->flAngle ) );

	av[4] = vec3(a->vPos.x,a->vPos.y,0);
	av[0] = mat.mult( vec3( a->pModel->m_AbsMin.x, a->pModel->m_AbsMin.y, 0 ) ) + av[4];
	av[1] = mat.mult( vec3( a->pModel->m_AbsMax.x, a->pModel->m_AbsMin.y, 0 ) ) + av[4];
	av[2] = mat.mult( vec3( a->pModel->m_AbsMax.x, a->pModel->m_AbsMax.y, 0 ) ) + av[4];
	av[3] = mat.mult( vec3( a->pModel->m_AbsMin.x, a->pModel->m_AbsMax.y, 0 ) ) + av[4];

	line[1] = cLine2( b, c, true );
	for ( int i=0 ; i<4 ; i++ )
	{
		line[0] = cLine2( vec2(av[segs[i][0]].x, av[segs[i][0]].y),
			vec2(av[segs[i][1]].x,av[segs[i][1]].y), true );

		if ( line[0].intersect( line[1], out ) )
		{
			hit = true;
			if ( (d = ( *out - b ).length()) < dist )
			{
				final = *out;
				dist = d;
			}
		}
	}

	if ( hit )
	{
		*out = final;
		return true;
	}

	return false;
}

bool clipSegmentToSegment (vec2 a, vec2 b, vec2 c, vec2 d, vec2 *out)
{
	cLine2	line[2];

	line[0] = cLine2(a,b,true);
	line[1] = cLine2(c,d,true);

	return line[0].intersect( line[1], out );
}

void cWorld::MoveObject (cObject *pObject)
{
	vec2	vOldPos;
	float	flOldAngle;

	vec2	vDelta;

	int			i;

	pObject->oldPos = pObject->vPos;
	pObject->oldAngle = pObject->flAngle;

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

		if ( !pObject->pModel && m_Objects[i]->pModel )
		{
			if ( ( m_Objects[i]->eType == object_tank ) &&
				&((cTank *)m_Objects[i])->m_Bullet == pObject )
				continue;

			if ( clipModelToSegment( m_Objects[i], vOldPos, pObject->vPos, &vDelta ) )
			{
				pObject->vPos = vDelta;
				pObject->Touch( m_Objects[i] );
			}
		}
		else if ( pObject->pModel && m_Objects[i]->pModel
			&& clipModelToModel( pObject, m_Objects[i] ) )
		{
			// ghetto action : simply remove all movement, add sparks

			AddEffect( pObject->vPos + (m_Objects[i]->vPos - pObject->vPos)/2, effect_sparks );	// add spark effect

			pObject->vPos = vOldPos;
			pObject->vVel = vec2(0,0);

			pObject->Touch( m_Objects[i] );

			return;
		}
	}

	// check for collision with window bounds
	if (pObject->vPos.x < m_vWorldMins.x)
	{
		if (!pObject->pModel)
			pObject->vPos.y = vOldPos.y + ( (pObject->vPos.y - vOldPos.y) / (pObject->vPos.x - vOldPos.x) * (m_vWorldMins.x - vOldPos.x) );

		pObject->vPos.x = m_vWorldMins.x;
		pObject->vVel.x = 0;

		pObject->Touch( NULL );
	}
	else if (pObject->vPos.x > m_vWorldMaxs.x)
	{
		if (!pObject->pModel)
			pObject->vPos.y = vOldPos.y + ( (pObject->vPos.y - vOldPos.y) / (pObject->vPos.x - vOldPos.x) * (m_vWorldMaxs.x - vOldPos.x) );

		pObject->vPos.x = m_vWorldMaxs.x;
		pObject->vVel.x = 0;

		pObject->Touch( NULL );
	}

	if (pObject->vPos.y < m_vWorldMins.y)
	{
		if (!pObject->pModel)
			pObject->vPos.x = vOldPos.x + ( (pObject->vPos.x - vOldPos.x) / (pObject->vPos.y - vOldPos.y) * (m_vWorldMins.y - vOldPos.y) );

		pObject->vPos.y = m_vWorldMins.y;
		pObject->vVel.y = 0;

		pObject->Touch( NULL );
	}
	else if (pObject->vPos.y > m_vWorldMaxs.y)
	{
		if (!pObject->pModel)
			pObject->vPos.x = vOldPos.x + ( (pObject->vPos.x - vOldPos.x) / (pObject->vPos.y - vOldPos.y) * (m_vWorldMaxs.y - vOldPos.y) );

		pObject->vPos.y = m_vWorldMaxs.y;
		pObject->vVel.y = 0;

		pObject->Touch( NULL );
	}
}

/*
===========================================================

Name	:	cWorld::AddSound

Purpose	:	sound!

===========================================================
*/

void cWorld::AddSound (char *szName)
{
	for ( int i=0 ; i<NUM_SOUNDS ; i++ )
	{
		if ( sound_index[i].name && stricmp( szName, sound_index[i].name ) == 0 )
		{
			g_Game->m_WriteSound( sound_index[i].index );
			pSound->playSound( sound_index[i].index, vec3(0,0,0), 1.0f, 0.0f );
		}
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
	g_Game->m_WriteEffect( eType, vPos, vec2(0,0), 0 );

	float	r, d;

	if (eType == effect_sparks)
	{
		int		i;
		cParticle	*p;

		if ( m_bWeakFX )
			return;

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

		// shock wave

		if ( (p = AddParticle()) == NULL )
			return;

		p->vPos = vPos;
		p->vVel = vec2(0,0);

		p->vColor = vec4(1.0f,1.0f,0.5f,0.5f);
		p->vColorVel = vec4(0,0,0,-p->vColor.a/(0.3f));
		p->flSize = 24.0;
		p->flSizeVel = 192.0f;
		p->bitFlags = PF_INVERT;

		p->flDrag = 0.95 - frand()*0.03;

		// smoke

		for (i=0 ; i<128 ; i++)
		{
			if ( (p = AddParticle()) == NULL )
				return;

			r = frand()*M_PI*2.0f;
			d = frand()*24;

			p->vPos = vPos + vec2(cos(r)*d,sin(r)*d);

			r = frand()*M_PI*2.0f;
			d = frand()*24;

			p->vVel = vec2(cos(r)*d,sin(r)*d);

			p->flSize = 4.0f + frand()*8.0f;
			p->flSizeVel = 2.0;

			p->vColor = vec4(0.5,0.5,0.5,0.1+frand()*0.1f);
			p->vColorVel = vec4(0,0,0,-p->vColor.a / (2+frand()*1.5f));

			p->flDrag = 0.98f - frand()*0.05;
		}

		// fire

		for (i=0 ; i<96 ; i++)
		{
			if ( (p = AddParticle()) == NULL )
				return;

			r = frand()*M_PI*2.0f;
			d = frand()*16;

			p->vPos = vPos + vec2(cos(r)*d,sin(r)*d);

			r = frand()*M_PI*2.0f;
			d = frand()*128;

			p->vVel = vec2(cos(r)*d,sin(r)*d);

			p->vColor = vec4(1.0f,frand(),0.0f,0.1f);
			p->vColorVel = vec4(0,0,0,-p->vColor.a/(0.5+frand()*frand()*2.5f));
			p->flSize = 8.0 + frand()*16.0f;
			p->flSizeVel = 1.0f;

			p->flDrag = 0.95 - frand()*0.03;
		}

		// debris

		for (i=0 ; i<16 ; i++)
		{
			if ( (p = AddParticle()) == NULL )
				return;

			r = frand()*M_PI*2.0f;
			d = frand()*2;

			p->vPos = vPos + vec2(cos(r)*d,sin(r)*d);

			r = frand()*M_PI*2.0f;
			d = frand()*128;

			p->vVel = vec2(cos(r)*d,sin(r)*d);

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
	float		r, d;
	cParticle	*p;

	g_Game->m_WriteEffect( effect_smoke, vPos, vVel, nCount );

	if ( m_bWeakFX )
		return;

	for (i=0 ; i<nCount ; i++)
	{
		if ( (p = AddParticle()) == NULL )
			return;

		r = frand()*M_PI*2.0f;
		d = frand();
		p->vPos = vPos + vec2(cos(r)*d,sin(r)*d);
		p->vVel = vVel * (0.25 + frand()*0.75) + vec2(crand()*24,crand()*24);

		p->flSize = 4.0f + frand()*8.0f;
		p->flSizeVel = 2.0;

		p->vColor = vec4(0.5,0.5,0.5,0.1+frand()*0.1f);
		p->vColorVel = vec4(0,0,0,-p->vColor.a / (1+frand()*1.0f));

		p->flDrag = 0.98f - frand()*0.05;
	}
}


void cWorld::AddFlagTrail (vec2 vPos, int nTeam)
{
	int			i;
	cParticle	*p;

//	g_Game->m_WriteEffect( effect_smoke, vPos, vVel, nCount );

	if ( m_bWeakFX )
		return;

	for (i=0 ; i<4 ; i++)
	{
		if ( (p = AddParticle()) == NULL )
			return;

		p->vPos = vPos + vec2(crand()*4,crand()*4);
		p->vVel = vec2(crand()*24,crand()*24);

		p->flSize = 4.0f + frand()*8.0f;
		p->flSizeVel = 2.0;

		if ( nTeam == 0 )	// red
			p->vColor = vec4(1.0,0.0,0.0,0.1+frand()*0.1f);
		else if ( nTeam == 1 )	// blue
			p->vColor = vec4(0.0,0.0,1.0,0.1+frand()*0.1f);

		p->vColorVel = vec4(0,0,0,-p->vColor.a / (1.0+frand()*0.5f));

		p->flDrag = 0.98f - frand()*0.05;
	}
}

void cWorld::ClearParticles ()
{
	int		i;
	
	memset (&m_Particles, 0, sizeof(m_Particles));

	pFreeParticles = &m_Particles[0];
	pActiveParticles = NULL;

	for (i=0 ;i<MAX_PARTICLES ; i++)
		m_Particles[i].pNext = &m_Particles[i+1];
	m_Particles[MAX_PARTICLES-1].pNext = NULL;
}

