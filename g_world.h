/*
===============================================================================

Name    :   g_world.h

Purpose :   World Object

Date    :   10/21/2004

===============================================================================
*/

#ifndef __G_WORLD_H__
#define __G_WORLD_H__

#include <memory>

#include "p_material.h"
#include "p_rigidbody.h"
#include "p_shape.h"

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

    virtual void    Touch (cObject *pOther, float impulse = 0);
    virtual void    Think ();

    virtual vec2    GetPos( float lerp );
    virtual float   GetAngle( float lerp );

    cModel  *pModel;
    vec4    vColor;

    vec2    oldPos;
    float   oldAngle;

    eObjectType eType;

    std::unique_ptr<physics::material> _material;
    std::unique_ptr<physics::rigid_body> _rigid_body;
    std::unique_ptr<physics::shape> _shape;
};

class cBullet : public cObject
{
public:
    cBullet ();
    ~cBullet () {}

    virtual void    Draw ();
    virtual void    Touch (cObject *pOther, float impulse = 0) override;
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
    virtual void    Touch (cObject *pOther, float impulse = 0) override;
    virtual void    Think ();

    float           GetTAngle( float lerp );

    void        UpdateKeys( int nKey, bool Down );

    void        UpdateSound ();

    cModel  *pTurret;
    float   flTAngle, flTVel;
    float   flDamage;
    int     nPlayerNum;
    float   flDeadTime;

    float _track_speed;

    float   oldTAngle;

    bool    m_Keys[8];

    float   flLastFire;
    cBullet m_Bullet;

    sndchan_t   *channels[3];
    game_client_t   *client;
};

#define MAX_PARTICLES   4096

class cWorld
{
    friend cParticle;
    friend cTank;
    friend cGame;

public:
    cWorld ()
        : pFreeParticles(NULL)
        , pActiveParticles(NULL)
        , _border_material{0,0}
        , _border_shapes{{vec2(0,0)}, {vec2(0,0)}}
    {}
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
    void    AddEffect (vec2 vPos, eEffects eType, float strength = 1);

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

    physics::material _border_material;
    physics::box_shape _border_shapes[2];
    cObject _border_objects[4];
    constexpr static int _border_thickness = 512;
};

extern cWorld *g_World;

#endif //__G_WORLD_H__
