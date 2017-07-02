/*
===============================================================================

Name    :   g_particles.cpp

Purpose :   handles cParticle and cWorld particle handling

===============================================================================
*/

#include "local.h"
#pragma hdrstop

namespace game {

//------------------------------------------------------------------------------
render::particle* world::add_particle ()
{
    _particles.emplace_back(render::particle{0});
    _particles.back().time = 1e-3f * g_Game->m_flTime;
    return &_particles.back();
}

//------------------------------------------------------------------------------
void world::free_particle (render::particle *p) const
{
    _particles[p - _particles.data()] = _particles.back();
    _particles.pop_back();
}

//------------------------------------------------------------------------------
void world::draw_particles () const
{
    float time = 1e-3f * g_Game->m_flTime;

    for (std::size_t ii = 0; ii < _particles.size(); ++ii) {
        float ptime = time - _particles[ii].time;
        if (_particles[ii].color.a + _particles[ii].color_velocity.a * ptime <= 0.0f) {
            free_particle(&_particles[ii]);
            --ii;
        }
    }

    g_Application->get_glWnd()->get_Render()->DrawParticles(
        time,
        _particles.data(),
        _particles.size());
}

} // namespace game
