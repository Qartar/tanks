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

cWorld *g_World;

/*
===========================================================

Name    :   cWorld::Init

Purpose :   Initializes world

===========================================================
*/

extern cvar_t   *g_arenaWidth;
extern cvar_t   *g_arenaHeight;

void cWorld::Init ()
{
    char    *command;

    ClearParticles ();
    g_World = this;

    if ( (command = strstr( g_Application->InitString(), "particles=" )) )
        m_bParticles = ( atoi(command+10) > 0 );
    else
        m_bParticles = true;

    _border_material = physics::material(0, 0);

    Reset();
}

void cWorld::Shutdown ()
{
}

void cWorld::Reset ()
{
    m_vWorldMins = vec2(0,0);
    m_vWorldMaxs = vec2(
        g_arenaWidth->getInt( ),
        g_arenaHeight->getInt( ) );

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
            spawn<cStatic>(std::move(body));
        }
    }
}

/*
===========================================================

Name    :   cWorld::AddObject / ::DelObject

Purpose :   adds and removes objects from the object list

===========================================================
*/

void cWorld::remove (cObject* object)
{
    _removed.insert(object);
}

/*
===========================================================

Name    :   cWorld::Draw

Purpose :   Renders the world to the screen

===========================================================
*/

void cWorld::Draw ()
{
    for (auto& obj : _objects) {
        obj->Draw();
    }

    m_DrawParticles( );
}

/*
===========================================================

Name    :   cWorld::RunFrame

Purpose :   Runs the world one frame, runs think funcs and movement

===========================================================
*/

