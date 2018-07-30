// g_world.h
//

#pragma once

#include "g_usercmd.h"
#include "g_object.h"

#include "p_material.h"
#include "p_rigidbody.h"
#include "p_shape.h"

#include "net_message.h"

#include "r_model.h"
#include "r_particle.h"

#include <array>
#include <memory>
#include <set>
#include <type_traits>
#include <vector>

#define MAX_PLAYERS 16

////////////////////////////////////////////////////////////////////////////////
namespace game {

class object;
class tank;
class world;

typedef struct game_client_s game_client_t;

//------------------------------------------------------------------------------
enum class effect_type
{
    smoke,
    sparks,
    blaster,
    missile_trail,
    cannon_impact,
    missile_impact,
    blaster_impact,
    explosion,
};

//------------------------------------------------------------------------------
class world
{
public:
    world ()
        : _spawn_id(0)
        , _players{}
        , _border_material{0,0}
        , _border_shapes{{vec2(0,0)}, {vec2(0,0)}}
        , _arena_width("g_arenaWidth", 640, config::archive|config::server|config::reset, "arena width")
        , _arena_height("g_arenaHeight", 480, config::archive|config::server|config::reset, "arena height")
    {}
    ~world () {}

    void init();
    void shutdown();
    void reset();

    void clear_particles();

    void run_frame ();
    void draw(render::system* renderer, float time) const;

    void read_snapshot(network::message& message);
    void write_snapshot(network::message& message) const;

    std::vector<std::unique_ptr<object>> const& objects() { return _objects; }

    template<typename T, typename... Args>
    T* spawn(Args&& ...args);

    game::object* find_object(std::size_t spawn_id) const;

    void remove(object* object);

    game::tank* spawn_player(int player_index);
    void remove_player(int player_index);

    void add_sound(sound::asset sound_asset, vec2 position, float volume = 1.0f);
    void add_effect(effect_type type, vec2 position, vec2 direction = vec2(0,0), float strength = 1);
    void add_trail_effect(effect_type type, vec2 position, vec2 old_position, vec2 direction = vec2(0,0), float strength = 1);

    vec2 mins() const { return _mins; }
    vec2 maxs() const { return _maxs; }
    int framenum() const { return _framenum; }

    game::tank* player( int index ) { return _players[ index ]; }

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

    game::object* spawn_snapshot(std::size_t spawn_id, object_type type);

    config::integer _arena_width;
    config::integer _arena_height;

    friend game::tank;

    game::tank* _players[MAX_PLAYERS];

    //
    // particle system
    //

    bool _use_particles;

    mutable std::vector<render::particle> _particles;

    render::particle* add_particle();
    void free_particle (render::particle* particle) const;

    void draw_particles(render::system* renderer, float time) const;

    vec2        _mins;
    vec2        _maxs;

    int _framenum;

    network::message_storage _message;

    physics::material _border_material;
    physics::box_shape _border_shapes[2];
    constexpr static int _border_thickness = 512;

protected:
    enum class message_type
    {
        none,
        frame,
        sound,
        effect,
    };

    void read_frame(network::message const& message);
    void read_sound(network::message const& message);
    void read_effect(network::message const& message);

    void write_sound(sound::asset sound_asset, vec2 position, float volume);
    void write_effect(effect_type type, vec2 position, vec2 direction, float strength);
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
