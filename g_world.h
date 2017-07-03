/*
===============================================================================

Name    :   g_world.h

Purpose :   World Object

Date    :   10/21/2004

===============================================================================
*/

#pragma once

#include "p_material.h"
#include "p_rigidbody.h"
#include "p_shape.h"

#include <memory>
#include <set>
#include <type_traits>
#include <vector>

typedef enum eEffects effects_t;

class cParticle;
class cModel;
class cGame;
class cTank;
class cWorld;

enum eObjectType
{
    object_object,
    object_static,
    object_bullet,
    object_tank,
};

//------------------------------------------------------------------------------
class cObject
{
public:
    cObject (eObjectType type, cObject* owner = nullptr);
    ~cObject () {}

    std::size_t spawn_id() const { return _spawn_id; }

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
    friend cWorld;

    cObject* _owner;

    std::size_t _spawn_id;

    physics::rigid_body _rigid_body;

    static physics::material _default_material;
    static physics::circle_shape _default_shape;
    constexpr static float _default_mass = 1.0f;
};

//------------------------------------------------------------------------------
class cStatic : public cObject
{
public:
    cStatic(physics::rigid_body&& rigid_body)
        : cObject(object_static)
    {
        _rigid_body = std::move(rigid_body);
    }
};

//------------------------------------------------------------------------------
class cBullet : public cObject
{
public:
    cBullet (cTank* owner, float damage);
    ~cBullet () {}

    virtual void    Draw ();
    virtual void    Touch (cObject *pOther, float impulse = 0) override;
    virtual void    Think ();

    static physics::circle_shape _shape;
    static physics::material _material;

protected:
    float _damage;
};

//------------------------------------------------------------------------------
class cTank : public cObject
{
public:
    cTank ();
    ~cTank ();

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

    sndchan_t   *channels[3];
    game_client_t   *client;

protected:
    static physics::material _material;
    static physics::box_shape _shape;
};

#define MAX_PARTICLES   4096

//------------------------------------------------------------------------------
class cWorld
{
    friend cParticle;
    friend cTank;
    friend cGame;

public:
    cWorld ()
        : pFreeParticles(NULL)
        , pActiveParticles(NULL)
        , _spawn_id(0)
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

    template<typename T, typename... Args>
    T* spawn(Args&& ...args);

    void remove(cObject* object);

    void    AddSound (char *szName);
    void    AddSmokeEffect (vec2 vPos, vec2 vVel, int nCount);
    void    AddEffect (vec2 vPos, eEffects eType, float strength = 1);

    void    AddFlagTrail (vec2 vPos, int nTeam);

private:
    //! Active objects in the world
    std::vector<std::unique_ptr<cObject>> _objects;

    //! Objects pending addition
    std::vector<std::unique_ptr<cObject>> _pending;

    //! Objects pending removal
    std::set<cObject*> _removed;

    //! Previous object spawn id
    std::size_t _spawn_id;

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
    constexpr static int _border_thickness = 512;
};

//------------------------------------------------------------------------------
template<typename T, typename... Args>
T* cWorld::spawn(Args&& ...args)
{
    static_assert(std::is_base_of<cObject, T>::value,
                  "'spawn': 'T' must be derived from 'cObject'");

    _pending.push_back(std::make_unique<T>(std::move(args)...));
    _pending.back()->_spawn_id = ++_spawn_id;
    return static_cast<T*>(_pending.back().get());
}

extern cWorld *g_World;