void cWorld::RunFrame ()
{
    for (auto& obj : _pending) {
        _objects.push_back(std::move(obj));
    }
    _pending.clear();

    for (auto& obj : _objects) {
        obj->Think();
    }

    for (auto& obj : _objects) {
        MoveObject(obj.get());
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

Name    :   cWorld::MoveObject

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

void cWorld::MoveObject (cObject *pObject)
{
    pObject->oldPos = pObject->get_position();
    pObject->oldAngle = pObject->get_rotation();

    if (pObject->get_linear_velocity().lengthsq() < 1e-12f
            && pObject->get_angular_velocity() < 1e-6f) {
        return;
    }

    if (pObject->eType == object_bullet)
    {
        cObject* bestObject = NULL;
        float bestFraction = 1.f;

        vec2 start = pObject->get_position();
        vec2 end = start + pObject->get_linear_velocity() * FRAMETIME;

        for (auto& other : _objects)
        {
            if (other.get() == pObject)
                continue;

            if (pObject->_owner == other.get())
                continue;

            auto tr = physics::trace(&other->rigid_body(), start, end);

            if (tr.get_fraction() < bestFraction)
            {
                bestFraction = tr.get_fraction();
                bestObject = other.get();
            }
        }

        if (bestObject)
        {
            pObject->set_position(start + (end - start) * bestFraction);
            pObject->Touch( bestObject );
        }
        else
        {
            pObject->set_position(end);
        }
    }
    else
    {
        for (auto& other : _objects)
        {
            if (other.get() == pObject)
                continue;

            if (other->_owner == pObject)
                continue;

            auto c = physics::collide(&pObject->rigid_body(), &other->rigid_body());

            if (c.has_contact()) {
                float impulse = c.get_contact().impulse.length();
                float strength = clamp((impulse - 5.0f) / 5.0f, 0.0f, 1.0f);
                AddEffect(c.get_contact().point, effect_sparks, strength);

                pObject->apply_impulse(
                    -c.get_contact().impulse,
                    c.get_contact().point
                );

                other->apply_impulse(
                    c.get_contact().impulse,
                    c.get_contact().point
                );

                pObject->Touch( other.get(), impulse );
            }
        }

        pObject->set_position(pObject->get_position() + pObject->get_linear_velocity() * FRAMETIME);
        pObject->set_rotation(pObject->get_rotation() + pObject->get_angular_velocity() * FRAMETIME);
    }
}

/*
===========================================================

Name    :   cWorld::AddSound

Purpose :   sound!

===========================================================
*/

void cWorld::AddSound (char *szName)
{
    for ( int i=0 ; i<NUM_SOUNDS ; i++ )
    {
        if ( sound_index[i].name && stricmp( szName, sound_index[i].name ) == 0 )
        {
            g_Game->m_WriteSound( sound_index[i].index );
            pSound->playSound( sound_index[i].index, vec3(0,0,0), 1.0f, 0.0f );
        }
    }
}

/*
===========================================================

Name    :   cWorld::AddEffect

Purpose :   adds particle effects

===========================================================
*/

void cWorld::AddEffect (vec2 vPos, eEffects eType, float strength)
{
    g_Game->m_WriteEffect( eType, vPos, vec2(0,0), 0 );

    float   r, d;

    if (eType == effect_sparks)
    {
        int     i;
        cParticle   *p;

        for (i=0 ; i<4 ; i++)
        {
            if ( (p = AddParticle()) == NULL )
                return;

            p->vPos = vPos + vec2(crand()*2,crand()*2);
            p->vVel = vec2(crand()*128,crand()*128);

            p->vColor = vec4(1,0.5+frand()*0.5,0,strength*(0.5f+frand()));
            p->vColorVel = vec4(0,-1.0f,0,-2.0f - frand());
            p->flSize = 2.0f;
            p->flSizeVel = 0.0f;
            p->flDrag = 0.99 - frand()*0.03;
        }

        for (i=0 ; i<2 ; i++)
        {
            if ( (p = AddParticle()) == NULL )
                return;

            r = frand()*M_PI*2.0f;
            d = frand()*24;

            p->vPos = vPos + vec2(cos(r)*d,sin(r)*d);

            r = frand()*M_PI*2.0f;
            d = frand()*24;

            p->vVel = vec2(cos(r)*d,sin(r)*d);

            p->flSize = 4.0f + frand()*8.0f;
            p->flSizeVel = 2.0;

            p->vColor = vec4(0.5,0.5,0.5,0.1+frand()*0.1f);
            p->vColorVel = vec4(0,0,0,-p->vColor.a / (2+frand()*1.5f));

            p->flDrag = 0.98f - frand()*0.05;
        }
    }
    else if (eType == effect_explosion)
    {
        int     i;
        cParticle   *p;

        // shock wave

        if ( (p = AddParticle()) == NULL )
            return;

        p->vPos = vPos;
        p->vVel = vec2(0,0);

        p->vColor = vec4(1.0f,1.0f,0.5f,0.5f);
        p->vColorVel = vec4(0,0,0,-p->vColor.a/(0.3f));
        p->flSize = 24.0;
        p->flSizeVel = 192.0f;
        p->bitFlags = PF_INVERT;

        p->flDrag = 0.95 - frand()*0.03;

        // smoke

        for (i=0 ; i<128 ; i++)
        {
            if ( (p = AddParticle()) == NULL )
                return;

            r = frand()*M_PI*2.0f;
            d = frand()*24;

            p->vPos = vPos + vec2(cos(r)*d,sin(r)*d);

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

        for (i=0 ; i<96 ; i++)
        {
            if ( (p = AddParticle()) == NULL )
                return;

            r = frand()*M_PI*2.0f;
            d = frand()*16;

            p->vPos = vPos + vec2(cos(r)*d,sin(r)*d);

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

        for (i=0 ; i<32 ; i++)
        {
            if ( (p = AddParticle()) == NULL )
                return;

            r = frand()*M_PI*2.0f;
            d = frand()*2;

            p->vPos = vPos + vec2(cos(r)*d,sin(r)*d);

            r = frand()*M_PI*2.0f;
            d = frand()*128;

            p->vVel = vec2(cos(r)*d,sin(r)*d);

            p->vColor = vec4(1,0.5+frand()*0.5,0,1);
            p->vColorVel = vec4(0,0,0,-1.5f-frand());
            p->flSize = 2.0f;
            p->flSizeVel = 0.0f;
        }
    }
}

void cWorld::AddSmokeEffect (vec2 vPos, vec2 vVel, int nCount)
{
    int         i;
    float       r, d;
    cParticle   *p;

    g_Game->m_WriteEffect( effect_smoke, vPos, vVel, nCount );

    for (i=0 ; i<nCount ; i++)
    {
        if ( (p = AddParticle()) == NULL )
            return;

        r = frand()*M_PI*2.0f;
        d = frand();
        p->vPos = vPos + vec2(cos(r)*d,sin(r)*d);
        p->vVel = vVel * (0.25 + frand()*0.75) + vec2(crand()*24,crand()*24);

        p->flSize = 4.0f + frand()*8.0f;
        p->flSizeVel = 2.0;

        p->vColor = vec4(0.5,0.5,0.5,0.1+frand()*0.1f);
        p->vColorVel = vec4(0,0,0,-p->vColor.a / (1+frand()*1.0f));

        p->flDrag = 0.98f - frand()*0.05;
    }
}


void cWorld::AddFlagTrail (vec2 vPos, int nTeam)
{
    int         i;
    cParticle   *p;

    for (i=0 ; i<4 ; i++)
    {
        if ( (p = AddParticle()) == NULL )
            return;

        p->vPos = vPos + vec2(crand()*4,crand()*4);
        p->vVel = vec2(crand()*24,crand()*24);

        p->flSize = 4.0f + frand()*8.0f;
        p->flSizeVel = 2.0;

        if ( nTeam == 0 )   // red
            p->vColor = vec4(1.0,0.0,0.0,0.1+frand()*0.1f);
        else if ( nTeam == 1 )  // blue
            p->vColor = vec4(0.0,0.0,1.0,0.1+frand()*0.1f);

        p->vColorVel = vec4(0,0,0,-p->vColor.a / (1.0+frand()*0.5f));

        p->flDrag = 0.98f - frand()*0.05;
    }
}

void cWorld::ClearParticles ()
{
    int     i;
    
    memset (&m_Particles, 0, sizeof(m_Particles));

    pFreeParticles = &m_Particles[0];
    pActiveParticles = NULL;

    for (i=0 ;i<MAX_PARTICLES ; i++)
        m_Particles[i].pNext = &m_Particles[i+1];
    m_Particles[MAX_PARTICLES-1].pNext = NULL;
}
