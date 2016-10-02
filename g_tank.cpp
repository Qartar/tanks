/*
===============================================================================

Name    :   g_tank.cpp

Purpose :   tanks!

===============================================================================
*/

#include "local.h"
#pragma hdrstop

#include "keys.h"

cTank::cTank ()
{
    memset( m_Keys,0,sizeof(m_Keys) );
    flDamage = 0;
    flLastFire = 0;
    flTAngle = 0;
    flTVel = 0;

    channels[0] = NULL;
    channels[1] = NULL;
    channels[2] = NULL;
   
    eType = object_tank;
}

void cTank::Draw ()
{
    float   flLerp = (g_Game->m_flTime - (g_Game->m_nFramenum-1) * FRAMEMSEC) / FRAMEMSEC;
    float   flTime = clamp(((g_Game->m_flTime - flLastFire) / 150.f) * client->refire_mod,0,20);
    float   flHealth = clamp((1-flDamage)*20,0,20);

    if ( flLerp > 1.0f ) {
        flLerp = 1.0f;
    } else if ( flLerp < 0.0f ) {
        flLerp = 0.0f;
    }

    vec2    pos = GetPos( flLerp );
    float   angle = GetAngle( flLerp );
    float   tangle = GetTAngle( flLerp );

    vec4    vHealth;
    vec4    vTime;

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
        g_Render->DrawModel( pModel, pos, angle, vec4(0.3f,0.3f,0.3f,1) );
        g_Render->DrawModel( pTurret, pos, tangle, vec4(0.3f,0.3f,0.3f,1) );
        return;
    }

    // status bars

    g_Render->DrawBox( vec2(20,2), pos + vec2(0,24), 0, vec4(0.5,0.5,0.5,1) );
    g_Render->DrawBox( vec2(flTime,2), pos + vec2(0,24), 0, vTime );
    g_Render->DrawBox( vec2(20,2), pos + vec2(0,22), 0, vec4(0.5,0.5,0.5,1) );
    g_Render->DrawBox( vec2(flHealth,2), pos + vec2(0,22), 0, vHealth );

    // actual body

    g_Render->DrawModel( pModel, pos, angle, vColor );
    g_Render->DrawModel( pTurret, pos, tangle, vColor );
}

void cTank::Touch (cObject *pOther)
{
    if ( !pOther )
        return;

    if ( pOther->eType == object_tank )
    {
        if ( flDamage < 1.0f )
        {
            flDamage += (1.0f / 3.0f) * FRAMETIME / client->armor_mod;

            if (flDamage >= 1.0f)
            {
                char    player[LONG_STRING];
                char    target[LONG_STRING];

                fmt( player, "\\c%02x%02x%02x%s\\cx", (int )(((cTank *)pOther)->vColor.r * 255), (int )(((cTank *)pOther)->vColor.g * 255), (int )(((cTank *)pOther)->vColor.b * 255),
                    g_Game->svs.clients[((cTank *)pOther)->nPlayerNum].name );
                fmt( target, "\\c%02x%02x%02x%s\\cx", (int )(vColor.r * 255), (int )(vColor.g * 255), (int )(vColor.b * 255),
                    g_Game->svs.clients[nPlayerNum].name );

                g_Game->AddScore( ((cTank *)pOther)->nPlayerNum, 1 );
                flDeadTime = g_Game->m_flTime;
                if (g_Game->bAutoRestart && !g_Game->m_bMultiplayer)
                    g_Game->flRestartTime = g_Game->m_flTime + RESTART_TIME;

                g_Game->m_WriteMessage( va("%s got a little too cozy with %s.", target, player ) );
            }
        }

        if ( ((cTank *)pOther)->flDamage < 1.0f )
        {
            ((cTank *)pOther)->flDamage += (1.0f / 3.0f) * FRAMETIME / ((cTank *)pOther)->client->armor_mod;

            if (((cTank *)pOther)->flDamage >= 1.0f)
            {
                char    player[LONG_STRING];
                char    target[LONG_STRING];

                fmt( target, "\\c%02x%02x%02x%s\\cx", (int )(((cTank *)pOther)->vColor.r * 255), (int )(((cTank *)pOther)->vColor.g * 255), (int )(((cTank *)pOther)->vColor.b * 255),
                    g_Game->svs.clients[((cTank *)pOther)->nPlayerNum].name );
                fmt( player, "\\c%02x%02x%02x%s\\cx", (int )(vColor.r * 255), (int )(vColor.g * 255), (int )(vColor.b * 255),
                    g_Game->svs.clients[nPlayerNum].name );

                g_Game->AddScore( nPlayerNum, 1 );
                ((cTank *)pOther)->flDeadTime = g_Game->m_flTime;
                if (g_Game->bAutoRestart && !g_Game->m_bMultiplayer)
                    g_Game->flRestartTime = g_Game->m_flTime + RESTART_TIME;

                g_Game->m_WriteMessage( va("%s got a little too cozy with %s.", target, player ) );
            }
        }

    }

    return;
}

