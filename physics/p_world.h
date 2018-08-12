// p_world.h
//

#pragma once

#include "cm_vector.h"
#include <functional>
#include <vector>

////////////////////////////////////////////////////////////////////////////////
namespace physics {

class rigid_body;
struct contact;
class trace;
class shape;

//------------------------------------------------------------------------------
class world
{
public:
    using filter_callback_type = std::function<bool(physics::rigid_body const* body_a, physics::rigid_body const* body_b)>;
    using collision_callback_type = std::function<bool(physics::rigid_body const* body_a, physics::rigid_body const* body_b, physics::contact const& contact)>;

    world(filter_callback_type filter_callback, collision_callback_type collision_callback);

    void add_body(physics::rigid_body* body);
    void remove_body(physics::rigid_body* body);

    void step(float delta_time);

protected:
    std::vector<physics::rigid_body*> _bodies;

    filter_callback_type _filter_callback;
    collision_callback_type _collision_callback;
};

} // namespace physics
