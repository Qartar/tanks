// p_collide.h
//

#pragma once

#include "oed_types.h"
#include <vector>

namespace physics {

class rigid_body;

//------------------------------------------------------------------------------
struct contact
{
    float distance; //!< Contact distance, negative for penetration
    vec2 point; //!< Contact point in world space
    vec2 normal; //!< Contact normal in world space
    vec2 impulse; //!< Impulse that should be applied to body_b to resolve the collision.
};

//------------------------------------------------------------------------------
class collide
{
public:
    collide(rigid_body const* body_a, rigid_body const* body_b);

    bool has_contact() const {
        return _has_contact;
    }

    contact const& get_contact() const {
        return _contact;
    }

protected:
    bool _has_contact;

    contact _contact;

    constexpr static int max_iterations = 64;
    constexpr static int max_vertices = 64;
    constexpr static float epsilon = 1e-12f;

    struct support_vertex
    {
        vec3 a; //!< point on object A
        vec3 b; //!< point on object B
        vec3 d; //!< "Minkowski difference"; a - b
    };

    //
    //  GJK
    //

    float minimum_distance(vec3& point, vec3& direction) const;

    support_vertex supporting_vertex(vec3 direction) const;

    vec3 body_supporting_vertex(rigid_body const* body, vec3 direction) const;

    vec3 nearest_difference(support_vertex a, support_vertex b) const;

    vec3 nearest_point(support_vertex a, support_vertex b) const;

    bool triangle_contains_origin(vec3 a, vec3 b, vec3 c) const;

    //
    //  EPA
    //

    float penetration_distance(support_vertex a, support_vertex b, support_vertex c, vec3& point, vec3& direction) const;

    int nearest_edge_index(vec3 normal, std::vector<support_vertex> const& vertices, vec3& direction) const;

protected:
    rigid_body const* _body[2];
};

} // namespace physics
