// p_collide.cpp
//

#include "p_collide.h"
#include "p_material.h"
#include "p_rigidbody.h"
#include "p_shape.h"

////////////////////////////////////////////////////////////////////////////////
namespace physics {

//------------------------------------------------------------------------------
collide::motion_data::motion_data(motion const& motion)
    : physics::motion(motion)
{
    local_to_world = mat3::transform(_position, _rotation);
    world_to_local = mat3::inverse_transform(_position, _rotation);
}

//------------------------------------------------------------------------------
collide::collide(motion const& motion_a, motion const& motion_b)
    : _motion{motion_a, motion_b}
{
    vec3 position = vec3_zero;
    vec3 direction = vec3(_motion[1].get_position() - _motion[0].get_position());

    float distance = minimum_distance(position, direction);

    // Calculate the relative velocity of the bodies at the contact point
    vec3 relative_velocity = vec3(_motion[1].get_linear_velocity(position.to_vec2()))
                           - vec3(_motion[0].get_linear_velocity(position.to_vec2()));

    if (distance < 0.0f && relative_velocity.dot(direction) < 0.f) {
        _has_contact = true;
    } else {
        _has_contact = false;
    }

    _contact.distance = distance;
    _contact.point = position.to_vec2();
    _contact.normal = direction.to_vec2();
}

//------------------------------------------------------------------------------
float collide::minimum_distance(vec3& point, vec3& direction) const
{
    support_vertex simplex[2], candidate;

    simplex[0] = supporting_vertex( direction);
    simplex[1] = supporting_vertex(-direction);

    direction = -nearest_difference(simplex[0], simplex[1]);

    for (int num_iterations = 0; ; ++num_iterations) {
        // Direction can be zero if two shapes have a coincident feature
        if (direction == vec3_zero) {
            point = nearest_point(simplex[0], simplex[1]);
            return 0.f;
        }

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
    vec3 a = motion_supporting_vertex(_motion[0],  direction);
    vec3 b = motion_supporting_vertex(_motion[1], -direction);
    return {a, b, a - b};
}

//------------------------------------------------------------------------------
vec3 collide::motion_supporting_vertex(motion_data const& motion, vec3 direction) const
{
    vec2 local_direction = (direction * motion.world_to_local).to_vec2();
    vec2 local_vertex = motion.get_shape()->supporting_vertex(local_direction);
    return vec3(local_vertex, 1) * motion.local_to_world;
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
