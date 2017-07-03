/*
===============================================================================

Name    :   g_world.h

Purpose :   World Object

Date    :   10/21/2004

===============================================================================
*/

#pragma once

#include <memory>

#include "p_material.h"
#include "p_rigidbody.h"
#include "p_shape.h"

#define MAX_OBJECTS 16

typedef enum eEffects effects_t;

class cParticle;
class cModel;
class cGame;
class cWorld;

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

    //! Get frame-interpolated position
    virtual vec2 get_position(float lerp) const;

    //! Get frame-interpolated rotation
    virtual float get_rotation(float lerp) const;

    physics::rigid_body const& rigid_body() const { return _rigid_body; }

    void set_position(vec2 position) { _rigid_body.set_position(position); }
    void set_rotation(float rotation) { _rigid_body.set_rotation(rotation); }
    void set_linear_velocity(vec2 linear_velocity) { _rigid_body.set_linear_velocity(linear_velocity); }
    void set_angular_velocity(float angular_velocity) { _rigid_body.set_angular_velocity(angular_velocity); }

    vec2 get_position() const { return _rigid_body.get_position(); }
    float get_rotation() const { return _rigid_body.get_rotation(); }
    vec2 get_linear_velocity() const { return _rigid_body.get_linear_velocity(); }
    float get_angular_velocity() const { return _rigid_body.get_angular_velocity(); }

    void apply_impulse(vec2 impulse) { _rigid_body.apply_impulse(impulse); }
    void apply_impulse(vec2 impulse, vec2 position) { _rigid_body.apply_impulse(impulse, position); }

    cModel  *pModel;
    vec4    vColor;

    vec2    oldPos;
    float   oldAngle;

    eObjectType eType;

protected:
    physics::rigid_body _rigid_body;

    static physics::material _default_material;
    static physics::circle_shape _default_shape;
    constexpr static float _default_mass = 1.0f;
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

    static physics::circle_shape _shape;
    static physics::material _material;

    float _damage;
};

class cTank : public cObject
{
public:
    cTank ();
    ~cTank () {}

    virtual void    Draw ();
    virtual void    Touch (cObject *pOther, float impulse = 0) override;
    virtual void    Think ();

    //! Get frame-interpolated turret rotation
    float get_turret_rotation(float lerp) const;

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

protected:
    static physics::material _material;
    static physics::box_shape _shape;
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

    vec2        m_vWorldMins;
    vec2        m_vWorldMaxs;

    physics::material _border_material;
    physics::box_shape _border_shapes[2];
    cObject _border_objects[4];
    constexpr static int _border_thickness = 512;
};

extern cWorld *g_World;
