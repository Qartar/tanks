// p_rigidbody.cpp
//

#include "p_rigidbody.h"
#include "p_shape.h"

////////////////////////////////////////////////////////////////////////////////
namespace physics {

//------------------------------------------------------------------------------
vec2 rigid_body::get_linear_velocity(vec2 position) const
{
    // v = l + a x r
    //   = l - r x a
    return _linear_velocity - (position - _position).cross(_angular_velocity);
}

//------------------------------------------------------------------------------
mat3 rigid_body::get_transform() const
{
    float cosa = cosf(_rotation);
    float sina = sinf(_rotation);

    return mat3(cosa, -sina, _position.x,
                sina,  cosa, _position.y,
                0.0f,  0.0f, 1.0f);
}

//------------------------------------------------------------------------------
float rigid_body::get_kinetic_energy() const
{
    float energy = 0.0f;

    if (_inverse_mass) {
        energy += 0.5f * _linear_velocity.dot(_linear_velocity) / _inverse_mass;
    }
    if (_inverse_inertia) {
        energy += 0.5f * _angular_velocity * _angular_velocity / _inverse_inertia;
    }

    return energy;
}

//------------------------------------------------------------------------------
void rigid_body::apply_impulse(vec2 impulse)
{
    _linear_velocity += impulse * _inverse_mass;
}

//------------------------------------------------------------------------------
void rigid_body::apply_impulse(vec2 impulse, vec2 position)
{
    _linear_velocity += impulse * _inverse_mass;
    _angular_velocity += vec3(position - _position).cross(vec3(impulse)).z * _inverse_inertia;
}

//------------------------------------------------------------------------------
void rigid_body::set_mass(float mass)
{
    if (mass && _inverse_mass) {
        float ratio = mass * _inverse_mass;
        _inverse_mass *= ratio;
        _inverse_inertia *= ratio;
    } else if (mass) {
        _inverse_mass = 1.0f / mass;
        _shape->calculate_mass_properties(_inverse_mass, _center_of_mass, _inverse_inertia);
    } else {
        _inverse_mass = 0.0f;
        _inverse_inertia = 0.0f;
    }
}

} // namespace physics
