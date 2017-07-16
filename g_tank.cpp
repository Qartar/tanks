// g_tank.cpp
//

#include "precompiled.h"
#pragma hdrstop

#include "p_collide.h"

////////////////////////////////////////////////////////////////////////////////
namespace game {

physics::material tank::_material(0.5f, 1.0f, 5.0f);
physics::box_shape tank::_shape(vec2(24, 16));

#define DAMAGE_FRONT    0.334f
#define DAMAGE_SIDE     0.5f
#define DAMAGE_REAR     1.0f
#define DAMAGE_FULL     1.0f

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
    , _dead_time(0)
    , _fire_time(0)
    , _usercmd{}
    , _channels{0}
    , _client(nullptr)
{
    _rigid_body = physics::rigid_body(&_shape, &_material, 1.0f);

    _model = &tank_body_model;
    _turret_model = &tank_turret_model;

    for (auto& chan : _channels) {
        if (chan == nullptr) {
            chan = pSound->allocate_channel();
        }
    }

    _sound_idle = pSound->load_sound("assets/sound/tank_idle.wav");
    _sound_move = pSound->load_sound("assets/sound/tank_move.wav");
    _sound_turret_move = pSound->load_sound("assets/sound/turret_move.wav");
    _sound_fire = pSound->load_sound("assets/sound/tank_fire.wav");
    _sound_explode = pSound->load_sound("assets/sound/tank_explode.wav");
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
void tank::draw(render::system* renderer) const
{
    float lerp = (g_Game->_frametime - (g_Game->_framenum-1) * FRAMEMSEC) / FRAMEMSEC;
    float reload = clamp(((g_Game->_frametime - _fire_time) / 150.f) * _client->refire_mod,0.0f,20.0f);
    float health = clamp((1.0f - _damage) * 20.f, 0.0f, 20.0f);

    if ( lerp > 1.0f ) {
        lerp = 1.0f;
    } else if ( lerp < 0.0f ) {
        lerp = 0.0f;
    }

    vec2    pos = get_position( lerp );
    float   angle = get_rotation( lerp );
    float   tangle = get_turret_rotation( lerp );

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
        _model->draw(pos, angle, color4(0.3f,0.3f,0.3f,1));
        _turret_model->draw(pos, tangle, color4(0.3f,0.3f,0.3f,1));
        return;
    }

    // status bars

    renderer->draw_box(vec2(20,2), pos + vec2(0,24), color4(0.5,0.5,0.5,1));
    renderer->draw_box(vec2(reload,2), pos + vec2(0,24), color_reload);
    renderer->draw_box(vec2(20,2), pos + vec2(0,22), color4(0.5,0.5,0.5,1));
    renderer->draw_box(vec2(health,2), pos + vec2(0,22), color_health);

    // actual body

    _model->draw(pos, angle, _color);
    _turret_model->draw(pos, tangle, _color);
}

//------------------------------------------------------------------------------
void tank::touch(object *other, physics::contact const* contact)
{
    if (!other || !contact) {
        return;
    }

    float impulse = contact->impulse.length();
    float strength = clamp((impulse - 5.0f) / 5.0f, 0.0f, 1.0f);
    _world->add_effect(effect_type::sparks, contact->point, -contact->normal, strength);

    if (other->_type != object_type::tank) {
        return;
    }

    tank* other_tank = static_cast<tank*>(other);

    this->collide(other_tank, contact);
    other_tank->collide(this, contact);
}

//------------------------------------------------------------------------------
void tank::collide(tank* other, physics::contact const* contact)
{
    if (other->_damage >= 1.0f) {
        return;
    }

    float base_damage = std::max<float>(0, contact->impulse.dot(contact->normal) - 10.0f) / 2.0f * FRAMETIME;

    vec2 direction = (contact->point - other->get_position()).normalize();
    vec2 forward = rotate(vec2(1,0), other->get_rotation());
    float impact_angle = direction.dot(forward);

    if (!g_Game->_extended_armor && !g_Game->_multiplayer) {
        base_damage *= DAMAGE_FULL;
    } else if (impact_angle > M_SQRT1_2) {
        base_damage *= DAMAGE_FRONT;
    } else if (impact_angle > -M_SQRT1_2) {
        base_damage *= DAMAGE_SIDE;
    } else {
        base_damage *= DAMAGE_REAR;
    }

    other->_damage += base_damage / other->_client->armor_mod;

    if (other->_damage >= 1.0f)
    {
        g_Game->add_score(_player_index, 1);
        other->_dead_time = g_Game->_frametime;

        g_Game->write_message( va("%s got a little too cozy with %s.", other->player_name(), player_name() ) );
    }
}

#define HACK_TIME       1000.0f

//------------------------------------------------------------------------------
void tank::think()
{
    _old_turret_rotation = _turret_rotation;

    if ( (_damage >= 1.0f) && (_dead_time+RESTART_TIME+HACK_TIME <= g_Game->_frametime) )
    {
        // respawn
        vec2 spawn_buffer = vec2(1,1) * SPAWN_BUFFER;
        vec2 spawn_size = _world->maxs() - _world->mins() - spawn_buffer * 2.0f;
        vec2 spawn_pos = _world->mins() + vec2(frand(), frand()) * spawn_size + spawn_buffer;

        _dead_time = 0.0f;

        set_position(spawn_pos);
        set_rotation(frand()*2.0f*M_PI);
        _turret_rotation = get_rotation();

        set_linear_velocity(vec2(0,0));
        set_angular_velocity(0);
        _turret_velocity = 0;

        _damage = 0.0f;
    }

    vec2 forward = rotate(vec2(1,0), get_rotation());
    float speed = forward.dot(get_linear_velocity());

    vec2 track_velocity = rotate(vec2(_track_speed,0), get_rotation());
    vec2 delta_velocity = track_velocity - get_linear_velocity();
    vec2 friction_impulse = delta_velocity * _rigid_body.get_mass() * _rigid_body.get_material()->sliding_friction() * FRAMETIME;

    _rigid_body.apply_impulse(friction_impulse);

    if (_damage >= 1.0f)
    {
        vec2 vVel = vec2(speed * 0.98 * (1-FRAMETIME),0);

        set_linear_velocity(rotate(vVel,get_rotation()));
        set_angular_velocity(get_angular_velocity() * 0.9f);
        _turret_velocity *= 0.9f;

        // extra explosion
        if (_dead_time && (g_Game->_frametime - _dead_time > 650) && (g_Game->_frametime - _dead_time < 650+HACK_TIME/2))
        {
            _world->add_sound(_sound_explode);
            _world->add_effect(effect_type::explosion, get_position(), vec2(0,0), _client->damage_mod);
            _dead_time -= HACK_TIME;    // dont do it again
        }
    }
    else
    {
        float new_speed = _track_speed * 0.9 * (1 - FRAMETIME) + _usercmd.move[0] * 192 * _client->speed_mod * FRAMETIME;
        new_speed = clamp(new_speed, -32 * _client->speed_mod, 48 * _client->speed_mod);

        set_linear_velocity(get_linear_velocity() + forward * (new_speed - _track_speed));
        _track_speed = new_speed;

        set_angular_velocity(deg2rad(-_usercmd.move[1] * 90));
        _turret_velocity = deg2rad(-_usercmd.look[0] * 90);
    }

    // update position here because Move doesn't
    _turret_rotation += _turret_velocity * FRAMETIME;

    if ((_fire_time + 2500/_client->refire_mod) > g_Game->_frametime)  // just fired
    {
        vec2 effect_origin(21,0);
        float power;

        power = 1.5 - (g_Game->_frametime - _fire_time)/1000.0f;
        power = clamp(power, 0.5f, 1.5f);

        _world->add_effect(
            effect_type::smoke,
            get_position() + rotate(effect_origin,_turret_rotation),
            rotate(effect_origin,_turret_rotation) * power * power * power * 2,
            power * power * 4 );
    }
    else if ((_fire_time + 3000/_client->refire_mod) < g_Game->_frametime) // can fire
    {
        // check for firing
        if (_usercmd.action == usercmd::action::attack && _damage < 1.0f)
        {
            vec2 effect_origin(21,0);

            _world->add_effect(
                effect_type::smoke,
                get_position() + rotate(effect_origin,_turret_rotation),
                rotate(effect_origin,_turret_rotation) * 16,
                64 );

            _fire_time = g_Game->_frametime;

            projectile* bullet = _world->spawn<projectile>(this, _client->damage_mod);

            bullet->set_position(get_position() + rotate(effect_origin,_turret_rotation), true);
            bullet->set_linear_velocity(rotate(vec2(1,0),_turret_rotation) * 20 * 96);

            _world->add_sound(_sound_fire);
        }
    }

    update_sound( );
}

//------------------------------------------------------------------------------
void tank::read_snapshot(network::message& message)
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
    _fire_time = message.read_float();