#define HACK_TIME       1000.0f

void cTank::Think ()
{
    float   flForward;
    float   flLeft;
    float   flTLeft;
    float   flVel;

    oldTAngle = flTAngle;

    if ( (flDamage >= 1.0f) && g_Game->m_bMultiserver && (flDeadTime+RESTART_TIME+HACK_TIME <= g_Game->m_flTime) )
    {
        // respawn

        int nWidth = g_World->m_vWorldMaxs.x - g_World->m_vWorldMins.x;
        int nHeight = g_World->m_vWorldMaxs.y - g_World->m_vWorldMins.y;

        nWidth -= SPAWN_BUFFER*2;
        nHeight -= SPAWN_BUFFER*2;

        flDeadTime = 0.0f;

        vPos.x = g_World->m_vWorldMins.x + nWidth*frand()+SPAWN_BUFFER;
        vPos.y = g_World->m_vWorldMins.y + nHeight*frand()+SPAWN_BUFFER;
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
        flVel = flVel * -1; // forward/backward

    if (flDamage >= 1.0f)
    {
        vVel = vec2(flVel * 0.98 * (1-FRAMETIME),0);
        vVel = rot(vVel,flAngle );

        flAVel *= 0.9f;
        flTVel *= 0.9f;

        // extra explosion
        if (flDeadTime && (g_Game->m_flTime - flDeadTime > 650) && (g_Game->m_flTime - flDeadTime < 650+HACK_TIME/2))
        {
            g_World->AddSound( "SOUND/TANK_EXPLODE.wav" );
            g_World->AddEffect( vPos, effect_explosion );
            flDeadTime -= HACK_TIME;    // dont do it again
        }
    }
    else
    {
        flVel = flVel * 0.9 * (1-FRAMETIME) + flForward * 192*client->speed_mod * FRAMETIME;
        flVel = clamp(flVel,-32*client->speed_mod,48*client->speed_mod);
        vVel = vec2(flVel,0);   
        vVel = rot(vVel,flAngle );

        flAVel = - flLeft * 90;

        flTVel = - flTLeft * 90;
    }

    // update position here because Move doesn't
    flTAngle += flTVel * FRAMETIME;

    if ((flLastFire + 2500/client->refire_mod) > g_Game->m_flTime)  // just fired
    {
        vec2    vOrg(21,0);
        float   flPower;

        flPower = 1.5 - (g_Game->m_flTime - flLastFire)/1000.0f;
        flPower = clamp( flPower, 0.5, 1.5 );

        g_World->AddSmokeEffect(
            vPos + rot(vOrg,flTAngle), 
            rot(vOrg,flTAngle) * flPower * flPower * flPower * 2,
            flPower * flPower * 4 );
    }
    else if ((flLastFire + 3000/client->refire_mod) < g_Game->m_flTime) // can fire
    {
        // check for firing
        if (m_Keys[KEY_FIRE] && flDamage < 1.0f)
        {
            vec2    vOrg(21,0);

            g_World->AddSmokeEffect( 
                vPos + rot(vOrg,flTAngle), 
                rot(vOrg,flTAngle) * 16,
                64 );

            flLastFire = g_Game->m_flTime;

            m_Bullet.vPos = vPos + rot(vOrg,flTAngle);
            m_Bullet.vVel = rot(vOrg,flTAngle) * 96;

            g_World->AddSound( sound_index[TANK_FIRE].name );
            g_World->AddObject( &m_Bullet );
            m_Bullet.bInGame = true;
        }
    }

    UpdateSound( );
}

float cTank::GetTAngle( float lerp ) {
    return oldTAngle + ( flTAngle - oldTAngle ) * lerp;
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

    case 'j':
    case 'J':
    case '4':
    case K_KP_LEFTARROW:
        m_Keys[KEY_TLEFT] = Down;
        break;

    case 'l':
    case 'L':
    case '6':
    case K_KP_RIGHTARROW:
        m_Keys[KEY_TRIGHT] = Down;
        break;

    case 'k':
    case 'K':
    case '5':
    case K_KP_5:
        m_Keys[KEY_FIRE] = Down;
        break;
    }
}

