// g_world.cpp
//

#include "local.h"
#pragma hdrstop

#include "p_collide.h"
#include "p_trace.h"

#include <algorithm>

////////////////////////////////////////////////////////////////////////////////
namespace game {

//------------------------------------------------------------------------------
void world::init()
{
    char const* command;

    clear_particles();

    if ( (command = strstr( g_Application->init_string(), "particles=" )) ) {
        _use_particles = ( atoi(command+10) > 0 );
    } else {
        _use_particles = true;
    }

    _border_material = physics::material(0, 0);

    reset();
}

//------------------------------------------------------------------------------
void world::shutdown()
{
}

//------------------------------------------------------------------------------
void world::reset()
{
    _mins = vec2(0,0);
    _maxs = vec2(_arena_width, _arena_height);

    _border_shapes[0] = physics::box_shape(vec2(_border_thickness + _arena_width, _border_thickness));
    _border_shapes[1] = physics::box_shape(vec2(_border_thickness, _border_thickness + _arena_height));

    _objects.clear();
    _pending.clear();
    _removed.clear();

    _spawn_id = 0;

    // Initialize border objects
    {
        vec2 mins = vec2(-_border_thickness / 2, -_border_thickness / 2);
        vec2 maxs = vec2(_arena_width, _arena_height) - mins;

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

//------------------------------------------------------------------------------
void world::remove(game::object* object)
{
    _removed.insert(object);
}

//------------------------------------------------------------------------------
void world::draw(render::system* renderer) const
{
    for (auto& obj : _objects) {
        obj->draw(renderer);
    }

    draw_particles(renderer);
}

//------------------------------------------------------------------------------
void world::run_frame()
{
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

    for (auto& obj : _pending) {
        _objects.push_back(std::move(obj));
    }
    _pending.clear();
}

//------------------------------------------------------------------------------
void world::move_object(game::object *object)
{
    object->_old_position = object->get_position();
    object->_old_rotation = object->get_rotation();

    if (object->get_linear_velocity().lengthsq() < 1e-12f
            && std::abs(object->get_angular_velocity()) < 1e-6f) {
        return;
    }

    if (object->_type == object_type::projectile) {
        game::object* best_object = NULL;
        float best_fraction = 1.f;
        physics::contact contact;

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

            if (tr.get_fraction() < best_fraction) {
                best_fraction = tr.get_fraction();
                best_object = other.get();
                contact = tr.get_contact();
            }
        }

        if (best_object) {
            object->set_position(start + (end - start) * best_fraction);
            object->touch(best_object, &contact);
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
                object->apply_impulse(
                    -c.get_contact().impulse,
                    c.get_contact().point
                );

                other->apply_impulse(
                    c.get_contact().impulse,
                    c.get_contact().point
                );

                object->touch(other.get(), &c.get_contact());
            }
        }

        object->set_position(object->get_position() + object->get_linear_velocity() * FRAMETIME);
        object->set_rotation(object->get_rotation() + object->get_angular_velocity() * FRAMETIME);
    }
}

//------------------------------------------------------------------------------
void world::add_sound(int sound_index)
{
    g_Game->write_sound(sound_index);
    pSound->play(sound_index, vec3(0,0,0), 1.0f, 0.0f);
}

//------------------------------------------------------------------------------
void world::add_effect(effect_type type, vec2 position, vec2 direction, float strength)
{
    g_Game->write_effect(static_cast<int>(type), position, direction, strength);

    float   r, d;

    switch (type) {
        case effect_type::smoke: {
            int count = strength;
            render::particle* p;

            for (int ii = 0; ii < count; ++ii) {
                if ( (p = add_particle()) == NULL )
                    return;

                r = frand()*M_PI*2.0f;
                d = frand();
                p->position = position + vec2(cos(r)*d,sin(r)*d);
                p->velocity = direction * (0.25 + frand()*0.75) + vec2(crand()*24,crand()*24);

                p->size = 2.0f + frand()*4.0f;
                p->size_velocity = 2.0 + frand()*2.0f;

                p->color = vec4(0.5,0.5,0.5,0.1+frand()*0.1f);
                p->color_velocity = vec4(0,0,0,-p->color.a / (1+frand()*1.0f));

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

                r = frand()*M_PI*2.0f;
                d = frand()*128;

                p->velocity = vec2(cos(r)*d,sin(r)*d);
                p->velocity += direction * d * 0.5f;

                p->color = vec4(1,0.5+frand()*0.5,0,strength*(0.5f+frand()));
                p->color_velocity = vec4(0,-1.0f,0,-2.0f - frand());
                p->size = 0.5f;
                p->size_velocity = 0.0f;
                p->drag = 0.5f + frand() * 0.5f;
                p->flags = render::particle::tail;
            }

            for (int ii = 0; ii < 2; ++ii) {
                if ( (p = add_particle()) == NULL )
                    return;

                r = frand()*M_PI*2.0f;
                d = frand()*24;

                p->position = position + vec2(cos(r)*d,sin(r)*d);

                r = frand()*M_PI*2.0f;
                d = frand()*24;

                p->velocity = vec2(cos(r)*d,sin(r)*d);
                p->velocity += direction * d * 0.5f;

                p->size = 4.0f + frand()*8.0f;
                p->size_velocity = 2.0;

                p->color = vec4(0.5,0.5,0.5,0.1+frand()*0.1f);
                p->color_velocity = vec4(0,0,0,-p->color.a / (2+frand()*1.5f));

                p->drag = 0.5f + frand() * 2.0f;
            }
            break;
        }

        case effect_type::explosion: {
            render::particle* p;
            float scale = std::sqrt(strength);

            // shock wave

            if ( (p = add_particle()) == NULL )
                return;

            p->position = position;
            p->velocity = direction * 48.0f * scale;

            p->color = vec4(1.0f,1.0f,0.5f,0.5f);
            p->color_velocity = -p->color * vec4(0,1,3,3);
            p->size = 12.0 * scale;
            p->size_velocity = 192.0f * scale;
            p->flags = render::particle::invert;

            // smoke

            for (int ii = 0; ii < 96 * scale; ++ii) {
                if ( (p = add_particle()) == NULL )
                    return;

                r = frand()*M_PI*2.0f;
                d = frand()*12 * scale;

                p->position = position + vec2(cos(r),sin(r))*d;

                r = frand()*M_PI*2.0f;
                d = sqrt(frand()) * 128.0f * strength;

                p->velocity = vec2(cos(r),sin(r)) * d;
                p->velocity += direction * d * 0.5f;

                p->size = (4.0f + frand()*8.0f) * scale;
                p->size_velocity = 2.0 * strength;

                p->color = vec4(0.5,0.5,0.5,0.1+frand()*0.1f);
                p->color_velocity = vec4(0,0,0,-p->color.a / (2+frand()*1.5f));

                p->drag = (3.0f + frand() * 1.0f) * scale;
            }

            // fire

            for (int ii = 0; ii < 64 * scale; ++ii) {
                if ( (p = add_particle()) == NULL )
                    return;

                r = frand()*M_PI*2.0f;
                d = frand()*8 * scale;

                p->position = position + vec2(cos(r),sin(r))*d;

                r = frand()*M_PI*2.0f;
                d = sqrt(frand()) * 128.0f * strength;

                p->velocity = vec2(cos(r),sin(r))*d;
                p->velocity += direction * d * 0.5f;

                p->color = vec4(1.0f,frand(),0.0f,0.1f);
                p->color_velocity = vec4(0,0,0,-p->color.a/(0.5+frand()*frand()*2.5f));
                p->size = (8.0 + frand()*16.0f) * scale;
                p->size_velocity = 1.0f * strength;

                p->drag = (2.0f + frand() * 2.0f) * scale;
            }

            // debris

            for (int ii = 0; ii < 32 * scale; ++ii) {
                if ( (p = add_particle()) == NULL )
                    return;

                r = frand()*M_PI*2.0f;
                d = frand()*2 * scale;

                p->position = position + vec2(cos(r)*d,sin(r)*d);

                r = frand()*M_PI*2.0f;
                d = frand()*128 * scale;

                p->velocity = vec2(cos(r)*d,sin(r)*d);
                p->velocity += direction * d * 0.5f;

                p->color = vec4(1,0.5+frand()*0.5,0,1);
                p->color_velocity = vec4(0,0,0,-1.5f-frand());
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
void world::clear_particles()
{
    _particles.clear();
}

} // namespace game
