//  r_particle.h

#pragma once

#include "oed_types.h"
#include "oed_shared.h"

namespace render {

struct particle
{
    enum flag_bits
    {
        invert = BIT(0),
    };

    float time;
    float size, size_velocity;
    vec2 position, velocity, acceleration;
    float drag;
    vec4 color, color_velocity;
    flag_bits flags;
};

} // namespace render
