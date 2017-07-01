// p_material.h
//

#pragma once

namespace physics {

//------------------------------------------------------------------------------
class material
{
public:
    material(float restitution, float friction)
        : _restitution(restitution)
        , _contact_friction(friction)
        , _sliding_friction(friction)
    {}

    material(float restitution, float contact_friction, float sliding_friction)
        : _restitution(restitution)
        , _contact_friction(contact_friction)
        , _sliding_friction(sliding_friction)
    {}

    float restitution() const {
        return _restitution;
    }

    float contact_friction() const {
        return _contact_friction;
    }

    float sliding_friction() const {
        return _sliding_friction;
    }

protected:
    float _restitution;
    float _contact_friction;
    float _sliding_friction;
};

} // namespace physics
