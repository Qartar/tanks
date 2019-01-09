// r_model.h
//

#pragma once

#include "cm_vector.h"
#include "cm_color.h"
#include <vector>

////////////////////////////////////////////////////////////////////////////////
namespace render {

//------------------------------------------------------------------------------
class model
{
public:
    struct rect
    {
        vec2 center;
        vec2 size;
        float gamma;
    };

    template<std::size_t Size>
    model(rect const (&rects)[Size])
        : model(rects, Size)
    {}

    void draw(vec2 position, float rotation, color4 color) const;

protected:
    std::vector<rect> _list;
    vec2 _mins;
    vec2 _maxs;

protected:
    model(rect const* rects, std::size_t num_rects);
};

} // namespace render

extern render::model tank_body_model;
extern render::model tank_turret_model;