    update_sound();
}

//------------------------------------------------------------------------------
void tank::write_snapshot(network::message& message) const
{
    message.write_byte(_player_index);
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
    message.write_float(_fire_time);
}

//------------------------------------------------------------------------------
float tank::get_turret_rotation(float lerp) const
{
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
            _channels[0]->set_volume(1.0f);
            _channels[0]->set_attenuation(0.0f);
            _channels[0]->set_frequency(1.0f);
        }
        else if (_channels[0]->playing()) {
            _channels[0]->stop();
        }
    }

    // tread noise
    if (_channels[1]) {
        if (get_linear_velocity().length_sqr() > 1.0f || fabs(get_angular_velocity()) > deg2rad(1.0f)) {
            if (!_channels[1]->playing()) {
                _channels[1]->loop(_sound_move);
            }
            _channels[1]->set_volume(1.0f);
            _channels[1]->set_attenuation(0.0f);
        }
        else if (_channels[1]->playing()) {
            _channels[1]->stop();
        }
    }

    // turret noise
    if (_channels[2]) {
        if (fabs(get_angular_velocity() - _turret_velocity) > deg2rad(1.0f)) {
            if (!_channels[2]->playing()) {
                _channels[2]->loop(_sound_turret_move);
            }
            _channels[2]->set_volume(1.0f);
            _channels[2]->set_attenuation(0.0f);
        }
        else if (_channels[2]->playing()) {
            _channels[2]->stop( );
        }
    }
}

