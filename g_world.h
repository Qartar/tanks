/*
===============================================================================

Name    :   g_world.h

Purpose :   World Object

Date    :   10/21/2004

===============================================================================
*/

#pragma once

#include "g_usercmd.h"

#include "p_material.h"
#include "p_rigidbody.h"
#include "p_shape.h"

#include "r_model.h"
#include "r_particle.h"

#include <array>
#include <memory>
#include <set>
#include <type_traits>
#include <vector>

namespace physics {
struct contact;
}

namespace game {

class tank;
class world;

typedef struct game_client_s game_client_t;

enum class object_type
{
    object,
    obstacle,
    projectile,
    tank,
};

enum class effect_type
{
    smoke,
    sparks,
    explosion,
};

//------------------------------------------------------------------------------
class object
{
public:
    object(object_type type, object* owner = nullptr);
    ~object() {}

    std::size_t spawn_id() const { return _spawn_id; }

    virtual void draw() const;
    virtual void touch(object *other, physics::contact const* contact);
    virtual void think();

    //! Get frame-interpolated position
    virtual vec2 get_position(float lerp) const;

    //! Get frame-interpolated rotation
    virtual float get_rotation(float lerp) const;

    physics::rigid_body const& rigid_body() const { return _rigid_body; }

    void set_position(vec2 position, bool teleport = false);
    void set_rotation(float rotation, bool teleport = false);
    void set_linear_velocity(vec2 linear_velocity) { _rigid_body.set_linear_velocity(linear_velocity); }
    void set_angular_velocity(float angular_velocity) { _rigid_body.set_angular_velocity(angular_velocity); }

    vec2 get_position() const { return _rigid_body.get_position(); }
    float get_rotation() const { return _rigid_body.get_rotation(); }
    vec2 get_linear_velocity() const { return _rigid_body.get_linear_velocity(); }
    float get_angular_velocity() const { return _rigid_body.get_angular_velocity(); }

    void apply_impulse(vec2 impulse) { _rigid_body.apply_impulse(impulse); }
    void apply_impulse(vec2 impulse, vec2 position) { _rigid_body.apply_impulse(impulse, position); }

    render::model const* _model;
    vec4 _color;

    vec2 _old_position;
    float _old_rotation;

    object_type _type;

protected:
    friend world;

    //! Game world which contains this object
    world* _world;

    object* _owner;

    std::size_t _spawn_id;

    physics::rigid_body _rigid_body;

    static physics::material _default_material;
    static physics::circle_shape _default_shape;
    constexpr static float _default_mass = 1.0f;
};

//------------------------------------------------------------------------------
class obstacle : public object
{
public:
    obstacle(physics::rigid_body&& rigid_body)
        : object(object_type::obstacle)
    {
        _rigid_body = std::move(rigid_body);
    }
};

//------------------------------------------------------------------------------
class projectile : public object
{
public:
    projectile(tank* owner, float damage);

    virtual void draw() const override;
    virtual void touch(object *other, physics::contact const* contact) override;

    static physics::circle_shape _shape;
    static physics::material _material;

protected:
    float _damage;

    int _sound_explode;
};

//------------------------------------------------------------------------------
class tank : public object
{
public:
    tank();
    ~tank();

    virtual void draw() const override;
    virtual void touch(object *other, physics::contact const* contact) override;
    virtual void think() override;

    //! Get frame-interpolated turret rotation
    float get_turret_rotation(float lerp) const;

    void set_turret_rotation(float rotation, bool teleport = false);
    void set_turret_velocity(float velocity) { _turret_velocity = velocity; }
    float get_turret_rotation() const { return _turret_rotation; }
    float get_turret_velocity() const { return _turret_velocity; }

    void update_usercmd(game::usercmd usercmd);

    void update_sound();

    char const* player_name() const;

    render::model const* _turret_model;
    float _turret_rotation;
    float _turret_velocity;
    float _old_turret_rotation;

    float _track_speed;

    float _damage;
    int _player_index;

    float _dead_time;
    float _fire_time;

    game::usercmd _usercmd;

    std::array<sndchan_t*,3> _channels;
    game_client_t* _client;

protected:
    void collide(tank* other, physics::contact const* contact);

protected:
    static physics::material _material;
    static physics::box_shape _shape;

    int _sound_idle;
    int _sound_move;
    int _sound_turret_move;
    int _sound_fire;
    int _sound_explode;
};

//------------------------------------------------------------------------------
class world
{
public:
    world ()
        : _spawn_id(0)
        , _border_material{0,0}
        , _border_shapes{{vec2(0,0)}, {vec2(0,0)}}
    {}
    ~world () {}

    void init();
    void shutdown();
    void reset();

    void clear_particles();

    void run_frame ();
    void draw() const;

    template<typename T, typename... Args>
    T* spawn(Args&& ...args);

    void remove(object* object);

    void add_sound(int sound_index);
    void add_effect(effect_type type, vec2 position, vec2 direction = vec2(0,0), float strength = 1);

    vec2 mins() const { return _mins; }
    vec2 maxs() const { return _maxs; }

private:
    //! Active objects in the world
    std::vector<std::unique_ptr<object>> _objects;

    //! Objects pending addition
    std::vector<std::unique_ptr<object>> _pending;

    //! Objects pending removal
    std::set<object*> _removed;

    //! Previous object spawn id
    std::size_t _spawn_id;

    void move_object(object* object);

    //
    // particle system
    //

    bool _use_particles;

    mutable std::vector<render::particle> _particles;

    render::particle* add_particle();
    void free_particle (render::particle* particle) const;

    void draw_particles() const;

    vec2        _mins;
    vec2        _maxs;

    physics::material _border_material;
    physics::box_shape _border_shapes[2];
    constexpr static int _border_thickness = 512;
};

//------------------------------------------------------------------------------
template<typename T, typename... Args>
T* world::spawn(Args&& ...args)
{
    static_assert(std::is_base_of<game::object, T>::value,
                  "'spawn': 'T' must be derived from 'game::object'");

    _pending.push_back(std::make_unique<T>(std::move(args)...));
    _pending.back()->_world = this;
    _pending.back()->_spawn_id = ++_spawn_id;
    return static_cast<T*>(_pending.back().get());
}

} // namespace game
