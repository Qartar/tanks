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
	float	flLerp = (g_Game->m_flTime - g_Game->m_nFramenum * FRAMEMSEC) / 1000.0f;
	float	flTime = clamp((g_Game->m_flTime - flLastFire) / 150.f,0,20);
	float	flHealth = clamp((1-flDamage)*20,0,20);

	vec4	vHealth;
	vec4	vTime;

	vHealth.r = ( flHealth < 10.0f ? 1.0f : (10.0f - (flHealth - 10.0f)) / 10.0f );
	vHealth.g = ( flHealth > 10.0f ? 1.0f : (flHealth / 10.0f) );
	vHealth.b = 0.0f;
	vHealth.a = 1.0f;

	vTime.r = 1.0f;
	vTime.g = flTime/20.0f;
	vTime.b = ( flTime == 20.0f ? 1.0f : 0.0f );
	vTime.a = 1.0f;

	if (flDamage >= 1.0f)
	{
		g_Render->DrawModel( pModel, vPos + vVel * flLerp, flAngle + flAVel * flLerp, vec4(0.3f,0.3f,0.3f,1) );
		g_Render->DrawModel( pTurret, vPos + vVel * flLerp, flTAngle + flTVel * flLerp, vec4(0.3f,0.3f,0.3f,1) );
		return;
	}

	// status bars

	g_Render->DrawBox( vec2(20,2), vPos + vec2(0,24) + vVel * flLerp, 0, vec4(0.5,0.5,0.5,1) );
	g_Render->DrawBox( vec2(flTime,2), vPos + vec2(0,24) + vVel * flLerp, 0, vTime );
	g_Render->DrawBox( vec2(20,2), vPos + vec2(0,22) + vVel * flLerp, 0, vec4(0.5,0.5,0.5,1) );
	g_Render->DrawBox( vec2(flHealth,2), vPos + vec2(0,22) + vVel * flLerp, 0, vHealth );

	// actual body

	g_Render->DrawModel( pModel, vPos + vVel * flLerp, flAngle + flAVel * flLerp, vColor );
	g_Render->DrawModel( pTurret, vPos + vVel * flLerp, flTAngle + flTVel * flLerp, vColor );
}

void cTank::Touch (cObject *pOther)
{
	if ( !pOther )
		return;

	if ( pOther->eType == object_tank )
	{
		if ( flDamage < 1.0f )
		{
			flDamage += (1.0f / 3.0f) * FRAMETIME;

			if (flDamage >= 1.0f)
			{
				g_Game->AddScore( ((cTank *)pOther)->nPlayerNum, 1 );
				flDeadTime = g_Game->m_flTime;
				if (g_Game->bAutoRestart)
					g_Game->flRestartTime = g_Game->m_flTime + RESTART_TIME;

				g_Game->m_WriteMessage( va("%s got a little too cozy with %s.", g_Game->m_clients[nPlayerNum].name, g_Game->m_clients[((cTank *)pOther)->nPlayerNum].name ) );
			}
		}

		if ( ((cTank *)pOther)->flDamage < 1.0f )
		{
			((cTank *)pOther)->flDamage += (1.0f / 3.0f) * FRAMETIME;

			if (((cTank *)pOther)->flDamage >= 1.0f)
			{
				g_Game->AddScore( nPlayerNum, 1 );
				((cTank *)pOther)->flDeadTime = g_Game->m_flTime;
				if (g_Game->bAutoRestart)
					g_Game->flRestartTime = g_Game->m_flTime + RESTART_TIME;

				g_Game->m_WriteMessage( va("%s got a little too cozy with %s.", g_Game->m_clients[((cTank *)pOther)->nPlayerNum].name, g_Game->m_clients[nPlayerNum].name ) );
			}
		}

	}

	return;
}

#define HACK_TIME		1000.0f

