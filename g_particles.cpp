// g_particles.cpp
//

#include "precompiled.h"
#pragma hdrstop

////////////////////////////////////////////////////////////////////////////////
namespace game {

//------------------------------------------------------------------------------
render::particle* world::add_particle ()
{
    _particles.emplace_back(render::particle{0});
    _particles.back().time = 1e-3f * g_Game->_frametime;
    return &_particles.back();
}

//------------------------------------------------------------------------------
void world::free_particle (render::particle *p) const
{
    _particles[p - _particles.data()] = _particles.back();
    _particles.pop_back();
}

//------------------------------------------------------------------------------
void world::draw_particles(render::system* renderer) const
{
    float time = 1e-3f * g_Game->_frametime;

    for (std::size_t ii = 0; ii < _particles.size(); ++ii) {
        float ptime = time - _particles[ii].time;
        if (_particles[ii].color.a + _particles[ii].color_velocity.a * ptime < 0.0f) {
            free_particle(&_particles[ii]);
            --ii;
        } else if (_particles[ii].size + _particles[ii].size_velocity * ptime < 0.0f) {
            free_particle(&_particles[ii]);
            --ii;
        }
    }

    renderer->draw_particles(
        time,
        _particles.data(),
        _particles.size());
}

} // namespace game
