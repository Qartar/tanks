// g_world.cpp
//

#include "precompiled.h"
#pragma hdrstop

#include "g_projectile.h"
#include "g_tank.h"
#include "p_collide.h"
#include "p_trace.h"

#include <algorithm>

extern bool debug_collide;

////////////////////////////////////////////////////////////////////////////////
namespace game {

//------------------------------------------------------------------------------
world::world()
    : _spawn_id(0)
    , _players{}
    , _border_material{0,0}
    , _border_shapes{{vec2(0,0)}, {vec2(0,0)}}
    , _arena_width("g_arenaWidth", 640, config::archive|config::server|config::reset, "arena width")
    , _arena_height("g_arenaHeight", 480, config::archive|config::server|config::reset, "arena height")
    , _physics(
        std::bind(&world::physics_filter_callback, this, std::placeholders::_1, std::placeholders::_2),
        std::bind(&world::physics_collide_callback, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3))
{}

//------------------------------------------------------------------------------
void world::init()
{
    reset();
}

//------------------------------------------------------------------------------
void world::shutdown()
{
    _objects.clear();
    _pending.clear();
    _removed.clear();
}

//------------------------------------------------------------------------------
void world::reset()
{
    clear_particles();

    _mins = vec2(0,0);
    _maxs = vec2(vec2i(_arena_width, _arena_height));
    _framenum = 0;

    _border_shapes[0] = physics::box_shape(vec2(vec2i(_border_thickness + _arena_width, _border_thickness)));
    _border_shapes[1] = physics::box_shape(vec2(vec2i(_border_thickness, _border_thickness + _arena_height)));

    _objects.clear();
    _pending.clear();
    _removed.clear();

    for (int ii = 0; ii < MAX_PLAYERS; ++ii) {
        _players[ii] = nullptr;
    }

    _spawn_id = 0;

    // Initialize border objects
    {
        vec2 mins = vec2(vec2i(-_border_thickness / 2, -_border_thickness / 2));
        vec2 maxs = vec2(vec2i(_arena_width, _arena_height)) - mins;

        vec2 positions[] = {
            {(mins.x+maxs.x)/2,mins.y}, // bottom
            {(mins.x+maxs.x)/2,maxs.y}, // top
            {mins.x,(mins.y+maxs.y)/2}, // left
            {maxs.x,(mins.y+maxs.y)/2}, // right
        };

        physics::shape* shapes[] = {
            &_border_shapes[0],
            &_border_shapes[0],
            &_border_shapes[1],
            &_border_shapes[1],
        };

        for (int ii = 0; ii < 4; ++ii) {
            physics::rigid_body body(shapes[ii], &_border_material, 0.0f);
            body.set_position(positions[ii]);
            spawn<obstacle>(std::move(body));
        }
    }


    {
        physics::box_shape box(vec2(2, 2));
        physics::circle_shape dot(1.f);

        physics::rigid_body box_body(&box, &_border_material, 0.f);
        physics::rigid_body dot_body(&dot, &_border_material, 0.f);

        box_body.set_position(vec2(0, 0));

        for (int ii = -512; ii < 512; ++ii) {
            dot_body.set_position(vec2(128.f, float(ii)));
            dot_body.set_linear_velocity(vec2(-256.f, -float(ii * 2)));

            physics::trace tr(&box_body, &dot_body, 1.f);
            if (tr.get_fraction() == 1.f || square(tr.get_contact().point.x) > 1e-6f) {
                debug_collide = true;
                physics::trace tr2(&box_body, &dot_body, 1.f);
                debug_collide = false;
                tr2;
                break;
            }
        }
    }
}

//------------------------------------------------------------------------------
void world::remove(game::object* object)
{
    _removed.insert(object);
}

//------------------------------------------------------------------------------
void world::draw(render::system* renderer, time_value time) const
{
    for (auto& obj : _objects) {
        obj->draw(renderer, time);
    }

    draw_particles(renderer, time);
}

//------------------------------------------------------------------------------
void world::run_frame()
{
    _message.reset();

    ++_framenum;

    for (auto obj : _removed) {
        _objects.erase(std::find_if(_objects.begin(), _objects.end(), [=](auto& it){
            return it.get() == obj;
        }));
    }
    _removed.clear();

    for (auto& obj : _objects) {
        obj->think();
    }

    for (auto& obj : _objects) {
        obj->_old_position = obj->get_position();
        obj->_old_rotation = obj->get_rotation();
    }
    _physics.step(FRAMETIME.to_seconds());

    for (auto& obj : _pending) {
        _objects.push_back(std::move(obj));
    }
    _pending.clear();
}

//------------------------------------------------------------------------------
void world::read_snapshot(network::message& message)
{
    for (auto& obj : _pending) {
        _objects.push_back(std::move(obj));
    }
    _pending.clear();

    for (auto obj : _removed) {
        _objects.erase(std::find_if(_objects.begin(), _objects.end(), [=](auto& it){
            return it.get() == obj;
        }));
    }
    _removed.clear();

    while (message.bytes_remaining()) {
        message_type type = static_cast<message_type>(message.read_byte());
        if (type == message_type::none) {
            break;
        }

        switch (type) {
            case message_type::frame:
                read_frame(message);
                break;

            case message_type::sound:
                read_sound(message);
                break;

            case message_type::effect:
                read_effect(message);
                break;

            default:
                break;
        }
    }

    for (auto& obj : _pending) {
        _objects.push_back(std::move(obj));
    }
    _pending.clear();
}

//------------------------------------------------------------------------------
void world::read_frame(network::message const& message)
{
    _framenum = message.read_long();
    _mins = message.read_vector();
    _maxs = message.read_vector();

    // read active objects
    for (std::size_t ii = 0; ; ++ii) {
        std::size_t spawn_id = message.read_long();

        if (!spawn_id) {
            for (; ii < _objects.size(); ++ii) {
                remove(_objects[ii].get());
            }
            break;
        }

        auto type = static_cast<object_type>(message.read_byte());

        while (ii < _objects.size() && _objects[ii]->_spawn_id < spawn_id) {
            remove(_objects[ii++].get());
        }

        if (ii >= _objects.size()) {
            game::object* obj = spawn_snapshot(spawn_id, type);
            obj->read_snapshot(message);
            obj->set_position(obj->get_position(), true);
        } else /*if (ii < _objects.size())*/ {
            assert(_objects[ii]->_spawn_id == spawn_id);
            _objects[ii]->read_snapshot(message);
        }
    }
}

//------------------------------------------------------------------------------
void world::read_sound(network::message const& message)
{
    int asset = message.read_long();
    vec2 position = message.read_vector();
    float volume = message.read_float();

    add_sound(static_cast<sound::asset>(asset), position, volume);
}

//------------------------------------------------------------------------------
void world::read_effect(network::message const& message)
{
    float time = message.read_float();
    int type = message.read_byte();
    vec2 pos = message.read_vector();
    vec2 vel = message.read_vector();
    float strength = message.read_float();

    add_effect(time_value::from_seconds(time), static_cast<game::effect_type>(type), pos, vel, strength);
}

//------------------------------------------------------------------------------
void world::write_snapshot(network::message& message) const
{
    message.write_byte(svc_snapshot);

    // write frame
    message.write_byte(narrow_cast<uint8_t>(message_type::frame));
    message.write_long(_framenum);
    message.write_vector(_mins);
    message.write_vector(_maxs);

    // write active objects
    for (auto const& obj : _objects) {
        message.write_long(narrow_cast<int>(obj->_spawn_id));
        message.write_byte(narrow_cast<uint8_t>(obj->_type));
        obj->write_snapshot(message);
    }
    message.write_long(0);

    // write sounds and effects
    message.write(_message);
    message.write_byte(narrow_cast<uint8_t>(message_type::none));

    _message.rewind();
}

//------------------------------------------------------------------------------
void world::write_sound(sound::asset sound_asset, vec2 position, float volume)
{
    _message.write_byte(narrow_cast<uint8_t>(message_type::sound));
    _message.write_long(narrow_cast<int>(sound_asset));
    _message.write_vector(position);
    _message.write_float(volume);
}

//------------------------------------------------------------------------------
void world::write_effect(time_value time, effect_type type, vec2 position, vec2 direction, float strength)
{
    _message.write_byte(narrow_cast<uint8_t>(message_type::effect));
    _message.write_float(time.to_seconds());
    _message.write_byte(narrow_cast<uint8_t>(type));
    _message.write_vector(position);
    _message.write_vector(direction);
    _message.write_float(strength);
}

//------------------------------------------------------------------------------
game::object* world::spawn_snapshot(std::size_t spawn_id, object_type type)
{
    switch (type) {
        case object_type::tank: {
            auto tank = spawn<game::tank>();
            tank->_spawn_id = _spawn_id = spawn_id;
            return tank;
        }

        case object_type::projectile: {
            auto proj = spawn<game::projectile>(nullptr, 1.0f, weapon_type::cannon);
            proj->_spawn_id = _spawn_id = spawn_id;
            return proj;
        }

        case object_type::obstacle: {
            auto obj = spawn<game::object>(object_type::object);
            obj->_spawn_id = _spawn_id = spawn_id;
            return obj;
        }

        case object_type::object:
        default:
            return nullptr;
    }
}

//------------------------------------------------------------------------------
game::tank* world::spawn_player(std::size_t player_index)
{
    assert(_players[player_index] == nullptr);
    _players[player_index] = spawn<game::tank>();
    _players[player_index]->_player_index = player_index;
    return _players[player_index];
}

//------------------------------------------------------------------------------
void world::remove_player(std::size_t player_index)
{
    assert(_players[player_index]);
    remove(_players[player_index]);
    _players[player_index] = nullptr;
}

//------------------------------------------------------------------------------
game::object* world::find_object(std::size_t spawn_id) const
{
    // search in active objects
    for (auto& obj : _objects) {
        if (obj->_spawn_id == spawn_id) {
            return obj.get();
        }
    }

    // look in pending objects
    for (auto& obj : _pending) {
        if (obj->_spawn_id == spawn_id) {
            return obj.get();
        }
    }

    return nullptr;
}

//------------------------------------------------------------------------------
void world::add_sound(sound::asset sound_asset, vec2 position, float volume)
{
    write_sound(sound_asset, position, volume);
    pSound->play(sound_asset, vec3(position), volume, 1.0f);
}

//------------------------------------------------------------------------------
void world::add_body(game::object* owner, physics::rigid_body* body)
{
    _physics.add_body(body);
    _physics_objects[body] = owner;
}

//------------------------------------------------------------------------------
void world::remove_body(physics::rigid_body* body)
{
    _physics.remove_body(body);
    _physics_objects.erase(body);
}

//------------------------------------------------------------------------------
bool world::physics_filter_callback(physics::rigid_body const* body_a, physics::rigid_body const* body_b)
{
    game::object* obj_a = _physics_objects[body_a];
    game::object* obj_b = _physics_objects[body_b];

    if (obj_b->_type == object_type::projectile) {
        return false;
    }

    game::object const* owner_a = obj_a->_owner ? obj_a->_owner : obj_a;
    game::object const* owner_b = obj_b->_owner ? obj_b->_owner : obj_b;

    if (owner_a == owner_b) {
        return false;
    }

    return true;
}

//------------------------------------------------------------------------------
bool world::physics_collide_callback(physics::rigid_body const* body_a, physics::rigid_body const* body_b, physics::collision const& collision)
{
    game::object* obj_a = _physics_objects[body_a];
    game::object* obj_b = _physics_objects[body_b];

    return obj_a->touch(obj_b, &collision);
}

} // namespace game
