// p_collide.cpp
//

#include "p_collide.h"
#include "p_material.h"
#include "p_rigidbody.h"
#include "p_shape.h"

#include "cm_filesystem.h"
#include "cm_visualize.h"

#include <algorithm>
#include <array>

visualize::debug g_debug;
bool debug_collide = false;

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

void extract_convex_hull(std::vector<vec3>& vertices)
{
    if (!vertices.size()) {
        return;
    }

    // find a point that is guaranteed to be on the convex hull
    vec3 pivot = vertices[0];
    for (std::size_t ii = 1; ii < vertices.size(); ++ii) {
        if (vertices[ii].x > pivot.x || (vertices[ii].x == pivot.x && vertices[ii].y > pivot.y)) {
            pivot = vertices[ii];
        }
    }

    // radial sort about an arbitrary point on the convex hull
    std::sort(vertices.begin(), vertices.end(), [pivot](vec3 a, vec3 b) {
        vec3 c = a - pivot;
        vec3 d = b - pivot;
        float s = c.y * d.x - d.y * c.x;

        if (c.y * d.y < 0.f) {
            return d.y < 0.f;
        } else if (s != 0.f) {
            return s < 0.f;
        } else {
            return c.length_sqr() < d.length_sqr();
        }
    });
}

//------------------------------------------------------------------------------
float collide::minimum_distance(vec3& point, vec3& direction) const
{
    support_vertex simplex[2], candidate;

    simplex[0] = supporting_vertex(direction);
    simplex[1] = supporting_vertex(-simplex[0].d);
    direction = -nearest_difference(simplex[0], simplex[1]);
    float distance = direction.length_sqr();

    struct writer {
        ~writer() {
            if (debug_collide) {
                g_debug.steps.insert(g_debug.steps.end(), _debug.steps.begin(), _debug.steps.end());
            }
        }
        void append(support_vertex a, support_vertex b, support_vertex c, vec3 d, vec3 p_a) {
            if (debug_collide) {
                visualize::debug::step step;

                // feature on a
                if (a.a == b.a) {
                    step.points.push_back({a.a, color4(0,0,1,1)});
                    _shape_a.push_back(a.a);
                } else {
                    step.lines.push_back({a.a, b.a, color4(0,0,1,1)});
                    _shape_a.push_back(a.a);
                    _shape_a.push_back(b.a);
                }
                // candidate on a
                if (c.a != a.a && c.a != b.a) {
                    step.lines.push_back({a.a, c.a, color4(0,0,1,.5f)});
                    step.lines.push_back({b.a, c.a, color4(0,0,1,.5f)});
                }
                // direction from a
                step.arrows.push_back({p_a, p_a - d * .5f, color4(0,0,1,1)});

                // feature on b
                if (a.b == b.b) {
                    step.points.push_back({a.b, color4(1,0,0,1)});
                    _shape_b.push_back(a.b);
                } else {
                    step.lines.push_back({a.b, b.b, color4(1,0,0,1)});
                    _shape_b.push_back(a.b);
                    _shape_b.push_back(b.b);
                }
                // candidate on b
                if (c.b != a.b && c.b != b.b) {
                    step.lines.push_back({a.b, c.b, color4(1,0,0,.5f)});
                    step.lines.push_back({b.b, c.b, color4(1,0,0,.5f)});
                }
                // direction from b
                step.arrows.push_back({p_a - d, p_a - d * .5f, color4(1,0,0,1)});

                // feature on minkowski difference
                step.lines.push_back({a.d, b.d, color4(1,1,1,1)});
                // candidate on minkowski difference
                step.lines.push_back({a.d, c.d, color4(1,1,1,.5f)});
                step.lines.push_back({b.d, c.d, color4(1,1,1,.5f)});
                _shape_d.push_back(a.d);
                _shape_d.push_back(b.d);
                // nearest difference on minkowski difference
                step.arrows.push_back({d, vec3_zero, color4(1,1,1,1)});

                // convex hull of a
                extract_convex_hull(_shape_a);
                for (std::size_t ii = 0; ii < _shape_a.size(); ++ii) {
                    step.lines.push_back({_shape_a[ii], _shape_a[(ii + 1) % _shape_a.size()], color4(0,0,1,.3f)});
                }
                // convex hull of b
                extract_convex_hull(_shape_b);
                for (std::size_t ii = 0; ii < _shape_b.size(); ++ii) {
                    step.lines.push_back({_shape_b[ii], _shape_b[(ii + 1) % _shape_b.size()], color4(1,0,0,.3f)});
                }
                // convex hull of d
                extract_convex_hull(_shape_d);
                for (std::size_t ii = 0; ii < _shape_d.size(); ++ii) {
                    step.lines.push_back({_shape_d[ii], _shape_d[(ii + 1) % _shape_d.size()], color4(1,1,1,.3f)});
                }

                _debug.steps.push_back(step);
            }
        }
        visualize::debug _debug;
        std::vector<vec3> _shape_a;
        std::vector<vec3> _shape_b;
        std::vector<vec3> _shape_d;
    } w;

    for (int num_iterations = 0; ; ++num_iterations) {
        candidate = supporting_vertex(direction);

        w.append(simplex[0], simplex[1], candidate, -direction, nearest_point(simplex[0], simplex[1]));

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

        float d = direction.length_sqr();

        // Check progress
        if (std::min(distance - d, d) < epsilon || num_iterations >= max_iterations) {
            point = nearest_point(simplex[0], simplex[1]);
            return direction.normalize_length();
        } else {
            distance = d;
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
    float num = -a.d.dot(v);
    float den = v.dot(v);

    if (num >= den) {
        return b.d;
    } else if (num < 0.0f) {
        return a.d;
    } else {
        float t = num / den;
        float s = 1.f - t;
        return a.d * s + b.d * t;
    }
}

//------------------------------------------------------------------------------
vec3 collide::nearest_point(support_vertex a, support_vertex b) const
{
    vec3 v = b.d - a.d;
    float num = -a.d.dot(v);
    float den = v.dot(v);

    if (num >= den) {
        return b.a;
    } else if (num < 0.0f) {
        return a.a;
    } else {
        float t = num / den;
        float s = 1.f - t;
        return a.a * s + b.a * t;
    }
}

//------------------------------------------------------------------------------
bool collide::triangle_contains_origin(vec3 a, vec3 b, vec3 c) const
{
    // Triangle plane normal
    vec3 n = (b - a).cross(c - b);
    if (n == vec3_zero) {
        return false;
    }

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
    std::array<support_vertex, max_vertices> vertices;
    std::size_t num_vertices = 0;
    support_vertex candidate;

    vec3 n = (b.d - c.d).cross(b.d - a.d);

    vertices[num_vertices++] = a;
    vertices[num_vertices++] = b;
    vertices[num_vertices++] = c;
    vertices[num_vertices++] = a; // avoid wrapping

    while (true) {
        // Find the edge nearest to the origin
        std::size_t edge_index = nearest_edge_index(n, vertices.data(), num_vertices, direction);

        // Get the vertex in direction of the edge normal (away from the origin)
        candidate = supporting_vertex(direction);

        // Check for termination
        float edge_distance = vertices[edge_index].d.dot(direction);
        float point_distance = candidate.d.dot(direction);
        float delta_sqr = (point_distance - edge_distance) * (point_distance - edge_distance) / direction.length_sqr();
        if (delta_sqr < epsilon || vertices.size() == max_vertices) {
            point = nearest_point(vertices[edge_index], vertices[edge_index-1]);
            return edge_distance / direction.normalize_length();
        }

        // Insert candidate point into the convex polygon
        vertices[num_vertices++] = candidate;
        std::rotate(vertices.data() + edge_index,
                    vertices.data() + num_vertices - 1,
                    vertices.data() + num_vertices);
    }
}

//------------------------------------------------------------------------------
std::size_t collide::nearest_edge_index(vec3 normal, support_vertex const* vertices, std::size_t num_vertices, vec3& direction) const
{
    float min_dist_sqr = FLT_MAX;
    std::size_t min_index = 1;

    for (std::size_t ii = 1; ii < num_vertices; ++ii) {
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
