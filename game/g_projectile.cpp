// g_projectile.cpp
//

#include "precompiled.h"
#pragma hdrstop

#include "g_projectile.h"
#include "g_tank.h"
#include "p_collide.h"

////////////////////////////////////////////////////////////////////////////////
namespace game {

physics::circle_shape projectile::_shape(1.0f);
physics::material projectile::_material(0.5f, 1.0f);

//------------------------------------------------------------------------------
projectile::projectile(tank* owner, float damage, weapon_type type)
    : object(object_type::projectile, owner)
    , _damage(damage)
    , _type(type)
{
    _rigid_body = physics::rigid_body(&_shape, &_material, 1.0f);
    _channel = pSound->allocate_channel();
    _sound_cannon_impact = pSound->load_sound("assets/sound/cannon_impact.wav");
    _sound_blaster_impact = pSound->load_sound("assets/sound/blaster_impact.wav");
    _sound_missile_flight = pSound->load_sound("assets/sound/missile_flight.wav");
}

//------------------------------------------------------------------------------
projectile::~projectile()
{
    if (_channel) {
        _channel->stop();
        pSound->free_channel(_channel);
    }
}

//------------------------------------------------------------------------------
void projectile::think()
{
    if (_type == weapon_type::missile) {
        update_homing();
    }

    update_effects();
    update_sound();
}

//------------------------------------------------------------------------------
void projectile::update_homing()
{
    vec2 direction = get_linear_velocity();
    float speed = direction.normalize_length();
    float bestValue = float(M_SQRT1_2);
    game::object* bestTarget = nullptr;

    for (auto& obj : _world->objects()) {
        if (obj->_type != object_type::tank) {
            continue;
        }
        if (static_cast<game::tank*>(obj.get())->_damage >= 1.0f) {
            continue;
        }

        vec2 displacement = obj->get_position() - get_position();
        float value = displacement.normalize().dot(direction);
        if (value > bestValue) {
            bestValue = value;
            bestTarget = obj.get();
        }
    }

    if (bestTarget && bestValue < 0.995f) {
        vec2 target_pos = bestTarget->get_position();
        vec2 closest_point = get_position() + direction * (target_pos - get_position()).dot(direction);
        vec2 displacement = (target_pos - closest_point).normalize();
        vec2 delta = displacement * speed * 0.5f * FRAMETIME;
        vec2 new_velocity = (get_linear_velocity() + delta).normalize() * speed;
        set_linear_velocity(new_velocity);
    }
}

//------------------------------------------------------------------------------
void projectile::update_effects()
{
    if (_type == weapon_type::missile) {
        _world->add_trail_effect(
            effect_type::missile_trail,
            get_position(),
            _old_position,
            get_linear_velocity() * -0.5f,
            4 );
    }
}

//------------------------------------------------------------------------------
void projectile::update_sound()
{
    if (_type == weapon_type::missile) {
        if (_channel) {
            if (!_channel->playing()) {
                _channel->loop(_sound_missile_flight);
            }
            _channel->set_volume(0.2f);
            _channel->set_attenuation(0.0f);
            _channel->set_origin(vec3(get_position()));
        }
    } else if (_channel && _channel->playing()) {
        _channel->stop();
    }
}

//------------------------------------------------------------------------------
bool projectile::touch(object *other, physics::contact const* contact)
{
    auto sound = _type == weapon_type::cannon ? _sound_cannon_impact :
                 _type == weapon_type::missile ? _sound_cannon_impact :
                 _type == weapon_type::blaster ? _sound_blaster_impact : sound::asset::invalid;

    auto effect = _type == weapon_type::cannon ? effect_type::cannon_impact :
                  _type == weapon_type::missile ? effect_type::missile_impact :
                  _type == weapon_type::blaster ? effect_type::blaster_impact : effect_type::smoke;

    if (other && other->_type != object_type::projectile && !other->touch(this, contact)) {
        return false;
    }

    if (contact) {
        _world->add_sound(sound, contact->point);
        _world->add_effect(effect, contact->point, -contact->normal, 0.5f * _damage);
    } else {
        _world->add_sound(sound, get_position());
        _world->add_effect(effect, get_position(), vec2_zero, 0.5f * _damage);
    }

    _world->remove(this);

    if (other && other->_type == object_type::tank) {
        tank* owner_tank = static_cast<tank*>(_owner);
        tank* other_tank = static_cast<tank*>(other);

        if (other_tank->_damage >= 1.0f) {
            return true; // dont add damage or score
        }

        vec2 direction = get_linear_velocity().normalize();
        vec2 impact_normal = contact ? -contact->normal : -direction;
        vec2 forward = rotate(vec2(1,0), other->get_rotation());
        float impact_angle = impact_normal.dot(forward);
        float damage = _damage / other_tank->_client->armor_mod;

        if (_type == weapon_type::cannon) {
            float surface_angle = impact_normal.dot(-direction);
            damage = 1.2f * damage * surface_angle;
            if (impact_angle > M_SQRT1_2) {
                other_tank->_damage += damage * DAMAGE_FRONT;
            } else if (impact_angle > -M_SQRT1_2) {
                other_tank->_damage += damage * DAMAGE_SIDE;
            } else {
                other_tank->_damage += damage * DAMAGE_REAR;
            }
        } else if (_type == weapon_type::missile) {
            other_tank->_damage += damage * DAMAGE_SIDE;
        } else if (_type == weapon_type::blaster) {
            if (impact_angle > M_SQRT1_2) {
                other_tank->_damage += damage * DAMAGE_FRONT / 1.25f;
            } else if (impact_angle > -M_SQRT1_2) {
                other_tank->_damage += damage * DAMAGE_SIDE * 1.25f;
            } else {
                other_tank->_damage += damage * DAMAGE_REAR * 1.5f;
            }
        }

        if (other_tank->_damage >= 1.0f) {
            g_Game->add_score( owner_tank->_player_index, 1 );
            other_tank->_dead_time = _world->framenum() * FRAMETIME;

            char const* fmt = "";

            switch (_type) {
                case weapon_type::cannon:
                    switch (rand()%3) {
                        case 0: fmt = "%s couldn't take %s's HEAT."; break;
                        case 1: fmt = "%s was on the wrong end of %s's cannon."; break;
                        case 2: fmt = "%s ate all 125mm of %s's boom stick."; break;
                    }
                    break;

                case weapon_type::missile:
                    switch (rand()%2) {
                        case 0: fmt = "%s couldn't hide from %s's heat seeker."; break;
                        case 1: fmt = "%s ate %s's rocket."; break;
                    }
                    break;

                case weapon_type::blaster:
                    switch (rand()%2) {
                        case 0: fmt = "%s was melted by %s's blaster."; break;
                        case 1: fmt = "%s was blasted by %s."; break;
                    }
                    break;
            }

            g_Game->write_message(va(fmt, other_tank->player_name(), owner_tank->player_name()));
        }
    }

    return true;
}

//------------------------------------------------------------------------------
void projectile::draw(render::system* renderer, float time) const
{
    float lerp = (time - _world->framenum() * FRAMETIME) / FRAMETIME;

    vec2 p1 = get_position(lerp);
    vec2 p2 = get_position(lerp + 0.4f);

    switch (_type) {
        case weapon_type::cannon:
            renderer->draw_line(p2, p1, color4(1,0.5,0,1), color4(1,0.5,0,0));
            break;

        case weapon_type::missile:
            renderer->draw_line(p2, p1, color4(1,1,1,1), color4(0,0,0,0));
            break;

        case weapon_type::blaster:
            renderer->draw_line(p2, p1, color4(1,0.1f,0,1), color4(1,0.7f,0,0));
            break;

        default:
            break;
    }
}

//------------------------------------------------------------------------------
void projectile::read_snapshot(network::message const& message)
{
    _old_position = get_position();

    _owner = _world->find_object(message.read_long());
    _damage = message.read_float();
    _type = static_cast<weapon_type>(message.read_byte());
    set_position(message.read_vector());
    set_linear_velocity(message.read_vector());

    update_effects();
    update_sound();
}

//------------------------------------------------------------------------------
void projectile::write_snapshot(network::message& message) const
{
    message.write_long(_owner->spawn_id());
    message.write_float(_damage);
    message.write_byte(static_cast<int>(_type));
    message.write_vector(get_position());
    message.write_vector(get_linear_velocity());
}

} // namespace game
