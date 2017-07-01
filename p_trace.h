// p_trace.h
//

#pragma once

#include "oed_types.h"
#include "p_collide.h"

namespace physics {

class rigid_body;

//------------------------------------------------------------------------------
class trace
{
public:
    trace(rigid_body const* body, vec2 start, vec2 end);

    float get_fraction() const { return _fraction; }

    contact const& get_contact() const {
        return _contact;
    }

protected:
    float _fraction;

    contact _contact;

    constexpr static int max_iterations = 64;
    constexpr static float epsilon = 1e-6f;
};

} // namespace physics
