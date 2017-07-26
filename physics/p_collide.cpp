// p_collide.cpp
//

#include "p_collide.h"
#include "p_material.h"
#include "p_rigidbody.h"
#include "p_shape.h"

////////////////////////////////////////////////////////////////////////////////
namespace physics {

//------------------------------------------------------------------------------
collide::collide(rigid_body const* body_a, rigid_body const* body_b)
    : _body{body_a, body_b}
{
    vec3 position, direction = vec3(body_b->get_position() - body_a->get_position());
    float distance = minimum_distance(position, direction);

    // Calculate the relative velocity of the bodies at the contact point
    vec3 relative_velocity = vec3(_body[1]->get_linear_velocity(position.to_vec2()))
                           - vec3(_body[0]->get_linear_velocity(position.to_vec2()));

    // Simple collision response for penetrating bodies
    if (distance < 0.0f && relative_velocity.dot(direction) < -0.f) {
        vec3 tangent = (relative_velocity - direction * relative_velocity.dot(direction)).normalize();

        // Use the geometric mean of both bodies' coefficient of restitution
        float restitution = sqrt(_body[0]->get_material()->restitution()
                               * _body[1]->get_material()->restitution());

        // Use the geometric mean of both bodies' coefficient of friction
        float mu = sqrt(_body[0]->get_material()->contact_friction()
                      * _body[1]->get_material()->contact_friction());

        // Calculate the inverse reduced mass of both bodies
        float inverse_reduced_mass = _body[0]->get_inverse_mass()
                                   + _body[1]->get_inverse_mass();

        vec3 ra = position - vec3(_body[0]->get_position());
        vec3 rb = position - vec3(_body[1]->get_position());

        // Change in normal velocity per change in momentum along normal
        float gx = inverse_reduced_mass
                 + _body[0]->get_inverse_inertia() * ra.cross(direction).length_sqr()
                 + _body[1]->get_inverse_inertia() * rb.cross(direction).length_sqr();

        // Change in tangent velocity per change in momentum along normal
        float gy = _body[0]->get_inverse_inertia() * ra.cross(direction).cross(ra).dot(-tangent)
                 + _body[1]->get_inverse_inertia() * rb.cross(direction).cross(rb).dot(-tangent);

        // Change in normal velocity per change in momentum along tangent
        float hx = _body[0]->get_inverse_inertia() * ra.cross(-tangent).cross(ra).dot(direction)
                 + _body[1]->get_inverse_inertia() * rb.cross(-tangent).cross(rb).dot(direction);

        // Change in tangent velocity per change in momentum along tangent
        float hy = inverse_reduced_mass
                 + _body[0]->get_inverse_inertia() * ra.cross(-tangent).length_sqr()
                 + _body[1]->get_inverse_inertia() * rb.cross(-tangent).length_sqr();

        float dvx = -(1.0f + restitution) * relative_velocity.dot(direction);
        float dvy = -relative_velocity.dot(-tangent);

        // Solve the vector equation:
        //
        //             | Gx  Hx |
        // dV = M dP = |        | dP
        //             | Gy  Hy |

        // Inverting the response matrix gives:
        //
        //           1      |  Hy  -Hx |
        // M' = ----------- |          |
        //      GxHy - HxGy | -Gy   Gx |

        float inv_det = 1.0f / (gx * hy - hx * gy);

        float dpx = inv_det * ( hy * dvx - hx * dvy);
        float dpy = inv_det * (-gy * dvx + gx * dvy);

        // Clamp friction impulse by friction coefficient
        if (abs(dpy) > mu * abs(dpx)) {
            // Find clamped vy using the original vector equation with dpy := mu * dpx
            float dvy0 = (gy + mu * hy) / (gx + mu * hx) * dvx;

            // Recalculate impulse using clamped friction
            dpx = inv_det * ( hy * dvx - hx * dvy0);
            dpy = inv_det * (-gy * dvx + gx * dvy0);
        }

        _has_contact = true;
        _contact.distance = distance;
        _contact.point = position.to_vec2();
        _contact.normal = direction.to_vec2();
        _contact.impulse = (direction * dpx - tangent * dpy).to_vec2();
    } else {
        _has_contact = false;
        _contact.distance = distance;
        _contact.point = position.to_vec2();
        _contact.normal = direction.to_vec2();
        _contact.impulse = vec2(0,0);
    }
}

//------------------------------------------------------------------------------
float collide::minimum_distance(vec3& point, vec3& direction) const
{
    support_vertex simplex[2], candidate;

    simplex[0] = supporting_vertex( direction);
    simplex[1] = supporting_vertex(-direction);

    direction = -nearest_difference(simplex[0], simplex[1]);

    for (int num_iterations = 0; ; ++num_iterations) {
        candidate = supporting_vertex(direction);

        vec3 n = (simplex[1].d - simplex[0].d).cross(candidate.d - simplex[1].d);

        // Check if the candidate point is any closer to the minimum
        if (n.length_sqr() < epsilon || num_iterations > max_iterations) {
            point = nearest_point(simplex[0], simplex[1]);
            float length = direction.length();
            direction /= length;
            return length;
        }

        // Check if simplex formed with candidate contains origin
        if (triangle_contains_origin(simplex[0].d, simplex[1].d, candidate.d)) {
            return -penetration_distance(simplex[0], simplex[1], candidate, point, direction);
        }

        // Update simplex
        vec3 d0 = nearest_difference(simplex[0], candidate);
        vec3 d1 = nearest_difference(simplex[1], candidate);

        if (d0.length_sqr() < d1.length_sqr()) {
            simplex[1] = candidate;
            direction = -d0;
        } else {
            simplex[0] = candidate;
            direction = -d1;
        }
    }
}

//------------------------------------------------------------------------------
collide::support_vertex collide::supporting_vertex(vec3 direction) const
{
    vec3 a = body_supporting_vertex(_body[0],  direction);
    vec3 b = body_supporting_vertex(_body[1], -direction);
    return {a, b, a - b};
}

//------------------------------------------------------------------------------
vec3 collide::body_supporting_vertex(rigid_body const* body, vec3 direction) const
{
    mat3 world_to_local; world_to_local.set_rotation<2>(-body->get_rotation());
    mat3 local_to_world; local_to_world.set_rotation<2>( body->get_rotation());

    vec2 v = body->get_shape()->supporting_vertex((direction * world_to_local).to_vec2());
    return vec3(v) * local_to_world + vec3(body->get_position());
}

//------------------------------------------------------------------------------
vec3 collide::nearest_difference(support_vertex a, support_vertex b) const
{
    vec3 v = b.d - a.d;
    float t = -a.d.dot(v) / v.length_sqr();

    if (t > 1.0f) {
        return b.d;
    } else if (t < 0.0f) {
        return a.d;
    } else {
        return a.d + v * t;
    }
}

//------------------------------------------------------------------------------
vec3 collide::nearest_point(support_vertex a, support_vertex b) const
{
    vec3 v = b.d - a.d;
    float t = -a.d.dot(v) / v.length_sqr();

    if (t > 1.0f) {
        return b.a;
    } else if (t < 0.0f) {
        return a.a;
    } else {
        return a.a + (b.a - a.a) * t;
    }
}

//------------------------------------------------------------------------------
bool collide::triangle_contains_origin(vec3 a, vec3 b, vec3 c) const
{
    // Triangle plane normal
    vec3 n = (b - a).cross(c - b);

    // Ensure that the winding is consistent
    if ((c - a).dot(n.cross(b - a)) > 0.0f) {
        n = -n;
    }

    if (a.dot(n.cross(b - a)) < 0.0f) {
        return false;
    } else if (b.dot(n.cross(c - b)) < 0.0f) {
        return false;
    } else if (c.dot(n.cross(a - c)) < 0.0f) {
        return false;
    } else {
        return true;
    }
}

//------------------------------------------------------------------------------
float collide::penetration_distance(support_vertex a, support_vertex b, support_vertex c, vec3& point, vec3& direction) const
{
    std::vector<support_vertex> vertices; vertices.reserve(max_vertices);
    support_vertex candidate;

    vec3 n = (b.d - c.d).cross(b.d - a.d);

    vertices.push_back(a);
    vertices.push_back(b);
    vertices.push_back(c);
    vertices.push_back(a); // avoid wrapping

    while (true) {
        // Find the edge nearest to the origin
        int edge_index = nearest_edge_index(n, vertices, direction);

        // Get the vertex in direction of the edge normal (away from the origin)
        candidate = supporting_vertex(direction);

        // Check for termination
        float edge_distance = vertices[edge_index].d.dot(direction);
        float point_distance = candidate.d.dot(direction);
        float delta_sqr = (point_distance - edge_distance) * (point_distance - edge_distance) / direction.length_sqr();
        if (delta_sqr < epsilon || vertices.size() == max_vertices) {
            point = nearest_point(vertices[edge_index], vertices[edge_index-1]);
            float length = direction.length();
            direction /= length;
            return edge_distance / length;
        }

        // Insert candidate point into the convex polygon
        vertices.insert(vertices.begin() + edge_index, candidate);
    }
}

//------------------------------------------------------------------------------
int collide::nearest_edge_index(vec3 normal, std::vector<support_vertex> const& vertices, vec3& direction) const
{
    float min_dist_sqr = FLT_MAX;
    int min_index = 1;

    for (int ii = 1; ii < vertices.size(); ++ii) {
        vec3 edge_normal = normal.cross(vertices[ii-1].d - vertices[ii].d);
        float dot_product = edge_normal.dot(vertices[ii].d);

        float dist_sqr = dot_product * dot_product / edge_normal.length_sqr();

        if (dist_sqr < min_dist_sqr) {
            min_dist_sqr = dist_sqr;
            direction = edge_normal;
            min_index = ii;
        }
    }

    return min_index;
}

} // namespace physics
