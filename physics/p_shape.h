//  p_shape.h
//

#pragma once

#include "cm_bounds.h"
#include "cm_vector.h"

#ifndef M_SQRT2
#define M_SQRT2    1.41421356237309504880
#endif

////////////////////////////////////////////////////////////////////////////////
namespace physics {

//------------------------------------------------------------------------------
class shape
{
public:
    virtual ~shape() {}

    virtual bool contains_point(vec2 point) const = 0;

    virtual vec2 supporting_vertex(vec2 direction) const = 0;

    virtual void calculate_mass_properties(float inverse_mass, vec2& center_of_mass, float& inverse_inertia) const = 0;

    virtual bounds calculate_bounds(vec2 position, float rotation) const = 0;
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
        return _half_size * mat2::scale(direction.x < 0.f ? -1.f : 1.f,
                                        direction.y < 0.f ? -1.f : 1.f);
    }

    virtual void calculate_mass_properties(float inverse_mass, vec2& center_of_mass, float& inverse_inertia) const override {
        center_of_mass = vec2(0,0);
        if (inverse_mass > 0.f) {
            inverse_inertia = 3.0f * inverse_mass / _half_size.dot(_half_size);
        } else {
            inverse_inertia = 0.0f;
        }
    }

    virtual bounds calculate_bounds(vec2 position, float /*rotation*/) const {
        return bounds(position - _half_size * float(M_SQRT2), position + _half_size * float(M_SQRT2));
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

    virtual bounds calculate_bounds(vec2 position, float /*rotation*/) const {
        return bounds(position - vec2(_radius), position + vec2(_radius));
    }

protected:
    float _radius;
};

} // namespace physics
