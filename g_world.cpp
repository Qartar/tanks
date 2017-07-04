/*
===============================================================================

Name    :   g_world.cpp

Purpose :   World Object

Date    :   10/21/2004

===============================================================================
*/

#include "local.h"
#pragma hdrstop

#include "p_collide.h"
#include "p_trace.h"

#include <algorithm>

game::world *g_World;

extern cvar_t   *g_arenaWidth;
extern cvar_t   *g_arenaHeight;

namespace game {

/*
===========================================================

Name    :   world::Init

Purpose :   Initializes world

===========================================================
*/

void world::init()
{
    char    *command;

    clear_particles();
    g_World = this;

    if ( (command = strstr( g_Application->InitString(), "particles=" )) )
        m_bParticles = ( atoi(command+10) > 0 );
    else
        m_bParticles = true;

    _border_material = physics::material(0, 0);

    reset();
}

void world::shutdown()
{
}

void world::reset()
{
    _mins = vec2(0,0);
    _maxs = vec2(g_arenaWidth->getInt(), g_arenaHeight->getInt());

    _border_shapes[0] = physics::box_shape(vec2(_border_thickness + g_arenaWidth->getInt(), _border_thickness));
    _border_shapes[1] = physics::box_shape(vec2(_border_thickness, _border_thickness + g_arenaHeight->getInt()));

    _objects.clear();
    _pending.clear();
    _removed.clear();

    _spawn_id = 0;

    // Initialize border objects
    {
        vec2 mins = vec2(-_border_thickness / 2, -_border_thickness / 2);
        vec2 maxs = vec2(g_arenaWidth->getInt(), g_arenaHeight->getInt()) - mins;

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
}

/*
===========================================================

Name    :   world::AddObject / ::DelObject

Purpose :   adds and removes objects from the object list

===========================================================
*/

void world::remove(game::object* object)
{
    _removed.insert(object);
}

/*
===========================================================

Name    :   world::Draw

Purpose :   Renders the world to the screen

===========================================================
*/

void world::draw() const
{
    for (auto& obj : _objects) {
        obj->draw();
    }

    m_DrawParticles( );
}

/*
===========================================================

Name    :   world::RunFrame

Purpose :   Runs the world one frame, runs think funcs and movement

===========================================================
*/

void world::run_frame()
{
    for (auto& obj : _pending) {
        _objects.push_back(std::move(obj));
    }
    _pending.clear();

    for (auto& obj : _objects) {
        obj->think();
    }

    for (auto& obj : _objects) {
        move_object(obj.get());
    }

    for (auto obj : _removed) {
        _objects.erase(std::find_if(_objects.begin(), _objects.end(), [=](auto& it){
            return it.get() == obj;
        }));
    }
    _removed.clear();
}

/*
===========================================================

Name    :   world::MoveObject

Purpose :   moves an object in the world according to its velocity

===========================================================
*/

#define NUM_STEPS   8
#define STEP_SIZE   0.125

int segs[4][2] = {
    { 0, 1 },
    { 1, 2 },
    { 2, 3 },
    { 3, 0 } };

void world::move_object(game::object *object)
{
    object->_old_position = object->get_position();
    object->_old_rotation = object->get_rotation();

    if (object->get_linear_velocity().lengthsq() < 1e-12f
            && std::abs(object->get_angular_velocity()) < 1e-6f) {
        return;
    }

    if (object->_type == object_type::projectile) {
        game::object* bestObject = NULL;
        float bestFraction = 1.f;

        vec2 start = object->get_position();
        vec2 end = start + object->get_linear_velocity() * FRAMETIME;

        for (auto& other : _objects) {
            if (other.get() == object) {
                continue;
            }

            if (object->_owner == other.get()) {
                continue;
            }

            auto tr = physics::trace(&other->rigid_body(), start, end);

            if (tr.get_fraction() < bestFraction) {
                bestFraction = tr.get_fraction();
                bestObject = other.get();
            }
        }

        if (bestObject) {
            object->set_position(start + (end - start) * bestFraction);
            object->touch( bestObject );
        } else {
            object->set_position(end);
        }
    } else {
        for (auto& other : _objects) {
            if (other.get() == object) {
                continue;
            }

            if (other->_owner == object) {
                continue;
            }

            auto c = physics::collide(&object->rigid_body(), &other->rigid_body());

            if (c.has_contact()) {
                float impulse = c.get_contact().impulse.length();
                float strength = clamp((impulse - 5.0f) / 5.0f, 0.0f, 1.0f);
                add_effect(c.get_contact().point, effect_type::sparks, strength);

                object->apply_impulse(
                    -c.get_contact().impulse,
                    c.get_contact().point
                );

                other->apply_impulse(
                    c.get_contact().impulse,
                    c.get_contact().point
                );

                object->touch(other.get(), impulse);
            }
        }

        object->set_position(object->get_position() + object->get_linear_velocity() * FRAMETIME);
        object->set_rotation(object->get_rotation() + object->get_angular_velocity() * FRAMETIME);
    }
}

/*
===========================================================

Name    :   world::AddSound

Purpose :   sound!

===========================================================
*/

void world::add_sound(char const* name) {
    for (int ii=0; ii < NUM_SOUNDS; ++ii) {
        if (sound_index[ii].name && stricmp( name, sound_index[ii].name) == 0) {
            g_Game->m_WriteSound(sound_index[ii].index);
            pSound->playSound(sound_index[ii].index, vec3(0,0,0), 1.0f, 0.0f);
        }
    }
}

/*
===========================================================

Name    :   world::AddEffect

Purpose :   adds particle effects

===========================================================
*/

void world::add_effect(vec2 position, effect_type type, float strength)
{
    g_Game->m_WriteEffect(static_cast<int>(type), position, vec2(0,0), 0);

    float   r, d;
    
    switch (type) {
        case effect_type::sparks: {
            cParticle   *p;

            for (int ii = 0; ii < 4; ++ii) {
                if ( (p = AddParticle()) == NULL )
                    return;

                p->vPos = position + vec2(crand()*2,crand()*2);
                p->vVel = vec2(crand()*128,crand()*128);

                p->vColor = vec4(1,0.5+frand()*0.5,0,strength*(0.5f+frand()));
                p->vColorVel = vec4(0,-1.0f,0,-2.0f - frand());
                p->flSize = 2.0f;
                p->flSizeVel = 0.0f;
                p->flDrag = 0.99 - frand()*0.03;
            }

            for (int ii = 0; ii < 2; ++ii) {
                if ( (p = AddParticle()) == NULL )
                    return;

                r = frand()*M_PI*2.0f;
                d = frand()*24;

                p->vPos = position + vec2(cos(r)*d,sin(r)*d);

                r = frand()*M_PI*2.0f;
                d = frand()*24;

                p->vVel = vec2(cos(r)*d,sin(r)*d);

                p->flSize = 4.0f + frand()*8.0f;
                p->flSizeVel = 2.0;

                p->vColor = vec4(0.5,0.5,0.5,0.1+frand()*0.1f);
                p->vColorVel = vec4(0,0,0,-p->vColor.a / (2+frand()*1.5f));

                p->flDrag = 0.98f - frand()*0.05;
            }
            break;
        }

        case effect_type::explosion: {
            cParticle   *p;

            // shock wave

            if ( (p = AddParticle()) == NULL )
                return;

            p->vPos = position;
            p->vVel = vec2(0,0);

            p->vColor = vec4(1.0f,1.0f,0.5f,0.5f);
            p->vColorVel = vec4(0,0,0,-p->vColor.a/(0.3f));
            p->flSize = 24.0;
            p->flSizeVel = 192.0f;
            p->bitFlags = PF_INVERT;

            p->flDrag = 0.95 - frand()*0.03;

            // smoke

            for (int ii = 0; ii < 128; ++ii) {
                if ( (p = AddParticle()) == NULL )
                    return;

                r = frand()*M_PI*2.0f;
                d = frand()*24;

                p->vPos = position + vec2(cos(r)*d,sin(r)*d);

                r = frand()*M_PI*2.0f;
                d = frand()*24;

                p->vVel = vec2(cos(r)*d,sin(r)*d);

                p->flSize = 4.0f + frand()*8.0f;
                p->flSizeVel = 2.0;

                p->vColor = vec4(0.5,0.5,0.5,0.1+frand()*0.1f);
                p->vColorVel = vec4(0,0,0,-p->vColor.a / (2+frand()*1.5f));

                p->flDrag = 0.98f - frand()*0.05;
            }

            // fire

            for (int ii = 0; ii < 96; ++ii) {
                if ( (p = AddParticle()) == NULL )
                    return;

                r = frand()*M_PI*2.0f;
                d = frand()*16;

                p->vPos = position + vec2(cos(r)*d,sin(r)*d);

                r = frand()*M_PI*2.0f;
                d = frand()*128;

                p->vVel = vec2(cos(r)*d,sin(r)*d);

                p->vColor = vec4(1.0f,frand(),0.0f,0.1f);
                p->vColorVel = vec4(0,0,0,-p->vColor.a/(0.5+frand()*frand()*2.5f));
                p->flSize = 8.0 + frand()*16.0f;
                p->flSizeVel = 1.0f;

                p->flDrag = 0.95 - frand()*0.03;
            }

            // debris

            for (int ii = 0; ii < 32; ++ii) {
                if ( (p = AddParticle()) == NULL )
                    return;

                r = frand()*M_PI*2.0f;
                d = frand()*2;

                p->vPos = position + vec2(cos(r)*d,sin(r)*d);

                r = frand()*M_PI*2.0f;
                d = frand()*128;

                p->vVel = vec2(cos(r)*d,sin(r)*d);

                p->vColor = vec4(1,0.5+frand()*0.5,0,1);
                p->vColorVel = vec4(0,0,0,-1.5f-frand());
                p->flSize = 2.0f;
                p->flSizeVel = 0.0f;
            }
            break;
        }

        default:
            break;
    }
}

void world::add_smoke_effect(vec2 position, vec2 velocity, int count)
{
    float       r, d;
    cParticle   *p;

    g_Game->m_WriteEffect(static_cast<int>(effect_type::smoke), position, velocity, count);

    for (int ii = 0; ii < count; ++ii) {
        if ( (p = AddParticle()) == NULL )
            return;

        r = frand()*M_PI*2.0f;
        d = frand();
        p->vPos = position + vec2(cos(r)*d,sin(r)*d);
        p->vVel = velocity * (0.25 + frand()*0.75) + vec2(crand()*24,crand()*24);

        p->flSize = 4.0f + frand()*8.0f;
        p->flSizeVel = 2.0;

        p->vColor = vec4(0.5,0.5,0.5,0.1+frand()*0.1f);
        p->vColorVel = vec4(0,0,0,-p->vColor.a / (1+frand()*1.0f));

        p->flDrag = 0.98f - frand()*0.05;
    }
}

void world::clear_particles()
{
    memset (&m_Particles, 0, sizeof(m_Particles));

    pFreeParticles = &m_Particles[0];
    pActiveParticles = NULL;

    for (int ii = 0; ii < MAX_PARTICLES; ++ii) {
        m_Particles[ii].pNext = &m_Particles[ii+1];
    }
    m_Particles[MAX_PARTICLES-1].pNext = NULL;
}

} // namespace game
