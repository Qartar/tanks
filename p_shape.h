//  p_shape.h
//

#pragma once

#include "oed_types.h"

namespace physics {

//------------------------------------------------------------------------------
class shape
{
public:
    virtual ~shape() {}

    virtual bool contains_point(vec2 point) const = 0;

    virtual vec2 supporting_vertex(vec2 direction) const = 0;

    virtual void calculate_mass_properties(float inverse_mass, vec2& center_of_mass, float& inverse_inertia) const = 0;
};

//------------------------------------------------------------------------------
class box_shape : public shape
{
public:
    box_shape(vec2 size)
        : _half_size(size * 0.5f)
    {
    }

    virtual bool contains_point(vec2 point) const override {
        return !(point.x < -_half_size.x || point.x > _half_size.x ||
                    point.y < -_half_size.y || point.y > _half_size.y);
    }

    virtual vec2 supporting_vertex(vec2 direction) const override {
        vec2 sign(direction.x < 0.f ? -1.f : 1.f,
                  direction.y < 0.f ? -1.f : 1.f);

        return sign * _half_size;
    }

    virtual void calculate_mass_properties(float inverse_mass, vec2& center_of_mass, float& inverse_inertia) const override {
        center_of_mass = vec2(0,0);
        if (inverse_mass > 0.f) {
            inverse_inertia = 3.0f * inverse_mass / _half_size.dot(_half_size);
        } else {
            inverse_inertia = 0.0f;
        }
    }

protected:
    vec2 _half_size;
};

//------------------------------------------------------------------------------
class circle_shape : public shape
{
public:
    circle_shape(float radius)
        : _radius(radius)
    {
    }

    virtual bool contains_point(vec2 point) const override {
        return point.dot(point) < _radius * _radius;
    }

    virtual vec2 supporting_vertex(vec2 direction) const override {
        return direction * _radius / direction.length();
    }

    virtual void calculate_mass_properties(float inverse_mass, vec2& center_of_mass, float& inverse_inertia) const override {
        center_of_mass = vec2(0,0);
        if (inverse_mass > 0.0f) {
            inverse_inertia = 2.0f * inverse_mass / (_radius * _radius); 
        } else {
            inverse_inertia = 0.0f;
        }
    }

protected:
    float _radius;
};

} // namespace physics
