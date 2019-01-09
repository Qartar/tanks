// g_particles.cpp
//

#include "precompiled.h"
#pragma hdrstop

////////////////////////////////////////////////////////////////////////////////
namespace game {

//------------------------------------------------------------------------------
render::particle* world::add_particle ()
{
    _particles.emplace_back(render::particle{});
    _particles.back().time = frametime();
    return &_particles.back();
}

//------------------------------------------------------------------------------
void world::free_particle (render::particle *p) const
{
    _particles[p - _particles.data()] = _particles.back();
    _particles.pop_back();
}

//------------------------------------------------------------------------------
void world::draw_particles(render::system* renderer, time_value time) const
{
    for (std::size_t ii = 0; ii < _particles.size(); ++ii) {
        float ptime = (time - _particles[ii].time).to_seconds();
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

//------------------------------------------------------------------------------
void world::clear_particles()
{
    _particles.clear();
}

//------------------------------------------------------------------------------
void world::add_effect(effect_type type, vec2 position, vec2 direction, float strength)
{
    write_effect(type, position, direction, strength);

    float   r, d;

    switch (type) {
        case effect_type::smoke: {
            int count = static_cast<int>(strength);
            render::particle* p;

            for (int ii = 0; ii < count; ++ii) {
                if ( (p = add_particle()) == NULL )
                    return;

                r = frand()*math::pi<float>*2.0f;
                d = frand();
                p->position = position + vec2(cos(r)*d,sin(r)*d);
                p->velocity = direction * (0.25f + frand()*0.75f) + vec2(crand()*24,crand()*24);

                p->size = 2.0f + frand()*4.0f;
                p->size_velocity = 2.0f + frand()*2.0f;

                p->color = color4(0.5f,0.5f,0.5f,0.1f+frand()*0.1f);
                p->color_velocity = color4(0,0,0,-p->color.a / (1+frand()*1.0f));

                p->drag = 2.5f + frand() * 1.5f;
            }
            break;
        }

        case effect_type::sparks: {
            render::particle* p;

            for (int ii = 0; ii < 4; ++ii) {
                if ( (p = add_particle()) == NULL )
                    return;

                p->position = position + vec2(crand()*2,crand()*2);

                r = frand()*math::pi<float>*2.0f;
                d = frand()*128;

                p->velocity = vec2(cos(r)*d,sin(r)*d);
                p->velocity += direction * d * 0.5f;

                p->color = color4(1,0.5f+frand()*0.5f,0,strength*(0.5f+frand()));
                p->color_velocity = color4(0,-1.0f,0,-2.0f - frand());
                p->size = 0.5f;
                p->size_velocity = 0.0f;
                p->drag = 0.5f + frand() * 0.5f;
                p->flags = render::particle::tail;
            }

            for (int ii = 0; ii < 2; ++ii) {
                if ( (p = add_particle()) == NULL )
                    return;

                r = frand()*math::pi<float>*2.0f;
                d = frand()*24;

                p->position = position + vec2(cos(r)*d,sin(r)*d);

                r = frand()*math::pi<float>*2.0f;
                d = frand()*24;

                p->velocity = vec2(cos(r)*d,sin(r)*d);
                p->velocity += direction * d * 0.5f;

                p->size = 4.0f + frand()*8.0f;
                p->size_velocity = 2.0;

                p->color = color4(0.5f,0.5f,0.5f,0.1f+frand()*0.1f);
                p->color_velocity = color4(0,0,0,-p->color.a / (2+frand()*1.5f));

                p->drag = 0.5f + frand() * 2.0f;
            }
            break;
        }

        case effect_type::cannon_impact:
        case effect_type::missile_impact:
        case effect_type::explosion: {
            render::particle* p;
            float scale = std::sqrt(strength);

            // shock wave

            if ( (p = add_particle()) == NULL )
                return;

            p->position = position;
            p->velocity = direction * 48.0f * scale;

            p->color = color4(1.0f,1.0f,0.5f,0.5f);
            p->color_velocity = -p->color * color4(0,1,3,3);
            p->size = 12.0f * scale;
            p->size_velocity = 192.0f * scale;
            p->flags = render::particle::invert;

            // smoke

            for (int ii = 0; ii < 96 * scale; ++ii) {
                if ( (p = add_particle()) == NULL )
                    return;

                r = frand()*math::pi<float>*2.0f;
                d = frand()*12 * scale;

                p->position = position + vec2(cos(r),sin(r))*d;

                r = frand()*math::pi<float>*2.0f;
                d = sqrt(frand()) * 128.0f * strength;

                p->velocity = vec2(cos(r),sin(r)) * d;
                p->velocity += direction * d * 0.5f;

                p->size = (4.0f + frand()*8.0f) * scale;
                p->size_velocity = 2.0f * strength;

                p->color = color4(0.5f,0.5f,0.5f,0.1f+frand()*0.1f);
                p->color_velocity = color4(0,0,0,-p->color.a / (2+frand()*1.5f));

                p->drag = (3.0f + frand() * 1.0f) * scale;
            }

            // fire

            for (int ii = 0; ii < 64 * scale; ++ii) {
                if ( (p = add_particle()) == NULL )
                    return;

                r = frand()*math::pi<float>*2.0f;
                d = frand()*8 * scale;

                p->position = position + vec2(cos(r),sin(r))*d;

                r = frand()*math::pi<float>*2.0f;
                d = sqrt(frand()) * 128.0f * strength;

                p->velocity = vec2(cos(r),sin(r))*d;
                p->velocity += direction * d * 0.5f;

                p->color = color4(1.0f,frand(),0.0f,0.1f);
                p->color_velocity = color4(0,0,0,-p->color.a/(0.5f+frand()*frand()*2.5f));
                p->size = (8.0f + frand()*16.0f) * scale;
                p->size_velocity = 1.0f * strength;

                p->drag = (2.0f + frand() * 2.0f) * scale;
            }

            // debris

            for (int ii = 0; ii < 32 * scale; ++ii) {
                if ( (p = add_particle()) == NULL )
                    return;

                r = frand()*math::pi<float>*2.0f;
                d = frand()*2 * scale;

                p->position = position + vec2(cos(r)*d,sin(r)*d);

                r = frand()*math::pi<float>*2.0f;
                d = frand()*128 * scale;

                p->velocity = vec2(cos(r)*d,sin(r)*d);
                p->velocity += direction * d * 0.5f;

                p->color = color4(1,0.5f+frand()*0.5f,0,1);
                p->color_velocity = color4(0,0,0,-1.5f-frand());
                p->size = 0.5f;
                p->size_velocity = 0.0f;
                p->drag = 0.5f + frand() * 0.5f;
                p->flags = render::particle::tail;
            }
            break;
        }

        case effect_type::blaster: {
            render::particle* p;

            // vortex

            if ( (p = add_particle()) == NULL )
                return;

            p->position = position;
            p->velocity = direction * 9.6f;

            p->color = color4(1,0,0,0);
            p->color_velocity = color4(0,1,0,2.5f);
            p->size = 19.2f;
            p->size_velocity = -96.0f;
            p->flags = render::particle::invert;

            // fire

            for (int ii = 0; ii < 8; ++ii) {
                if ( (p = add_particle()) == NULL )
                    return;

                r = frand()*math::pi<float>*2.0f;
                d = frand()*4;

                p->position = position + vec2(cos(r),sin(r))*d;

                r = frand()*math::pi<float>*2.0f;
                d = sqrt(frand()) * 32.0f;

                p->velocity = vec2(cos(r),sin(r))*d;
                p->velocity += direction * d * 0.5f;

                p->color = color4(1.0f,0.5f*frand(),0.0f,0.1f);
                p->color_velocity = color4(0,0,0,-p->color.a/(0.25f+frand()*frand()));
                p->size = 2.0f + frand()*4.0f;
                p->size_velocity = 0.5f;
                p->flags = render::particle::invert;

                p->drag = 3.0f + frand() * 3.0f;

                // vortex

                render::particle* p2;

                if ( (p2 = add_particle()) == NULL )
                    return;

                p2->position = p->position;
                p2->velocity = p->velocity;
                p2->drag = p->drag;

                p2->color = color4(1,0,0,0);
                p2->color_velocity = color4(0,1,0,1);
                p2->size = 4.8f;
                p2->size_velocity = -18.0f;
                p2->flags = render::particle::invert;
            }

            // debris

            for (int ii = 0; ii < 4; ++ii) {
                if ( (p = add_particle()) == NULL )
                    return;

                r = frand()*math::pi<float>*2.0f;
                d = frand()*0.5f;

                p->position = position + vec2(cos(r)*d,sin(r)*d);

                r = frand()*math::pi<float>*2.0f;
                d = frand()*64.0f;

                p->velocity = vec2(cos(r)*d,sin(r)*d);
                p->velocity += direction * frand() * 96.0f;

                r = frand()*math::pi<float>*2.0f;
                d = 64.0f + frand()*64.0f;

                p->acceleration = vec2(cos(r), sin(r))*d;

                p->color = color4(1,frand()*0.5f,0,1);
                p->color_velocity = color4(0,0,0,-1.5f-2.0f*frand());
                p->size = 0.5f;
                p->size_velocity = 0.0f;
                p->drag = 2.0f + frand() * 2.0f;
                p->flags = render::particle::tail;
            }
            break;
        }

        case effect_type::blaster_impact: {
            render::particle* p;
            float scale = std::sqrt(strength);

            // vortex

            if ( (p = add_particle()) == NULL )
                return;

            p->position = position;
            p->velocity = direction * 48.0f * scale;

            p->color = color4(1,0,0,0);
            p->color_velocity = color4(0,1,0,2.5f);
            p->size = 96.0f * scale;
            p->size_velocity = -480.0f * scale;
            p->flags = render::particle::invert;

            // shock wave

            if ( (p = add_particle()) == NULL )
                return;

            p->position = position;
            p->velocity = direction * 48.0f * scale;

            p->color = color4(1.0f,0.25f,0.0f,0.5f);
            p->color_velocity = -p->color * color4(0,1,3,3);
            p->size = 12.0f * scale;
            p->size_velocity = 192.0f * scale;
            p->flags = render::particle::invert;

            // smoke

            for (int ii = 0; ii < 64 * scale; ++ii) {
                if ( (p = add_particle()) == NULL )
                    return;

                r = frand()*math::pi<float>*2.0f;
                d = frand()*12 * scale;

                p->position = position + vec2(cos(r),sin(r))*d;

                r = frand()*math::pi<float>*2.0f;
                d = sqrt(frand()) * 128.0f * strength;

                p->velocity = vec2(cos(r),sin(r)) * d;
                p->velocity += direction * d * 0.5f;

                p->size = (4.0f + frand()*8.0f) * scale;
                p->size_velocity = 2.0f * strength;

                p->color = color4(0.5f,0.5f,0.5f,0.1f+frand()*0.1f);
                p->color_velocity = color4(0,0,0,-p->color.a / (2+frand()*1.5f));

                p->drag = (3.0f + frand() * 1.0f) * scale;
            }

            // fire

            for (int ii = 0; ii < 64 * scale; ++ii) {
                if ( (p = add_particle()) == NULL )
                    return;

                r = frand()*math::pi<float>*2.0f;
                d = frand()*16 * scale;

                p->position = position + vec2(cos(r),sin(r))*d;

                r = frand()*math::pi<float>*2.0f;
                d = sqrt(frand()) * 128.0f * strength;

                p->velocity = vec2(cos(r),sin(r))*d;
                p->velocity += direction * d * 0.5f;

                p->color = color4(1.0f,0.5f*frand(),0.0f,0.1f);
                p->color_velocity = color4(0,0,0,-p->color.a/(0.5f+frand()*frand()*1.5f));
                p->size = (8.0f + frand()*16.0f) * scale;
                p->size_velocity = 1.0f * strength;

                p->drag = (2.0f + frand() * 2.0f) * scale;
            }

            // debris

            for (int ii = 0; ii < 32 * scale; ++ii) {
                if ( (p = add_particle()) == NULL )
                    return;

                r = frand()*math::pi<float>*2.0f;
                d = frand()*2 * scale;

                p->position = position + vec2(cos(r)*d,sin(r)*d);

                r = frand()*math::pi<float>*2.0f;
                d = frand()*128 * scale;

                p->velocity = vec2(cos(r)*d,sin(r)*d);
                p->velocity += direction * d * 0.5f;

                p->color = color4(1,frand()*0.5f,0,1);
                p->color_velocity = color4(0,0,0,-1.5f-frand());
                p->size = 0.5f;
                p->size_velocity = 0.0f;
                p->drag = 0.5f + frand() * 0.5f;
                p->flags = render::particle::tail;
            }
            break;
        }

        default:
            break;
    }
}

//------------------------------------------------------------------------------
void world::add_trail_effect(effect_type type, vec2 position, vec2 old_position, vec2 direction, float strength)
{
    float   r, d;

    vec2 lerp = position - old_position;

    switch (type) {
        case effect_type::missile_trail: {
            int count = static_cast<int>(strength);
            render::particle* p;

            // smoke

            for (int ii = 0; ii < count; ++ii) {
                if ( (p = add_particle()) == NULL )
                    return;

                r = frand()*math::pi<float>*2.0f;
                d = frand();
                p->position = position + vec2(cos(r)*d,sin(r)*d) + lerp * float(ii) / float(count);
                p->velocity = direction * (0.25f + frand()*0.75f) + vec2(crand()*24,crand()*24);

                p->size = 2.0f + frand()*4.0f;
                p->size_velocity = 2.0f + frand()*2.0f;

                p->color = color4(0.5f,0.5f,0.5f,0.1f+frand()*0.1f);
                p->color_velocity = color4(0,0,0,-p->color.a / (1+frand()*1.0f));

                p->drag = 2.5f + frand() * 1.5f;
                p->time += FRAMETIME * ii / count;
            }

            // fire

            for (int ii = 0; ii < count; ++ii) {
                if ( (p = add_particle()) == NULL )
                    return;

                r = frand()*math::pi<float>*2.0f;
                d = frand();
                p->position = position + vec2(cos(r)*d,sin(r)*d) + lerp * float(ii) / float(count);
                p->velocity = direction * (0.25f + frand()*0.75f) + vec2(crand()*24,crand()*24);

                p->size = 4.0f + frand()*2.0f;
                p->size_velocity = 4.0f + frand()*4.0f;

                p->color = color4(1.0f,0.5f+0.5f*frand(),0.0f,0.1f);
                p->color_velocity = color4(0,0,0,-p->color.a/(0.15f+0.15f*frand()));

                p->drag = (1.5f + frand() * 1.0f);
                p->time += FRAMETIME * ii / count;
            }

            // debris

            for (int ii = 0; ii < count; ++ii) {
                if ( (p = add_particle()) == NULL )
                    return;

                r = frand()*math::pi<float>*2.0f;
                d = frand();

                p->position = position + vec2(cos(r),sin(r))*d + lerp * float(ii) / float(count);

                r = frand()*math::pi<float>*2.0f;
                d = frand() * 64.0f;

                p->velocity = direction * (0.25f + frand()*0.75f) + vec2(crand()*48,crand()*48);

                p->color = color4(1,0.5f+frand()*0.5f,0,1);
                p->color_velocity = color4(0,0,0,-3.0f-15.0f*(1.0f-square(frand())));
                p->size = 0.5f;
                p->size_velocity = 0.0f;
                p->drag = 1.5f + frand() * 1.5f;
                p->time += FRAMETIME * ii / count;
                p->flags = render::particle::tail;
            }

            break;
        }
        default:
            break;
    }
}

} // namespace game
