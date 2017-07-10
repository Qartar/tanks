//  r_particle.h
//

#pragma once

#include "oed_types.h"
#include "oed_shared.h"

////////////////////////////////////////////////////////////////////////////////
namespace render {

//------------------------------------------------------------------------------
struct particle
{
    enum flag_bits
    {
        //! Fade alpha towards the particle center instead of outside
        invert = BIT(0),

        //! Draw particle as a half-circle with an elliptical tail
        tail = BIT(1),
    };

    float time;
    float size, size_velocity;
    vec2 position, velocity, acceleration;
    float drag;
    vec4 color, color_velocity;
    flag_bits flags;
};

} // namespace render
