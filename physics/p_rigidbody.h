// p_rigidbody.h
//

#pragma once

#include "cm_bounds.h"
#include "cm_vector.h"
#include "cm_matrix.h"

////////////////////////////////////////////////////////////////////////////////
namespace physics {

class shape;
class material;

//------------------------------------------------------------------------------
class rigid_body
{
public:
    rigid_body(shape const* shape, material const* material, float mass)
        : _shape(shape)
        , _material(material)
        , _position(0,0)
        , _rotation(0)
        , _linear_velocity(0,0)
        , _angular_velocity(0)
        , _inverse_mass(0)
        , _inverse_inertia(0)
    {
        set_mass(mass);
    }

    //
    //  position
    //

    vec2 get_position() const {
        return _position;
    }

    void set_position(vec2 position) {
        _position = position;
    }

    float get_rotation() const {
        return _rotation;
    }

    void set_rotation(float rotation) {
        _rotation = rotation;
    }

    mat3 get_transform() const;

    mat3 get_inverse_transform() const;

    bounds get_bounds() const;

    //
    //  velocity
    //

    vec2 get_linear_velocity() const {
        return _linear_velocity;
    }

    vec2 get_linear_velocity(vec2 position) const;

    void set_linear_velocity(vec2 linear_velocity) {
        _linear_velocity = linear_velocity;
    }

    float get_angular_velocity() const {
        return _angular_velocity;
    }

    void set_angular_velocity(float angular_velocity) {
        _angular_velocity = angular_velocity;
    }

    float get_kinetic_energy() const;

    //
    //  dynamics
    //

    void apply_impulse(vec2 impulse);

    void apply_impulse(vec2 impulse, vec2 position);

    //
    //  properties
    //

    float get_mass() const {
        return _inverse_mass ? 1.0f / _inverse_mass : 0.0f;
    }

    void set_mass(float mass);

    float get_inverse_mass() const {
        return _inverse_mass;
    }

    float get_inverse_inertia() const {
        return _inverse_inertia;
    }

    shape const* get_shape() const {
        return _shape;
    }

    material const* get_material() const {
        return _material;
    }

protected:
    vec2 _position;
    float _rotation;

    float _inverse_mass;
    float _inverse_inertia;
    vec2 _center_of_mass;

    vec2 _linear_velocity;
    float _angular_velocity;

    shape const* _shape;
    material const* _material;
};

} // namespace physics