void cTank::UpdateSound ()
{
    for ( int i=0 ; i<3 ; i++ )
        if ( channels[i] == NULL )
            if ( (channels[i] = pSound->allocChan( )) == NULL )
                return;

    // engine noise
    if ( flDamage < 1 )
    {
        if ( !channels[0]->isPlaying( ) )
            channels[0]->playLoop( sound_index[TANK_IDLE].index );

        channels[0]->setVolume( 1.0f );
        channels[0]->setAttenuation( 0.0f );
        channels[0]->setFrequency( 1.0f );
    }
    else if ( channels[0]->isPlaying( ) )
        channels[0]->stopSound( );

    // tread noise
    if ( (fabs( vVel.x ) > 1) || (fabs( vVel.y) > 1) || (fabs( flAVel ) > 1) )
    {
        if ( !channels[1]->isPlaying( ) )
            channels[1]->playLoop( sound_index[TANK_MOVE].index );

        channels[1]->setVolume( 1.0f );
        channels[1]->setAttenuation( 0.0f );
    }
    else if ( channels[1]->isPlaying( ) )
        channels[1]->stopSound( );

    // turret noise
    if ( (fabs( flAVel - flTVel ) > 1) )
    {
        if ( !channels[2]->isPlaying( ) )
            channels[2]->playLoop( sound_index[TURRET_MOVE].index );

        channels[2]->setVolume( 1.0f );
        channels[2]->setAttenuation( 0.0f );
    }
    else if ( channels[2]->isPlaying( ) )
        channels[2]->stopSound( );
}

/*
===========================================================

Name    :   cBullet

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

#define DAMAGE_FRONT    0.334f
#define DAMAGE_SIDE     0.5f
#define DAMAGE_REAR     1.0f
#define DAMAGE_FULL     1.0f

void cBullet::Touch (cObject *pOther)
{
    g_World->AddSound( "SOUND/BULLET_EXPLODE.wav" );
    g_World->AddEffect( vPos, effect_explosion );

    g_World->DelObject( this );
    bInGame = false;

    if (!pOther)
        return;

    if ( pOther->eType == object_tank )
    {
        cTank   *pTank = static_cast<cTank *>(pOther);
        vec2    vOrg, vDir;
        float   flDot, damage;

        if (pTank->flDamage >= 1.0f)
            return; // dont add damage or score

        vOrg = vVel * (-1/(sqrt(vVel.x * vVel.x + vVel.y * vVel.y)));
        vDir = vec2(1,0);
        vDir = rot(vDir,pOther->flAngle);
        flDot = (vOrg.x*vDir.x + vOrg.y*vDir.y);

        damage = g_Game->gameClients[nPlayer].damage_mod / pTank->client->armor_mod;

        if (!g_Game->bExtendedArmor && !g_Game->m_bMultiplayer)
            pTank->flDamage += damage * DAMAGE_FULL;
        else if (flDot > M_SQRT1_2)     // sin(45°)
            pTank->flDamage += damage * DAMAGE_FRONT;   // round up, 3 shot kill
        else if (flDot > -M_SQRT1_2)
            pTank->flDamage += damage * DAMAGE_SIDE;
        else
            pTank->flDamage += damage * DAMAGE_REAR;

        if (pTank->flDamage >= 1.0f)
        {
            char    player[LONG_STRING];
            char    target[LONG_STRING];

            g_Game->AddScore( nPlayer, 1 );
            pTank->flDeadTime = g_Game->m_flTime;
            if (g_Game->bAutoRestart && !g_Game->m_bMultiplayer)
                g_Game->flRestartTime = g_Game->m_flTime + RESTART_TIME;

            fmt( target, "\\c%02x%02x%02x%s\\cx", (int )(pTank->vColor.r * 255), (int )(pTank->vColor.g * 255), (int )(pTank->vColor.b * 255),
                g_Game->svs.clients[pTank->nPlayerNum].name );
            fmt( player, "\\c%02x%02x%02x%s\\cx", (int )(g_Game->Player(nPlayer)->vColor.x * 255), (int )(g_Game->Player(nPlayer)->vColor.y * 255), (int )(g_Game->Player(nPlayer)->vColor.z * 255),
                g_Game->svs.clients[nPlayer].name );

            int r = rand()%3;
            switch ( r )
            {
            case 0:
                g_Game->m_WriteMessage( va("%s couldn't take %s's HEAT.", target, player ) );
                break;
            case 1:
                g_Game->m_WriteMessage( va("%s was on the wrong end of %s's cannon.", target, player ) );
                break;
            case 2:
                g_Game->m_WriteMessage( va("%s ate all 125mm of %s's boom stick.", target, player ) );
                break;
            }
        }
    }
}

void cBullet::Draw ()
{
    float   flLerp = (g_Game->m_flTime - (g_Game->m_nFramenum-1) * FRAMEMSEC) / FRAMEMSEC;
    vec2    p1, p2;

    p1 = GetPos( flLerp - 0.4f );
    p2 = GetPos( flLerp );

    g_Render->DrawLine( p2, p1, vec4(1,0.5,0,1), vec4(1,0.5,0,0) );
}

void cBullet::Think ()
{
    return;
}
