/*
===============================================================================

Name    :   g_world.h

Purpose :   World Object

Date    :   10/21/2004

===============================================================================
*/

#ifndef __G_WORLD_H__
#define __G_WORLD_H__

#define MAX_OBJECTS 16

typedef enum eEffects effects_t;

class cParticle;
class cModel;
class cGame;

enum eObjectType
{
    object_object,
    object_bullet,
    object_tank
};

class cObject
{
public:
    cObject ();
    ~cObject () {}

    virtual void    Draw ();

    virtual void    Touch (cObject *pOther);
    virtual void    Think ();

    virtual vec2    GetPos( float lerp );
    virtual float   GetAngle( float lerp );

    cModel  *pModel;
    vec2    vPos, vVel;         // state and rate of change
    float   flAngle, flAVel;    //
    vec4    vColor;

    vec2    oldPos;
    float   oldAngle;

    eObjectType eType;
};

class cBullet : public cObject
{
public:
    cBullet ();
    ~cBullet () {}

    virtual void    Draw ();
    virtual void    Touch (cObject *pOther);
    virtual void    Think ();

    bool    bInGame;
    int     nPlayer;
};

class cTank : public cObject
{
public:
    cTank ();
    ~cTank () {}

    virtual void    Draw ();
    virtual void    Touch (cObject *pOther);
    virtual void    Think ();

    float           GetTAngle( float lerp );

    void        UpdateKeys( int nKey, bool Down );

    void        UpdateSound ();

    cModel  *pTurret;
    float   flTAngle, flTVel;
    float   flDamage;
    int     nPlayerNum;
    float   flDeadTime;

    float   oldTAngle;

    bool    m_Keys[8];

    float   flLastFire;
    cBullet m_Bullet;

    sndchan_t   *channels[3];
    game_client_t   *client;
};

#define MAX_PARTICLES   1024

class cWorld
{
    friend cParticle;
    friend cTank;
    friend cGame;

public:
    cWorld () : pFreeParticles(NULL), pActiveParticles(NULL) {}
    ~cWorld () {}

    void    Init ();
    void    Shutdown ();
    void    Reset ();

    void    ClearParticles ();

    void    RunFrame ();
    void    Draw ();

    void    AddObject (cObject *newObject);
    void    DelObject (cObject *oldObject);

    void    AddSound (char *szName);
    void    AddSmokeEffect (vec2 vPos, vec2 vVel, int nCount);
    void    AddEffect (vec2 vPos, eEffects eType);

    void    AddFlagTrail (vec2 vPos, int nTeam);

private:
    cObject *m_Objects[MAX_OBJECTS];

    void    MoveObject (cObject *pObject);

    cParticle   *pFreeParticles;
    cParticle   *pActiveParticles;

    cParticle   m_Particles[MAX_PARTICLES];
    cParticle   *AddParticle ();
    void        FreeParticle (cParticle *pParticle);

    void        m_DrawParticles ();

    bool        m_bParticles;
    bool        m_bWeakFX;

    vec2        m_vWorldMins;
    vec2        m_vWorldMaxs;
};

extern cWorld *g_World;

#endif //__G_WORLD_H__
