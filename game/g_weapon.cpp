// g_weapon.cpp
//

#include "precompiled.h"
#pragma hdrstop

#include "g_weapon.h"
#include "g_ship.h"

////////////////////////////////////////////////////////////////////////////////
namespace game {

//------------------------------------------------------------------------------
weapon::weapon(game::ship* owner, weapon_info const& info, vec2 position)
    : object(object_type::weapon, owner)
    , _info(info)
    , _last_attack_time(time_value::zero)
    , _target(nullptr)
    , _target_pos(vec2_zero)
    , _target_end(vec2_zero)
    , _is_attacking(false)
    , _is_repeating(false)
    , _projectile_target(nullptr)
    , _projectile_target_pos(vec2_zero)
    , _projectile_count(0)
    , _beam_target(nullptr)
    , _beam_sweep_start(vec2_zero)
    , _beam_sweep_end(vec2_zero)
{
    set_position(position, true);
    set_linear_velocity(vec2_zero);
    _rigid_body.set_mass(0.f);
}

//------------------------------------------------------------------------------
weapon::~weapon()
{
}

//------------------------------------------------------------------------------
void weapon::draw(render::system* renderer, time_value time) const
{
    if (_beam_target && time - _last_attack_time < _info.beam_duration) {
        float t = (time - _last_attack_time) / _info.beam_duration;
        vec2 beam_start = get_position() * _owner->rigid_body().get_transform();
        vec2 beam_end = (_beam_sweep_end * t + _beam_sweep_start * (1.f - t)) * _beam_target->rigid_body().get_transform();

        renderer->draw_line(beam_start, beam_end, color4(1, 0.5, 0, 0.5), color4(1, 0.5, 0, 0.5));
    }
}

//------------------------------------------------------------------------------
void weapon::think()
{
    time_value time = _world->frametime();

    if (_is_attacking && time - _last_attack_time > _info.reload_time) {
        if (_info.type != weapon_type::laser) {
            _projectile_target = _target;
            _projectile_target_pos = _target_pos;
            _projectile_count = 0;
        } else {
            _beam_target = _target;
            _beam_sweep_start = _target_pos;
            _beam_sweep_end = _target_end;
        }
        _last_attack_time = time;
        _is_attacking = _is_repeating;
    }

    //
    // update projectiles
    //

    if (_projectile_target && time - _last_attack_time <= _info.projectile_count * _info.projectile_delay) {
        if (_projectile_count < _info.projectile_count && _projectile_count * _info.projectile_delay <= time - _last_attack_time) {
            game::projectile* proj = _world->spawn<projectile>(_owner, _info.projectile_damage, _info.type);
            vec2 start = get_position() * _owner->rigid_body().get_transform();
            vec2 end = _projectile_target_pos * _projectile_target->rigid_body().get_transform();

            vec2 relative_velocity = _projectile_target->get_linear_velocity();
            if (_info.projectile_inertia) {
                relative_velocity -= _owner->get_linear_velocity();
            }

            // lead target based on relative velocity
            end += relative_velocity * (end - start).length() / _info.projectile_speed;

            vec2 dir = (end - start).normalize();

            vec2 projectile_velocity = dir * _info.projectile_speed;
            if (_info.projectile_inertia) {
                projectile_velocity += _owner->get_linear_velocity();
            }

            proj->set_position(start, true);
            proj->set_linear_velocity(projectile_velocity);
            proj->set_rotation(std::atan2(dir.y, dir.x), true);

            _world->add_effect(time, effect_type::blaster, start, dir);

            ++_projectile_count;
        }
    }

    //
    // update beam
    //

    if (_beam_target && time - _last_attack_time < _info.beam_duration) {
        float t = (time - _last_attack_time) / _info.beam_duration;
        vec2 beam_start = get_position() * _owner->rigid_body().get_transform();
        vec2 beam_end = (_beam_sweep_end * t + _beam_sweep_start * (1.f - t)) * _beam_target->rigid_body().get_transform();
        vec2 beam_dir = (beam_end - beam_start).normalize();

        _world->add_effect(time, effect_type::sparks, beam_end, -beam_dir, .5f);
        // damage
    }
}

//------------------------------------------------------------------------------
void weapon::read_snapshot(network::message const& /*message*/)
{
}

//------------------------------------------------------------------------------
void weapon::write_snapshot(network::message& /*message*/) const
{
}

//------------------------------------------------------------------------------
void weapon::attack_projectile(game::object* target, vec2 target_pos, bool repeat)
{
    _target = target;
    _target_pos = target_pos;// / target->rigid_body.get_transform();
    _is_attacking = true;
    _is_repeating = repeat;
}

//------------------------------------------------------------------------------
void weapon::attack_beam(game::object* target, vec2 sweep_start, vec2 sweep_end, bool repeat)
{
    _target = target;
    _target_pos = sweep_start;// / target->rigid_body().get_transform();
    _target_end = sweep_end;// / target->rigid_body().get_transform();
    _is_attacking = true;
    _is_repeating = repeat;
}

//------------------------------------------------------------------------------
void weapon::cancel()
{
    _is_attacking = false;
}

} // namespace game
