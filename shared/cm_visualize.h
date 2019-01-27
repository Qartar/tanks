// cm_visualize.h
//

#include "cm_color.h"
#include "cm_vector.h"

#include <vector>

////////////////////////////////////////////////////////////////////////////////
namespace visualize {

//------------------------------------------------------------------------------
struct debug {
    struct line {
        vec3 l1, l2;
        color4 c;
    };
    struct point {
        vec3 p;
        color4 c;
    };
    struct step {
        std::vector<line> lines;
        std::vector<line> arrows;
        std::vector<point> points;
    };
    std::vector<step> steps;
};

//------------------------------------------------------------------------------
struct state {
    std::size_t step;
    vec2 origin;
    float zoom;
};

} // namespace visualize
