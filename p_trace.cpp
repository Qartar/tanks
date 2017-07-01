// p_trace.cpp
//

#include "p_trace.h"
#include "p_material.h"
#include "p_rigidbody.h"
#include "p_shape.h"

namespace physics {

trace::trace(rigid_body const* body, vec2 start, vec2 end)
{
    physics::material material(0,0);
    physics::circle_shape shape(0);
    physics::rigid_body point_body(&shape, &material, 0);

    vec2 direction = end - start;
    float length = direction.length();
    float fraction = 0.0f;

    for (int num_iterations = 0; num_iterations < max_iterations && fraction < 1.0f; ++num_iterations) {
        point_body.set_position(start + direction * fraction);
        _contact = physics::collide(&point_body, body).get_contact();

        if (_contact.normal.dot(direction) < 0.0f) {
            _fraction = 1.0f;
            return;
        }

        if (_contact.distance < epsilon) {
            break;
        }

        fraction += _contact.distance / _contact.normal.dot(direction);
    }

    _fraction = fraction > 1.0f ? 1.0f : fraction;
}

} // namespace physics
