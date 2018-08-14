// g_projectile.h
//

#pragma once

#include "g_object.h"

////////////////////////////////////////////////////////////////////////////////
namespace game {

//------------------------------------------------------------------------------
enum class weapon_type
{
    cannon,
    missile,
    blaster,
};

//------------------------------------------------------------------------------
class projectile : public object
{
public:
    projectile(tank* owner, float damage, weapon_type type);
    ~projectile();

    virtual void draw(render::system* renderer, time_value time) const override;
    virtual bool touch(object *other, physics::contact const* contact) override;
    virtual void think() override;

    virtual void read_snapshot(network::message const& message) override;
    virtual void write_snapshot(network::message& message) const override;

    float damage() const { return _damage; }

    static physics::circle_shape _shape;
    static physics::material _material;

protected:
    float _damage;

    weapon_type _type;

    sound::channel* _channel;

    sound::asset _sound_cannon_impact;
    sound::asset _sound_blaster_impact;
    sound::asset _sound_missile_flight;

protected:
    void update_homing();
    void update_effects();
    void update_sound();
};

} // namespace game
