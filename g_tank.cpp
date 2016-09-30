/*
===============================================================================

Name	:	g_tank.cpp

Purpose	:	tanks!

===============================================================================
*/

#include "local.h"

#include "keys.h"

cTank::cTank ()
{
	memset( m_Keys,0,sizeof(m_Keys) );
	flDamage = 0;
	flLastFire = 0;
	flTAngle = 0;
	flTVel = 0;

	eType = object_tank;
}

void cTank::Draw ()
{
	float	flLerp = (g_Game->m_flTime - g_Game->m_nFramenum * 100) / 1000.0f;
	float	flTime = clamp((g_Game->m_flTime - flLastFire) / 250.f,0,20);
	float	flHealth = clamp((1-flDamage)*20,0,20);

	if (flDamage >= 1.0f)
	{
		g_Application->get_glWnd()->get_Render()->DrawModel( pModel, vPos + vVel * flLerp, flAngle + flAVel * flLerp, vec4(0.3,0.3,0.3,1) );
		g_Application->get_glWnd()->get_Render()->DrawModel( pTurret, vPos + vVel * flLerp, flTAngle + flTVel * flLerp, vec4(0.3,0.3,0.3,1) );
		return;
	}

	// status bars

	g_Application->get_glWnd()->get_Render()->DrawBox( vec2(20,2), vPos + vec2(0,24) + vVel * flLerp, 0, vec4(0.5,0.5,0.5,1) );
	g_Application->get_glWnd()->get_Render()->DrawBox( vec2(flTime,2), vPos + vec2(0,24) + vVel * flLerp, 0, vec4(1,1,0,1) );
	g_Application->get_glWnd()->get_Render()->DrawBox( vec2(20,2), vPos + vec2(0,22) + vVel * flLerp, 0, vec4(0.5,0.5,0.5,1) );
	g_Application->get_glWnd()->get_Render()->DrawBox( vec2(flHealth,2), vPos + vec2(0,22) + vVel * flLerp, 0, vec4(0,1,0,1) );

	// actual body

	g_Application->get_glWnd()->get_Render()->DrawModel( pModel, vPos + vVel * flLerp, flAngle + flAVel * flLerp, vColor );
	g_Application->get_glWnd()->get_Render()->DrawModel( pTurret, vPos + vVel * flLerp, flTAngle + flTVel * flLerp, vColor );
}

void cTank::Touch (cObject *pOther)
{
	return;
}

#define KEY_FORWARD	0
#define KEY_BACK	1
#define KEY_LEFT	2
#define KEY_RIGHT	3
#define KEY_TLEFT	4
#define KEY_TRIGHT	5
#define KEY_FIRE	6

void cTank::Think ()
{
	float	flForward;
	float	flLeft;
	float	flTLeft;

	flForward = m_Keys[KEY_FORWARD] - m_Keys[KEY_BACK];
	flLeft = m_Keys[KEY_LEFT] - m_Keys[KEY_RIGHT];
	flTLeft = m_Keys[KEY_TLEFT] - m_Keys[KEY_TRIGHT];

	if (flDamage >= 1.0f)
	{
		vVel = vVel * 0.9f;
		flAVel *= 0.9f;
		flTVel *= 0.9f;
	}
	else
	{
		vVel = vec2(flForward*64,0);
		vVel = vVel.rot( flAngle );

		flAVel = - flLeft * 90;

		flTVel = - flTLeft * 90;
	}

	// update position here because Move doesn't
	flTAngle += flTVel * FRAMETIME;

	if (flLastFire + 1500 > g_Game->m_flTime)	// just fired
	{
		vec2	vOrg(21,0);
		float	flPower;

		flPower = 1.5 - (g_Game->m_flTime - flLastFire)/1000.0f;

		g_World->AddSmokeEffect(
			vPos + vOrg.rot(flTAngle), 
			vOrg.rot(flTAngle) * flPower * flPower * flPower * 2,
			flPower * flPower * 4 );
	}
	else if (flLastFire + 5000 < g_Game->m_flTime)	// can fire
	{
		// check for firing
		if (m_Keys[KEY_FIRE] && flDamage < 1.0f)
		{
			vec2	vOrg(21,0);

			g_World->AddSmokeEffect( 
				vPos + vOrg.rot(flTAngle), 
				vOrg.rot(flTAngle) * 16,
				32 );

			flLastFire = g_Game->m_flTime;

			m_Bullet.vPos = vPos + vOrg.rot(flTAngle);
			m_Bullet.vVel = vOrg.rot(flTAngle) * 64;

			g_World->AddObject( &m_Bullet );
		}
	}
}

void cTank::UpdateKeys (int nKey, bool Down)
{
	switch ( nKey )
	{
	case 'w':
	case 'W':
	case K_UPARROW:
		m_Keys[KEY_FORWARD] = Down;
		break;

	case 's':
	case 'S':
	case K_DOWNARROW:
		m_Keys[KEY_BACK] = Down;
		break;

	case 'a':
	case 'A':
	case K_LEFTARROW:
		m_Keys[KEY_LEFT] = Down;
		break;

	case 'd':
	case 'D':
	case K_RIGHTARROW:
		m_Keys[KEY_RIGHT] = Down;
		break;

	case 'f':
	case 'F':
	case '4':
	case K_KP_LEFTARROW:
		m_Keys[KEY_TLEFT] = Down;
		break;

	case 'h':
	case 'H':
	case '6':
	case K_KP_RIGHTARROW:
		m_Keys[KEY_TRIGHT] = Down;
		break;

	case 'g':
	case 'G':
	case '5':
	case K_KP_5:
		m_Keys[KEY_FIRE] = Down;
		break;
	}
}

/*
===========================================================

Name	:	cBullet

===========================================================
*/

cBullet::cBullet ()
{
	flAngle = 0;
	flAVel = 0;

	vVel = vec2(0,0);
	pModel = NULL;

	eType = object_bullet;
}

void cBullet::Touch (cObject *pOther)
{
	g_World->AddEffect( vPos, effect_explosion );

	g_World->DelObject( this );

	if (!pOther)
		return;

	if ( pOther->eType == object_tank )
	{
		cTank	*pTank = static_cast<cTank *>(pOther);

		if (pTank->flDamage >= 1.0f)
			return;	// dont add any more score

		pTank->flDamage += 1.0f;

		if (pTank->flDamage >= 1.0f)
			g_Game->AddScore( pTank->nPlayerNum ^ 1, 1 );
	}
}

void cBullet::Draw ()
{
	float	flLerp = (g_Game->m_flTime - g_Game->m_nFramenum * 100) / 1000.0f;
	cParticle	p;

	memset( &p, 0, sizeof(p) );

	p.vPos = vPos + vVel * flLerp;
	p.vColor = vec4(1,0.5,0,1);
	p.flSize = 2.0f;
#if 0
	g_Application->get_glWnd()->get_Render()->DrawParticles( &p );
#else
	g_Application->get_glWnd()->get_Render()->DrawLine( vPos + vVel * (flLerp + 0.02), vPos + vVel * flLerp, vec4(1,0.5,0,1), vec4(1,0.5,0,0) );
#endif
}

void cBullet::Think ()
{
	return;
}