//------------------------------------------------------------------------------
char const* tank::player_name() const
{
    return va("\\c%02x%02x%02x%s\\cx",
              int(_color.r * 255),
              int(_color.g * 255),
              int(_color.b * 255),
              g_Game->svs.clients[_player_index].name);
}

////////////////////////////////////////////////////////////////////////////////
physics::circle_shape projectile::_shape(1.0f);
physics::material projectile::_material(0.5f, 1.0f);

//------------------------------------------------------------------------------
projectile::projectile(tank* owner, float damage)
    : object(object_type::projectile, owner)
    , _damage(damage)
{
    _rigid_body = physics::rigid_body(&_shape, &_material, 1.0f);
    _sound_explode = pSound->load_sound("assets/sound/bullet_explode.wav");
}

//------------------------------------------------------------------------------
void projectile::touch(object *other, physics::contact const* contact)
{
    _world->add_sound(_sound_explode);
    _world->add_effect(effect_type::explosion, get_position(), contact ? -contact->normal : vec2(0,0), 0.5f * _damage);

    _world->remove(this);

    if (!other)
        return;

    if (other->_type == object_type::tank) {
        tank* owner_tank = static_cast<tank*>(_owner);
        tank* other_tank = static_cast<tank*>(other);

        vec2    impact_normal, forward;
        float   impact_angle, damage;

        if (other_tank->_damage >= 1.0f) {
            return; // dont add damage or score
        }

        impact_normal = contact ? -contact->normal
                                : -get_linear_velocity().normalize();
        forward = rotate(vec2(1,0), other->get_rotation());
        impact_angle = impact_normal.dot(forward);

        damage = owner_tank->_client->damage_mod / other_tank->_client->armor_mod;

        if (!g_Game->_extended_armor && !g_Game->_multiplayer) {
            other_tank->_damage += damage * DAMAGE_FULL;
        } else if (impact_angle > M_SQRT1_2) {
            other_tank->_damage += damage * DAMAGE_FRONT;
        } else if (impact_angle > -M_SQRT1_2) {
            other_tank->_damage += damage * DAMAGE_SIDE;
        } else {
            other_tank->_damage += damage * DAMAGE_REAR;
        }

        if (other_tank->_damage >= 1.0f) {
            g_Game->add_score( owner_tank->_player_index, 1 );
            other_tank->_dead_time = g_Game->_frametime;

            int r = rand()%3;
            switch (r) {
                case 0:
                    g_Game->write_message(va("%s couldn't take %s's HEAT.", other_tank->player_name(), owner_tank->player_name()));
                    break;
                case 1:
                    g_Game->write_message(va("%s was on the wrong end of %s's cannon.", other_tank->player_name(), owner_tank->player_name()));
                    break;
                case 2:
                    g_Game->write_message(va("%s ate all 125mm of %s's boom stick.", other_tank->player_name(), owner_tank->player_name()));
                    break;
            }
        }
    }
}

//------------------------------------------------------------------------------
void projectile::draw(render::system* renderer) const
{
    float   lerp = (g_Game->_frametime - (g_Game->_framenum-1) * FRAMEMSEC) / FRAMEMSEC;
    vec2    p1, p2;

    p1 = get_position( lerp );
    p2 = get_position( lerp + 0.4f );

    renderer->draw_line(p2, p1, color4(1,0.5,0,1), color4(1,0.5,0,0));
}

//------------------------------------------------------------------------------
void projectile::read_snapshot(network::message& message)
{
    _old_position = get_position();

    _owner = _world->find_object(message.read_long());
    _damage = message.read_float();
    set_position(message.read_vector());
    set_linear_velocity(message.read_vector());
}

//------------------------------------------------------------------------------
void projectile::write_snapshot(network::message& message) const
{
    message.write_long(_owner->spawn_id());
    message.write_float(_damage);
    message.write_vector(get_position());
    message.write_vector(get_linear_velocity());
}

} // namespace game
