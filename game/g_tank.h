// g_ship.h
//

#pragma once

#include "g_object.h"

////////////////////////////////////////////////////////////////////////////////
namespace game {

static constexpr float DAMAGE_FRONT = 0.334f;
static constexpr float DAMAGE_SIDE = 0.5f;
static constexpr float DAMAGE_REAR = 1.0f;
static constexpr float DAMAGE_FULL = 1.0f;

//------------------------------------------------------------------------------
class tank : public object
{
public:
    tank();
    ~tank();

    virtual void draw(render::system* renderer, time_value time) const override;
    virtual bool touch(object *other, physics::collision const* collision) override;
    virtual void think() override;

    virtual void read_snapshot(network::message const& message) override;
    virtual void write_snapshot(network::message& message) const override;

    //! Get frame-interpolated turret rotation
    float get_turret_rotation(time_value time) const;

    void set_turret_rotation(float rotation, bool teleport = false);
    void set_turret_velocity(float velocity) { _turret_velocity = velocity; }
    float get_turret_rotation() const { return _turret_rotation; }
    float get_turret_velocity() const { return _turret_velocity; }

    void respawn();

    void update_usercmd(game::usercmd usercmd);

    void update_sound();

    string::view player_name() const;

    render::model const* _turret_model;
    float _turret_rotation;
    float _turret_velocity;
    float _old_turret_rotation;

    float _track_speed;

    float _damage;
    std::size_t _player_index;

    time_value _dead_time;
    time_value _fire_time;

    game::usercmd _usercmd;

    std::array<sound::channel*,3> _channels;
    game_client_t* _client;

    weapon_type _weapon;

protected:
    void collide(tank* other, physics::collision const* collision);

    void launch_projectile();

    void update_effects();

protected:
    static physics::material _material;
    static physics::box_shape _shape;

    constexpr static float cannon_speed = 1920.0f;
    constexpr static float missile_speed = 288.0f;
    constexpr static float blaster_speed = 768.0f;

    constexpr static time_delta cannon_reload = time_delta::from_seconds(3.0f);
    constexpr static time_delta missile_reload = time_delta::from_seconds(6.0f);
    constexpr static time_delta blaster_reload = time_delta::from_seconds(0.3f);

    sound::asset _sound_idle;
    sound::asset _sound_move;
    sound::asset _sound_turret_move;
    sound::asset _sound_explode;
    sound::asset _sound_blaster_fire;
    sound::asset _sound_cannon_fire;
};

} // namespace game
