// g_tank.cpp
//

#include "precompiled.h"
#pragma hdrstop

#include "g_tank.h"
#include "g_projectile.h"
#include "p_collide.h"

////////////////////////////////////////////////////////////////////////////////
namespace game {

physics::material tank::_material(0.5f, 1.0f, 5.0f);
physics::box_shape tank::_shape(vec2(24, 16));

//------------------------------------------------------------------------------
tank::tank()
    : object(object_type::tank)
    , _turret_model(nullptr)
    , _turret_rotation(0)
    , _turret_velocity(0)
    , _old_turret_rotation(0)
    , _track_speed(0)
    , _damage(0)
    , _player_index(0)
    , _dead_time(time_value::zero)
    , _fire_time(time_value::zero)
    , _usercmd{}
    , _channels{0}
    , _client(nullptr)
    , _weapon(weapon_type::cannon)
{
    _rigid_body = physics::rigid_body(&_shape, &_material, 1.0f);

    _model = &tank_body_model;
    _turret_model = &tank_turret_model;

    for (auto& chan : _channels) {
        chan = pSound->allocate_channel();
    }

    _sound_idle = pSound->load_sound("assets/sound/tank_idle.wav");
    _sound_move = pSound->load_sound("assets/sound/tank_move.wav");
    _sound_turret_move = pSound->load_sound("assets/sound/turret_move.wav");
    _sound_explode = pSound->load_sound("assets/sound/tank_explode.wav");
    _sound_blaster_fire = pSound->load_sound("assets/sound/blaster_fire.wav");
    _sound_cannon_fire = pSound->load_sound("assets/sound/cannon_fire.wav");
}

//------------------------------------------------------------------------------
tank::~tank()
{
    for (auto& chan : _channels) {
        if (chan) {
            chan->stop();
            pSound->free_channel(chan);
        }
    }
}

//------------------------------------------------------------------------------
void tank::draw(render::system* renderer, time_value time) const
{
    float lerp = (time - _world->framenum() * FRAMETIME) / FRAMETIME;

    time_delta denominator = _weapon == weapon_type::cannon ? cannon_reload :
                             _weapon == weapon_type::missile ? missile_reload :
                             _weapon == weapon_type::blaster ? blaster_reload : time_delta::from_seconds(1);

    float reload = 20.0f * clamp((time - _fire_time) * _client->refire_mod / denominator, 0.0f, 1.0f);
    float health = 20.0f * clamp(1.0f - _damage, 0.0f, 1.0f);

    if ( lerp > 1.0f ) {
        lerp = 1.0f;
    } else if ( lerp < 0.0f ) {
        lerp = 0.0f;
    }

    vec2 pos = get_position(time);
    float angle = get_rotation(time);
    float tangle = get_turret_rotation(time);

    color4 color_health;
    color4 color_reload;

    color_health.r = ( health < 10.0f ? 1.0f : (10.0f - (health - 10.0f)) / 10.0f );
    color_health.g = ( health > 10.0f ? 1.0f : (health / 10.0f) );
    color_health.b = 0.0f;
    color_health.a = 1.0f;

    color_reload.r = 1.0f;
    color_reload.g = reload/20.0f;
    color_reload.b = ( reload == 20.0f ? 1.0f : 0.0f );
    color_reload.a = 1.0f;

    if (_damage >= 1.0f)
    {
        renderer->draw_model(_model, mat3::transform(pos, angle), color4(0.3f,0.3f,0.3f,1));
        renderer->draw_model(_turret_model, mat3::transform(pos, tangle), color4(0.3f,0.3f,0.3f,1));
        return;
    }

    // status bars

    renderer->draw_box(vec2(20,2), pos + vec2(0,25), color4(0.5,0.5,0.5,1));
    renderer->draw_box(vec2(reload,2), pos + vec2(0,25), color_reload);
    renderer->draw_box(vec2(20,2), pos + vec2(0,22), color4(0.5,0.5,0.5,1));
    renderer->draw_box(vec2(health,2), pos + vec2(0,22), color_health);

    // actual body

    renderer->draw_model(_model, mat3::transform(pos, angle), _color);
    renderer->draw_model(_turret_model, mat3::transform(pos, tangle), _color);
}

//------------------------------------------------------------------------------
bool tank::touch(object *other, physics::collision const* collision)
{
    if (!other || !collision) {
        return false;
    }

    float impulse = collision->impulse.length();
    float strength = clamp((impulse - 5.0f) / 5.0f, 0.0f, 1.0f);
    _world->add_effect(effect_type::sparks, collision->point, -collision->normal, strength);

    if (other->_type == object_type::projectile) {
        return true;
    } else if (other->_type != object_type::tank) {
        return false;
    }

    tank* other_tank = static_cast<tank*>(other);

    this->collide(other_tank, collision);
    other_tank->collide(this, collision);
    return true;
}

//------------------------------------------------------------------------------
void tank::collide(tank* other, physics::collision const* collision)
{
    if (other->_damage >= 1.0f) {
        return;
    }

    float base_damage = std::max<float>(0, collision->impulse.dot(collision->normal) - 10.0f) / 2.0f * FRAMETIME.to_seconds();

    vec2 direction = (collision->point - other->get_position()).normalize();
    vec2 forward = rotate(vec2(1,0), other->get_rotation());
    float impact_angle = direction.dot(forward);

    if (impact_angle > .5f * math::sqrt2<float>) {
        base_damage *= DAMAGE_FRONT;
    } else if (impact_angle > -.5f * math::sqrt2<float>) {
        base_damage *= DAMAGE_SIDE;
    } else {
        base_damage *= DAMAGE_REAR;
    }

    other->_damage += base_damage / other->_client->armor_mod;

    if (other->_damage >= 1.0f)
    {
        g_Game->add_score(_player_index, 1);
        other->_dead_time = _world->frametime();

        g_Game->write_message( va("%s got a little too cozy with %s.", other->player_name().c_str(), player_name().c_str() ) );
    }
}

#define HACK_TIME       time_delta::from_seconds(1.0f)

//------------------------------------------------------------------------------
void tank::respawn()
{
    vec2 spawn_size = _world->maxs() - _world->mins() - vec2(SPAWN_BUFFER) * 2.0f;
    vec2 spawn_pos = _world->mins() + spawn_size * mat2::scale(_random.uniform_real(), _random.uniform_real()) + vec2(SPAWN_BUFFER);

    _dead_time = time_value::zero;

    set_position(spawn_pos, true);
    set_rotation(_random.uniform_real(2.f * math::pi<float>), true);
    set_turret_rotation(get_rotation(), true);

    set_linear_velocity(vec2_zero);
    set_angular_velocity(0.0f);
    _turret_velocity = 0.0f;
    _track_speed = 0.0f;

    _damage = 0.0f;
}

//------------------------------------------------------------------------------
void tank::think()
{
    time_value const time = _world->frametime();

    _old_turret_rotation = _turret_rotation;

    if ((_damage >= 1.0f) && (_dead_time+RESTART_TIME+HACK_TIME <= time)) {
        respawn();
    }

    vec2 forward = rotate(vec2(1,0), get_rotation());
    float speed = forward.dot(get_linear_velocity());

    vec2 track_velocity = rotate(vec2(_track_speed,0), get_rotation());
    vec2 delta_velocity = track_velocity - get_linear_velocity();
    vec2 friction_impulse = delta_velocity * _rigid_body.get_mass() * _rigid_body.get_material()->sliding_friction() * FRAMETIME.to_seconds();

    _rigid_body.apply_impulse(friction_impulse);

    if (_damage >= 1.0f)
    {
        vec2 vVel = vec2(speed * 0.98f * (1-FRAMETIME.to_seconds()),0);

        set_linear_velocity(rotate(vVel,get_rotation()));
        set_angular_velocity(get_angular_velocity() * 0.9f);
        _turret_velocity *= 0.9f;

        // extra explosion
        if (_dead_time != time_value::zero && (time - _dead_time > time_delta::from_seconds(0.65f)) && (time - _dead_time < time_delta::from_seconds(0.65f)+HACK_TIME/2))
        {
            _world->add_sound(_sound_explode, get_position());
            _world->add_effect(effect_type::explosion, get_position());
            _dead_time -= HACK_TIME;    // dont do it again
        }
    }
    else
    {
        float new_speed = _track_speed * 0.9f * (1 - FRAMETIME.to_seconds()) + _usercmd.move[1] * 192 * _client->speed_mod * FRAMETIME.to_seconds();
        new_speed = clamp(new_speed, -32 * _client->speed_mod, 48 * _client->speed_mod);

        set_linear_velocity(get_linear_velocity() + forward * (new_speed - _track_speed));
        _track_speed = new_speed;

        set_angular_velocity(math::deg2rad(_usercmd.move[0] * 90));
        _turret_velocity = math::deg2rad(_usercmd.look[0] * 90);
    }

    // update position here because Move doesn't
    _turret_rotation += _turret_velocity * FRAMETIME.to_seconds();

    if (_usercmd.action == usercmd::action::attack && _damage < 1.0f) {
        launch_projectile();
    }

    update_effects();
    update_sound();
}

//------------------------------------------------------------------------------
void tank::launch_projectile()
{
    constexpr vec2 effect_origin(21,0);

    time_value const time = _world->frametime();

    switch (_weapon) {
        case weapon_type::cannon: {
            if ((_fire_time + cannon_reload / _client->refire_mod) < time) {
                _fire_time = time;

                projectile* proj = _world->spawn<projectile>(this, _client->damage_mod, weapon_type::cannon);

                float launch_rotation = _turret_rotation + _random.uniform_real(-1.f, 1.f) * math::pi<float> / 180.f;
                vec2 launch_direction = rotate(vec2(1, 0), launch_rotation);
                vec2 launch_position = get_position() + rotate(effect_origin, _turret_rotation);

                proj->set_position(launch_position, true);
                proj->set_linear_velocity(launch_direction * cannon_speed);

                _world->add_sound(_sound_cannon_fire, launch_position);
                _world->add_effect(
                    effect_type::smoke,
                    launch_position,
                    launch_direction * 21 * 16,
                    64 );

            }
            break;
        }

        case weapon_type::missile: {
            if ((_fire_time + missile_reload / _client->refire_mod) < time) {
                _fire_time = time;

                projectile* proj = _world->spawn<projectile>(this, _client->damage_mod, weapon_type::missile);

                vec2 launch_direction = rotate(vec2(1, 0), _turret_rotation);
                vec2 launch_position = get_position() + rotate(effect_origin, _turret_rotation);

                proj->set_position(launch_position, true);
                proj->set_linear_velocity(launch_direction * missile_speed);

                _world->add_sound(_sound_cannon_fire, launch_position);
            }
            break;
        }

        case weapon_type::blaster: {
            if ((_fire_time + blaster_reload / _client->refire_mod) < time) {
                _fire_time = time;

                projectile* proj = _world->spawn<projectile>(this, _client->damage_mod * 0.1f, weapon_type::blaster);

                float launch_rotation = _turret_rotation + _random.uniform_real(-1.f, 1.f) * math::pi<float> / 180.f;
                vec2 launch_direction = rotate(vec2(1, 0), launch_rotation);
                vec2 launch_position = get_position() + rotate(effect_origin, _turret_rotation);

                proj->set_position(launch_position, true);
                proj->set_linear_velocity(launch_direction * blaster_speed);

                _world->add_sound(_sound_blaster_fire, launch_position);

                _world->add_effect(
                    effect_type::blaster,
                    launch_position,
                    launch_direction * 2);
            }
            break;
        }

        default:
            break;
    }
}

//------------------------------------------------------------------------------
void tank::update_effects()
{
    constexpr vec2 effect_origin(21,0);
    time_value const time = _world->frametime();

    switch (_weapon) {
        case weapon_type::cannon: {
            if ((_fire_time + time_delta::from_seconds(2.5f)/_client->refire_mod) > time) {
                float power;

                power = 1.5f - (time - _fire_time).to_seconds();
                power = clamp(power, 0.5f, 1.5f);

                _world->add_effect(
                    effect_type::smoke,
                    get_position() + rotate(effect_origin, _turret_rotation),
                    rotate(effect_origin, _turret_rotation) * power * power * power * 2,
                    power * power * 4 );
            }
            break;
        }

        case weapon_type::missile: {
            if ((_fire_time + time_delta::from_seconds(2.5f)/_client->refire_mod) > time) {
                float power;

                power = 1.5f - (time - _fire_time).to_seconds();
                power = clamp(power, 0.5f, 1.5f);

                _world->add_effect(
                    effect_type::smoke,
                    get_position() + rotate(effect_origin, _turret_rotation),
                    rotate(effect_origin, _turret_rotation) * power * power * power * 2,
                    power * power * 2 );
            }
            break;
        }

        case weapon_type::blaster:
        default:
            break;
    }
}

//------------------------------------------------------------------------------
void tank::read_snapshot(network::message const& message)
{
    _old_position = get_position();
    _old_rotation = get_rotation();
    _old_turret_rotation = get_turret_rotation();

    _player_index = message.read_byte();
    _client = g_Game->_clients + _player_index;
    _world->_players[_player_index] = this;
    _color.r = message.read_float();
    _color.g = message.read_float();
    _color.b = message.read_float();
    set_position(message.read_vector());
    set_linear_velocity(message.read_vector());
    set_rotation(message.read_float());
    set_angular_velocity(message.read_float());
    _turret_rotation = message.read_float();
    _turret_velocity = message.read_float();
    _damage = message.read_float();
    _fire_time = time_value::from_seconds(message.read_float());

    update_sound();
}

//------------------------------------------------------------------------------
void tank::write_snapshot(network::message& message) const
{
    message.write_byte(narrow_cast<uint8_t>(_player_index));
    message.write_float(_color.r);
    message.write_float(_color.g);
    message.write_float(_color.b);
    message.write_vector(get_position());
    message.write_vector(get_linear_velocity());
    message.write_float(get_rotation());
    message.write_float(get_angular_velocity());
    message.write_float(_turret_rotation);
    message.write_float(_turret_velocity);
    message.write_float(_damage);
    message.write_float(_fire_time.to_seconds());
}

//------------------------------------------------------------------------------
float tank::get_turret_rotation(time_value time) const
{
    float lerp = (time - _world->frametime()) / FRAMETIME;
    return _old_turret_rotation + (_turret_rotation - _old_turret_rotation) * lerp;
}

//------------------------------------------------------------------------------
void tank::set_turret_rotation(float rotation, bool teleport/* = false*/)
{
    _turret_rotation = rotation;
    if (teleport) {
        _old_turret_rotation = rotation;
    }
}

//------------------------------------------------------------------------------
void tank::update_usercmd(game::usercmd usercmd)
{
    _usercmd = usercmd;
}

//------------------------------------------------------------------------------
void tank::update_sound()
{
    // engine noise
    if (_channels[0]) {
        if (_damage < 1.0f) {
            if (!_channels[0]->playing()) {
                _channels[0]->loop(_sound_idle);
            }
            _channels[0]->set_origin(vec3(get_position()));
            _channels[0]->set_volume(0.5f);
            _channels[0]->set_attenuation(1.0f);
            _channels[0]->set_frequency(1.0f);
        }
        else if (_channels[0]->playing()) {
            _channels[0]->stop();
        }
    }

    // tread noise
    if (_channels[1]) {
        if (get_linear_velocity().length_sqr() > 1.0f || fabs(get_angular_velocity()) > math::deg2rad(1.0f)) {
            if (!_channels[1]->playing()) {
                _channels[1]->loop(_sound_move);
            }
            _channels[1]->set_origin(vec3(get_position()));
            _channels[1]->set_volume(0.5f);
            _channels[1]->set_attenuation(1.0f);
        }
        else if (_channels[1]->playing()) {
            _channels[1]->stop();
        }
    }

    // turret noise
    if (_channels[2]) {
        if (fabs(get_angular_velocity() - _turret_velocity) > math::deg2rad(1.0f)) {
            if (!_channels[2]->playing()) {
                _channels[2]->loop(_sound_turret_move);
            }
            _channels[2]->set_origin(vec3(get_position()));
            _channels[2]->set_volume(0.3f);
            _channels[2]->set_attenuation(1.0f);
        }
        else if (_channels[2]->playing()) {
            _channels[2]->stop( );
        }
    }
}

//------------------------------------------------------------------------------
string::view tank::player_name() const
{
    return va("^%x%x%x%s^xxx",
              int(_color.r * 15.5f),
              int(_color.g * 15.5f),
              int(_color.b * 15.5f),
              g_Game->svs.clients[_player_index].info.name.data());
}

} // namespace game