void cTank::Think ()
{
	float	flForward;
	float	flLeft;
	float	flTLeft;
	float	flVel;

	if ( (flDamage >= 1.0f) && g_Game->m_bMultiserver && (flDeadTime+RESTART_TIME+HACK_TIME <= g_Game->m_flTime) )
	{
		// respawn

		int	nWidth = g_Application->get_glWnd()->get_WndParams().nSize[0];
		int	nHeight = g_Application->get_glWnd()->get_WndParams().nSize[1];

		nWidth -= SPAWN_BUFFER*2;
		nHeight -= SPAWN_BUFFER*2;

		flDeadTime = 0.0f;

		vPos.x = nWidth*frand()+SPAWN_BUFFER;
		vPos.y = nHeight*frand()+SPAWN_BUFFER;
		flAngle = frand()*360;
		flTAngle = flAngle;

		vVel = vec2(0,0);
		flAVel = 0;
		flTVel = 0;

		flDamage = 0.0f;
	}

	flForward = m_Keys[KEY_FORWARD] - m_Keys[KEY_BACK];
	flLeft = m_Keys[KEY_LEFT] - m_Keys[KEY_RIGHT];
	flTLeft = m_Keys[KEY_TLEFT] - m_Keys[KEY_TRIGHT];

	flVel = sqrt(vVel.x * vVel.x + vVel.y * vVel.y);
	if ((sin(DEG2RAD(flAngle))*vVel.y) + ((cos(DEG2RAD(flAngle))*vVel.x)) <= 0)
		flVel = flVel * -1;	// forward/backward

	if (flDamage >= 1.0f)
	{
		vVel = vec2(flVel * 0.98 * (1-FRAMETIME),0);
		vVel = vVel.rot( flAngle );

		flAVel *= 0.9f;
		flTVel *= 0.9f;

		// extra explosion
		if (flDeadTime && (g_Game->m_flTime - flDeadTime > 350) && (g_Game->m_flTime - flDeadTime < 350+HACK_TIME/2))
		{
			g_World->AddEffect( vPos, effect_explosion );
			flDeadTime -= HACK_TIME;	// dont do it again
		}
	}
	else
	{
		flVel = flVel * 0.9 * (1-FRAMETIME) + flForward * 256 * FRAMETIME;
		flVel = clamp(flVel,-48,64);
		vVel = vec2(flVel,0);	
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
	else if (flLastFire + 3000 < g_Game->m_flTime)	// can fire
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
			m_Bullet.vVel = vOrg.rot(flTAngle) * 96;

			g_World->AddObject( &m_Bullet );
			m_Bullet.bInGame = true;
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

#ifndef M_SQRT1_2
#define M_SQRT1_2  0.707106781186547524401
#endif // M_SQRT1_2

void cBullet::Touch (cObject *pOther)
{
	g_World->AddEffect( vPos, effect_explosion );

	g_World->DelObject( this );
	bInGame = false;

	if (!pOther)
		return;

	if ( pOther->eType == object_tank )
	{
		cTank	*pTank = static_cast<cTank *>(pOther);
		vec2	vOrg, vDir;
		float	flDot;

		if (pTank->flDamage >= 1.0f)
			return;	// dont add any more score

		vOrg = vVel * (-1/(sqrt(vVel.x * vVel.x + vVel.y * vVel.y)));
		vDir = vec2(1,0);
		vDir = vDir.rot(pOther->flAngle);
		flDot = (vOrg.x*vDir.x + vOrg.y*vDir.y);

		if (!g_Game->bExtendedArmor && !g_Game->m_bMultiplayer)
			pTank->flDamage += 1.0f;
		else if (flDot > M_SQRT1_2)		// sin(45°)
			pTank->flDamage += 0.34f;	// round up, 3 shot kill
		else if (flDot > -M_SQRT1_2)
			pTank->flDamage += 0.5f;
		else
	        pTank->flDamage += 1.0f;

		if (pTank->flDamage >= 1.0f)
		{
			g_Game->AddScore( nPlayer, 1 );
			pTank->flDeadTime = g_Game->m_flTime;
			if (g_Game->bAutoRestart && !g_Game->m_bMultiserver)
				g_Game->flRestartTime = g_Game->m_flTime + RESTART_TIME;

			int r = rand()%3;
			switch ( r )
			{
			case 0:
				g_Game->m_WriteMessage( va("%s couldn't take %s's HEAT.", g_Game->m_clients[pTank->nPlayerNum].name, g_Game->m_clients[nPlayer].name ) );
				break;
			case 1:
				g_Game->m_WriteMessage( va("%s was on the wrong end of %s's cannon.", g_Game->m_clients[pTank->nPlayerNum].name, g_Game->m_clients[nPlayer].name ) );
				break;
			case 2:
				g_Game->m_WriteMessage( va("%s ate all 125mm of %s's boom stick.", g_Game->m_clients[pTank->nPlayerNum].name, g_Game->m_clients[nPlayer].name ) );
				break;
			}
		}
	}
}

void cBullet::Draw ()
{
	float	flLerp = (g_Game->m_flTime - g_Game->m_nFramenum * FRAMEMSEC) / 1000.0f;

	g_Render->DrawLine( vPos + vVel * (flLerp + 0.02), vPos + vVel * flLerp, vec4(1,0.5,0,1), vec4(1,0.5,0,0) );
}

void cBullet::Think ()
{
	return;
}
